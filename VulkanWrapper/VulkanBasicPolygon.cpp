//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanBasicPolygon                                       **//
//**                                                                                     **//
//*****************************************************************************************//


#include "VulkanBasicPolygon.h"
#include "Shader/ShaderBasicPolygon.h"

VulkanBasicPolygon::VulkanBasicPolygon(Device* dev) {
	device = dev;
	vs = vsShaderBasicPolygon;
	fs = fsShaderBasicPolygon;
}

VulkanBasicPolygon::~VulkanBasicPolygon() {
	for (uint32_t s = 0; s < numSwap; s++) {
		vkDestroyBuffer(device->device, uniform[s].vkBuf, nullptr);
		vkFreeMemory(device->device, uniform[s].mem, nullptr);
	}
	vkDestroyDescriptorSetLayout(device->device, descSetLayout, nullptr);
	vkDestroyPipeline(device->device, pipeline, nullptr);
	vkDestroyPipelineCache(device->device, pipelineCache, nullptr);
	vkDestroyPipelineLayout(device->device, pipelineLayout, nullptr);
	vkDestroyBuffer(device->device, vertices.first, nullptr);
	vkFreeMemory(device->device, vertices.second, nullptr);
	for (uint32_t i = 0; i < numMaterial; i++) {
		vkDestroyBuffer(device->device, index[i].first, nullptr);
		vkFreeMemory(device->device, index[i].second, nullptr);
		for (uint32_t s = 0; s < numSwap; s++) {
			vkDestroyBuffer(device->device, material[s][i].vkBuf, nullptr);
			vkFreeMemory(device->device, material[s][i].mem, nullptr);
			vkFreeDescriptorSets(device->device, descPool[s][i], descSetCnt, &descSet[s][i]);
			vkDestroyDescriptorPool(device->device, descPool[s][i], nullptr);
		}
	}
	for (uint32_t s = 0; s < numSwap; s++) {
		ARR_DELETE(material[s]);
	}
}

void VulkanBasicPolygon::create(uint32_t comIndex, int32_t difTexInd, int32_t norTexInd, int32_t speTexInd, Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum) {

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
	create0<Vertex3D>(comIndex, 1, tex, sw, ver, num, &ind, &indNum, attrDescs, 4, vsShaderBasicPolygon, fsShaderBasicPolygon);
}

void VulkanBasicPolygon::setMaterialParameter(uint32_t swapIndex, VECTOR3 diffuse, VECTOR3 specular, VECTOR3 ambient, uint32_t materialIndex) {
	material[swapIndex][materialIndex].uni.diffuse.as(diffuse.x, diffuse.y, diffuse.z, 1.0f);
	material[swapIndex][materialIndex].uni.specular.as(specular.x, specular.y, specular.z, 1.0f);
	material[swapIndex][materialIndex].uni.ambient.as(ambient.x, ambient.y, ambient.z, 1.0f);
}

void VulkanBasicPolygon::update0(uint32_t swapIndex, VECTOR3 pos, VECTOR3 theta, VECTOR3 scale, MATRIX* bone, uint32_t numBone) {

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
	MATRIX vm;
	MatrixMultiply(&vm, &world, &device->view);
	MatrixMultiply(&uniform[swapIndex].uni.mvp, &vm, &device->proj);
	uniform[swapIndex].uni.world = world;
	if (numBone > 0)memcpy(uniform[swapIndex].uni.bone, bone, sizeof(MATRIX) * numBone);
	device->updateUniform(uniform[swapIndex]);

	for (uint32_t m = 0; m < numMaterial; m++) {
		Device::Uniform<Device::Material>& mat = material[swapIndex][m];
		mat.uni.viewPos.as(device->viewPos.x, device->viewPos.y, device->viewPos.z, 0.0f);
		memcpy(mat.uni.lightPos, device->lightPos, sizeof(VECTOR4) * device->numLight);
		memcpy(mat.uni.lightColor, device->lightColor, sizeof(VECTOR4) * device->numLight);
		mat.uni.numLight.x = (float)device->numLight;
		mat.uni.numLight.y = device->attenuation1;
		mat.uni.numLight.z = device->attenuation2;
		mat.uni.numLight.w = device->attenuation3;
		device->updateUniform(mat);
	}
}

void VulkanBasicPolygon::update(uint32_t swapIndex, VECTOR3 pos, VECTOR3 theta, VECTOR3 scale) {
	update0(swapIndex, pos, theta, scale, nullptr, 0);
}

void VulkanBasicPolygon::draw(uint32_t swapIndex, uint32_t comIndex) {
	static VkViewport vp = { 0.0f, 0.0f, (float)device->width, (float)device->height, 0.0f, 1.0f };
	static VkRect2D sc = { { 0, 0 }, { device->width, device->height } };
	static VkDeviceSize offsets[] = { 0 };

	vkCmdBindPipeline(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdSetViewport(device->commandBuffer[comIndex], 0, 1, &vp);
	vkCmdSetScissor(device->commandBuffer[comIndex], 0, 1, &sc);
	vkCmdBindVertexBuffers(device->commandBuffer[comIndex], 0, 1, &vertices.first, offsets);

	for (uint32_t m = 0; m < numMaterial; m++) {
		if (numIndex[m] <= 0)continue;
		vkCmdBindDescriptorSets(device->commandBuffer[comIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
			&descSet[swapIndex][m], 0, nullptr);
		vkCmdBindIndexBuffer(device->commandBuffer[comIndex], index[m].first, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(device->commandBuffer[comIndex], numIndex[m], 1, 0, 0, 0);
	}
}