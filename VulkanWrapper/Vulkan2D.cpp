//*****************************************************************************************//
//**                                                                                     **//
//**                               Vulkan2D.cpp                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#include "Vulkan2D.h"
#include "Shader/Shader2D.h"

Vulkan2D::Vulkan2D(Device* dev) {
	device = dev;
}

Vulkan2D::~Vulkan2D() {
	for (uint32_t s = 0; s < numSwap; s++) {
		vkDestroyBuffer(device->device, uniform[s].vkBuf, nullptr);
		vkFreeMemory(device->device, uniform[s].mem, nullptr);
	}
	vkDestroyDescriptorSetLayout(device->device, descSetLayout, nullptr);
	for (uint32_t s = 0; s < numSwap; s++) {
		vkFreeDescriptorSets(device->device, descPool[s], descSetCnt, &descSet[s]);
		vkDestroyDescriptorPool(device->device, descPool[s], nullptr);
	}
	vkDestroyPipeline(device->device, pipeline, nullptr);
	vkDestroyPipelineCache(device->device, pipelineCache, nullptr);
	vkDestroyPipelineLayout(device->device, pipelineLayout, nullptr);
	vkDestroyBuffer(device->device, vertices.first, nullptr);
	vkFreeMemory(device->device, vertices.second, nullptr);
	vkDestroyBuffer(device->device, index.first, nullptr);
	vkFreeMemory(device->device, index.second, nullptr);
}

void Vulkan2D::create(uint32_t comIndex, Vertex2D* ver, uint32_t num, uint32_t* ind, uint32_t indNum) {
	numIndex = indNum;
	static VkVertexInputBindingDescription bindDesc =
	{
		0, sizeof(Vertex2D), VK_VERTEX_INPUT_RATE_VERTEX
	};
	static VkVertexInputAttributeDescription attrDescs[] =
	{
		{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 2 }
	};

	VkShaderModule vsModule = device->createShaderModule(vsShader2D);
	VkShaderModule fsModule = device->createShaderModule(fsShader2D);

	vertices = device->createVertexBuffer<Vertex2D>(comIndex, ver, sizeof(Vertex2D) * num, false);
	index = device->createVertexBuffer<uint32_t>(comIndex, ind, sizeof(uint32_t) * indNum, true);

	device->descriptorAndPipelineLayouts2D(false, pipelineLayout, descSetLayout);

	for (uint32_t i = 0; i < numSwap; i++) {
		device->createUniform(uniform[i]);
		device->createDescriptorPool(false, descPool[i]);
		Device::VkTexture* tex = nullptr;

		descSetCnt = device->upDescriptorSet2D(false, *tex, uniform[i], descSet[i], descPool[i], descSetLayout);
	}

	pipelineCache = device->createPipelineCache();
	pipeline = device->createGraphicsPipelineVF(false, vsModule, fsModule, bindDesc, attrDescs, 2, pipelineLayout, device->renderPass, pipelineCache);
	vkDestroyShaderModule(device->device, vsModule, nullptr);
	vkDestroyShaderModule(device->device, fsModule, nullptr);
}

void Vulkan2D::update(uint32_t swapIndex, VECTOR2 pos) {
	uniform[swapIndex].uni.world.as(pos.x, pos.y);
	device->updateUniform(uniform[swapIndex]);
}

void Vulkan2D::draw(uint32_t swapIndex, uint32_t comIndex) {
	static VkViewport vp = { 0.0f, 0.0f, (float)device->width, (float)device->height, 0.0f, 1.0f };
	static VkRect2D sc = { { 0, 0 }, { device->width, device->height } };
	static VkDeviceSize offsets[] = { 0 };

	vkCmdBindPipeline(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdSetViewport(device->commandBuffer[comIndex], 0, 1, &vp);
	vkCmdSetScissor(device->commandBuffer[comIndex], 0, 1, &sc);
	vkCmdBindVertexBuffers(device->commandBuffer[comIndex], 0, 1, &vertices.first, offsets);
	vkCmdBindDescriptorSets(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
		&descSet[swapIndex], 0, nullptr);
	vkCmdBindIndexBuffer(device->commandBuffer[comIndex], index.first, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(device->commandBuffer[comIndex], numIndex, 1, 0, 0, 0);
}