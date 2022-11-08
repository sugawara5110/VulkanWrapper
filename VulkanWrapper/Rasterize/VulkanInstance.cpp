//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanInstance.cpp                                     **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "VulkanInstance.h"
#define COUNTOF(array) (sizeof(array) / sizeof(array[0]))

static CoordTf::VECTOR3 CalcTangent(CoordTf::VECTOR3 v0, CoordTf::VECTOR3 v1, CoordTf::VECTOR3 v2,
    CoordTf::VECTOR2 uv0, CoordTf::VECTOR2 uv1, CoordTf::VECTOR2 uv2) {

    CoordTf::VECTOR3 cp0[3] = {
        {v0.x, uv0.x, uv0.y},
        {v0.y, uv0.x, uv0.y},
        {v0.z, uv0.x, uv0.y}
    };
    CoordTf::VECTOR3 cp1[3] = {
        {v1.x, uv1.x, uv1.y},
        {v1.y, uv1.x, uv1.y},
        {v1.z, uv1.x, uv1.y}
    };
    CoordTf::VECTOR3 cp2[3] = {
        {v2.x, uv2.x, uv2.y},
        {v2.y, uv2.x, uv2.y},
        {v2.z, uv2.x, uv2.y}
    };

    CoordTf::VECTOR3 tangent = {};
    float tan[3] = {};

    for (int i = 0; i < 3; i++) {
        CoordTf::VECTOR3 v1 = {};
        v1.as(cp1[i].x - cp0[i].x,
            cp1[i].y - cp0[i].y,
            cp1[i].z - cp0[i].z);
        CoordTf::VECTOR3 v2 = {};
        v2.as(cp2[i].x - cp1[i].x,
            cp2[i].y - cp1[i].y,
            cp2[i].z - cp1[i].z);
        CoordTf::VECTOR3 t = {};
        CoordTf::VectorCross(&t, &v1, &v2);

        if (t.x == 0.0f) {
            tan[0] = 0.0f;
            tan[1] = 0.0f;
            tan[2] = 0.0f;
            break;
        }
        else {
            tan[i] = -t.y / t.x;
        }
    }

    tangent.x = tan[0];
    tangent.y = tan[1];
    tangent.z = tan[2];
    CoordTf::VECTOR3 ret = {};
    CoordTf::VectorNormalize(&ret, &tangent);
    return ret;
}

void vkUtil::checkError(VkResult res) {
    if (res != VK_SUCCESS)
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_ERROR, "VulkanERROR", "NOT_VK_SUCCESS");
#else
        throw std::runtime_error(std::to_string(res).c_str());
#endif
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkUtil::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT, uint64_t object,
    size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
#ifdef __ANDROID__
#else
    OutputDebugString(L"Vulkan DebugCall: "); OutputDebugStringA(pMessage); OutputDebugString(L"\n");
#endif
    return VK_FALSE;
}

void vkUtil::calculationMatrixWorld(CoordTf::MATRIX& World, CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale) {
    using namespace CoordTf;
    MATRIX mov;
    MATRIX rotZ, rotY, rotX, rotZY, rotZYX;
    MATRIX sca;
    MATRIX scro;
    MatrixScaling(&sca, scale.x, scale.y, scale.z);
    MatrixRotationZ(&rotZ, theta.z);
    MatrixRotationY(&rotY, theta.y);
    MatrixRotationX(&rotX, theta.x);
    MatrixMultiply(&rotZY, &rotZ, &rotY);
    MatrixMultiply(&rotZYX, &rotZY, &rotX);
    MatrixTranslation(&mov, pos.x, pos.y, pos.z);
    MatrixMultiply(&scro, &rotZYX, &sca);
    MatrixMultiply(&World, &scro, &mov);
}

void vkUtil::addChar::addStr(char* str1, char* str2) {
    size_t size1 = strlen(str1);
    size_t size2 = strlen(str2);
    size = size1 + size2 + 1;
    str = new char[size];
    memcpy(str, str1, size1 + 1);
    strncat(str, str2, size2 + 1);
}

void vkUtil::createTangent(int numMaterial, unsigned int* indexCntArr,
    void* vertexArr, unsigned int** indexArr, int structByteStride,
    int posBytePos, int texBytePos, int tangentBytePos) {

    unsigned char* b_posSt = (unsigned char*)vertexArr + posBytePos;
    unsigned char* b_texSt = (unsigned char*)vertexArr + texBytePos;
    unsigned char* b_tanSt = (unsigned char*)vertexArr + tangentBytePos;
    for (int i = 0; i < numMaterial; i++) {
        unsigned int cnt = 0;
        while (indexCntArr[i] > cnt) {
            CoordTf::VECTOR3* posVec[3] = {};
            CoordTf::VECTOR2* texVec[3] = {};
            CoordTf::VECTOR3* tanVec[3] = {};

            for (int ind = 0; ind < 3; ind++) {
                unsigned int index = indexArr[i][cnt++] * structByteStride;
                unsigned char* b_pos = b_posSt + index;
                unsigned char* b_tex = b_texSt + index;
                unsigned char* b_tan = b_tanSt + index;
                posVec[ind] = (CoordTf::VECTOR3*)b_pos;
                texVec[ind] = (CoordTf::VECTOR2*)b_tex;
                tanVec[ind] = (CoordTf::VECTOR3*)b_tan;
            }
            CoordTf::VECTOR3 tangent = CalcTangent(*(posVec[0]), *(posVec[1]), *(posVec[2]),
                *(texVec[0]), *(texVec[1]), *(texVec[2]));
            *(tanVec[0]) = *(tanVec[1]) = *(tanVec[2]) = tangent;
        }
    }
}

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

VulkanDevice* VulkanDevice::DevicePointer = nullptr;

void VulkanDevice::BufferSet::createUploadBuffer(VkDeviceSize size, VkBufferUsageFlags usage, void* allocateMemory_add_pNext) {
    DevicePointer->createUploadBuffer(size, usage, buffer, mem, allocateMemory_add_pNext);
    Size = size;
    info.range = VK_WHOLE_SIZE;
    info.buffer = buffer;
    info.offset = 0;
}

void VulkanDevice::BufferSet::createDefaultBuffer(VkDeviceSize size, VkBufferUsageFlags usage, void* allocateMemory_add_pNext) {
    DevicePointer->createDefaultBuffer(size, usage, buffer, mem, allocateMemory_add_pNext);
    Size = size;
    info.range = VK_WHOLE_SIZE;
    info.buffer = buffer;
    info.offset = 0;
}

void VulkanDevice::BufferSet::memoryMap(void* pData) {
    memcpy(Map(), pData, (size_t)Size);
    UnMap();
}

void* VulkanDevice::BufferSet::Map() {
    return DevicePointer->Map(mem);
}

void VulkanDevice::BufferSet::UnMap() {
    DevicePointer->UnMap(mem);
}

void VulkanDevice::BufferSet::destroy() {
    VkDevice d = DevicePointer->device;
    vkDestroyBuffer(d, buffer, nullptr);
    vkFreeMemory(d, mem, nullptr);
    buffer = VK_NULL_HANDLE;
    mem = VK_NULL_HANDLE;
}

void VulkanDevice::ImageSet::barrierResource(uint32_t comBufindex, VkImageLayout dstImageLayout) {
    DevicePointer->barrierResource(comBufindex, image, info.imageLayout, dstImageLayout);
    info.imageLayout = dstImageLayout;
}

void VulkanDevice::ImageSet::createImage(uint32_t width, uint32_t height, VkFormat format,
    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {

    DevicePointer->createImage(width, height, format, tiling, usage, properties,
        image, mem);

    info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void VulkanDevice::ImageSet::createImageView(VkFormat format, VkImageAspectFlags mask,
    VkComponentMapping components) {

    info.imageView = DevicePointer->createImageView(image, format, mask, components);
}

void VulkanDevice::ImageSet::destroy() {
    VkDevice d = DevicePointer->device;
    vkDestroyImageView(d, info.imageView, nullptr);
    vkDestroySampler(d, info.sampler, nullptr);
    vkDestroyImage(d, image, nullptr);
    vkFreeMemory(d, mem, nullptr);
    info.imageView = VK_NULL_HANDLE;
    info.sampler = VK_NULL_HANDLE;
    image = VK_NULL_HANDLE;
    mem = VK_NULL_HANDLE;
}

void VulkanDevice::swapchainBuffer::destroySwapchain() {
    if (swapchainAlive) {
        VkDevice d = DevicePointer->device;
        vkDestroyImageView(d, depth.view, nullptr);
        vkDestroyImage(d, depth.image, nullptr);
        vkFreeMemory(d, depth.mem, nullptr);
        vkDestroyRenderPass(d, renderPass, nullptr);
        vkDestroyFence(d, swFence, nullptr);
        depth.view = VK_NULL_HANDLE;
        depth.image = VK_NULL_HANDLE;
        depth.mem = VK_NULL_HANDLE;
        renderPass = VK_NULL_HANDLE;
        swFence = VK_NULL_HANDLE;
        for (uint32_t i = 0; i < imageCount; i++) {
            vkDestroyImageView(d, views[i], nullptr);
            vkDestroyFramebuffer(d, frameBuffer[i], nullptr);
            views[i] = VK_NULL_HANDLE;
            frameBuffer[i] = VK_NULL_HANDLE;
        }
        vkDestroySwapchainKHR(d, swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
        swapchainAlive = false;
    }
}

void VulkanDevice::swapchainBuffer::createswapchain(VkSurfaceKHR surface) {

    VkSwapchainCreateInfoKHR scinfo{};
    VkPhysicalDevice pd = DevicePointer->pDev;
    //デバイスが,スワップチェーンをサポートしているか確認
    VkBool32 surfaceSupported;
    vkGetPhysicalDeviceSurfaceSupportKHR(pd, DevicePointer->queueFamilyIndex, surface, &surfaceSupported);
    //サーフェスの機能取得
    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface, &surfaceCaps);
    //サーフェスフォーマット数取得
    uint32_t surfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &surfaceFormatCount, nullptr);
    //サーフェスフォーマット取得
    BackBufferFormat = std::make_unique<VkSurfaceFormatKHR[]>(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &surfaceFormatCount, BackBufferFormat.get());
    //サーフェスでサポートされるプレゼンテーションモード数取得
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface, &presentModeCount, nullptr);
    //サーフェスでサポートされるプレゼンテーションモード取得
    auto presentModes = std::make_unique<VkPresentModeKHR[]>(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface, &presentModeCount, presentModes.get());

    for (uint32_t i = 0; i < surfaceFormatCount; i++) {
        auto c = BackBufferFormat[i];
#ifdef __ANDROID__
#else
        OutputDebugString(L"Supported Format Check...");
#endif
    }

    wh = surfaceCaps.currentExtent;

    scinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    scinfo.pNext = nullptr;
    scinfo.surface = surface;
    scinfo.minImageCount = surfaceCaps.minImageCount;
    scinfo.imageFormat = BackBufferFormat.get()->format;
    scinfo.imageColorSpace = BackBufferFormat.get()->colorSpace;
    scinfo.imageExtent = wh;
    scinfo.imageArrayLayers = surfaceCaps.maxImageArrayLayers;
    scinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    scinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    scinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    scinfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

#ifdef __ANDROID__
    scinfo.presentMode = presentModes[0];
#else
    scinfo.presentMode = DevicePointer->presentMode;
#endif
    scinfo.clipped = VK_TRUE;
    VkDevice d = DevicePointer->device;
    //スワップチェーン生成
    auto res = vkCreateSwapchainKHR(d, &scinfo, nullptr, &swapchain);
    vkUtil::checkError(res);

    //ウインドウに直接表示する画像のオブジェクト生成
    res = vkGetSwapchainImagesKHR(d, swapchain, &imageCount,
        nullptr);//個数imageCount取得
    vkUtil::checkError(res);
    images = std::make_unique<VkImage[]>(imageCount);
    res = vkGetSwapchainImagesKHR(d, swapchain, &imageCount,
        images.get());//個数分生成
    vkUtil::checkError(res);

    //ビュー生成
    views = std::make_unique<VkImageView[]>(imageCount);
    format = scinfo.imageFormat;

    for (uint32_t i = 0; i < imageCount; i++) {
        views[i] = DevicePointer->createImageView(images[i], format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
             VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A });
    }
}

void VulkanDevice::swapchainBuffer::createDepth() {

    const VkFormat depth_format = VK_FORMAT_D16_UNORM;
    VkImageTiling tiling;
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(DevicePointer->pDev, depth_format, &props);
    if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        tiling = VK_IMAGE_TILING_LINEAR;
    }
    else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        tiling = VK_IMAGE_TILING_OPTIMAL;
    }
    else {
#ifdef __ANDROID__
#else
        OutputDebugString(L"depth_formatUnsupported.\n");
#endif
        exit(-1);
    }

    //深度Image生成
    DevicePointer->createImage(wh.width, wh.height, depth_format,
        tiling, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depth.image, depth.mem);

    VkImageAspectFlags depthMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    //view生成
    depth.view = DevicePointer->createImageView(depth.image, depth_format, depthMask,
        { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
         VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A });
    depth.format = depth_format;
}

void VulkanDevice::swapchainBuffer::createFence() {
    VkFenceCreateInfo finfo{};
    finfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    auto res = vkCreateFence(DevicePointer->device, &finfo, nullptr, &swFence);
    vkUtil::checkError(res);
    firstswFence = false;
}

void VulkanDevice::swapchainBuffer::createRenderPass(bool clearBackBuffer) {

    VkAttachmentDescription attachmentDesc[2]{};
    attachmentDesc[0].format = format;
    attachmentDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDesc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDesc[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachmentDesc[1].format = depth.format;
    attachmentDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDesc[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDesc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachmentDesc[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDesc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDesc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachmentDesc[1].flags = 0;

    if (!clearBackBuffer) {
        attachmentDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    VkAttachmentReference attachmentRef{};
    attachmentRef.attachment = 0;
    attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {};
    depth_reference.attachment = 1;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachmentRef;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depth_reference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachmentDesc;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    auto res = vkCreateRenderPass(DevicePointer->device, &renderPassInfo, nullptr, &renderPass);
    vkUtil::checkError(res);
}

void VulkanDevice::swapchainBuffer::createFramebuffers() {

    frameBuffer = std::make_unique<VkFramebuffer[]>(imageCount);

    VkImageView attachmentViews[2];
    attachmentViews[1] = depth.view;

    VkFramebufferCreateInfo fbinfo{};
    fbinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbinfo.pNext = nullptr;
    fbinfo.attachmentCount = 2;
    fbinfo.renderPass = renderPass;
    fbinfo.pAttachments = attachmentViews;
    fbinfo.width = wh.width;
    fbinfo.height = wh.height;
    fbinfo.layers = 1;
    fbinfo.flags = 0;

    for (uint32_t i = 0; i < imageCount; i++) {
        attachmentViews[0] = views[i];
        auto res = vkCreateFramebuffer(DevicePointer->device, &fbinfo, nullptr, &frameBuffer[i]);
        vkUtil::checkError(res);
    }
}

void VulkanDevice::swapchainBuffer::create(VkSurfaceKHR surface, bool clearBackBuffer) {
    createswapchain(surface);
    createDepth();
    createFence();
    createRenderPass(clearBackBuffer);
    createFramebuffers();
    swapchainAlive = true;
}

void VulkanDevice::swapchainBuffer::acquireNextImageAndWait() {
    //vkAcquireNextImageKHR:命令はバックバッファのスワップを行い,次に描画されるべきImageのインデックスを返す
    auto res = vkAcquireNextImageKHR(DevicePointer->device, swapchain,
        UINT64_MAX, DevicePointer->presentCompletedSem, VK_NULL_HANDLE,
        &currentFrameIndex);
    vkUtil::checkError(res);
    if (!firstswFence) {
        firstswFence = true;
        return;
    }
    DevicePointer->waitForFence(swFence);
}

void VulkanDevice::swapchainBuffer::beginRenderPass(uint32_t comBufindex) {
    static VkClearValue clearValue[2];
    clearValue[0].color.float32[0] = 0.0f;
    clearValue[0].color.float32[1] = 0.0f;
    clearValue[0].color.float32[2] = 0.0f;
    clearValue[0].color.float32[3] = 1.0f;
    clearValue[1].depthStencil.depth = 1.0f;
    clearValue[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo rpinfo{};
    rpinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpinfo.framebuffer = getFramebuffer();
    rpinfo.renderPass = renderPass;
    rpinfo.renderArea.extent.width = wh.width;
    rpinfo.renderArea.extent.height = wh.height;
    rpinfo.clearValueCount = 2;
    rpinfo.pClearValues = clearValue;

    vkCmdBeginRenderPass(DevicePointer->commandBuffer[comBufindex], &rpinfo, VK_SUBPASS_CONTENTS_INLINE);
}

VulkanDevice::VulkanDevice(VkPhysicalDevice pd, uint32_t numCommandBuffer, bool V_SYNC) {
    pDev = pd;
    DevicePointer = this;
    commandBufferCount = numCommandBuffer;
    if (V_SYNC) {
        presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }
    else {
        presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
}

VulkanDevice::~VulkanDevice() {
    destroySwapchain();
    destroyTexture();
    vkFreeCommandBuffers(device, commandPool, commandBufferCount, commandBuffer.get());
    vkDestroyFence(device, sFence, nullptr);
    vkDestroySemaphore(device, presentCompletedSem, nullptr);
    vkDestroySemaphore(device, renderCompletedSem, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDeviceWaitIdle(device);
    vkDestroyDevice(device, nullptr);
}

void VulkanDevice::create(std::vector<const char*>* requiredExtensions, const void* pNext) {
    VkDeviceCreateInfo devInfo{};
    VkDeviceQueueCreateInfo queueInfo{};

    //グラフィックス用のデバイスキューのファミリー番号を取得:VK_QUEUE_GRAPHICS_BIT
    uint32_t propertyCount;
    //nullptr指定でプロパティ数取得
    vkGetPhysicalDeviceQueueFamilyProperties(pDev, &propertyCount, nullptr);
    auto properties = std::make_unique<VkQueueFamilyProperties[]>(propertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(pDev, &propertyCount, properties.get());
    for (uint32_t i = 0; i < propertyCount; i++) {
        if ((properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            queueFamilyIndex = i;
            break;
        }
    }
    if (queueFamilyIndex == 0xffffffff)
        throw std::runtime_error("No Graphics queues available on current device.");

    const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
    std::vector<const char*> extensions = { "VK_KHR_swapchain" };//スワップチェーンで必須

    if (requiredExtensions) {
        for (auto& e : (*requiredExtensions)) {
            extensions.push_back(e);
        }
    }

    static float qPriorities[] = { 0.0f };
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.pNext = nullptr;
    queueInfo.queueCount = 1;
    queueInfo.queueFamilyIndex = queueFamilyIndex;
    queueInfo.pQueuePriorities = qPriorities;

    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.pNext = pNext;
    devInfo.queueCreateInfoCount = 1;
    devInfo.pQueueCreateInfos = &queueInfo;
    devInfo.enabledLayerCount = (uint32_t)COUNTOF(layers);
    devInfo.ppEnabledLayerNames = layers;
    devInfo.enabledExtensionCount = (uint32_t)extensions.size();
    devInfo.ppEnabledExtensionNames = extensions.data();
    devInfo.pEnabledFeatures = nullptr;

    //論理デバイス生成,キューも生成される
    auto res = vkCreateDevice(pDev, &devInfo, nullptr, &device);
    vkUtil::checkError(res);
    //キュー取得
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &devQueue);
    //VRAMプロパティ取得:頂点バッファ取得時使用
    vkGetPhysicalDeviceMemoryProperties(pDev, &memProps);
}

void VulkanDevice::createCommandPool() {
    VkCommandPoolCreateInfo info{};

    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.queueFamilyIndex = queueFamilyIndex;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    //コマンドプールの作成:コマンドバッファーメモリが割り当てられるオブジェクト
    auto res = vkCreateCommandPool(device, &info, nullptr, &commandPool);
    vkUtil::checkError(res);
}

void VulkanDevice::createSFence() {
    VkFenceCreateInfo finfo{};
    finfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    auto res = vkCreateFence(device, &finfo, nullptr, &sFence);
    vkUtil::checkError(res);
}

void VulkanDevice::createSemaphore() {
    VkSemaphoreCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(device, &ci, nullptr, &renderCompletedSem);
    vkCreateSemaphore(device, &ci, nullptr, &presentCompletedSem);
}

void VulkanDevice::createCommandBuffers() {

    VkCommandBufferAllocateInfo cbAllocInfo{};

    cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbAllocInfo.commandPool = commandPool;
    cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbAllocInfo.commandBufferCount = commandBufferCount;
    //コマンドバッファの作成
    commandBuffer = std::make_unique<VkCommandBuffer[]>(commandBufferCount);
    auto res = vkAllocateCommandBuffers(device, &cbAllocInfo, commandBuffer.get());
    vkUtil::checkError(res);
}

void VulkanDevice::beginCommandWithFramebuffer(uint32_t comBufindex, VkFramebuffer fb) {
    VkCommandBufferInheritanceInfo inhInfo{};
    VkCommandBufferBeginInfo beginInfo{};

    inhInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inhInfo.framebuffer = fb;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pInheritanceInfo = &inhInfo;
    //コマンド記録開始
    vkBeginCommandBuffer(commandBuffer[comBufindex], &beginInfo);
}

void VulkanDevice::submitCommands(uint32_t comBufindex, VkFence fence, bool useRender) {
    VkSubmitInfo sinfo{};
    static const VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sinfo.commandBufferCount = 1;
    sinfo.pCommandBuffers = &commandBuffer[comBufindex];
    sinfo.pWaitDstStageMask = &waitStageMask;
    if (useRender) {
        sinfo.waitSemaphoreCount = 1;
        sinfo.pWaitSemaphores = &presentCompletedSem;
        sinfo.signalSemaphoreCount = 1;
        sinfo.pSignalSemaphores = &renderCompletedSem;
        resetFence(fence);
    }
    auto res = vkQueueSubmit(devQueue, 1, &sinfo, fence);
    vkUtil::checkError(res);
}

VkResult VulkanDevice::waitForFence(VkFence fence) {
    //コマンドの処理が指定数(ここでは1)終わるまで待つ
    //条件が満たされた場合待ち解除でVK_SUCCESSを返す
    //条件が満たされない,かつ
    //タイムアウト(ここではUINT64_MAX)に達した場合,待ち解除でVK_TIMEOUTを返す
    return vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
}

void VulkanDevice::present() {
    VkPresentInfoKHR pinfo{};

    pinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pinfo.swapchainCount = 1;
    pinfo.pSwapchains = swBuf.getSwapchain();
    pinfo.pImageIndices = swBuf.getCurrentFrameIndex();
    pinfo.waitSemaphoreCount = 1;
    pinfo.pWaitSemaphores = &renderCompletedSem;

    auto res = vkQueuePresentKHR(devQueue, &pinfo);
    vkUtil::checkError(res);
}

void VulkanDevice::resetFence(VkFence fence) {
    //指定数(ここでは1)のフェンスをリセット
    auto res = vkResetFences(device, 1, &fence);
    vkUtil::checkError(res);
}

void VulkanDevice::barrierResource(uint32_t comBufindex, VkImage image,
    VkImageLayout srcImageLayout, VkImageLayout dstImageLayout) {

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = srcImageLayout;
    barrier.newLayout = dstImageLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    switch (srcImageLayout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        barrier.srcAccessMask = 0;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        break;
    };

    switch (dstImageLayout) {
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        break;
    }

    vkCmdPipelineBarrier(commandBuffer[comBufindex], srcStage, dstStage,
        0, 0, nullptr,
        0, nullptr,
        1, &barrier);
}

void VulkanDevice::copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
            width,
            height,
            1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VulkanDevice::createImage(uint32_t width, uint32_t height, VkFormat format,
    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
    VkImage& image, VkDeviceMemory& imageMemory) {

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto res = vkCreateImage(device, &imageInfo, nullptr, &image);
    vkUtil::checkError(res);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    AllocateMemory(usage, memRequirements, properties, imageMemory, nullptr);

    res = vkBindImageMemory(device, image, imageMemory, 0);
    vkUtil::checkError(res);
}

auto VulkanDevice::createTextureImage(uint32_t comBufindex, Texture& inByte) {

    VkDeviceSize imageSize = inByte.width * inByte.height * 4;

    if (!inByte.byte) {
        throw std::runtime_error("failed to load texture image!");
    }

    BufferSet stagingBuffer;
    stagingBuffer.createUploadBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, nullptr);

    stagingBuffer.memoryMap(inByte.byte);

    VkTexture texture;
    texture.createImage(inByte.width, inByte.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    beginCommandWithFramebuffer(comBufindex, VkFramebuffer());
    texture.barrierResource(comBufindex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(commandBuffer[comBufindex], stagingBuffer.getBuffer(), texture.image.getImage(),
        static_cast<uint32_t>(inByte.width), static_cast<uint32_t>(inByte.height));
    texture.barrierResource(comBufindex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkEndCommandBuffer(commandBuffer[comBufindex]);
    submitCommandsDoNotRender(comBufindex);

    stagingBuffer.destroy();

    return texture;
}

VkImageView VulkanDevice::createImageView(VkImage image, VkFormat format,
    VkImageAspectFlags mask, VkComponentMapping components) {

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components.r = components.r;
    viewInfo.components.g = components.g;
    viewInfo.components.b = components.b;
    viewInfo.components.a = components.a;
    viewInfo.subresourceRange.aspectMask = mask;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VkResult res = vkCreateImageView(device, &viewInfo, nullptr, &imageView);
    vkUtil::checkError(res);
    return imageView;
}

void VulkanDevice::createTextureSampler(VkSampler& textureSampler) {

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    //samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;//VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXTの場合はこれにする
    //samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;//VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXTの場合はこれにする
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    //samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;//VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXTの場合はこれにする
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.flags = VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT;
    auto res = vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler);
    vkUtil::checkError(res);
}

void VulkanDevice::destroyTexture() {
    for (uint32_t i = 0; i < numTexture; i++) {
        texture[i].destroy();
    }
    texture[numTextureMax].destroy();
    texture[numTextureMax + 1].destroy();
}

char* VulkanDevice::getNameFromPass(char* pass) {

    uint32_t len = (uint32_t)strlen(pass);
    pass += len;//終端文字を指している

    for (uint32_t i = 0; i < len; i++) {
        pass--;
        if (*pass == '\\' || *pass == '/') {
            pass++;
            break;
        }
    }
    return pass;//ポインタ操作してるので返り値を使用させる
}

void VulkanDevice::AllocateMemory(VkBufferUsageFlags usage, VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties,
    VkDeviceMemory& bufferMemory, void* add_pNext) {

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    allocInfo.pNext = add_pNext;

    auto res = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
    vkUtil::checkError(res);
}

void VulkanDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
    VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext) {

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto res = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
    vkUtil::checkError(res);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    AllocateMemory(usage, memRequirements, properties, bufferMemory, allocateMemory_add_pNext);

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanDevice::createUploadBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
    VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext) {

    createBuffer(size, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        buffer, bufferMemory, allocateMemory_add_pNext);
}

void VulkanDevice::createDefaultBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
    VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext) {

    createBuffer(size, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        buffer, bufferMemory, allocateMemory_add_pNext);
}

void VulkanDevice::copyBuffer(uint32_t comBufindex, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

    beginCommandWithFramebuffer(comBufindex, VkFramebuffer());

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer[comBufindex], srcBuffer, dstBuffer, 1, &copyRegion);

    auto res = vkEndCommandBuffer(commandBuffer[comBufindex]);
    vkUtil::checkError(res);
    submitCommandsDoNotRender(comBufindex);
}

uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {

    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to findMemoryType");
}

void* VulkanDevice::Map(VkDeviceMemory mem) {
    void* mdata;
    auto res = vkMapMemory(device, mem, 0, VK_WHOLE_SIZE, 0, &mdata);
    vkUtil::checkError(res);
    return mdata;
}

void VulkanDevice::UnMap(VkDeviceMemory mem) {
    vkUnmapMemory(device, mem);
}

void VulkanDevice::memoryMap(void* pData, VkDeviceMemory mem, VkDeviceSize size) {
    memcpy(Map(mem), pData, (size_t)size);
    UnMap(mem);
}

void VulkanDevice::descriptorAndPipelineLayouts(bool useTexture, VkPipelineLayout& pipelineLayout, VkDescriptorSetLayout& descSetLayout) {
    VkDescriptorSetLayoutBinding layout_bindings[5];
    uint32_t bCnt = 0;
    VkDescriptorSetLayoutBinding& bufferMat = layout_bindings[bCnt];
    bufferMat.binding = bCnt++;
    bufferMat.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMat.descriptorCount = 1;
    bufferMat.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bufferMat.pImmutableSamplers = nullptr;

    if (useTexture) {
        VkDescriptorSetLayoutBinding& texSampler = layout_bindings[bCnt];
        texSampler.binding = bCnt++;
        texSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texSampler.descriptorCount = 1;
        texSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        texSampler.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding& norSampler = layout_bindings[bCnt];
        norSampler.binding = bCnt++;
        norSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        norSampler.descriptorCount = 1;
        norSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        norSampler.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding& speSampler = layout_bindings[bCnt];
        speSampler.binding = bCnt++;
        speSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        speSampler.descriptorCount = 1;
        speSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        speSampler.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding& bufferMaterial = layout_bindings[bCnt];
        bufferMaterial.binding = bCnt++;
        bufferMaterial.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bufferMaterial.descriptorCount = 1;
        bufferMaterial.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bufferMaterial.pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext = NULL;
    descriptor_layout.flags = 0;
    descriptor_layout.bindingCount = bCnt;
    descriptor_layout.pBindings = layout_bindings;

    VkResult res;

    res = vkCreateDescriptorSetLayout(device, &descriptor_layout, nullptr, &descSetLayout);
    vkUtil::checkError(res);

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = nullptr;
    pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &descSetLayout;

    res = vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    vkUtil::checkError(res);
}

void  VulkanDevice::descriptorAndPipelineLayouts2D(bool useTexture, VkPipelineLayout& pipelineLayout, VkDescriptorSetLayout& descSetLayout) {
    VkDescriptorSetLayoutBinding layout_bindings[2];
    uint32_t bCnt = 0;
    VkDescriptorSetLayoutBinding& bufferMat = layout_bindings[bCnt];
    bufferMat.binding = bCnt++;
    bufferMat.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMat.descriptorCount = 1;
    bufferMat.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bufferMat.pImmutableSamplers = nullptr;

    if (useTexture) {
        VkDescriptorSetLayoutBinding& texSampler = layout_bindings[bCnt];
        texSampler.binding = bCnt++;
        texSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texSampler.descriptorCount = 1;
        texSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        texSampler.pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext = NULL;
    descriptor_layout.flags = 0;
    descriptor_layout.bindingCount = bCnt;
    descriptor_layout.pBindings = layout_bindings;

    VkResult res;

    res = vkCreateDescriptorSetLayout(device, &descriptor_layout, nullptr, &descSetLayout);
    vkUtil::checkError(res);

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = nullptr;
    pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &descSetLayout;

    res = vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    vkUtil::checkError(res);
}

static std::vector<uint32_t> CompileShader(const char* fname, char* source, VkShaderStageFlags stage) {

    shaderc_env_version env_version = shaderc_env_version_vulkan_1_0;
    switch (ApiVersion) {
    case VK_API_VERSION_1_0:
        env_version = shaderc_env_version_vulkan_1_0;
        break;
    case VK_API_VERSION_1_1:
        env_version = shaderc_env_version_vulkan_1_1;
        break;
    case VK_API_VERSION_1_2:
        env_version = shaderc_env_version_vulkan_1_2;
        break;
    case VK_API_VERSION_1_3:
        env_version = shaderc_env_version_vulkan_1_3;
        break;
    }

    shaderc::Compiler compiler;
    auto sourceText = std::string(source, strlen(source));
    // コンパイラに設定するオプションの指定.
    shaderc::CompileOptions options{};
    options.SetTargetEnvironment(
        shaderc_target_env_vulkan, env_version);
#if _DEBUG
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
    options.SetGenerateDebugInfo();
#else
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif
    // シェーダーの種類.
    shaderc_shader_kind kind;
    switch (stage) {
    case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        kind = shaderc_raygen_shader;
        break;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        kind = shaderc_closesthit_shader;
        break;
    case VK_SHADER_STAGE_MISS_BIT_KHR:
        kind = shaderc_miss_shader;
        break;
    case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        kind = shaderc_anyhit_shader;
        break;
    case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        kind = shaderc_intersection_shader;
        break;
    case VK_SHADER_STAGE_VERTEX_BIT:
        kind = shaderc_vertex_shader;
        break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
        kind = shaderc_fragment_shader;
        break;
    case VK_SHADER_STAGE_COMPUTE_BIT:
        kind = shaderc_compute_shader;
        break;
    }
    // コンパイルの実行.
    auto result = compiler.CompileGlslToSpv(
        sourceText, kind, fname, options);
    auto status = result.GetCompilationStatus();
    std::vector<uint32_t> compiled;
    if (status == shaderc_compilation_status_success) {
        // コンパイルに成功. SPIR-Vコードを取得.
        compiled.assign(result.cbegin(), result.cend());
    }
    else {
        auto errMessage = result.GetErrorMessage();
        OutputDebugStringA(errMessage.c_str());
    }
    return compiled;
}

VkPipelineShaderStageCreateInfo VulkanDevice::createShaderModule(const char* fname, char* shader, VkShaderStageFlagBits stage) {

    std::vector<uint32_t> spv = CompileShader(fname, shader, stage);

    VkShaderModuleCreateInfo shaderInfo{};
    VkShaderModule mod;
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = spv.size() * sizeof(uint32_t);
    shaderInfo.pCode = spv.data();

    auto res = vkCreateShaderModule(device, &shaderInfo, nullptr, &mod);
    vkUtil::checkError(res);

    VkPipelineShaderStageCreateInfo stageInfo = {};

    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = stage;
    stageInfo.module = mod;
    stageInfo.pName = "main";

    return stageInfo;
}

void VulkanDevice::createDescriptorPool(bool useTexture, VkDescriptorPool& descPool) {
    VkResult res;
    VkDescriptorPoolSize type_count[5];
    uint32_t bCnt = 0;
    VkDescriptorPoolSize& bufferMat = type_count[bCnt++];
    bufferMat.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMat.descriptorCount = 1;
    if (useTexture) {
        VkDescriptorPoolSize& texSampler = type_count[bCnt++];
        texSampler.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texSampler.descriptorCount = 1;

        VkDescriptorPoolSize& norSampler = type_count[bCnt++];
        norSampler.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        norSampler.descriptorCount = 1;

        VkDescriptorPoolSize& speSampler = type_count[bCnt++];
        speSampler.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        speSampler.descriptorCount = 1;

        VkDescriptorPoolSize& bufferMaterial = type_count[bCnt++];
        bufferMaterial.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bufferMaterial.descriptorCount = 1;
    }

    VkDescriptorPoolCreateInfo descriptor_pool = {};
    descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool.pNext = nullptr;
    descriptor_pool.maxSets = 1;
    descriptor_pool.poolSizeCount = bCnt;
    descriptor_pool.pPoolSizes = type_count;

    res = vkCreateDescriptorPool(device, &descriptor_pool, nullptr, &descPool);
    vkUtil::checkError(res);
}

void VulkanDevice::createDescriptorPool2D(bool useTexture, VkDescriptorPool& descPool) {
    VkResult res;
    VkDescriptorPoolSize type_count[2];
    uint32_t bCnt = 0;
    VkDescriptorPoolSize& bufferMat = type_count[bCnt++];
    bufferMat.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMat.descriptorCount = 1;
    if (useTexture) {
        VkDescriptorPoolSize& texSampler = type_count[bCnt++];
        texSampler.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texSampler.descriptorCount = 1;
    }

    VkDescriptorPoolCreateInfo descriptor_pool = {};
    descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool.pNext = nullptr;
    descriptor_pool.maxSets = 1;
    descriptor_pool.poolSizeCount = bCnt;
    descriptor_pool.pPoolSizes = type_count;

    res = vkCreateDescriptorPool(device, &descriptor_pool, nullptr, &descPool);
    vkUtil::checkError(res);
}

uint32_t VulkanDevice::upDescriptorSet(bool useTexture, VkTexture& difTexture, VkTexture& norTexture, VkTexture& speTexture, Uniform<MatrixSet>& uni,
    Uniform<Material>& material, VkDescriptorSet& descriptorSet, VkDescriptorPool& descPool, VkDescriptorSetLayout& descSetLayout) {

    VkResult res;

    VkDescriptorSetAllocateInfo alloc_info[1];
    alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info[0].pNext = nullptr;
    alloc_info[0].descriptorPool = descPool;
    alloc_info[0].descriptorSetCount = 1;
    alloc_info[0].pSetLayouts = &descSetLayout;

    res = vkAllocateDescriptorSets(device, alloc_info, &descriptorSet);
    vkUtil::checkError(res);

    VkWriteDescriptorSet writes[5];
    uint32_t bCnt = 0;
    VkWriteDescriptorSet& bufferMat = writes[bCnt];
    bufferMat = {};
    bufferMat.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    bufferMat.pNext = nullptr;
    bufferMat.dstSet = descriptorSet;
    bufferMat.descriptorCount = 1;
    bufferMat.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMat.pBufferInfo = &uni.buf.info;
    bufferMat.dstArrayElement = 0;
    bufferMat.dstBinding = bCnt++;

    if (useTexture) {
        VkWriteDescriptorSet& texSampler = writes[bCnt];
        texSampler = {};
        texSampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        texSampler.dstSet = descriptorSet;
        texSampler.dstBinding = bCnt++;
        texSampler.descriptorCount = 1;
        texSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texSampler.pImageInfo = &difTexture.image.info;
        texSampler.dstArrayElement = 0;

        VkWriteDescriptorSet& norSampler = writes[bCnt];
        norSampler = {};
        norSampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        norSampler.dstSet = descriptorSet;
        norSampler.dstBinding = bCnt++;
        norSampler.descriptorCount = 1;
        norSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        norSampler.pImageInfo = &norTexture.image.info;
        norSampler.dstArrayElement = 0;

        VkWriteDescriptorSet& speSampler = writes[bCnt];
        speSampler = {};
        speSampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        speSampler.dstSet = descriptorSet;
        speSampler.dstBinding = bCnt++;
        speSampler.descriptorCount = 1;
        speSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        speSampler.pImageInfo = &speTexture.image.info;
        speSampler.dstArrayElement = 0;

        VkWriteDescriptorSet& bufferMaterial = writes[bCnt];
        bufferMaterial = {};
        bufferMaterial.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        bufferMaterial.pNext = nullptr;
        bufferMaterial.dstSet = descriptorSet;
        bufferMaterial.descriptorCount = 1;
        bufferMaterial.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bufferMaterial.pBufferInfo = &material.buf.info;
        bufferMaterial.dstArrayElement = 0;
        bufferMaterial.dstBinding = bCnt++;
    }

    vkUpdateDescriptorSets(device, bCnt, writes, 0, nullptr);
    return bCnt;
}

uint32_t VulkanDevice::upDescriptorSet2D(bool useTexture, VkTexture& texture, Uniform<MatrixSet2D>& uni,
    VkDescriptorSet& descriptorSet, VkDescriptorPool& descPool,
    VkDescriptorSetLayout& descSetLayout) {

    VkResult res;

    VkDescriptorSetAllocateInfo alloc_info[1];
    alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info[0].pNext = nullptr;
    alloc_info[0].descriptorPool = descPool;
    alloc_info[0].descriptorSetCount = 1;
    alloc_info[0].pSetLayouts = &descSetLayout;

    res = vkAllocateDescriptorSets(device, alloc_info, &descriptorSet);
    vkUtil::checkError(res);

    VkWriteDescriptorSet writes[2];
    uint32_t bCnt = 0;
    VkWriteDescriptorSet& bufferMat = writes[bCnt];
    bufferMat = {};
    bufferMat.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    bufferMat.pNext = nullptr;
    bufferMat.dstSet = descriptorSet;
    bufferMat.descriptorCount = 1;
    bufferMat.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMat.pBufferInfo = &uni.buf.info;
    bufferMat.dstArrayElement = 0;
    bufferMat.dstBinding = bCnt++;

    if (useTexture) {
        VkWriteDescriptorSet& texSampler = writes[bCnt];
        texSampler = {};
        texSampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        texSampler.dstSet = descriptorSet;
        texSampler.dstBinding = bCnt++;
        texSampler.descriptorCount = 1;
        texSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texSampler.pImageInfo = &texture.image.info;
        texSampler.dstArrayElement = 0;
    }

    vkUpdateDescriptorSets(device, bCnt, writes, 0, nullptr);
    return bCnt;
}

VkPipelineCache VulkanDevice::createPipelineCache() {
    VkPipelineCacheCreateInfo cacheInfo{};
    VkPipelineCache pipelineCache;
    cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    auto res = vkCreatePipelineCache(device, &cacheInfo, nullptr, &pipelineCache);
    vkUtil::checkError(res);
    return pipelineCache;
}

VkPipeline VulkanDevice::createGraphicsPipelineVF(bool useAlpha,
    const VkPipelineShaderStageCreateInfo& vshader, const VkPipelineShaderStageCreateInfo& fshader,
    const VkVertexInputBindingDescription& bindDesc, const VkVertexInputAttributeDescription* attrDescs, uint32_t numAttr,
    const VkPipelineLayout& pLayout, const VkRenderPass renderPass, const VkPipelineCache& pCache) {

    static VkViewport vports[] = { {0.0f, 0.0f, (float)swBuf.getSize().width, (float)swBuf.getSize().height, 0.0f, 1.0f} };
    static VkRect2D scissors[] = { {{0, 0}, {swBuf.getSize().width,swBuf.getSize().height}} };
    static VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineShaderStageCreateInfo stageInfo[2]{};
    stageInfo[0] = vshader;
    stageInfo[1] = fshader;

    VkPipelineVertexInputStateCreateInfo vinStateInfo{};
    vinStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vinStateInfo.vertexBindingDescriptionCount = 1;
    vinStateInfo.pVertexBindingDescriptions = &bindDesc;
    vinStateInfo.vertexAttributeDescriptionCount = numAttr;
    vinStateInfo.pVertexAttributeDescriptions = attrDescs;

    VkPipelineInputAssemblyStateCreateInfo iaInfo{};
    iaInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    iaInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo vpInfo{};
    vpInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vpInfo.viewportCount = 1;
    vpInfo.pViewports = vports;
    vpInfo.scissorCount = 1;
    vpInfo.pScissors = scissors;

    VkPipelineRasterizationStateCreateInfo rasterizerStateInfo{};
    rasterizerStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerStateInfo.depthClampEnable = VK_FALSE;
    rasterizerStateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerStateInfo.cullMode = VK_CULL_MODE_NONE;
    rasterizerStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerStateInfo.depthBiasEnable = VK_FALSE;
    rasterizerStateInfo.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo msInfo{};
    msInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    msInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    msInfo.sampleShadingEnable = VK_FALSE;
    msInfo.alphaToCoverageEnable = VK_FALSE;
    msInfo.alphaToOneEnable = VK_FALSE;
    if (useAlpha) {
        msInfo.alphaToCoverageEnable = VK_TRUE;
    }

    VkPipelineColorBlendAttachmentState blendState{};
    blendState.blendEnable = VK_FALSE;
    blendState.colorWriteMask = VK_COLOR_COMPONENT_A_BIT
        | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_R_BIT;

    VkPipelineColorBlendStateCreateInfo blendInfo{};
    blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendInfo.logicOpEnable = VK_FALSE;
    blendInfo.attachmentCount = 1;
    blendInfo.pAttachments = &blendState;

    VkPipelineDynamicStateCreateInfo dynamicInfo{};
    dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicInfo.dynamicStateCount = (uint32_t)COUNTOF(dynamicStates);
    dynamicInfo.pDynamicStates = dynamicStates;

    VkPipelineDepthStencilStateCreateInfo ds;
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.pNext = NULL;
    ds.flags = 0;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.stencilTestEnable = VK_FALSE;
    ds.back.failOp = VK_STENCIL_OP_KEEP;
    ds.back.passOp = VK_STENCIL_OP_KEEP;
    ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
    ds.back.compareMask = 0;
    ds.back.reference = 0;
    ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
    ds.back.writeMask = 0;
    ds.minDepthBounds = 0;
    ds.maxDepthBounds = 0;
    ds.stencilTestEnable = VK_FALSE;
    ds.front = ds.back;

    VkGraphicsPipelineCreateInfo gpInfo{};
    gpInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gpInfo.stageCount = (uint32_t)COUNTOF(stageInfo);
    gpInfo.pStages = stageInfo;
    gpInfo.pVertexInputState = &vinStateInfo;
    gpInfo.pInputAssemblyState = &iaInfo;
    gpInfo.pViewportState = &vpInfo;
    gpInfo.pRasterizationState = &rasterizerStateInfo;
    gpInfo.pMultisampleState = &msInfo;
    gpInfo.pColorBlendState = &blendInfo;
    gpInfo.pDynamicState = &dynamicInfo;
    gpInfo.pDepthStencilState = &ds;
    gpInfo.layout = pLayout;
    gpInfo.renderPass = renderPass;
    gpInfo.subpass = 0;

    VkPipeline pipeline;
    auto res = vkCreateGraphicsPipelines(device, pCache, 1, &gpInfo, nullptr, &pipeline);
    vkUtil::checkError(res);
    return pipeline;
}

void VulkanDevice::createDevice(std::vector<const char*>* requiredExtensions, const void* pNext) {
    create(requiredExtensions, pNext);
    createCommandPool();
    createSFence();
    createSemaphore();
    createCommandBuffers();

    //ダミーテクスチャ生成(テクスチャーが無い場合に代わりに入れる)
    unsigned char* dummyNor = new unsigned char[64 * 4 * 64];
    unsigned char* dummyDifSpe = new unsigned char[64 * 4 * 64];
    unsigned char nor[4] = { 128,128,255,0 };
    unsigned char difspe[4] = { 255,255,255,255 };
    for (int i = 0; i < 64 * 4 * 64; i += 4) {
        memcpy(&dummyNor[i], nor, sizeof(unsigned char) * 4);
        memcpy(&dummyDifSpe[i], difspe, sizeof(unsigned char) * 4);
    }

    texture[numTextureMax].width = 64;
    texture[numTextureMax].height = 64;
    texture[numTextureMax].setByte(dummyNor);
    texture[numTextureMax + 1].width = 64;
    texture[numTextureMax + 1].height = 64;
    texture[numTextureMax + 1].setByte(dummyDifSpe);
    vkUtil::ARR_DELETE(dummyNor);
    vkUtil::ARR_DELETE(dummyDifSpe);
}

void VulkanDevice::createSwapchain(VkSurfaceKHR surface, bool clearBackBuffer) {
    swBuf.create(surface, clearBackBuffer);
}

void VulkanDevice::destroySwapchain() {
    swBuf.destroySwapchain();
}

void VulkanDevice::createVkTexture(VkTexture& tex, uint32_t comBufindex, Texture& inByte) {
    tex = createTextureImage(comBufindex, inByte);
    tex.createImageView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    createTextureSampler(tex.image.info.sampler);
}

void VulkanDevice::createTextureSet(uint32_t comIndex, textureIdSet& texSet) {
    VulkanDevice* d = VulkanDevice::GetInstance();
    if (texSet.diffuseId < 0) {
        d->createVkTexture(texSet.difTex, comIndex,
            d->texture[d->numTextureMax + 1]);
    }
    else {
        d->createVkTexture(texSet.difTex, comIndex,
            d->texture[texSet.diffuseId]);
    }

    if (texSet.normalId < 0) {
        d->createVkTexture(texSet.norTex, comIndex,
            d->texture[d->numTextureMax]);
    }
    else {
        d->createVkTexture(texSet.norTex, comIndex,
            d->texture[texSet.normalId]);
    }

    if (texSet.specularId < 0) {
        d->createVkTexture(texSet.speTex, comIndex,
            d->texture[d->numTextureMax + 1]);
    }
    else {
        d->createVkTexture(texSet.speTex, comIndex,
            d->texture[texSet.specularId]);
    }
}

void VulkanDevice::GetTexture(uint32_t comBufindex, char* fileName, unsigned char* byteArr, uint32_t width, uint32_t height) {
    //ファイル名登録
    char* filename = getNameFromPass(fileName);
    if (strlen(filename) >= (size_t)numTexFileNamelenMax)
        throw std::runtime_error("The file name limit has been.");
    strcpy(textureNameList[numTexture], filename);

    //テクスチャ登録
    texture[numTexture].width = width;
    texture[numTexture].height = height;
    texture[numTexture].setByte(byteArr);
    numTexture++;
    if (numTexture >= numTextureMax)
        throw std::runtime_error("The file limit has been.");
}

int32_t VulkanDevice::getTextureNo(char* pass) {
    for (uint32_t i = 0; i < numTexture; i++) {
        size_t len1 = strlen(textureNameList[i]);
        size_t len2 = strlen(pass);

        if (len1 == len2 && !strcmp(textureNameList[i], pass))return i;
    }
    return -1;
}

void VulkanDevice::updateProjection(float AngleView, float Near, float Far) {
    MatrixPerspectiveFovLH(&proj, AngleView, (float)swBuf.getSize().width / (float)swBuf.getSize().height, Near, Far);
}

void VulkanDevice::updateView(CoordTf::VECTOR3 vi, CoordTf::VECTOR3 gaze, CoordTf::VECTOR3 up) {
    MatrixLookAtLH(&view, vi, gaze, up);
    viewPos.as(vi.x, vi.y, vi.z, 0.0f);
}

void VulkanDevice::setNumLight(uint32_t num) {
    numLight = num;
}

void VulkanDevice::setLightAttenuation(float att1, float att2, float att3) {
    attenuation1 = att1;
    attenuation2 = att2;
    attenuation3 = att3;
}

void VulkanDevice::setLight(uint32_t index, CoordTf::VECTOR3 pos, CoordTf::VECTOR3 color) {
    lightPos[index].as(pos.x, pos.y, pos.z, 0.0f);
    lightColor[index].as(color.x, color.y, color.z, 1.0f);
}

void VulkanDevice::beginCommandNextImage(uint32_t comBufindex) {
    swBuf.acquireNextImageAndWait();
    beginCommandWithFramebuffer(comBufindex, swBuf.getFramebuffer());
}

void VulkanDevice::beginDraw(uint32_t comBufindex) {
    swBuf.beginRenderPass(comBufindex);
}

void VulkanDevice::endDraw(uint32_t comBufindex) {
    vkCmdEndRenderPass(commandBuffer[comBufindex]);
}

void VulkanDevice::endCommand(uint32_t comBufindex) {
    vkEndCommandBuffer(commandBuffer[comBufindex]);
}

void VulkanDevice::Present(uint32_t comBufindex) {
    submitCommands(comBufindex, swBuf.getFence(), true);
    present();
}

void VulkanDevice::beginCommand(uint32_t comBufindex) {
    beginCommandWithFramebuffer(comBufindex, VkFramebuffer());
}

void VulkanDevice::submitCommandsDoNotRender(uint32_t comBufindex) {
    submitCommands(comBufindex, sFence, false);
    waitForFence(sFence);
    resetFence(sFence);
}

void VulkanDevice::DeviceWaitIdle() {
    vkDeviceWaitIdle(device);
}

void VulkanDevice::InstanceCreate(VkPhysicalDevice pd, uint32_t numCommandBuffer, bool V_SYNC) {
    if (DevicePointer == nullptr)DevicePointer = new VulkanDevice(pd, numCommandBuffer, V_SYNC);
}

VulkanDevice* VulkanDevice::GetInstance() {
    return DevicePointer;
}

void VulkanDevice::DeleteInstance() {
    if (DevicePointer != nullptr) {
        delete DevicePointer;
        DevicePointer = nullptr;
    }
}