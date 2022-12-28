//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanBloom.h                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanBloom_Header
#define VulkanBloom_Header

#include "../CommonDevice/VulkanDevice.h"

class VulkanBloom {

public:
	struct InstanceParam {
		uint32_t EmissiveInstanceId = 0;
		float thresholdLuminance = 0.0f;
		float bloomStrength = 1.0f;
		float numGaussFilter = 1.0f;
	};
	std::vector<InstanceParam> iParam;

private:
	uint32_t Width = 0;
	uint32_t Height = 0;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline Pipeline = VK_NULL_HANDLE;
	VkDescriptorSetLayout dsLayout = VK_NULL_HANDLE;
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	const static int numMaxFilter = 7;

	const uint32_t gaBaseSize[numMaxFilter] = { 1024,512,256,128,64,32,16 };

	VulkanDevice::ImageSet* pRaytracedImage = nullptr;//ポインタ受け取り
	VulkanDevice::ImageSet* pInstanceIdMap = nullptr;//ポインタ受け取り

	VulkanDevice::BufferSet GaussianFilter;

	struct FilterSet {
		VulkanDevice::ImageSet Luminance[numMaxFilter];
		VulkanDevice::ImageSet inOutBloom0[numMaxFilter];
		VulkanDevice::ImageSet inOutBloom1[numMaxFilter];//サンプラーも
	};
	std::unique_ptr<FilterSet[]> fset;

	VulkanDevice::ImageSet Output;

	struct BloomParam {
		float GaussianWid;
		float bloomStrength;
		float thresholdLuminance;
		float numGaussFilter;
		int   InstanceID;
	};
	BloomParam bParam = {};
	VulkanDevice::Uniform<BloomParam>* bParamUBO = nullptr;

	VkWriteDescriptorSet getDescriptorSet(uint32_t binding, VkDescriptorType type, VkDescriptorImageInfo* infoI, VkDescriptorBufferInfo* infoB);

	void createGaussianFilter(uint32_t comIndex, float sigma);
	void createBuffer(uint32_t comIndex);
	void createLayouts();
	void createPipeline();
	void createDescriptorSets();

public:
	~VulkanBloom();

	void setImage(
		VulkanDevice::ImageSet* raytracedImage, VulkanDevice::ImageSet* instanceIdMap,
		uint32_t width, uint32_t height,
		std::vector<InstanceParam> instanceParam);

	void Create(uint32_t comIndex, float sigma = 10.0f);

	void Compute(uint32_t comIndex);
};

#endif