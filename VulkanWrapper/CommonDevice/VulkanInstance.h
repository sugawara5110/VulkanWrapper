//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanInstance.h                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanInstance_Header
#define VulkanInstance_Header

#include "vkUtil.h"

#define COUNTOF(array) (sizeof(array) / sizeof(array[0]))

class VulkanInstance final {

private:
    //Debug Layer Extensions
    PFN_vkCreateDebugReportCallbackEXT _vkCreateDebugReportCallbackEXT;
    PFN_vkDebugReportMessageEXT _vkDebugReportMessageEXT;
    PFN_vkDestroyDebugReportCallbackEXT _vkDestroyDebugReportCallbackEXT;
    VkDebugReportCallbackEXT debugReportCallback;
    VkInstance instance = nullptr;
    VkSurfaceKHR surface;//画面を定義するオブジェクト,windows,android
    bool surfaceAlive = false;
    std::unique_ptr<VkPhysicalDevice[]> adapters = nullptr;
    uint32_t adapterCount = 0;

    void createinstance(char* appName, uint32_t apiVersion, uint32_t applicationVersion, uint32_t engineVersion);

    void createDebugReportCallback();

    void createPhysicalDevice();

public:
    ~VulkanInstance();

    void createInstance(char* appName, uint32_t apiVersion = VK_API_VERSION_1_0, uint32_t applicationVersion = VK_MAKE_VERSION(0, 0, 1), uint32_t engineVersion = 1);

#ifdef __ANDROID__
    void createSurfaceAndroid(ANativeWindow* Window);
#else
    void createSurfaceHwnd(HWND hWnd);
#endif
    void destroySurface();

    VkPhysicalDevice getPhysicalDevice(int index = 0, char* deviceName = nullptr);

    VkSurfaceKHR getSurface();

    VkInstance getInstance();

    uint32_t getApiVersion()const;
};

#endif