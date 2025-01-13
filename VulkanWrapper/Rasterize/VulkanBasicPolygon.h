//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanBasicPolygon.h                                     **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanBasicPolygon_Header
#define VulkanBasicPolygon_Header

#include "RasterizeDescriptor.h"

class VulkanSkinMesh;

class VulkanBasicPolygon {

private:
	friend VulkanSkinMesh;

	VulkanDevice::textureIdSet* texId = nullptr;

	VulkanDevice::BufferSet vertices;
	std::unique_ptr<VulkanDevice::BufferSet[]> index;
	std::unique_ptr<uint32_t[]> numIndex;
	VkDescriptorSetLayout descSetLayout;
	const static uint32_t numSwap = 2;
	std::unique_ptr<VkDescriptorSet[]> descSet[numSwap];
	uint32_t descSetCnt = 0;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkPipeline pipeline;
	VulkanDevice::Uniform<RasterizeDescriptor::MatrixSet>* uniform[numSwap] = {};
	RasterizeDescriptor::MatrixSet matset[numSwap];
	VulkanDevice::Uniform<RasterizeDescriptor::Material>** material[numSwap] = {};
	RasterizeDescriptor::Material* materialset[numSwap];
	uint32_t numMaterial = 1;
	char* vs = nullptr;
	char* fs = nullptr;

	uint32_t InstancingCnt = 0;

	template<typename T>
	void create0(uint32_t QueueIndex, uint32_t comIndex, bool useAlpha,
		int32_t numMat, VulkanDevice::textureIdSet* texid, float* uvSw,
		T* ver, uint32_t num,
		uint32_t** ind, uint32_t* indNum,
		VkVertexInputAttributeDescription* attrDescs, uint32_t numAttr, char* vs, char* fs) {

		static VkVertexInputBindingDescription bindDesc =
		{
			0, sizeof(T), VK_VERTEX_INPUT_RATE_VERTEX
		};

		VulkanDevice* device = VulkanDevice::GetInstance();

		numMaterial = numMat;
		texId = NEW VulkanDevice::textureIdSet[numMaterial];
		memcpy(texId, texid, sizeof(VulkanDevice::textureIdSet) * numMaterial);
		index = std::make_unique<VulkanDevice::BufferSet[]>(numMaterial);
		numIndex = std::make_unique<uint32_t[]>(numMaterial);
		for (uint32_t i = 0; i < numSwap; i++) {
			descSet[i] = std::make_unique<VkDescriptorSet[]>(numMaterial);
		}

		VkPipelineShaderStageCreateInfo vsInfo = device->createShaderModule("BPvs", vs, VK_SHADER_STAGE_VERTEX_BIT);
		VkPipelineShaderStageCreateInfo fsInfo = device->createShaderModule("BPfs", fs, VK_SHADER_STAGE_FRAGMENT_BIT);

		vertices = device->createVertexBuffer<T>(QueueIndex, comIndex, ver, num, false, nullptr, nullptr);
		RasterizeDescriptor* rd = RasterizeDescriptor::GetInstance();
		rd->descriptorAndPipelineLayouts(true, pipelineLayout, descSetLayout);

		for (uint32_t i = 0; i < numSwap; i++) {
			material[i] = NEW VulkanDevice::Uniform<RasterizeDescriptor::Material>*[numMaterial];
			materialset[i] = NEW RasterizeDescriptor::Material[numMaterial];
			uniform[i] = NEW VulkanDevice::Uniform<RasterizeDescriptor::MatrixSet>(1);
			for (uint32_t m = 0; m < numMaterial; m++) {
				if (i == 0)numIndex[m] = indNum[m];
				material[i][m] = NEW VulkanDevice::Uniform<RasterizeDescriptor::Material>(1);
				if (numIndex[m] <= 0)continue;
				materialset[i][m].UvSwitch.x = uvSw[m];
				material[i][m]->update(0, &materialset[i][m]);
				if (i == 0)index[m] = device->createVertexBuffer<uint32_t>(QueueIndex, comIndex, ind[m], indNum[m],
					true, nullptr, nullptr);

				if (i == 0)device->createTextureSet(QueueIndex, comIndex, texId[m]);

				descSetCnt = rd->upDescriptorSet(true, texId[m].difTex, texId[m].norTex, texId[m].speTex,
					uniform[i], material[i][m],
					descSet[i][m], descSetLayout);
			}
		}

		pipelineCache = rd->createPipelineCache();
		VulkanSwapchain* sw = VulkanSwapchain::GetInstance();
		pipeline = rd->createGraphicsPipelineVF(useAlpha, vsInfo, fsInfo, bindDesc, attrDescs, numAttr,
			pipelineLayout, sw->getRenderPass(), pipelineCache);
		_vkDestroyShaderModule(device->getDevice(), vsInfo.module, nullptr);
		_vkDestroyShaderModule(device->getDevice(), fsInfo.module, nullptr);
	}

	void update0(uint32_t swapIndex, CoordTf::MATRIX* bone, uint32_t numBone);

public:
	VulkanBasicPolygon();
	~VulkanBasicPolygon();

	void create(uint32_t QueueIndex, uint32_t comIndex, bool useAlpha, int32_t difTexInd, int32_t norTexInd, int32_t speTexInd,
		VulkanDevice::Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum);

	void setMaterialParameter(uint32_t swapIndex, CoordTf::VECTOR3 diffuse, CoordTf::VECTOR3 specular, CoordTf::VECTOR3 ambient,
		uint32_t materialIndex = 0);

	void Instancing(uint32_t swapIndex, CoordTf::VECTOR3 pos = { 0.0f,0.0f,0.0f },
		CoordTf::VECTOR3 theta = { 0.0f,0.0f,0.0f }, CoordTf::VECTOR3 scale = { 1.0f,1.0f,1.0f },
		float px = 1.0f, float py = 1.0f, float mx = 0.0f, float my = 0.0f);//px,py:幅の倍率, mx,my:何個目か

	void Instancing_update(uint32_t swapIndex);

	void update(uint32_t swapIndex, CoordTf::VECTOR3 pos = { 0.0f,0.0f,0.0f },
		CoordTf::VECTOR3 theta = { 0.0f,0.0f,0.0f }, CoordTf::VECTOR3 scale = { 1.0f,1.0f,1.0f },
		float px = 1.0f, float py = 1.0f, float mx = 0.0f, float my = 0.0f);

	void draw(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex);
};

#endif