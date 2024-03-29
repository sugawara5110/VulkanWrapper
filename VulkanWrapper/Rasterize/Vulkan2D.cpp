﻿//*****************************************************************************************//
//**                                                                                     **//
//**                               Vulkan2D.cpp                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#include "Vulkan2D.h"
#include "Shader/Shader2D.h"

Vulkan2D::Vulkan2D() {

}

Vulkan2D::~Vulkan2D() {
	VulkanDevice* device = VulkanDevice::GetInstance();
	VkDevice vd = device->getDevice();
	texture.destroy();
	for (uint32_t s = 0; s < numSwap; s++) {
		vkUtil::S_DELETE(uniform[s]);
	}
	_vkDestroyDescriptorSetLayout(vd, descSetLayout, nullptr);
	_vkDestroyPipeline(vd, pipeline, nullptr);
	_vkDestroyPipelineCache(vd, pipelineCache, nullptr);
	_vkDestroyPipelineLayout(vd, pipelineLayout, nullptr);
	vertices.destroy();
	index.destroy();
}

void Vulkan2D::createColor(uint32_t QueueIndex, uint32_t comIndex, Vertex2D* ver, uint32_t num, uint32_t* ind, uint32_t indNum) {
	static VkVertexInputAttributeDescription attrDescs[] =
	{
		{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 2 }
	};
	create(QueueIndex, comIndex, ver, num, ind, indNum, attrDescs, 2, vsShader2D, fsShader2D, -1);
}

void Vulkan2D::createTexture(uint32_t QueueIndex, uint32_t comIndex, Vertex2DTex* ver, uint32_t num, uint32_t* ind, uint32_t indNum, int textureId) {
	static VkVertexInputAttributeDescription attrDescs[] =
	{
		{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 2 }
	};
	create(QueueIndex, comIndex, ver, num, ind, indNum, attrDescs, 2, vsShader2DTex, fsShader2DTex, textureId);
}

void Vulkan2D::update(uint32_t swapIndex, CoordTf::VECTOR2 pos) {
	VulkanDevice* device = VulkanDevice::GetInstance();
	mat2d[swapIndex].world.as(pos.x, pos.y);
	uniform[swapIndex]->update(0, &mat2d[swapIndex]);
}

void Vulkan2D::draw(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex) {
	VulkanDevice* device = VulkanDevice::GetInstance();
	VulkanSwapchain* sw = VulkanSwapchain::GetInstance();
	uint32_t width = sw->getSize().width;
	uint32_t height = sw->getSize().height;
	static VkViewport vp = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	static VkRect2D sc = { { 0, 0 }, { width, height } };
	static VkDeviceSize offsets[] = { 0 };

	VkCommandBuffer comb = device->getCommandObj(QueueIndex)->getCommandBuffer(comIndex);

	_vkCmdBindPipeline(comb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	_vkCmdSetViewport(comb, 0, 1, &vp);
	_vkCmdSetScissor(comb, 0, 1, &sc);
	_vkCmdBindVertexBuffers(comb, 0, 1, vertices.getBufferAddress(), offsets);
	_vkCmdBindDescriptorSets(comb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
		&descSet[swapIndex], 0, nullptr);
	_vkCmdBindIndexBuffer(comb, index.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	_vkCmdDrawIndexed(comb, numIndex, 1, 0, 0, 0);
}