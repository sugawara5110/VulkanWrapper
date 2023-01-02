//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanBloom.cpp                                        **//
//**                                                                                     **//
//*****************************************************************************************//

#include "VulkanBloom.h"
#include "Shader/ShaderBloom.h"

namespace {

	float* gaussian(float sigma, int& numWid) {//sigma:分散 数値が大きい程全体的に同等になる

		int wCnt = 0;
		while (expf(-(float)(wCnt * wCnt) / (sigma * sigma * 2.0f)) > 0.0001f) {
			if (wCnt++ > 1000)break;
		}

		numWid = wCnt;
		float total = 0.0f;
		float* gaArr = new float[numWid];

		for (int x = 0; x < numWid; x++)
		{
			total +=
				gaArr[x] = expf(-(float)(x * x) / (sigma * sigma * 2.0f));
		}

		for (int i = 0; i < numWid; i++)
		{
			gaArr[i] /= total;
		}

		return gaArr;
	}

	VkDescriptorSetLayoutBinding getLayout(
		uint32_t binding, VkDescriptorType type, uint32_t descriptorCount) {

		VkDescriptorSetLayoutBinding b{};
		b.binding = binding;
		b.descriptorType = type;
		b.descriptorCount = descriptorCount;
		b.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		return b;
	};

	VkWriteDescriptorSet getDescriptorSet(
		VkDescriptorSet descriptorSet, uint32_t binding, VkDescriptorType type,
		uint32_t descriptorCount,
		VkDescriptorImageInfo* infoI, VkDescriptorBufferInfo* infoB) {

		VkWriteDescriptorSet ds{
		   VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
		};
		ds.dstSet = descriptorSet;
		ds.dstBinding = binding;
		ds.descriptorCount = descriptorCount;
		ds.descriptorType = type;
		ds.pImageInfo = infoI;
		ds.pBufferInfo = infoB;
		return ds;
	};
}

VulkanBloom::~VulkanBloom() {
	Output.destroy();
	vkUtil::S_DELETE(bParamUBO);

	VkDevice d = VulkanDevice::GetInstance()->getDevice();

	vkDestroyPipeline(d, Pipeline, nullptr);
	vkDestroyPipelineLayout(d, pipelineLayout, nullptr);
	VulkanDevice::GetInstance()->DeallocateDescriptorSet(descriptorSet);
	vkDestroyDescriptorSetLayout(d, dsLayout, nullptr);
}

void VulkanBloom::setImage(
	VulkanDevice::ImageSet* RenderedImage, VulkanDevice::ImageSet* instanceIdMap,
	uint32_t width, uint32_t height,
	std::vector<InstanceParam> instanceParams,
	std::vector<std::vector<uint32_t>>* gausSizes) {

	Width = width;
	Height = height;
	pRenderedImage = RenderedImage;

	size_t numBloom = instanceParams.size();
	bloom.resize(numBloom);

	for (size_t i = 0; i < numBloom; i++) {
		if (gausSizes) {
			bloom[i].setImage(
				RenderedImage, instanceIdMap,
				width, height,
				instanceParams[i],
				(*gausSizes)[i]);
		}
		else {
			bloom[i].setImage(
				RenderedImage, instanceIdMap,
				width, height,
				instanceParams[i]);
		}
	}
}

void VulkanBloom::createBuffer(uint32_t comIndex) {

	VulkanDevice* dev = VulkanDevice::GetInstance();
	VulkanSwapchain* sw = VulkanSwapchain::GetInstance();
	VkFormat format = sw->getFormat();

	VkImageUsageFlags usage =
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_STORAGE_BIT;

	VkMemoryPropertyFlags devMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	dev->beginCommand(comIndex);

	Output.createImage(Width, Height, format, VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);
	Output.createImageView(format, VK_IMAGE_ASPECT_COLOR_BIT);
	Output.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);

	dev->endCommand(comIndex);
	dev->submitCommandsDoNotRender(comIndex);

	bParamUBO = new VulkanDevice::Uniform<BloomParam>(1);
	bParam.numInstance = (int)bloom.size();
	bParamUBO->update(0, &bParam);
}

void VulkanBloom::createLayouts() {

	std::vector<VkDescriptorSetLayoutBinding> bind = {
		getLayout(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		getLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
		getLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, (uint32_t)bloom.size()),
		getLayout(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1)
	};

	VkDescriptorSetLayoutCreateInfo dsLayoutCI{
		   VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
	};
	dsLayoutCI.bindingCount = uint32_t(bind.size());
	dsLayoutCI.pBindings = bind.data();

	VulkanDevice* dev = VulkanDevice::GetInstance();

	vkCreateDescriptorSetLayout(
		dev->getDevice(), &dsLayoutCI, nullptr, &dsLayout);

	VkPipelineLayoutCreateInfo pipelineLayoutCI{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
	};
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &dsLayout;
	vkCreatePipelineLayout(dev->getDevice(),
		&pipelineLayoutCI, nullptr, &pipelineLayout);
}

void VulkanBloom::createPipeline() {

	VulkanDevice* dev = VulkanDevice::GetInstance();

	auto Stage = dev->createShaderModule("Bloom4", ShaderBloom4, VK_SHADER_STAGE_COMPUTE_BIT);

	VkComputePipelineCreateInfo compPipelineCI{
				VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO
	};
	compPipelineCI.layout = pipelineLayout;
	compPipelineCI.stage = Stage;

	vkCreateComputePipelines(dev->getDevice(), VK_NULL_HANDLE, 1,
		&compPipelineCI, nullptr, &Pipeline);

	vkDestroyShaderModule(dev->getDevice(), Stage.module, nullptr);
}

void VulkanBloom::createDescriptorSets() {

	VulkanDevice* dev = VulkanDevice::GetInstance();
	descriptorSet = dev->AllocateDescriptorSet(dsLayout);

	for (size_t i = 0; i < bloom.size(); i++) {
		blOutput_Info.push_back(bloom[i].getOutput()->info);
	}

	std::vector<VkWriteDescriptorSet> write = {
		getDescriptorSet(descriptorSet, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, nullptr, &bParamUBO->getBufferSet()->info),
		getDescriptorSet(descriptorSet, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &pRenderedImage->info, nullptr),
		getDescriptorSet(descriptorSet, 2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, (uint32_t)blOutput_Info.size(), blOutput_Info.data(), nullptr),
		getDescriptorSet(descriptorSet, 3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &Output.info, nullptr),
	};
	vkUpdateDescriptorSets(dev->getDevice(), uint32_t(write.size()), write.data(), 0, nullptr);
}

void VulkanBloom::Create(uint32_t comIndex, std::vector<float>* sigma) {
	for (size_t i = 0; i < bloom.size(); i++) {
		if (sigma) {
			bloom[i].Create(comIndex, (*sigma)[i]);
		}
		else {
			bloom[i].Create(comIndex);
		}
	}

	createBuffer(comIndex);
	createLayouts();
	createPipeline();
	createDescriptorSets();
}

void VulkanBloom::Compute(uint32_t comIndex) {

	for (size_t i = 0; i < bloom.size(); i++) {
		bloom[i].Compute(comIndex);
	}

	VulkanDevice* dev = VulkanDevice::GetInstance();
	VulkanSwapchain* sw = VulkanSwapchain::GetInstance();
	auto command = dev->getCommandBuffer(comIndex);

	vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
	vkCmdBindDescriptorSets(
		command, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipelineLayout, 0,
		1, &descriptorSet,
		0,
		nullptr);

	vkCmdDispatch(command, Width, Height, 1);

	VkMemoryBarrier barrier{
		VK_STRUCTURE_TYPE_MEMORY_BARRIER,
	};
	barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	vkCmdPipelineBarrier(
		command,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0, 1, &barrier,
		0, nullptr,
		0, nullptr);

	VkImageCopy region{};
	region.extent = { Width, Height, 1 };
	region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };

	Output.barrierResource(comIndex,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

	dev->barrierResource(comIndex,
		sw->getCurrentImage(),
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

	vkCmdCopyImage(command,
		Output.getImage(), Output.info.imageLayout,
		sw->getCurrentImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &region);

	dev->barrierResource(comIndex,
		sw->getCurrentImage(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);

	Output.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
}

VulkanBloom::Bloom::~Bloom() {

	GaussianFilter.destroy();
	for (size_t i = 0; i < fset.size(); i++) {
		fset[i].inOutBloom0.destroy();
		fset[i].inOutBloom1.destroy();
		fset[i].Luminance.destroy();
	}
	Output.destroy();
	vkUtil::S_DELETE(bParamUBO);

	VkDevice d = VulkanDevice::GetInstance()->getDevice();

	for (size_t i = 0; i < dset.size(); i++) {
		vkDestroyPipeline(d, dset[i].Pipeline, nullptr);
		vkDestroyPipelineLayout(d, dset[i].pipelineLayout, nullptr);
		VulkanDevice::GetInstance()->DeallocateDescriptorSet(dset[i].descriptorSet);
		vkDestroyDescriptorSetLayout(d, dset[i].dsLayout, nullptr);
	}
}

void VulkanBloom::Bloom::setImage(
	VulkanDevice::ImageSet* RenderedImage, VulkanDevice::ImageSet* instanceIdMap,
	uint32_t width, uint32_t height,
	InstanceParam instanceParam,
	std::vector<uint32_t> gausSize) {

	GausSize = gausSize;
	pRenderedImage = RenderedImage;
	pInstanceIdMap = instanceIdMap;
	Width = width;
	Height = height;
	iParam = instanceParam;
}

void VulkanBloom::Bloom::createLayouts() {

	auto Create = [this](
		std::vector<VkDescriptorSetLayoutBinding> bind,
		uint32_t index) {

			VkDescriptorSetLayoutCreateInfo dsLayoutCI{
		   VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
			};
			dsLayoutCI.bindingCount = uint32_t(bind.size());
			dsLayoutCI.pBindings = bind.data();

			VulkanDevice* dev = VulkanDevice::GetInstance();

			vkCreateDescriptorSetLayout(
				dev->getDevice(), &dsLayoutCI, nullptr, &dset[index].dsLayout);

			VkPipelineLayoutCreateInfo pipelineLayoutCI{
				VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
			};
			pipelineLayoutCI.setLayoutCount = 1;
			pipelineLayoutCI.pSetLayouts = &dset[index].dsLayout;
			vkCreatePipelineLayout(dev->getDevice(),
				&pipelineLayoutCI, nullptr, &dset[index].pipelineLayout);
	};

	auto Bind = [Create](
		VkDescriptorType bind1, uint32_t descriptorCount1, uint32_t createIndex) {

			std::vector<VkDescriptorSetLayoutBinding> bind = {
				getLayout(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
				getLayout(1, bind1, descriptorCount1),
				getLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
				getLayout(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1)
			};
			Create(bind, createIndex);
	};

	uint32_t cnt = 0;
	for (size_t i = 0; i < fset.size(); i++) {
		Bind(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, cnt++);
		Bind(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, cnt++);
		Bind(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, cnt++);
	}
	std::vector<VkDescriptorSetLayoutBinding> bind = {
				getLayout(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
				getLayout(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)fset.size()),
				getLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
	};
	Create(bind, cnt++);
}

void VulkanBloom::Bloom::createPipeline() {

	VulkanDevice* dev = VulkanDevice::GetInstance();

	vkUtil::addChar bl0[1] = {};
	vkUtil::addChar bl1[1] = {};
	vkUtil::addChar bl2[1] = {};
	vkUtil::addChar bl3[1] = {};

	bl0[0].addStr(ShaderBloom_Com, ShaderBloom0);
	bl1[0].addStr(ShaderBloom_Com, ShaderBloom1);
	bl2[0].addStr(ShaderBloom_Com, ShaderBloom2);
	bl3[0].addStr(ShaderBloom_Com, ShaderBloom3);

	VkPipelineShaderStageCreateInfo Stage[4] = {};
	Stage[0] = dev->createShaderModule("Bloom0", bl0[0].str, VK_SHADER_STAGE_COMPUTE_BIT);
	Stage[1] = dev->createShaderModule("Bloom1", bl1[0].str, VK_SHADER_STAGE_COMPUTE_BIT);
	Stage[2] = dev->createShaderModule("Bloom2", bl2[0].str, VK_SHADER_STAGE_COMPUTE_BIT);
	Stage[3] = dev->createShaderModule("Bloom3", bl3[0].str, VK_SHADER_STAGE_COMPUTE_BIT);

	auto pipe = [this, dev](
		VkPipelineShaderStageCreateInfo Stage, uint32_t index) {

			VkComputePipelineCreateInfo compPipelineCI{
				VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO
			};
			compPipelineCI.layout = dset[index].pipelineLayout;
			compPipelineCI.stage = Stage;

			vkCreateComputePipelines(dev->getDevice(), VK_NULL_HANDLE, 1,
				&compPipelineCI, nullptr, &dset[index].Pipeline);
	};

	uint32_t cnt = 0;
	for (size_t i = 0; i < GausSize.size(); i++) {
		pipe(Stage[0], cnt);
		dset[cnt].sizeWH.width = GausSize[i];
		dset[cnt++].sizeWH.height = GausSize[i];
		pipe(Stage[1], cnt);
		dset[cnt].sizeWH.width = GausSize[i];
		dset[cnt++].sizeWH.height = GausSize[i];
		pipe(Stage[2], cnt);
		dset[cnt].sizeWH.width = GausSize[i];
		dset[cnt++].sizeWH.height = GausSize[i];
	}
	pipe(Stage[3], cnt);
	dset[cnt].sizeWH.width = Width;
	dset[cnt++].sizeWH.height = Height;

	for (int i = 0; i < COUNTOF(Stage); i++) {
		vkDestroyShaderModule(dev->getDevice(), Stage[i].module, nullptr);
	}
}

void VulkanBloom::Bloom::createDescriptorSets() {

	VulkanDevice* dev = VulkanDevice::GetInstance();

	for (size_t i = 0; i < dset.size(); i++) {
		dset[i].descriptorSet = dev->AllocateDescriptorSet(dset[i].dsLayout);
	}

	auto deset = [dev](std::vector<VkWriteDescriptorSet> write) {
		vkUpdateDescriptorSets(dev->getDevice(), uint32_t(write.size()), write.data(), 0, nullptr);
	};

	uint32_t cnt = 0;

	for (size_t i = 0; i < fset.size(); i++) {
		VkDescriptorSet ds0 = dset[cnt++].descriptorSet;
		std::vector<VkWriteDescriptorSet> write0 = {
			getDescriptorSet(ds0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, nullptr, &bParamUBO->getBufferSet()->info),
			getDescriptorSet(ds0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &pRenderedImage->info, nullptr),
			getDescriptorSet(ds0, 2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &pInstanceIdMap->info, nullptr),
			getDescriptorSet(ds0, 3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &fset[i].Luminance.info, nullptr),
		};
		deset(write0);

		VkDescriptorSet ds1 = dset[cnt++].descriptorSet;
		std::vector<VkWriteDescriptorSet> write1 = {
			getDescriptorSet(ds1, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, nullptr, &bParamUBO->getBufferSet()->info),
			getDescriptorSet(ds1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, nullptr, &GaussianFilter.info),
			getDescriptorSet(ds1, 2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &fset[i].Luminance.info, nullptr),
			getDescriptorSet(ds1, 3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &fset[i].inOutBloom0.info, nullptr),
		};
		deset(write1);

		VkDescriptorSet ds2 = dset[cnt++].descriptorSet;
		std::vector<VkWriteDescriptorSet> write2 = {
			getDescriptorSet(ds2, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, nullptr, &bParamUBO->getBufferSet()->info),
			getDescriptorSet(ds2, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, nullptr, &GaussianFilter.info),
			getDescriptorSet(ds2, 2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &fset[i].inOutBloom0.info, nullptr),
			getDescriptorSet(ds2, 3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &fset[i].inOutBloom1.info, nullptr),
		};
		deset(write2);
	}

	VkDescriptorSet ds3 = dset[cnt++].descriptorSet;
	std::vector<VkWriteDescriptorSet> write3 = {
		getDescriptorSet(ds3, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, nullptr, &bParamUBO->getBufferSet()->info),
		getDescriptorSet(ds3, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)inOutBloom1_Info.size(), inOutBloom1_Info.data(), nullptr),
		getDescriptorSet(ds3, 2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &Output.info, nullptr),
	};
	deset(write3);
}

void VulkanBloom::Bloom::createGaussianFilter(uint32_t comIndex, float sigma) {

	float* gaFil = nullptr;
	int GaussianWid = 0;
	gaFil = gaussian(sigma, GaussianWid);
	bParam.GaussianWid = (float)GaussianWid;

	VkImageUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	GaussianFilter = VulkanDevice::GetInstance()->createDefaultCopiedBuffer(
		comIndex, gaFil, GaussianWid,
		nullptr, &usage);

	vkUtil::ARR_DELETE(gaFil);
}

void VulkanBloom::Bloom::createBuffer(uint32_t comIndex) {

	VulkanDevice* dev = VulkanDevice::GetInstance();
	VulkanSwapchain* sw = VulkanSwapchain::GetInstance();
	VkFormat format = sw->getFormat();

	int numFilter = (int)GausSize.size();//ガウシアンフィルタ計算回数
	int numFilterDispatch = numGausShader * numFilter;//ガウシアンフィルタのシェーダー実行回数
	int numDispatch = numFilterDispatch + 1;

	dset.resize(numDispatch);
	fset.resize(numFilter);

	VkImageUsageFlags usage =
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_STORAGE_BIT;
	VkImageUsageFlags usage2 =
		usage | VK_IMAGE_USAGE_SAMPLED_BIT;
	VkMemoryPropertyFlags devMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	dev->beginCommand(comIndex);

	for (size_t i = 0; i < fset.size(); i++) {
		uint32_t s = GausSize[i];
		VulkanDevice::ImageSet& Luminance = fset[i].Luminance;
		Luminance.createImage(s, s, format, VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);
		Luminance.createImageView(format, VK_IMAGE_ASPECT_COLOR_BIT);
		Luminance.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
		VulkanDevice::ImageSet& Bloom0 = fset[i].inOutBloom0;
		Bloom0.createImage(s, s, format, VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);
		Bloom0.createImageView(format, VK_IMAGE_ASPECT_COLOR_BIT);
		Bloom0.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
		VulkanDevice::ImageSet& Bloom1 = fset[i].inOutBloom1;
		Bloom1.createImage(s, s, format, VK_IMAGE_TILING_OPTIMAL, usage2, devMemProps);
		Bloom1.createImageView(format, VK_IMAGE_ASPECT_COLOR_BIT);
		Bloom1.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
		dev->createTextureSampler(Bloom1.info.sampler);
		inOutBloom1_Info.push_back(fset[i].inOutBloom1.info);
	}

	Output.createImage(sw->getSize().width, sw->getSize().height, format, VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);
	Output.createImageView(format, VK_IMAGE_ASPECT_COLOR_BIT);
	Output.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);

	dev->endCommand(comIndex);
	dev->submitCommandsDoNotRender(comIndex);

	bParamUBO = new VulkanDevice::Uniform<BloomParam>(1);
}

void VulkanBloom::Bloom::Create(uint32_t comIndex, float sigma) {

	createGaussianFilter(comIndex, sigma);
	createBuffer(comIndex);
	createLayouts();
	createPipeline();
	createDescriptorSets();
}

void VulkanBloom::Bloom::Compute(uint32_t comIndex) {
	VulkanDevice* dev = VulkanDevice::GetInstance();
	auto command = dev->getCommandBuffer(comIndex);

	bParam.bloomStrength = iParam.bloomStrength;
	bParam.InstanceID = iParam.EmissiveInstanceId;
	bParam.numGaussFilter = (float)GausSize.size();
	bParam.thresholdLuminance = iParam.thresholdLuminance;

	bParamUBO->update(0, &bParam);

	for (size_t i = 0; i < dset.size(); i++) {

		vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_COMPUTE, dset[i].Pipeline);
		vkCmdBindDescriptorSets(
			command, VK_PIPELINE_BIND_POINT_COMPUTE,
			dset[i].pipelineLayout, 0,
			1, &dset[i].descriptorSet,
			0,
			nullptr);

		vkCmdDispatch(command, dset[i].sizeWH.width, dset[i].sizeWH.height, 1);

		VkMemoryBarrier barrier{
			VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		};
		barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		vkCmdPipelineBarrier(
			command,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0, 1, &barrier,
			0, nullptr,
			0, nullptr);
	}
}
