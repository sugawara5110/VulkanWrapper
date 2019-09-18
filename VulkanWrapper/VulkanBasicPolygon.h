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
	float color[4];
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
	Device::Uniform uniform;

public:
	VulkanBasicPolygon(Device* device, uint32_t comIndex = 0);
	~VulkanBasicPolygon();
	void create(Vertex3D* ver, uint32_t num, uint32_t* ind, uint32_t indNum);
	void draw(VECTOR3 pos = { 0.0f,0.0f,0.0f }, VECTOR3 theta = { 0.0f,0.0f,0.0f }, VECTOR3 scale = { 1.0f,1.0f,1.0f });
};

#endif