//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanInstance.cpp                                     **//
//**                                                                                     **//
//*****************************************************************************************//

#include "VulkanInstance.h"

static uint32_t ApiVersion = VK_API_VERSION_1_0;

void VulkanInstance::createinstance(char* appName, uint32_t apiVersion, uint32_t applicationVersion, uint32_t engineVersion) {

    VkInstanceCreateInfo instanceInfo{};
    VkApplicationInfo appInfo{};
    //"VK_KHR_surface", "VK_KHR_win32_surface":Windows環境で必須, "VK_EXT_debug_report":検証レイヤで必須
#ifdef __ANDROID__
    const char* extensions[] = { "VK_KHR_surface", "VK_KHR_android_surface" };
    instanceInfo.ppEnabledLayerNames = nullptr;
    instanceInfo.enabledLayerCount = 0;
#else
    const char* extensions[] =
    { "VK_KHR_get_physical_device_properties2",
        "VK_KHR_surface", "VK_KHR_win32_surface",
        "VK_EXT_debug_report","VK_EXT_debug_utils" };

    const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
    instanceInfo.ppEnabledLayerNames = layers;
    instanceInfo.enabledLayerCount = (uint32_t)COUNTOF(layers);
#endif

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.applicationVersion = applicationVersion;
    appInfo.pApplicationName = appName;
    appInfo.pEngineName = appName;
    appInfo.engineVersion = engineVersion;
    appInfo.apiVersion = apiVersion;
    ApiVersion = apiVersion;

    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = (uint32_t)COUNTOF(extensions);
    instanceInfo.ppEnabledExtensionNames = extensions;

    //Vulkanインスタンス生成
    auto res = vkCreateInstance(&instanceInfo, nullptr, &instance);
    vkUtil::checkError(res);

    //デバック
#ifdef __ANDROID__
#else
    _vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(
        instance, "vkCreateDebugReportCallbackEXT"));
    _vkDebugReportMessageEXT = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(
        instance, "vkDebugReportMessageEXT"));
    _vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(
        instance, "vkDestroyDebugReportCallbackEXT"));
#endif
}

void VulkanInstance::createDebugReportCallback() {

    VkDebugReportCallbackCreateInfoEXT callbackInfo{};

    callbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    callbackInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT
        | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
    callbackInfo.pfnCallback = &vkUtil::debugCallback;
    //デバック有効化
    auto res = _vkCreateDebugReportCallbackEXT(instance, &callbackInfo, nullptr,
        &debugReportCallback);
    vkUtil::checkError(res);
}

void VulkanInstance::createPhysicalDevice() {

    //物理デバイス出力先にnullptrを指定:adapterCountに物理デバイス個数出力
    auto res = vkEnumeratePhysicalDevices(instance, &adapterCount, nullptr);
    vkUtil::checkError(res);
    adapters = std::make_unique<VkPhysicalDevice[]>(adapterCount);
    //個数分の物理デバイス出力
    res = vkEnumeratePhysicalDevices(instance, &adapterCount, adapters.get());
    vkUtil::checkError(res);
    //物理デバイスのプロパティ情報出力
#ifdef __ANDROID__
#else
    OutputDebugString(L"=== Physical Device Enumeration ===\n");
#endif
    for (uint32_t i = 0; i < adapterCount; i++) {
        static VkPhysicalDeviceProperties props;
        static VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceProperties(adapters[i], &props);
        vkGetPhysicalDeviceMemoryProperties(adapters[i], &memProps);
#ifdef __ANDROID__
#else
        OutputDebugString(L"#"); OutputDebugString(std::to_wstring(i).c_str()); OutputDebugString(L": \n");
        OutputDebugString(L"  Name: "); OutputDebugStringA(props.deviceName); OutputDebugString(L"\n");
        OutputDebugString(L"  API Version: "); OutputDebugString(std::to_wstring(props.apiVersion).c_str()); OutputDebugString(L"\n");
#endif
    }
}

VulkanInstance::~VulkanInstance() {
    destroySurface();
#ifdef __ANDROID__
#else
    _vkDestroyDebugReportCallbackEXT(instance, debugReportCallback, nullptr);
#endif
    vkDestroyInstance(instance, nullptr);
}

#ifdef __ANDROID__
void VulkanInstance::createInstance(char* appName) {
    VulkanPFN();
    createinstance(appName);//インスタンス生成
    createPhysicalDevice();//物理デバイス生成
}
#else
void VulkanInstance::createInstance(char* appName, uint32_t apiVersion, uint32_t applicationVersion, uint32_t engineVersion) {
    createinstance(appName, apiVersion, applicationVersion, engineVersion);//インスタンス生成
    createDebugReportCallback();//デバック
    createPhysicalDevice();//物理デバイス生成
}
#endif

#ifdef __ANDROID__
void VulkanInstance::createSurfaceAndroid(ANativeWindow* Window) {
    VkAndroidSurfaceCreateInfoKHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.pNext = nullptr;
    surfaceInfo.flags = 0;
    surfaceInfo.window = Window;
    //android用のサーフェース生成
    auto res = vkCreateAndroidSurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
    checkError(res);
    surfaceAlive = true;
}
#else
void VulkanInstance::createSurfaceHwnd(HWND hWnd) {
    VkWin32SurfaceCreateInfoKHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hinstance = GetModuleHandle(nullptr);
    surfaceInfo.hwnd = hWnd;
    //Windows用のサーフェース生成
    auto res = vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
    vkUtil::checkError(res);
    surfaceAlive = true;
}
#endif

void VulkanInstance::destroySurface() {
    if (surfaceAlive) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surfaceAlive = false;
    }
}

VkPhysicalDevice VulkanInstance::getPhysicalDevice(int index) {
    return adapters[index];
}

VkSurfaceKHR VulkanInstance::getSurface() {
    return surface;
}

VkInstance VulkanInstance::getInstance() {
    return instance;
}

uint32_t VulkanInstance::getApiVersion()const {
    return ApiVersion;
}