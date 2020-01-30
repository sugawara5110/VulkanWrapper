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
	Device::VkTexture texture;
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

		VkShaderModule vsModule = device->createShaderModule(vs);
		VkShaderModule fsModule = device->createShaderModule(fs);

		vertices = device->createVertexBuffer<T>(comIndex, ver, sizeof(T) * num, false);
		index = device->createVertexBuffer<uint32_t>(comIndex, ind, sizeof(uint32_t) * indNum, true);

		device->descriptorAndPipelineLayouts2D(useTexture, pipelineLayout, descSetLayout);

		for (uint32_t i = 0; i < numSwap; i++) {
			device->createUniform(uniform[i]);
			device->createDescriptorPool2D(useTexture, descPool[i]);
			if (textureId < 0) {
				if (i == 0)device->createVkTexture(texture, comIndex, device->texture[device->numTextureMax]);
				//-1の場合テクスチャー無いので, ダミーを入れる
			}
			else {
				if (i == 0)device->createVkTexture(texture, comIndex, device->texture[textureId]);
			}
			descSetCnt = device->upDescriptorSet2D(useTexture, texture, uniform[i], descSet[i], descPool[i], descSetLayout);
		}

		pipelineCache = device->createPipelineCache();
		pipeline = device->createGraphicsPipelineVF(false, vsModule, fsModule, bindDesc, attrDescs, 2, pipelineLayout, device->renderPass, pipelineCache);
		vkDestroyShaderModule(device->device, vsModule, nullptr);
		vkDestroyShaderModule(device->device, fsModule, nullptr);
	}

public:
	Vulkan2D(Device* device);
	~Vulkan2D();
	void createColor(uint32_t comIndex, Vertex2D* ver, uint32_t num, uint32_t* ind, uint32_t indNum);
	void createTexture(uint32_t comIndex, Vertex2DTex* ver, uint32_t num, uint32_t* ind, uint32_t indNum, int textureId);
	void update(uint32_t swapIndex, VECTOR2 pos = { 0.0f,0.0f });
	void draw(uint32_t swapIndex, uint32_t comIndex);
};

#endif
