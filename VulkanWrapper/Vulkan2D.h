//*****************************************************************************************//
//**                                                                                     **//
//**                               Vulkan2D.h                                            **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Vulkan2D_Header
#define Vulkan2D_Header

#include "VulkanInstance.h"

struct Vertex2D {
	float pos[2];
	float color[4];
};

struct Vertex2DTex {
	float pos[2];
	float uv[2];
};

class Vulkan2D {

private:
	Device* device = nullptr;
	std::pair<VkBuffer, VkDeviceMemory> vertices;
	std::pair<VkBuffer, VkDeviceMemory> index;
	uint32_t numIndex;
	VkDescriptorSetLayout descSetLayout;
	const static uint32_t numSwap = 2;
	VkDescriptorPool descPool[numSwap];
	VkDescriptorSet descSet[numSwap];
	uint32_t descSetCnt = 0;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkPipeline pipeline;
	Device::Uniform<Device::MatrixSet2D> uniform[numSwap];

public:
	Vulkan2D(Device* device);
	~Vulkan2D();
	void create(uint32_t comIndex, Vertex2D* ver, uint32_t num, uint32_t* ind, uint32_t indNum);
	void update(uint32_t swapIndex, VECTOR2 pos = { 0.0f,0.0f });
	void draw(uint32_t swapIndex, uint32_t comIndex);
};

#endif
