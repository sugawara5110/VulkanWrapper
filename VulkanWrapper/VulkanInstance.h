//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanInstance.h                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanInstance_Header
#define VulkanInstance_Header

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <tuple>
#include <functional>
#include "VulkanTransformation.h"
#pragma comment(lib, "vulkan-1")
#define S_DELETE(p)   if(p){delete p;      p=nullptr;}
#define ARR_DELETE(p) if(p){delete[] p;    p=nullptr;}

void checkError(VkResult res);

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT, uint64_t object,
	size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);

class Vulkan2D;
class VulkanBasicPolygon;

class VulkanInstance final {

private:
	//Debug Layer Extensions
	PFN_vkCreateDebugReportCallbackEXT	_vkCreateDebugReportCallbackEXT;
	PFN_vkDebugReportMessageEXT			_vkDebugReportMessageEXT;
	PFN_vkDestroyDebugReportCallbackEXT	_vkDestroyDebugReportCallbackEXT;
	VkDebugReportCallbackEXT debugReportCallback;
	VkInstance instance = nullptr;
	VkSurfaceKHR surface = nullptr;//画面を定義するオブジェクト,現状はwindowsのみ, 後でandroidも
	std::unique_ptr<VkPhysicalDevice[]> adapters = nullptr;
	uint32_t adapterCount = 0;

	void createinstance(char* appName);
	void createDebugReportCallback();
	void createSurfaceHwnd(HWND hWnd);
	void createPhysicalDevice();

public:
	~VulkanInstance();
	void createInstance(HWND hWnd, char* appName);
	VkPhysicalDevice getPhysicalDevice(int index = 0);
	VkSurfaceKHR getSurface();
};

class Device final {

private:
	friend Vulkan2D;
	friend VulkanBasicPolygon;
	VkPhysicalDevice pDev;//VulkanInstanceからポインタを受け取る
	VkSurfaceKHR surface;//VulkanInstanceからポインタを受け取る
	VkDevice device;
	VkQueue devQueue;
	uint32_t queueFamilyIndex = 0xffffffff;
	VkPhysicalDeviceMemoryProperties memProps;
	VkCommandPool commandPool;
	VkFence fence;

	struct swapchainBuffer {
		VkSwapchainKHR swapchain;
		uint32_t imageCount = 0;
		std::unique_ptr <VkImage[]> images = nullptr;//スワップチェーンの画像表示に使う
		std::unique_ptr <VkImageView[]> views = nullptr;
		std::unique_ptr <VkFramebuffer[]> frameBuffer = nullptr;
	};
	swapchainBuffer swBuf;

	struct Depth {
		VkFormat format;
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	};
	Depth depth;

	uint32_t width, height;
	VkRenderPass renderPass;
	uint32_t commandBufferCount = 1;
	std::unique_ptr <VkCommandBuffer[]> commandBuffer = nullptr;
	uint32_t currentFrameIndex = 0;

	MATRIX proj, view;

	struct Uniform {
		VkBuffer vkBuf;
		VkDeviceMemory mem;
		VkDeviceSize memSize;
		MATRIX mvp;
		VkDescriptorBufferInfo info;
	};

	struct Texture {
		VkImage vkIma;
		VkDeviceMemory mem;
		VkDeviceSize memSize;
		uint32_t width;
		uint32_t height;
		VkDescriptorImageInfo info;
	};
	Texture texture[256];
	uint32_t numTexture = 0;
	VkSampler textureSampler;

	Device() {}
	void create();
	void createCommandPool();
	void createFence();
	void createSwapchain();
	void createDepth();
	void createCommonRenderPass();
	void createFramebuffers();
	void createCommandBuffers();
	void initialImageLayouting(uint32_t comBufindex);

	void beginCommandWithFramebuffer(uint32_t comBufindex, VkFramebuffer fb);
	void submitCommandAndWait(uint32_t comBufindex);
	void acquireNextImageAndWait(uint32_t& currentFrameIndex);
	void submitCommands(uint32_t comBufindex);
	VkResult waitForFence();
	void present(uint32_t currentframeIndex);
	void resetFence();

	void barrierResource(uint32_t currentframeIndex, uint32_t comBufindex,
		VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VkAccessFlags srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
		VkAccessFlags dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VkImageLayout srcImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	void beginRenderPass(uint32_t currentframeIndex, uint32_t comBufindex);

	//テクスチャ
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
	void createImage(uint32_t width, uint32_t height, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
		VkImage& image, VkDeviceMemory& imageMemory);
	auto createTextureImage(unsigned char* byteArr, uint32_t width, uint32_t height);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags mask,
		VkComponentMapping components = { VK_COMPONENT_SWIZZLE_IDENTITY });
	void createTextureSampler(VkSampler& textureSampler);
	void destroyTexture();
	//テクスチャ

	//モデル毎(モデル側から呼ばれる)
	template<typename T>
	auto createVertexBuffer(T* ver, int num, bool typeIndex) {

		VkDeviceSize bufferSize = sizeof(T) * num;
		VkDeviceSize size;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory, size);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, ver, (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (typeIndex)usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory, size);

		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		std::pair<VkBuffer, VkDeviceMemory> buf = std::make_pair(vertexBuffer, vertexBufferMemory);
		return buf;
	}

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize& memSize);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createUniform(Uniform& uni);
	void updateUniform(Uniform& uni, MATRIX move);
	void descriptorAndPipelineLayouts(bool useTexture, VkPipelineLayout& pipelineLayout, VkDescriptorSetLayout& descSetLayout);
	VkPipelineLayout createPipelineLayout2D();
	VkShaderModule createShaderModule(char* shader);
	void createDescriptorPool(bool useTexture, VkDescriptorPool& descPool);
	void upDescriptorSet(bool useTexture, Texture texture, Uniform& uni, VkDescriptorSet& descriptorSet,
		VkDescriptorPool& descPool, VkDescriptorSetLayout& descSetLayout);
	VkPipelineCache createPipelineCache();
	VkPipeline createGraphicsPipelineVF(
		const VkShaderModule& vshader, const VkShaderModule& fshader,
		const VkVertexInputBindingDescription& bindDesc, const VkVertexInputAttributeDescription* attrDescs, uint32_t numAttr,
		const VkPipelineLayout& pLayout, const VkRenderPass renderPass, const VkPipelineCache& pCache);
	//モデル毎

public:
	Device(VkPhysicalDevice pd, VkSurfaceKHR surface, uint32_t width = 640, uint32_t height = 480);
	~Device();
	void createDevice();
	void GetTexture(unsigned char* byteArr, uint32_t width, uint32_t height);
	void updateProjection(float AngleView = 45.0f, float Near = 1.0f, float Far = 100.0f);
	void updateView(VECTOR3 view, VECTOR3 gaze, VECTOR3 up);
	void beginCommand(uint32_t comBufindex);
	void endCommand(uint32_t comBufindex);
	void waitFence(uint32_t comBufindex);
};
#endif
