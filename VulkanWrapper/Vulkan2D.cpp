//*****************************************************************************************//
//**                                                                                     **//
//**                               Vulkan2D.h                                            **//
//**                                                                                     **//
//*****************************************************************************************//

#include "Vulkan2D.h"
#include "Shader/Shader2D.h"

Vulkan2D::Vulkan2D(Device* dev, uint32_t comindex) {
	device = dev;
	comIndex = comindex;
}

Vulkan2D::~Vulkan2D() {
	vkDestroyPipeline(device->device, pipeline, nullptr);
	vkDestroyPipelineCache(device->device, pipelineCache, nullptr);
	vkDestroyPipelineLayout(device->device, pipelineLayout, nullptr);
	vkDestroyShaderModule(device->device, vsModule, nullptr);
	vkDestroyShaderModule(device->device, fsModule, nullptr);
	vkDestroyBuffer(device->device, vertices.first, nullptr);
	vkFreeMemory(device->device, vertices.second, nullptr);
}

void Vulkan2D::create(Vertex2D* ver, uint32_t num) {

	numVer = num;
	static VkVertexInputBindingDescription bindDesc =
	{
		0, sizeof(Vertex2D), VK_VERTEX_INPUT_RATE_VERTEX
	};
	static VkVertexInputAttributeDescription attrDescs[] =
	{
		{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 2 }
	};
	vertices = device->createVertexBuffer<Vertex2D>(comIndex, ver, sizeof(Vertex2D) * num, false);
	vsModule = device->createShaderModule(vsShader2D);
	fsModule = device->createShaderModule(fsShader2D);

	pipelineLayout = device->createPipelineLayout2D();
	pipelineCache = device->createPipelineCache();
	pipeline = device->createGraphicsPipelineVF(vsModule, fsModule, bindDesc, attrDescs, 2, pipelineLayout, device->renderPass, pipelineCache);
}

void Vulkan2D::draw() {
	static VkViewport vp = { 0.0f, 0.0f, (float)device->width, (float)device->height, 0.0f, 1.0f };
	static VkRect2D sc = { { 0, 0 }, { device->width, device->height } };
	static VkDeviceSize offsets[] = { 0 };

	vkCmdBindPipeline(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdSetViewport(device->commandBuffer[comIndex], 0, 1, &vp);
	vkCmdSetScissor(device->commandBuffer[comIndex], 0, 1, &sc);
	vkCmdBindVertexBuffers(device->commandBuffer[comIndex], 0, 1, &vertices.first, offsets);
	vkCmdDraw(device->commandBuffer[comIndex], numVer, 1, 0, 0);
}