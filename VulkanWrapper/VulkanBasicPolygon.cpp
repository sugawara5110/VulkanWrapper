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
	vkDestroyBuffer(device->device, uniform.vkBuf, nullptr);
	vkFreeMemory(device->device, uniform.mem, nullptr);
	vkDestroyDescriptorSetLayout(device->device, descSetLayout, nullptr);
	vkDestroyDescriptorPool(device->device, descPool, nullptr);
	vkDestroyPipeline(device->device, pipeline, nullptr);
	vkDestroyPipelineCache(device->device, pipelineCache, nullptr);
	vkDestroyPipelineLayout(device->device, pipelineLayout, nullptr);
	vkDestroyShaderModule(device->device, vsModule, nullptr);
	vkDestroyShaderModule(device->device, fsModule, nullptr);
	vkDestroyBuffer(device->device, vertices.first, nullptr);
	vkFreeMemory(device->device, vertices.second, nullptr);
	vkDestroyBuffer(device->device, index.first, nullptr);
	vkFreeMemory(device->device, index.second, nullptr);
}

void VulkanBasicPolygon::create(Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum) {

	numIndex = indNum;
	static VkVertexInputBindingDescription bindDesc =
	{
		0, sizeof(Vertex3D), VK_VERTEX_INPUT_RATE_VERTEX
	};
	static VkVertexInputAttributeDescription attrDescs[] =
	{
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 3 },
		{ 2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 7 },
	};
	vertices = device->createVertexBuffer<Vertex3D>(comIndex, ver, sizeof(Vertex3D) * num, false);
	index = device->createVertexBuffer<uint32_t>(comIndex, ind, sizeof(uint32_t) * indNum, true);
	vsModule = device->createShaderModule(vsShaderBasicPolygon);
	fsModule = device->createShaderModule(fsShaderBasicPolygon);

	device->createUniform(uniform);
	device->descriptorAndPipelineLayouts(true, pipelineLayout, descSetLayout);
	device->createDescriptorPool(true, descPool);
	device->upDescriptorSet(true, device->texture[0], uniform, descSet, descPool, descSetLayout);
	pipelineCache = device->createPipelineCache();
	pipeline = device->createGraphicsPipelineVF(vsModule, fsModule, bindDesc, attrDescs, 3, pipelineLayout, device->renderPass, pipelineCache);
}

void VulkanBasicPolygon::draw(VECTOR3 pos, VECTOR3 theta, VECTOR3 scale) {
	static VkViewport vp = { 0.0f, 0.0f, (float)device->width, (float)device->height, 0.0f, 1.0f };
	static VkRect2D sc = { { 0, 0 }, { device->width, device->height } };
	static VkDeviceSize offsets[] = { 0 };

	MATRIX mov;
	MATRIX rotZ, rotY, rotX, rotZY, rotZYX;
	MATRIX sca;
	MATRIX scro;
	MATRIX world;
	MatrixScaling(&sca, scale.x, scale.y, scale.z);
	MatrixRotationZ(&rotZ, theta.z);
	MatrixRotationY(&rotY, theta.y);
	MatrixRotationX(&rotX, theta.x);
	MatrixMultiply(&rotZY, &rotZ, &rotY);
	MatrixMultiply(&rotZYX, &rotZY, &rotX);
	MatrixTranslation(&mov, pos.x, pos.y, pos.z);
	MatrixMultiply(&scro, &rotZYX, &sca);
	MatrixMultiply(&world, &scro, &mov);

	device->updateUniform(uniform, world);

	vkCmdBindPipeline(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
		&descSet, 0, nullptr);
	vkCmdSetViewport(device->commandBuffer[comIndex], 0, 1, &vp);
	vkCmdSetScissor(device->commandBuffer[comIndex], 0, 1, &sc);
	vkCmdBindVertexBuffers(device->commandBuffer[comIndex], 0, 1, &vertices.first, offsets);
	vkCmdBindIndexBuffer(device->commandBuffer[comIndex], index.first, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(device->commandBuffer[comIndex], numIndex, 1, 0, 0, 0);
}