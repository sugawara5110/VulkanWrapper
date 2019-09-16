﻿//*****************************************************************************************//
//**                                                                                     **//
//**                               Vulkan2D.h                                            **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Vulkan2D_Header
#define Vulkan2D_Header

#include "VulkanInstance.h"

struct Vertex2D {
	float pos[2];
	float color[4];
};

class Vulkan2D {

private:
	Device* device = nullptr;
	std::pair<VkBuffer, VkDeviceMemory> vertices;
	uint32_t numVer;
	VkShaderModule vsModule;
	VkShaderModule fsModule;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkPipeline pipeline;
	uint32_t comIndex = 0;

public:
	Vulkan2D(Device* device, uint32_t comIndex = 0);
	~Vulkan2D();
	void create(Vertex2D* ver, uint32_t num);
	void draw();
};

#endif