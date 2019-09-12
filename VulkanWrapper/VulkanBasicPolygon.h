//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanBasicPolygon                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanBasicPolygon_Header
#define VulkanBasicPolygon_Header

#include "VulkanInstance.h"

class VulkanBasicPolygon {

private:
	Device* device = nullptr;
	std::pair<VkBuffer, VkDeviceMemory> vertices;
	VkPipeline pipeline;
	uint32_t comIndex = 0;

public:
	/*VulkanBasicPolygon(Device* device, uint32_t comIndex = 0);
	void create(Vertex2D* ver, uint32_t num);
	void draw();*/
};

#endif