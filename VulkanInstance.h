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
#pragma comment(lib, "vulkan-1")
#define S_DELETE(p)   if(p){delete p;      p=nullptr;}
#define ARR_DELETE(p) if(p){delete[] p;    p=nullptr;}

void checkError(VkResult res);

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT, uint64_t object,
	size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);

class VulkanInstance final {

private:
	//Debug Layer Extensions
	PFN_vkCreateDebugReportCallbackEXT	_vkCreateDebugReportCallbackEXT;
	PFN_vkDebugReportMessageEXT			_vkDebugReportMessageEXT;
	PFN_vkDestroyDebugReportCallbackEXT	_vkDestroyDebugReportCallbackEXT;
	VkDebugReportCallbackEXT debugReportCallback;
	VkInstance instance = nullptr;
	VkSurfaceKHR surface = nullptr;
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
	VkPhysicalDevice pDev;
	VkSurfaceKHR surface;
	VkDevice device;
	VkQueue devQueue;
	uint32_t queueFamilyIndex = 0xffffffff;
	VkPhysicalDeviceMemoryProperties memProps;
	VkCommandPool commandPool;
	VkFence fence;
	VkSwapchainKHR swapchain;
	uint32_t width, height;

	uint32_t imageCount = 0;
	std::unique_ptr <VkImage[]> images = nullptr;
	std::unique_ptr <VkImageView[]> views = nullptr;
	VkRenderPass renderPass;
	std::unique_ptr <VkFramebuffer[]> frameBuffer = nullptr;
	std::unique_ptr <VkCommandBuffer[]> commandBuffer = nullptr;

	Device() {}
	void create();
	void createCommandPool();
	void createSwapchain();
	void retrieveImagesFromSwapchain();
	void createImageViews();
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

		auto res = vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer);
		checkError(res);

		VkMemoryRequirements memreq;
		VkMemoryAllocateInfo allocInfo{};
		vkGetBufferMemoryRequirements(device, vertexBuffer, &memreq);
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memreq.size;
		allocInfo.memoryTypeIndex = UINT32_MAX;
		// Search memory index can be visible from host
		for (size_t i = 0; i < this->memProps.memoryTypeCount; i++)
		{
			if ((this->memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
			{
				allocInfo.memoryTypeIndex = i;
				break;
			}
		}
		if (allocInfo.memoryTypeIndex == UINT32_MAX) throw std::runtime_error("No found available heap.");

		res = vkAllocateMemory(device, &allocInfo, nullptr, &deviceMemory);
		checkError(res);

		// Set data
		uint8_t* pData;
		res = vkMapMemory(device, deviceMemory, 0, sizeof(T) * num, 0, reinterpret_cast<void**>(&pData));
		checkError(res);
		memcpy(pData, ver, sizeof(T) * num);
		vkUnmapMemory(device, deviceMemory);

		// Associate memory to buffer
		res = vkBindBufferMemory(device, vertexBuffer, deviceMemory, 0);
		checkError(res);
		std::pair<VkBuffer, VkDeviceMemory> buf = std::make_pair(vertexBuffer, deviceMemory);
		return buf;
	}

	auto createShaderModule(char* shader);
	auto createPipelineLayout();
	auto createPipelineCache();
	auto createGraphicsPipelineVF(
		const VkShaderModule& vshader, const VkShaderModule& fshader,
		const VkVertexInputBindingDescription& bindDesc, const VkVertexInputAttributeDescription* attrDescs, uint32_t numAttr,
		const VkPipelineLayout& pLayout, const VkRenderPass renderPass, const VkPipelineCache& pCache);
	//モデル毎

	void createCommandBuffers();
	void createFence();
	void submitCommandAndWait(uint32_t comBufindex);
	void acquireNextImageAndWait(uint32_t currentFrameIndex);
	void submitCommands(uint32_t comBufindex);
	auto waitForFence();
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

	//2Dtest
	uint32_t currentFrameIndex = 0;
	void d2test();
	void d2testDraw();
};
#endif
