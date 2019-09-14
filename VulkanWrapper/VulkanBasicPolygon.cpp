//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanBasicPolygon                                       **//
//**                                                                                     **//
//*****************************************************************************************//


#include "VulkanBasicPolygon.h"
#include "Shader/ShaderBasicPolygon.h"

VulkanBasicPolygon::VulkanBasicPolygon(Device* dev, uint32_t comindex) {
	device = dev;
	comIndex = comindex;
}

VulkanBasicPolygon::~VulkanBasicPolygon() {
	vkDestroyDescriptorSetLayout(device->device, descSetLayout, nullptr);
	vkDestroyDescriptorPool(device->device, descPool, nullptr);
	vkDestroyPipeline(device->device, pipeline, nullptr);
	vkDestroyPipelineCache(device->device, pipelineCache, nullptr);
	vkDestroyPipelineLayout(device->device, pipelineLayout, nullptr);
	vkDestroyShaderModule(device->device, vsModule, nullptr);
	vkDestroyShaderModule(device->device, fsModule, nullptr);
	vkDestroyBuffer(device->device, vertices.first, nullptr);
	vkFreeMemory(device->device, vertices.second, nullptr);
}

void VulkanBasicPolygon::create(Vertex3D* ver, uint32_t num) {

	static VkVertexInputBindingDescription bindDesc =
	{
		0, sizeof(Vertex3D), VK_VERTEX_INPUT_RATE_VERTEX
	};
	static VkVertexInputAttributeDescription attrDescs[] =
	{
		{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 2 }
	};
	vertices = device->createVertexBuffer<Vertex3D>(ver, sizeof(Vertex3D) * num);
	vsModule = device->createShaderModule(vsShaderBasicPolygon);
	fsModule = device->createShaderModule(fsShaderBasicPolygon);

	device->descriptorAndPipelineLayouts(pipelineLayout, descSetLayout);
	device->createDescriptorPool(descPool);
	device->upDescriptorSet(descSet, descPool, descSetLayout);
	pipelineCache = device->createPipelineCache();
	pipeline = device->createGraphicsPipelineVF(vsModule, fsModule, bindDesc, attrDescs, 2, pipelineLayout, device->renderPass, pipelineCache);
}

void VulkanBasicPolygon::draw() {
	static VkViewport vp = { 0.0f, 0.0f, device->width, device->height, 0.0f, 1.0f };
	static VkRect2D sc = { { 0, 0 }, { device->width, device->height } };
	static VkDeviceSize offsets[] = { 0 };

	vkCmdBindPipeline(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
		&descSet, 0, nullptr);
	vkCmdSetViewport(device->commandBuffer[comIndex], 0, 1, &vp);
	vkCmdSetScissor(device->commandBuffer[comIndex], 0, 1, &sc);
	vkCmdBindVertexBuffers(device->commandBuffer[comIndex], 0, 1, &vertices.first, offsets);
	vkCmdDraw(device->commandBuffer[comIndex], 3, 1, 0, 0);
}