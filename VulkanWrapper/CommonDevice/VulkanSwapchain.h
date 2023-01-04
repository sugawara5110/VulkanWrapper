//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanSwapchain.h                                      **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanSwapchain_Header
#define VulkanSwapchain_Header

#include "VulkanDevice.h"

class VulkanSwapchain {

private:
    static VulkanSwapchain* swP;

    VulkanSwapchain() {}
    VulkanSwapchain(const VulkanSwapchain& obj) = delete;
    void operator=(const VulkanSwapchain& obj) = delete;

    ~VulkanSwapchain();

    VulkanDevice::ImageSet depth;
    VkExtent2D wh = {};
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::unique_ptr<VkSurfaceFormatKHR[]> BackBufferFormat;
    uint32_t imageCount = 0;
    uint32_t currentFrameIndex = 0;
    std::unique_ptr<VkImage[]> images = nullptr;
    std::unique_ptr<VkImageView[]> views = nullptr;
    std::unique_ptr<VkFramebuffer[]> frameBuffer = nullptr;
    VkFence swFence = nullptr;
    bool firstswFence = false;
    VkFormat format = {};
    bool swapchainAlive = false;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkSemaphore renderCompletedSem, presentCompletedSem;

    void createSemaphore();

    void createswapchain(uint32_t QueueIndex, uint32_t comBufindex,
        VkPhysicalDevice pd, VkSurfaceKHR surface, bool V_SYNC);

    void createDepth(VkPhysicalDevice pd);
    void createFence();
    void createRenderPass(bool clearBackBuffer);
    void createFramebuffers();

    void acquireNextImageAndWait();
    void present(uint32_t QueueIndex);

public:
    static void InstanceCreate();
    static VulkanSwapchain* GetInstance();
    static void DeleteInstance();

    void create(uint32_t QueueIndex, uint32_t comBufindex,
        VkPhysicalDevice pd, VkSurfaceKHR surface, bool clearBackBuffer, bool V_SYNC = false);

    void beginCommandNextImage(uint32_t QueueIndex, uint32_t comBufindex);
    void beginDraw(uint32_t QueueIndex, uint32_t comBufindex);
    void endDraw(uint32_t QueueIndex, uint32_t comBufindex);
    void Present(uint32_t QueueIndex);

    const VkSurfaceFormatKHR getBackBufferFormat(uint32_t index) { return BackBufferFormat[index]; }
    const VkImage getCurrentImage() { return images[currentFrameIndex]; }
    const VkImage getImage(uint32_t index) { return images[index]; }
    const VkFence getFence() { return swFence; }
    const uint32_t getImageCount() { return imageCount; }
    const VkFramebuffer getFramebuffer() { return frameBuffer[currentFrameIndex]; }
    const VkSwapchainKHR* getSwapchain() { return &swapchain; }
    const VkFormat getFormat() { return format; }
    const uint32_t* getCurrentFrameIndex() { return &currentFrameIndex; }
    VulkanDevice::ImageSet* getDepthImageSet() { return &depth; }
    const VkExtent2D getSize() { return wh; }
    const VkRenderPass getRenderPass() { return renderPass; }
};

#endif
