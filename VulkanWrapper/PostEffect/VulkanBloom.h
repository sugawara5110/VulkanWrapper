//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanBloom.h                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanBloom_Header
#define VulkanBloom_Header

#include "../CommonDevice/VulkanSwapchain.h"

class VulkanBloom {

public:
	struct InstanceParam {
		uint32_t EmissiveInstanceId = 0;
		float thresholdLuminance = 0.0f;
		float bloomStrength = 1.0f;
	};

private:
	class Bloom {

	private:
		InstanceParam iParam;
		uint32_t Width;
		uint32_t Height;

		const static int numGausShader = 3;

		struct DescSet {
			VkPipelineLayout pipelineLayout;
			VkPipeline Pipeline;
			VkDescriptorSetLayout dsLayout;
			VkDescriptorSet descriptorSet;
			VkExtent2D sizeWH;
		};
		std::vector<DescSet> dset;

		std::vector<uint32_t> GausSize;

		VulkanDevice::ImageSet* pRenderedImage = nullptr;//ポインタ受け取り
		VulkanDevice::ImageSet* pInstanceIdMap = nullptr;//ポインタ受け取り

		VulkanDevice::BufferSet GaussianFilter;

		struct FilterSet {
			VulkanDevice::ImageSet Luminance;
			VulkanDevice::ImageSet inOutBloom0;
			VulkanDevice::ImageSet inOutBloom1;//サンプラーも
		};
		std::vector<FilterSet> fset;

		std::vector<VkDescriptorImageInfo> inOutBloom1_Info;

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

		void createGaussianFilter(uint32_t QueueIndex, uint32_t comIndex, float sigma);
		void createBuffer(uint32_t QueueIndex, uint32_t comIndex);
		void createLayouts();
		void createPipeline();
		void createDescriptorSets();

	public:
		~Bloom();

		void setImage(
			VulkanDevice::ImageSet* RenderedImage, VulkanDevice::ImageSet* instanceIdMap,
			uint32_t width, uint32_t height,
			InstanceParam instanceParam,
			std::vector<uint32_t> gausSize = { 512,256,128,64,32 });

		void Create(uint32_t QueueIndex, uint32_t comIndex, float sigma = 10.0f);

		void Compute(uint32_t QueueIndex, uint32_t comIndex);

		VulkanDevice::ImageSet* getOutput() {
			return &Output;
		}
	};

	uint32_t Width;
	uint32_t Height;

	static const uint32_t numSwap = 3;

	VkPipelineLayout pipelineLayout[numSwap] = {};
	VkPipeline Pipeline[numSwap] = {};
	VkDescriptorSetLayout dsLayout[numSwap] = {};
	VkDescriptorSet descriptorSet[numSwap] = {};

	struct BloomParam {
		int numInstance;
	};
	BloomParam bParam = {};
	VulkanDevice::Uniform<BloomParam>* bParamUBO = nullptr;

	VulkanDevice::ImageSet* pRenderedImage = nullptr;//ポインタ受け取り

	std::vector<Bloom> bloom;
	std::vector<VkDescriptorImageInfo> blOutput_Info;
	VulkanDevice::ImageSet Output[numSwap] = {};

	void createBuffer(uint32_t QueueIndex, uint32_t comIndex);
	void createLayouts();
	void createPipeline();
	void createDescriptorSets();

public:
	~VulkanBloom();

	void setImage(
		VulkanDevice::ImageSet* RenderedImage, VulkanDevice::ImageSet* instanceIdMap,
		uint32_t width, uint32_t height,
		std::vector<InstanceParam> instanceParams,
		std::vector<std::vector<uint32_t>>* gausSizes = nullptr);

	void Create(uint32_t QueueIndex, uint32_t comIndex, std::vector<float>* sigma = nullptr);

	void Compute(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex);

	void CopyImage(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex);
};

#endif