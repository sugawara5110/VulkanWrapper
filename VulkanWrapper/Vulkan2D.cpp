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

void Vulkan2D::create(Vertex2D* ver, uint32_t num) {

	static VkVertexInputBindingDescription bindDesc =
	{
		0, sizeof(Vertex2D), VK_VERTEX_INPUT_RATE_VERTEX
	};
	static VkVertexInputAttributeDescription attrDescs[] =
	{
		{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 2 }
	};
	vertices = device->createVertexBuffer<Vertex2D>(ver, sizeof(Vertex2D) * num);
	auto vs = device->createShaderModule(vsShader);
	auto fs = device->createShaderModule(fsShader);

	auto pLayout = device->createPipelineLayout();
	auto pCache = device->createPipelineCache();
	pipeline = device->createGraphicsPipelineVF(vs, fs, bindDesc, attrDescs, 2, pLayout, device->renderPass, pCache);
}

void Vulkan2D::draw() {
	static VkViewport vp = { 0.0f, 0.0f, device->width, device->height, 0.0f, 1.0f };
	static VkRect2D sc = { { 0, 0 }, { device->width, device->height } };
	static VkDeviceSize offsets[] = { 0 };

	vkCmdBindPipeline(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdSetViewport(device->commandBuffer[comIndex], 0, 1, &vp);
	vkCmdSetScissor(device->commandBuffer[comIndex], 0, 1, &sc);
	vkCmdBindVertexBuffers(device->commandBuffer[comIndex], 0, 1, &vertices.first, offsets);
	vkCmdDraw(device->commandBuffer[comIndex], 3, 1, 0, 0);
}