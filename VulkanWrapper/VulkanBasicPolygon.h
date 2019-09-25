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
	Device::UniformSet uniform;
	Device::UniformSetMaterial material;

	template<typename T>
	void create0(uint32_t difTexInd, uint32_t norTexInd,
		T* ver, uint32_t num,
		uint32_t* ind, uint32_t indNum,
		VkVertexInputAttributeDescription* attrDescs, uint32_t numAttr, char* vs) {

		numIndex = indNum;
		static VkVertexInputBindingDescription bindDesc =
		{
			0, sizeof(T), VK_VERTEX_INPUT_RATE_VERTEX
		};

		vertices = device->createVertexBuffer<T>(comIndex, ver, sizeof(T) * num, false);
		index = device->createVertexBuffer<uint32_t>(comIndex, ind, sizeof(uint32_t) * indNum, true);
		VkShaderModule vsModule = device->createShaderModule(vs);
		VkShaderModule fsModule = device->createShaderModule(fsShaderBasicPolygon);

		device->createUniform(uniform, material);
		device->descriptorAndPipelineLayouts(true, pipelineLayout, descSetLayout);
		device->createDescriptorPool(true, descPool);
		device->upDescriptorSet(true, device->texture[difTexInd], device->texture[norTexInd], uniform, material, descSet, descPool, descSetLayout);
		pipelineCache = device->createPipelineCache();
		pipeline = device->createGraphicsPipelineVF(vsModule, fsModule, bindDesc, attrDescs, numAttr, pipelineLayout, device->renderPass, pipelineCache);
		vkDestroyShaderModule(device->device, vsModule, nullptr);
		vkDestroyShaderModule(device->device, fsModule, nullptr);
	}

public:
	VulkanBasicPolygon(Device* device, uint32_t comIndex = 0);
	~VulkanBasicPolygon();
	void create(uint32_t difTexInd, uint32_t norTexInd, Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum);
	void setMaterialParameter(VECTOR3 diffuse, VECTOR3 specular, VECTOR3 ambient);
	void draw(VECTOR3 pos = { 0.0f,0.0f,0.0f }, VECTOR3 theta = { 0.0f,0.0f,0.0f }, VECTOR3 scale = { 1.0f,1.0f,1.0f });
};

#endif