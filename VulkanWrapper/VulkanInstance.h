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

	Device() {}
	void create();
	void createCommandPool();
	void createSwapchain();
	void createDepth();
	void createCommonRenderPass();
	void createFramebuffers();

	//モデル毎(モデル側から呼ばれる)
	template<typename T>
	auto createVertexBuffer(T* ver, int num) {
		VkBuffer vertexBuffer;
		VkDeviceMemory deviceMemory;
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.size = sizeof(T) * num;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		//頂点バッファオブジェクト生成
		auto res = vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer);
		checkError(res);

		VkMemoryRequirements memreq;
		VkMemoryAllocateInfo allocInfo{};
		//頂点バッファに対してのメモリ条件取得
		vkGetBufferMemoryRequirements(device, vertexBuffer, &memreq);
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memreq.size;
		allocInfo.memoryTypeIndex = UINT32_MAX;
		// Search memory index can be visible from host
		for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
		{
			if ((memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
			{
				allocInfo.memoryTypeIndex = i;//メモリタイプインデックス検索
				break;
			}
		}
		if (allocInfo.memoryTypeIndex == UINT32_MAX) throw std::runtime_error("No found available heap.");

		//メモリオブジェクトの割り当て
		res = vkAllocateMemory(device, &allocInfo, nullptr, &deviceMemory);
		checkError(res);

		uint8_t* pData;
		//メモリオブジェクトをマップ
		res = vkMapMemory(device, deviceMemory, 0, sizeof(T) * num, 0, reinterpret_cast<void**>(&pData));
		checkError(res);
		//頂点配列コピー
		memcpy(pData, ver, sizeof(T) * num);
		//アンマップ
		vkUnmapMemory(device, deviceMemory);

		//バッファオブジェクトとメモリオブジェクト関連付け
		res = vkBindBufferMemory(device, vertexBuffer, deviceMemory, 0);
		checkError(res);
		std::pair<VkBuffer, VkDeviceMemory> buf = std::make_pair(vertexBuffer, deviceMemory);
		return buf;
	}

	VkShaderModule createShaderModule(char* shader);
	VkPipelineLayout createPipelineLayout();
	VkPipelineCache createPipelineCache();
	VkPipeline createGraphicsPipelineVF(
		const VkShaderModule& vshader, const VkShaderModule& fshader,
		const VkVertexInputBindingDescription& bindDesc, const VkVertexInputAttributeDescription* attrDescs, uint32_t numAttr,
		const VkPipelineLayout& pLayout, const VkRenderPass renderPass, const VkPipelineCache& pCache);
	//モデル毎

	void createCommandBuffers();
	void createFence();
	void submitCommandAndWait(uint32_t comBufindex);
	void acquireNextImageAndWait(uint32_t& currentFrameIndex);
	void submitCommands(uint32_t comBufindex);
	VkResult waitForFence();
	void present(uint32_t currentframeIndex);
	void resetFence();

	void initialImageLayouting(uint32_t comBufindex);
	void beginCommandWithFramebuffer(uint32_t comBufindex, VkFramebuffer fb);
	void barrierResource(uint32_t currentframeIndex, uint32_t comBufindex,
		VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VkAccessFlags srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
		VkAccessFlags dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VkImageLayout srcImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	void beginRenderPass(uint32_t currentframeIndex, uint32_t comBufindex);

public:
	Device(VkPhysicalDevice pd, VkSurfaceKHR surface, uint32_t width = 640, uint32_t height = 480);
	~Device();
	void createDevice();
	void beginCommand(uint32_t comBufindex);
	void endCommand(uint32_t comBufindex);
	void waitFence(uint32_t comBufindex);
};
#endif
