//*****************************************************************************************//
//**                                                                                     **//
//**                               Vulkan2D.h                                            **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Vulkan2D_Header
#define Vulkan2D_Header

#include "RasterizeDescriptor.h"

class Vulkan2D {

public:
	struct Vertex2D {
		float pos[2];
		float color[4];
	};

	struct Vertex2DTex {
		float pos[2];
		float uv[2];
	};

private:
	VulkanDevice::ImageSet texture;
	VulkanDevice::BufferSet vertices;
	VulkanDevice::BufferSet index;
	uint32_t numIndex;
	VkDescriptorSetLayout descSetLayout;
	const static uint32_t numSwap = 2;
	VkDescriptorSet descSet[numSwap];
	uint32_t descSetCnt = 0;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkPipeline pipeline;
	VulkanDevice::Uniform<RasterizeDescriptor::MatrixSet2D>* uniform[numSwap] = {};
	RasterizeDescriptor::MatrixSet2D mat2d[numSwap] = {};

	template<typename T>
	void create(uint32_t comIndex, T* ver, uint32_t num, uint32_t* ind, uint32_t indNum,
		VkVertexInputAttributeDescription* attrDescs, uint32_t numAttr, char* vs, char* fs, int textureId) {

		bool useTexture = false;
		if (sizeof(T) == sizeof(Vertex2DTex))useTexture = true;

		numIndex = indNum;
		static VkVertexInputBindingDescription bindDesc =
		{
			0, sizeof(T), VK_VERTEX_INPUT_RATE_VERTEX
		};

		VulkanDevice* device = VulkanDevice::GetInstance();
		RasterizeDescriptor* rd = RasterizeDescriptor::GetInstance();

		VkPipelineShaderStageCreateInfo vsInfo = device->createShaderModule("2Dvs", vs, VK_SHADER_STAGE_VERTEX_BIT);
		VkPipelineShaderStageCreateInfo fsInfo = device->createShaderModule("2Dfs", fs, VK_SHADER_STAGE_FRAGMENT_BIT);

		vertices = device->createVertexBuffer<T>(comIndex, ver, sizeof(T) * num, false, nullptr, nullptr);
		index = device->createVertexBuffer<uint32_t>(comIndex, ind, sizeof(uint32_t) * indNum,
			true, nullptr, nullptr);

		rd->descriptorAndPipelineLayouts2D(useTexture, pipelineLayout, descSetLayout);

		for (uint32_t i = 0; i < numSwap; i++) {
			uniform[i] = new VulkanDevice::Uniform<RasterizeDescriptor::MatrixSet2D>(1);
			if (textureId < 0) {
				if (i == 0)device->createVkTexture(texture, comIndex, device->getTexture(device->numTextureMax));
				//-1の場合テクスチャー無いので, ダミーを入れる
			}
			else {
				if (i == 0)device->createVkTexture(texture, comIndex, device->getTexture(textureId));
			}
			descSetCnt = rd->upDescriptorSet2D(useTexture, texture, uniform[i], descSet[i], descSetLayout);
		}

		pipelineCache = rd->createPipelineCache();
		VulkanSwapchain* sw = VulkanSwapchain::GetInstance();
		pipeline = rd->createGraphicsPipelineVF(false, vsInfo, fsInfo,
			bindDesc, attrDescs, 2, pipelineLayout, sw->getRenderPass(), pipelineCache);
		vkDestroyShaderModule(device->getDevice(), vsInfo.module, nullptr);
		vkDestroyShaderModule(device->getDevice(), fsInfo.module, nullptr);
	}

public:
	Vulkan2D();
	~Vulkan2D();
	void createColor(uint32_t comIndex, Vertex2D* ver, uint32_t num, uint32_t* ind, uint32_t indNum);
	void createTexture(uint32_t comIndex, Vertex2DTex* ver, uint32_t num, uint32_t* ind, uint32_t indNum, int textureId);
	void update(uint32_t swapIndex, CoordTf::VECTOR2 pos = { 0.0f,0.0f });
	void draw(uint32_t swapIndex, uint32_t comIndex);
};

#endif
