//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanBloom.cpp                                        **//
//**                                                                                     **//
//*****************************************************************************************//

#include "VulkanBloom.h"
#include "Shader/ShaderBloom.h"

namespace {

	float* gaussian(float sigma, int& numWid) {//sigma:•ªŽU ”’l‚ª‘å‚«‚¢’ö‘S‘Ì“I‚É“¯“™‚É‚È‚é

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

	VkDescriptorSetLayoutBinding getLayout(uint32_t binding, VkDescriptorType type) {
		VkDescriptorSetLayoutBinding b{};
		b.binding = binding;
		b.descriptorType = type;
		b.descriptorCount = 1;
		b.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		return b;
	}
}

VulkanBloom::~VulkanBloom() {

	GaussianFilter.destroy();
	for (uint32_t i = 0; i < iParam.size(); i++) {
		for (uint32_t j = 0; j < numMaxFilter; j++) {
			fset[i].inOutBloom0[j].destroy();
			fset[i].inOutBloom1[j].destroy();
			fset[i].Luminance[j].destroy();
		}
	}
	Output.destroy();
	vkUtil::S_DELETE(bParamUBO);

	VkDevice d = VulkanDevice::GetInstance()->getDevice();

	vkDestroyPipeline(d, Pipeline, nullptr);
	vkDestroyPipelineLayout(d, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(d, dsLayout, nullptr);
	VulkanDevice::GetInstance()->DeallocateDescriptorSet(descriptorSet);
}

VkWriteDescriptorSet VulkanBloom::getDescriptorSet(uint32_t binding, VkDescriptorType type,
	VkDescriptorImageInfo* infoI, VkDescriptorBufferInfo* infoB) {

	VkWriteDescriptorSet ds{
   VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
	};
	ds.dstSet = descriptorSet;
	ds.dstBinding = binding;
	ds.descriptorCount = 1;
	ds.descriptorType = type;
	ds.pImageInfo = infoI;
	ds.pBufferInfo = infoB;
	return ds;
}

void VulkanBloom::setImage(
	VulkanDevice::ImageSet* raytracedImage, VulkanDevice::ImageSet* instanceIdMap,
	uint32_t width, uint32_t height,
	std::vector<InstanceParam> instanceParam) {

	pRaytracedImage = raytracedImage;
	pInstanceIdMap = instanceIdMap;
	Width = width;
	Height = height;
	iParam = instanceParam;
}

void VulkanBloom::createLayouts() {

	std::vector<VkDescriptorSetLayoutBinding> bind = {
		getLayout(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		getLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
		getLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
		getLayout(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
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

	vkUtil::addChar bl[1] = {};

	bl[0].addStr(ShaderBloom_Com, ShaderBloom0);

	auto shaderStage = dev->createShaderModule("Bloom", bl[0].str, VK_SHADER_STAGE_COMPUTE_BIT);

	VkComputePipelineCreateInfo compPipelineCI{
		VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO
	};
	compPipelineCI.layout = pipelineLayout;
	compPipelineCI.stage = shaderStage;

	vkCreateComputePipelines(dev->getDevice(), VK_NULL_HANDLE, 1,
		&compPipelineCI, nullptr, &Pipeline);

	vkDestroyShaderModule(dev->getDevice(), shaderStage.module, nullptr);
}

void VulkanBloom::createDescriptorSets() {

	VulkanDevice* dev = VulkanDevice::GetInstance();
	descriptorSet = dev->AllocateDescriptorSet(dsLayout);

	std::vector<VkWriteDescriptorSet> writeSets = {
		getDescriptorSet(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,nullptr, &bParamUBO->getBufferSet()->info),
		getDescriptorSet(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,&pRaytracedImage->info,nullptr),
		getDescriptorSet(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,&pInstanceIdMap->info,nullptr),
		getDescriptorSet(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,&fset[0].Luminance[0].info,nullptr),
	};

	vkUpdateDescriptorSets(dev->getDevice(), uint32_t(writeSets.size()), writeSets.data(),
		0, nullptr);
}

void VulkanBloom::createGaussianFilter(uint32_t comIndex, float sigma) {

	float* gaFil = nullptr;
	int GaussianWid = 0;
	gaFil = gaussian(sigma, GaussianWid);
	bParam.GaussianWid = (float)GaussianWid;

	GaussianFilter = VulkanDevice::GetInstance()->createDefaultCopiedBuffer(
		comIndex, gaFil, GaussianWid,
		nullptr, nullptr);

	vkUtil::ARR_DELETE(gaFil);
}

void VulkanBloom::createBuffer(uint32_t comIndex) {

	VulkanDevice* dev = VulkanDevice::GetInstance();
	VulkanDevice::swapchainBuffer* sw = dev->getSwapchainObj();
	VkFormat format = sw->getFormat();

	VkImageUsageFlags usage =
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_STORAGE_BIT;
	VkImageUsageFlags usage2 =
		usage | VK_IMAGE_USAGE_SAMPLED_BIT;
	VkMemoryPropertyFlags devMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	fset = std::make_unique<FilterSet[]>(iParam.size());

	dev->beginCommand(comIndex);

	for (uint32_t i = 0; i < iParam.size(); i++) {
		for (uint32_t j = 0; j < numMaxFilter; j++) {
			uint32_t s = gaBaseSize[j];
			VulkanDevice::ImageSet& Luminance = fset[i].Luminance[j];
			Luminance.createImage(Width, Height, format, VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);
			Luminance.createImageView(format, VK_IMAGE_ASPECT_COLOR_BIT);
			Luminance.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
			VulkanDevice::ImageSet& Bloom0 = fset[i].inOutBloom0[j];
			Bloom0.createImage(s, s, format, VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);
			Bloom0.createImageView(format, VK_IMAGE_ASPECT_COLOR_BIT);
			Bloom0.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
			VulkanDevice::ImageSet& Bloom1 = fset[i].inOutBloom1[j];
			Bloom1.createImage(s, s, format, VK_IMAGE_TILING_OPTIMAL, usage2, devMemProps);
			Bloom1.createImageView(format, VK_IMAGE_ASPECT_COLOR_BIT);
			Bloom1.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
			dev->createTextureSampler(Bloom1.info.sampler);
		}
	}

	Output.createImage(sw->getSize().width, sw->getSize().height, format, VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);
	Output.createImageView(format, VK_IMAGE_ASPECT_COLOR_BIT);
	Output.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);

	dev->endCommand(comIndex);
	dev->submitCommandsDoNotRender(comIndex);

	bParamUBO = new VulkanDevice::Uniform<BloomParam>(1);
}

void VulkanBloom::Create(uint32_t comIndex, float sigma) {

	createGaussianFilter(comIndex, sigma);
	createBuffer(comIndex);
	createLayouts();
	createPipeline();
	createDescriptorSets();
}

void VulkanBloom::Compute(uint32_t comIndex) {
	VulkanDevice* dev = VulkanDevice::GetInstance();
	auto command = dev->getCommandBuffer(comIndex);

	bParam.bloomStrength = iParam[0].bloomStrength;
	bParam.InstanceID = iParam[0].EmissiveInstanceId;
	bParam.numGaussFilter = iParam[0].numGaussFilter;
	bParam.thresholdLuminance = iParam[0].thresholdLuminance;

	bParamUBO->update(0, &bParam);

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

	VulkanDevice::swapchainBuffer* sw = dev->getSwapchainObj();

	fset[0].Luminance[0].barrierResource(comIndex,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

	dev->barrierResource(comIndex,
		sw->getCurrentImage(),
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

	vkCmdCopyImage(command,
		fset[0].Luminance[0].getImage(), fset[0].Luminance[0].info.imageLayout,
		sw->getCurrentImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &region);

	dev->barrierResource(comIndex,
		sw->getCurrentImage(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);

	fset[0].Luminance[0].barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
}
