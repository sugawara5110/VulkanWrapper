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

class VulkanBasicPolygon {

private:
	Device* device = nullptr;
	std::pair<VkBuffer, VkDeviceMemory> vertices;
	std::pair<VkBuffer, VkDeviceMemory> index;
	uint32_t numIndex;
	VkShaderModule vsModule;
	VkShaderModule fsModule;
	VkDescriptorSetLayout descSetLayout;
	VkDescriptorPool descPool;
	VkDescriptorSet descSet;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkPipeline pipeline;
	uint32_t comIndex = 0;
	Device::UniformSet uniform;
	Device::UniformSetMaterial material;

public:
	VulkanBasicPolygon(Device* device, uint32_t comIndex = 0);
	~VulkanBasicPolygon();
	void create(uint32_t difTexInd, uint32_t norTexInd, Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum);
	void setMaterialParameter(VECTOR3 diffuse, VECTOR3 specular, VECTOR3 ambient);
	void draw(VECTOR3 pos = { 0.0f,0.0f,0.0f }, VECTOR3 theta = { 0.0f,0.0f,0.0f }, VECTOR3 scale = { 1.0f,1.0f,1.0f });
};

#endif