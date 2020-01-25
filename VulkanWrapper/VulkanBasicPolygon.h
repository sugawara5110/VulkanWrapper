//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanBasicPolygon.h                                     **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanBasicPolygon_Header
#define VulkanBasicPolygon_Header

#include "VulkanInstance.h"

struct Vertex3D {
	float pos[3];
	float normal[3];
	float difUv[2];
	float speUv[2];
};

class VulkanSkinMesh;

class VulkanBasicPolygon {

private:
	friend VulkanSkinMesh;

	struct textureIdSet {
		int diffuseId = -1;
		char difUvName[256] = {};
		Device::VkTexture difTex;
		int normalId = -1;
		char norUvName[256] = {};
		Device::VkTexture norTex;
		int specularId = -1;
		char speUvName[256] = {};
		Device::VkTexture speTex;
		void destroy(VkDevice& device) {
			difTex.destroy(device);
			norTex.destroy(device);
			speTex.destroy(device);
		}
	};
	textureIdSet* texId = nullptr;

	Device* device = nullptr;
	std::pair<VkBuffer, VkDeviceMemory> vertices;
	std::unique_ptr<std::pair<VkBuffer, VkDeviceMemory>[]> index;
	std::unique_ptr<uint32_t[]> numIndex;
	VkDescriptorSetLayout descSetLayout;
	const static uint32_t numSwap = 2;
	std::unique_ptr<VkDescriptorPool[]> descPool[numSwap];
	std::unique_ptr<VkDescriptorSet[]> descSet[numSwap];
	uint32_t descSetCnt = 0;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkPipeline pipeline;
	Device::Uniform<Device::MatrixSet> uniform[numSwap];
	Device::Uniform<Device::Material>* material[numSwap];
	uint32_t numMaterial = 1;
	char* vs = nullptr;
	char* fs = nullptr;

	template<typename T>
	void create0(uint32_t comIndex, bool useAlpha,
		int32_t numMat, textureIdSet* texid, float* uvSw,
		T* ver, uint32_t num,
		uint32_t** ind, uint32_t* indNum,
		VkVertexInputAttributeDescription* attrDescs, uint32_t numAttr, char* vs, char* fs) {

		static VkVertexInputBindingDescription bindDesc =
		{
			0, sizeof(T), VK_VERTEX_INPUT_RATE_VERTEX
		};

		numMaterial = numMat;
		texId = new textureIdSet[numMaterial];
		memcpy(texId, texid, sizeof(textureIdSet) * numMaterial);
		index = std::make_unique<std::pair<VkBuffer, VkDeviceMemory>[]>(numMaterial);
		numIndex = std::make_unique<uint32_t[]>(numMaterial);
		for (uint32_t i = 0; i < numSwap; i++) {
			descPool[i] = std::make_unique<VkDescriptorPool[]>(numMaterial);
			descSet[i] = std::make_unique<VkDescriptorSet[]>(numMaterial);
			material[i] = new Device::Uniform<Device::Material>[numMaterial];
		}

		VkShaderModule vsModule = device->createShaderModule(vs);
		VkShaderModule fsModule = device->createShaderModule(fs);

		vertices = device->createVertexBuffer<T>(comIndex, ver, num, false);
		device->descriptorAndPipelineLayouts(true, pipelineLayout, descSetLayout);

		for (uint32_t i = 0; i < numSwap; i++) {
			device->createUniform(uniform[i]);
			for (uint32_t m = 0; m < numMaterial; m++) {
				device->createUniform(material[i][m]);
				numIndex[m] = indNum[m];
				if (numIndex[m] <= 0)continue;
				material[i][m].uni.UvSwitch.x = uvSw[m];
				index[m] = device->createVertexBuffer<uint32_t>(comIndex, ind[m], indNum[m], true);

				device->createDescriptorPool(true, descPool[i][m]);

				Device::VkTexture* diff = nullptr;
				if (texId[m].diffuseId < 0) {
					if (i == 0)device->createVkTexture(texId[m].difTex, comIndex, device->texture[device->numTextureMax]);
					diff = &texId[m].difTex;//-1の場合テクスチャー無いので, ダミーを入れる
				}
				else {
					if (i == 0)device->createVkTexture(texId[m].difTex, comIndex, device->texture[texId[m].diffuseId]);
					diff = &texId[m].difTex;
				}
				Device::VkTexture* nor = nullptr;
				if (texId[m].normalId < 0) {
					if (i == 0)device->createVkTexture(texId[m].norTex, comIndex, device->texture[device->numTextureMax]);
					nor = &texId[m].norTex;
				}
				else {
					if (i == 0)device->createVkTexture(texId[m].norTex, comIndex, device->texture[texId[m].normalId]);
					nor = &texId[m].norTex;
				}
				Device::VkTexture* spe = nullptr;
				if (texId[m].specularId < 0) {
					if (i == 0)device->createVkTexture(texId[m].speTex, comIndex, device->texture[device->numTextureMax + 1]);
					spe = &texId[m].speTex;
				}
				else {
					if (i == 0)device->createVkTexture(texId[m].speTex, comIndex, device->texture[texId[m].specularId]);
					spe = &texId[m].speTex;
				}
				descSetCnt = device->upDescriptorSet(true, *diff, *nor, *spe, uniform[i], material[i][m],
					descSet[i][m], descPool[i][m], descSetLayout);
			}
		}

		pipelineCache = device->createPipelineCache();
		pipeline = device->createGraphicsPipelineVF(useAlpha, vsModule, fsModule, bindDesc, attrDescs, numAttr,
			pipelineLayout, device->renderPass, pipelineCache);
		vkDestroyShaderModule(device->device, vsModule, nullptr);
		vkDestroyShaderModule(device->device, fsModule, nullptr);
	}

	void update0(uint32_t swapIndex, VECTOR3 pos, VECTOR3 theta, VECTOR3 scale, MATRIX* bone, uint32_t numBone);

public:
	VulkanBasicPolygon(Device* device);
	~VulkanBasicPolygon();
	void create(uint32_t comIndex, bool useAlpha, int32_t difTexInd, int32_t norTexInd, int32_t speTexInd,
		Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum);
	void setMaterialParameter(uint32_t swapIndex, VECTOR3 diffuse, VECTOR3 specular, VECTOR3 ambient,
		uint32_t materialIndex = 0);
	void update(uint32_t swapIndex, VECTOR3 pos = { 0.0f,0.0f,0.0f },
		VECTOR3 theta = { 0.0f,0.0f,0.0f }, VECTOR3 scale = { 1.0f,1.0f,1.0f });
	void draw(uint32_t swapIndex, uint32_t comIndex);
};

#endif