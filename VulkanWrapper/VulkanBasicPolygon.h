//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanBasicPolygon                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanBasicPolygon_Header
#define VulkanBasicPolygon_Header

#include "VulkanInstance.h"

struct Vertex3D {
	float pos[3];
	float normal[3];
	float uv[2];
};

class VulkanSkinMesh;

class VulkanBasicPolygon {

private:
	friend VulkanSkinMesh;
	Device* device = nullptr;
	std::pair<VkBuffer, VkDeviceMemory> vertices;
	std::pair<VkBuffer, VkDeviceMemory> index;
	uint32_t numIndex;
	VkDescriptorSetLayout descSetLayout;
	VkDescriptorPool descPool;
	VkDescriptorSet descSet;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkPipeline pipeline;
	uint32_t comIndex = 0;
	Device::Uniform<Device::MatrixSet> uniform;
	Device::Uniform<Device::Material> material;
	char* vs = nullptr;
	char* fs = nullptr;

	template<typename T>
	void create0(int32_t difTexInd, int32_t norTexInd,
		T* ver, uint32_t num,
		uint32_t* ind, uint32_t indNum,
		VkVertexInputAttributeDescription* attrDescs, uint32_t numAttr, char* vs, char* fs) {

		numIndex = indNum;
		static VkVertexInputBindingDescription bindDesc =
		{
			0, sizeof(T), VK_VERTEX_INPUT_RATE_VERTEX
		};

		vertices = device->createVertexBuffer<T>(comIndex, ver, num, false);
		index = device->createVertexBuffer<uint32_t>(comIndex, ind, indNum, true);
		VkShaderModule vsModule = device->createShaderModule(vs);
		VkShaderModule fsModule = device->createShaderModule(fs);

		device->createUniform(uniform, material);
		device->descriptorAndPipelineLayouts(true, pipelineLayout, descSetLayout);
		device->createDescriptorPool(true, descPool);

		Device::Texture* diff = nullptr;
		if (difTexInd < 0) {
			diff = &device->texture[device->numTextureMax];//-1の場合テクスチャー無いので, ダミーを入れる
		}
		else {
			diff = &device->texture[difTexInd];
		}
		Device::Texture* nor = nullptr;
		if (norTexInd < 0) {
			nor = &device->texture[device->numTextureMax];
		}
		else {
			nor = &device->texture[norTexInd];
		}

		device->upDescriptorSet(true, *diff, *nor, uniform, material, descSet, descPool, descSetLayout);
		pipelineCache = device->createPipelineCache();
		pipeline = device->createGraphicsPipelineVF(vsModule, fsModule, bindDesc, attrDescs, numAttr, pipelineLayout, device->renderPass, pipelineCache);
		vkDestroyShaderModule(device->device, vsModule, nullptr);
		vkDestroyShaderModule(device->device, fsModule, nullptr);
	}

	void draw0(VECTOR3 pos, VECTOR3 theta, VECTOR3 scale, MATRIX* bone = nullptr, uint32_t numBone = 0);

public:
	VulkanBasicPolygon(Device* device, uint32_t comIndex = 0);
	~VulkanBasicPolygon();
	void create(int32_t difTexInd, int32_t norTexInd, Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum);
	void setMaterialParameter(VECTOR3 diffuse, VECTOR3 specular, VECTOR3 ambient);
	void draw(VECTOR3 pos = { 0.0f,0.0f,0.0f }, VECTOR3 theta = { 0.0f,0.0f,0.0f }, VECTOR3 scale = { 1.0f,1.0f,1.0f });
};

#endif