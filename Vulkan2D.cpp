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

	static VkVertexInputBindingDescription bindDesc
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

	device->beginCommandWithFramebuffer(comIndex, VkFramebuffer());
	device->initialImageLayouting(comIndex);
	auto res = vkEndCommandBuffer(device->commandBuffer[comIndex]);
	checkError(res);
	device->submitCommandAndWait(comIndex);

	// Acquire First
	device->acquireNextImageAndWait(device->currentFrameIndex);
}

void Vulkan2D::draw() {
	static VkViewport vp = { 0.0f, 0.0f, device->width, device->height, 0.0f, 1.0f };
	static VkRect2D sc = { { 0, 0 }, { device->width, device->height } };
	static VkDeviceSize offsets[] = { 0 };

	device->beginCommandWithFramebuffer(comIndex, device->frameBuffer[device->currentFrameIndex]);
	device->barrierResource(device->currentFrameIndex, comIndex,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	device->beginRenderPass(device->currentFrameIndex, comIndex);
	vkCmdBindPipeline(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdSetViewport(device->commandBuffer[comIndex], 0, 1, &vp);
	vkCmdSetScissor(device->commandBuffer[comIndex], 0, 1, &sc);
	vkCmdBindVertexBuffers(device->commandBuffer[comIndex], 0, 1, &vertices.first, offsets);
	vkCmdDraw(device->commandBuffer[comIndex], 3, 1, 0, 0);
	vkCmdEndRenderPass(device->commandBuffer[comIndex]);
	device->barrierResource(device->currentFrameIndex, comIndex,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	vkEndCommandBuffer(device->commandBuffer[comIndex]);

	// Submit and Wait with Fences
	device->submitCommands(comIndex);
	switch (device->waitForFence())
	{
	case VK_SUCCESS: device->present(device->currentFrameIndex); break;
	case VK_TIMEOUT: throw std::runtime_error("Command execution timed out."); break;
	default: OutputDebugString(L"waitForFence returns unknown value.\n");
	}
	device->resetFence();

	// Acquire next
	device->acquireNextImageAndWait(device->currentFrameIndex);
}