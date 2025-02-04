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
	VkDescriptorSet descSet[RasterizeDescriptor::numSwap];
	uint32_t descSetCnt = 0;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkPipeline pipeline;
	VulkanDevice::Uniform<RasterizeDescriptor::Instancing>* uniform[RasterizeDescriptor::numSwap] = {};
	std::vector<RasterizeDescriptor::Instancing> mat2d[RasterizeDescriptor::numSwap] = {};

	uint32_t maxInstancingCnt = 256;
	uint32_t InstancingCnt = 0;

	void createShader(
		VkPipelineShaderStageCreateInfo& vsInfo,
		VkPipelineShaderStageCreateInfo& fsInfo,
		char* vs, char* fs);

	template<typename T>
	void create(uint32_t QueueIndex, uint32_t comIndex, bool useAlpha, bool blending,
		T* ver, uint32_t num, uint32_t* ind, uint32_t indNum,
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

		VkPipelineShaderStageCreateInfo vsInfo = {};
		VkPipelineShaderStageCreateInfo fsInfo = {};
		createShader(vsInfo, fsInfo, vs, fs);

		vertices = device->createVertexBuffer<T>(QueueIndex, comIndex, ver, sizeof(T) * num, false, nullptr, nullptr);
		index = device->createVertexBuffer<uint32_t>(QueueIndex, comIndex, ind, sizeof(uint32_t) * indNum,
			true, nullptr, nullptr);

		rd->descriptorAndPipelineLayouts2D(useTexture, pipelineLayout, descSetLayout);

		for (uint32_t i = 0; i < RasterizeDescriptor::numSwap; i++) {
			uniform[i] = NEW VulkanDevice::Uniform<RasterizeDescriptor::Instancing>(maxInstancingCnt);
			mat2d[i].resize(maxInstancingCnt);

			if (textureId < 0) {
				VulkanDevice::Texture tex = device->getTexture(device->numTextureMax);
				VulkanDevice::BufferSet bs;
				if (i == 0)device->createVkTexture(texture, QueueIndex, comIndex, tex, bs);
				bs.destroy();
				//-1の場合テクスチャー無いので, ダミーを入れる
			}
			else {
				VulkanDevice::Texture tex = device->getTexture(textureId);
				VulkanDevice::BufferSet bs;
				if (i == 0)device->createVkTexture(texture, QueueIndex, comIndex, tex, bs);
				bs.destroy();
			}
			rd->upDescriptorSet2D(useTexture, texture, uniform[i], descSet[i], descSetLayout);
		}

		pipelineCache = rd->createPipelineCache();
		VulkanSwapchain* sw = VulkanSwapchain::GetInstance();
		pipeline = rd->createGraphicsPipelineVF(useAlpha, blending, vsInfo, fsInfo,
			bindDesc, attrDescs, 2, pipelineLayout, sw->getRenderPass(), pipelineCache);
		_vkDestroyShaderModule(device->getDevice(), vsInfo.module, nullptr);
		_vkDestroyShaderModule(device->getDevice(), fsInfo.module, nullptr);
	}

public:
	Vulkan2D();
	~Vulkan2D();

	void setNumMaxInstancing(uint32_t num);

	void createColor(uint32_t QueueIndex, uint32_t comIndex, bool useAlpha, bool blending, Vertex2D* ver, uint32_t num, uint32_t* ind, uint32_t indNum);
	void createTexture(uint32_t QueueIndex, uint32_t comIndex, bool useAlpha, bool blending, Vertex2DTex* ver, uint32_t num, uint32_t* ind, uint32_t indNum, int textureId);

	void Instancing(uint32_t swapIndex, CoordTf::VECTOR3 pos = {},
		float theta = 0.0f, CoordTf::VECTOR2 scale = { 1.0f,1.0f },
		CoordTf::VECTOR4 addCol = {},
		float px = 1.0f, float py = 1.0f, float mx = 0.0f, float my = 0.0f);//px,py:幅の倍率, mx,my:何個目か

	void Instancing(uint32_t swapIndex, CoordTf::MATRIX world,
		CoordTf::VECTOR4 addCol = {},
		float px = 1.0f, float py = 1.0f, float mx = 0.0f, float my = 0.0f);

	void Instancing_update(uint32_t swapIndex);

	void update(uint32_t swapIndex, CoordTf::VECTOR3 pos = {},
		float theta = 0.0f, CoordTf::VECTOR2 scale = { 1.0f,1.0f },
		CoordTf::VECTOR4 addCol = {},
		float px = 1.0f, float py = 1.0f, float mx = 0.0f, float my = 0.0f);

	void draw(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex);
};

#endif
