//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanBloom.h                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanBloom_Header
#define VulkanBloom_Header

#include "../CommonDevice/VulkanDevice.h"

class VulkanBloom {

private:
	uint32_t Width = 0;
	uint32_t Height = 0;
	uint32_t EmissiveInstanceId[VulkanDevice::numLightMax] = {};
	uint32_t NumEmissive = 0;

	const static int numFilter = 7;

	const unsigned int gaBaseSize[numFilter] = { 1024,512,256,128,64,32,16 };

	VulkanDevice::ImageSet* pRaytracedImage = nullptr;
	VulkanDevice::ImageSet* pInstanceIdMap = nullptr;

	VulkanDevice::BufferSet GaussianFilterBufferUp;
	VulkanDevice::BufferSet GaussianFilterBuffer;

	struct FilterSet {
		VulkanDevice::ImageSet Luminance[numFilter];
		VulkanDevice::ImageSet Bloom[numFilter];
	};
	std::unique_ptr<FilterSet[]> fset;

	VulkanDevice::ImageSet Output;

	struct BloomParam {
		float GaussianWid;
		float bloomStrength;
		float thresholdLuminance;
		float numGaussFilter;
	};

public:
	void setImage(
		VulkanDevice::ImageSet* raytracedImage, VulkanDevice::ImageSet* instanceIdMap,
		uint32_t width, uint32_t height,
		uint32_t* emissiveInstanceId, uint32_t numEmissive);
};

#endif