//*****************************************************************************************//
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
	vkDestroyDescriptorSetLayout(vd, descSetLayout, nullptr);
	vkDestroyPipeline(vd, pipeline, nullptr);
	vkDestroyPipelineCache(vd, pipelineCache, nullptr);
	vkDestroyPipelineLayout(vd, pipelineLayout, nullptr);
	vertices.destroy();
	index.destroy();
}

void Vulkan2D::createColor(uint32_t comIndex, Vertex2D* ver, uint32_t num, uint32_t* ind, uint32_t indNum) {
	static VkVertexInputAttributeDescription attrDescs[] =
	{
		{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 2 }
	};
	create(comIndex, ver, num, ind, indNum, attrDescs, 2, vsShader2D, fsShader2D, -1);
}

void Vulkan2D::createTexture(uint32_t comIndex, Vertex2DTex* ver, uint32_t num, uint32_t* ind, uint32_t indNum, int textureId) {
	static VkVertexInputAttributeDescription attrDescs[] =
	{
		{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 2 }
	};
	create(comIndex, ver, num, ind, indNum, attrDescs, 2, vsShader2DTex, fsShader2DTex, textureId);
}

void Vulkan2D::update(uint32_t swapIndex, CoordTf::VECTOR2 pos) {
	VulkanDevice* device = VulkanDevice::GetInstance();
	mat2d[swapIndex].world.as(pos.x, pos.y);
	uniform[swapIndex]->update(0, &mat2d[swapIndex]);
}

void Vulkan2D::draw(uint32_t swapIndex, uint32_t comIndex) {
	VulkanDevice* device = VulkanDevice::GetInstance();
	uint32_t width = device->getSwapchainObj()->getSize().width;
	uint32_t height = device->getSwapchainObj()->getSize().height;
	static VkViewport vp = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	static VkRect2D sc = { { 0, 0 }, { width, height } };
	static VkDeviceSize offsets[] = { 0 };

	VkCommandBuffer comb = device->getCommandBuffer(comIndex);

	vkCmdBindPipeline(comb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdSetViewport(comb, 0, 1, &vp);
	vkCmdSetScissor(comb, 0, 1, &sc);
	vkCmdBindVertexBuffers(comb, 0, 1, vertices.getBufferAddress(), offsets);
	vkCmdBindDescriptorSets(comb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
		&descSet[swapIndex], 0, nullptr);
	vkCmdBindIndexBuffer(comb, index.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(comb, numIndex, 1, 0, 0, 0);
}