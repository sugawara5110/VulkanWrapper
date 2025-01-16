//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanBasicPolygon.cpp                                   **//
//**                                                                                     **//
//*****************************************************************************************//


#include "VulkanBasicPolygon.h"
#include "Shader/ShaderBasicPolygon.h"

VulkanBasicPolygon::VulkanBasicPolygon() {
	vs = vsShaderBasicPolygon;
	fs = fsShaderBasicPolygon;
}

VulkanBasicPolygon::~VulkanBasicPolygon() {

	for (uint32_t s = 0; s < numSwap; s++) {
		vkUtil::S_DELETE(uniform[s]);
		for (uint32_t i = 0; i < numMaterial; i++) {
			vkUtil::S_DELETE(material[s][i]);
		}
		vkUtil::ARR_DELETE(material[s]);
		vkUtil::ARR_DELETE(materialset[s]);
	}

	VulkanDevice* device = VulkanDevice::GetInstance();
	VkDevice vd = device->getDevice();

	_vkDestroyDescriptorSetLayout(vd, descSetLayout, nullptr);
	_vkDestroyPipeline(vd, pipeline, nullptr);
	_vkDestroyPipelineCache(vd, pipelineCache, nullptr);
	_vkDestroyPipelineLayout(vd, pipelineLayout, nullptr);
	vertices.destroy();
	for (uint32_t i = 0; i < numMaterial; i++) {
		if (numIndex[i] <= 0)continue;
		index[i].destroy();
		texId[i].destroy();
	}
	vkUtil::ARR_DELETE(texId);
}

void VulkanBasicPolygon::create(uint32_t QueueIndex, uint32_t comIndex, bool useAlpha, int32_t difTexInd, int32_t norTexInd, int32_t speTexInd, VulkanDevice::Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum) {

	static VkVertexInputAttributeDescription attrDescs[] =
	{
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 },
		{ 2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6 },
		{ 3, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 8 }
	};

	const uint32_t numMaterial = 1;
	VulkanDevice::textureIdSet tex[numMaterial];
	tex[0].diffuseId = difTexInd;
	tex[0].normalId = norTexInd;
	tex[0].specularId = speTexInd;
	float sw[1] = {};
	create0<VulkanDevice::Vertex3D>(QueueIndex, comIndex, useAlpha, numMaterial, tex, sw, ver, num, &ind, &indNum,
		attrDescs, 4, vsShaderBasicPolygon, fsShaderBasicPolygon);
}

void VulkanBasicPolygon::setMaterialParameter(uint32_t swapIndex,
	CoordTf::VECTOR3 diffuse, CoordTf::VECTOR3 specular, CoordTf::VECTOR3 ambient, uint32_t materialIndex) {

	materialset[swapIndex][materialIndex].diffuse.as(diffuse.x, diffuse.y, diffuse.z, 1.0f);
	materialset[swapIndex][materialIndex].specular.as(specular.x, specular.y, specular.z, 1.0f);
	materialset[swapIndex][materialIndex].ambient.as(ambient.x, ambient.y, ambient.z, 1.0f);
	material[swapIndex][materialIndex]->update(0, &materialset[swapIndex][materialIndex]);
}

void VulkanBasicPolygon::Instancing0(uint32_t swapIndex, CoordTf::MATRIX world, float px, float py, float mx, float my) {

	if (InstancingCnt >= RasterizeDescriptor::numInstancingMax) {
		throw std::runtime_error("InstancingCnt The value of numInstancingMax reached.");
	}

	using namespace CoordTf;
	VulkanDevice* device = VulkanDevice::GetInstance();

	MATRIX vm;
	MATRIX vi = device->getCameraView();
	MatrixMultiply(&vm, &world, &vi);
	MATRIX pro = device->getProjection();
	MatrixMultiply(&matset[swapIndex].mvp[InstancingCnt], &vm, &pro);
	matset[swapIndex].world[InstancingCnt] = world;
	matset[swapIndex].pXpYmXmY[InstancingCnt] = { px, py, mx, my };

	if (InstancingCnt < RasterizeDescriptor::numInstancingMax) {
		InstancingCnt++;
	}
}

void VulkanBasicPolygon::Instancing(
	uint32_t swapIndex,
	CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale,
	float px, float py, float mx, float my) {

	VulkanDevice* device = VulkanDevice::GetInstance();
	using namespace CoordTf;
	MATRIX world;

	vkUtil::calculationMatrixWorld(world, pos, theta, scale);

	Instancing0(swapIndex, world, px, py, mx, my);
}

void VulkanBasicPolygon::Instancing(uint32_t swapIndex, CoordTf::MATRIX world,
	float px, float py, float mx, float my) {

	Instancing0(swapIndex, world, px, py, mx, my);
}

void VulkanBasicPolygon::update0(uint32_t swapIndex, CoordTf::MATRIX* bone, uint32_t numBone) {

	VulkanDevice* device = VulkanDevice::GetInstance();
	using namespace CoordTf;

	if (numBone > 0)memcpy(matset[swapIndex].bone, bone, sizeof(MATRIX) * numBone);
	uniform[swapIndex]->update(0, &matset[swapIndex]);

	RasterizeDescriptor* rd = RasterizeDescriptor::GetInstance();

	for (uint32_t m = 0; m < numMaterial; m++) {
		if (numIndex[m] <= 0)continue;
		RasterizeDescriptor::Material& mat = materialset[swapIndex][m];
		mat.viewPos.as(device->getCameraViewPos().x, device->getCameraViewPos().y, device->getCameraViewPos().z, 0.0f);
		memcpy(mat.lightPos, rd->lightPos, sizeof(VECTOR4) * rd->numLight);
		memcpy(mat.lightColor, rd->lightColor, sizeof(VECTOR4) * rd->numLight);
		mat.numLight.x = (float)rd->numLight;
		mat.numLight.y = rd->attenuation1;
		mat.numLight.z = rd->attenuation2;
		mat.numLight.w = rd->attenuation3;
		material[swapIndex][m]->update(0, &mat);
	}
}

void VulkanBasicPolygon::Instancing_update(uint32_t swapIndex) {
	update0(swapIndex, nullptr, 0);
}

void VulkanBasicPolygon::update(uint32_t swapIndex,
	CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale,
	float px, float py, float mx, float my) {

	Instancing(swapIndex, pos, theta, scale, px, py, mx, my);
	update0(swapIndex, nullptr, 0);
}

void VulkanBasicPolygon::draw(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex) {
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

	for (uint32_t m = 0; m < numMaterial; m++) {
		if (numIndex[m] <= 0)continue;
		_vkCmdBindDescriptorSets(comb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
			&descSet[swapIndex][m], 0, nullptr);
		_vkCmdBindIndexBuffer(comb, index[m].getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		_vkCmdDrawIndexed(comb, numIndex[m], InstancingCnt, 0, 0, 0);
	}
	InstancingCnt = 0;
}