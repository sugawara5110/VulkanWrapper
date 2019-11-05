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
	vs = vsShaderBasicPolygon;
	fs = fsShaderBasicPolygon;
}

VulkanBasicPolygon::~VulkanBasicPolygon() {
	vkDestroyBuffer(device->device, uniform.vkBuf, nullptr);
	vkFreeMemory(device->device, uniform.mem, nullptr);
	vkDestroyDescriptorSetLayout(device->device, descSetLayout, nullptr);
	vkDestroyPipeline(device->device, pipeline, nullptr);
	vkDestroyPipelineCache(device->device, pipelineCache, nullptr);
	vkDestroyPipelineLayout(device->device, pipelineLayout, nullptr);
	vkDestroyBuffer(device->device, vertices.first, nullptr);
	vkFreeMemory(device->device, vertices.second, nullptr);
	for (uint32_t i = 0; i < numMaterial; i++) {
		vkDestroyBuffer(device->device, index[i].first, nullptr);
		vkFreeMemory(device->device, index[i].second, nullptr);
		vkDestroyDescriptorPool(device->device, descPool[i], nullptr);
	}
	ARR_DELETE(material);
}

void VulkanBasicPolygon::create(int32_t difTexInd, int32_t norTexInd, int32_t speTexInd, Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum) {

	static VkVertexInputAttributeDescription attrDescs[] =
	{
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 },
		{ 2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6 },
		{ 3, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 8 }
	};

	textureIdSet tex[1];
	tex[0].diffuseId = difTexInd;
	tex[0].normalId = norTexInd;
	tex[0].specularId = speTexInd;
	float sw[1] = {};
	create0<Vertex3D>(1, tex, sw, ver, num, &ind, &indNum, attrDescs, 4, vsShaderBasicPolygon, fsShaderBasicPolygon);
}

void VulkanBasicPolygon::setMaterialParameter(VECTOR3 diffuse, VECTOR3 specular, VECTOR3 ambient, uint32_t materialIndex) {
	material[materialIndex].uni.diffuse.as(diffuse.x, diffuse.y, diffuse.z, 1.0f);
	material[materialIndex].uni.specular.as(specular.x, specular.y, specular.z, 1.0f);
	material[materialIndex].uni.ambient.as(ambient.x, ambient.y, ambient.z, 1.0f);
}

void VulkanBasicPolygon::draw0(VECTOR3 pos, VECTOR3 theta, VECTOR3 scale, MATRIX* bone, uint32_t numBone) {

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

	if (numBone > 0)memcpy(uniform.uni.bone, bone, sizeof(MATRIX) * numBone);

	static VkViewport vp = { 0.0f, 0.0f, (float)device->width, (float)device->height, 0.0f, 1.0f };
	static VkRect2D sc = { { 0, 0 }, { device->width, device->height } };
	static VkDeviceSize offsets[] = { 0 };

	vkCmdBindPipeline(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdSetViewport(device->commandBuffer[comIndex], 0, 1, &vp);
	vkCmdSetScissor(device->commandBuffer[comIndex], 0, 1, &sc);
	vkCmdBindVertexBuffers(device->commandBuffer[comIndex], 0, 1, &vertices.first, offsets);

	for (uint32_t m = 0; m < numMaterial; m++) {
		if (numIndex[m] <= 0)continue;
		uniform.uni.UvSwitch.x = uvSwitch[m];
		device->updateUniform(uniform, world, material[m]);
		vkCmdBindDescriptorSets(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
			&descSet[m], 0, nullptr);
		vkCmdBindIndexBuffer(device->commandBuffer[comIndex], index[m].first, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(device->commandBuffer[comIndex], numIndex[m], 1, 0, 0, 0);
	}
}

void VulkanBasicPolygon::draw(VECTOR3 pos, VECTOR3 theta, VECTOR3 scale) {
	draw0(pos, theta, scale, nullptr, 0);
}