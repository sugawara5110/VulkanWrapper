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
	vkDestroyBuffer(device->device, vertices.first, nullptr);
	vkFreeMemory(device->device, vertices.second, nullptr);
	vkDestroyBuffer(device->device, index.first, nullptr);
	vkFreeMemory(device->device, index.second, nullptr);
}

void VulkanBasicPolygon::create(uint32_t difTexInd, uint32_t norTexInd, Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum) {

	static VkVertexInputAttributeDescription attrDescs[] =
	{
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 },
		{ 2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6 },
	};

	create0<Vertex3D>(difTexInd, norTexInd, ver, num, ind, indNum, attrDescs, 3, vsShaderBasicPolygon);
}

void VulkanBasicPolygon::setMaterialParameter(VECTOR3 diffuse, VECTOR3 specular, VECTOR3 ambient) {
	material.uni.diffuse.as(diffuse.x, diffuse.y, diffuse.z, 1.0f);
	material.uni.specular.as(specular.x, specular.y, specular.z, 1.0f);
	material.uni.ambient.as(ambient.x, ambient.y, ambient.z, 1.0f);
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

	device->updateUniform(uniform, world, material);

	vkCmdBindPipeline(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
		&descSet, 0, nullptr);
	vkCmdSetViewport(device->commandBuffer[comIndex], 0, 1, &vp);
	vkCmdSetScissor(device->commandBuffer[comIndex], 0, 1, &sc);
	vkCmdBindVertexBuffers(device->commandBuffer[comIndex], 0, 1, &vertices.first, offsets);
	vkCmdBindIndexBuffer(device->commandBuffer[comIndex], index.first, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(device->commandBuffer[comIndex], numIndex, 1, 0, 0, 0);
}