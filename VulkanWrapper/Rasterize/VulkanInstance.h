﻿//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanInstance.h                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanInstance_Header
#define VulkanInstance_Header

#ifdef __ANDROID__
#include <android/log.h>
#include <android/native_window.h>
#else
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#endif
#include "VulkanPFN.h"
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include <tuple>
#include <functional>
#include <shaderc/shaderc.hpp>
#include "../../CoordTf/CoordTf.h"
#pragma comment(lib, "vulkan-1")
#pragma comment(lib, "shaderc_shared.lib")

namespace vkUtil {
    template<typename TYPE>
    void S_DELETE(TYPE p) { if (p) { delete p;    p = nullptr; } }
    template<typename TYPE>
    void ARR_DELETE(TYPE p) { if (p) { delete[] p;    p = nullptr; } }
    void checkError(VkResult res);

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT, uint64_t object,
        size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);

    void calculationMatrixWorld(CoordTf::MATRIX& World, CoordTf::VECTOR3 pos, CoordTf::VECTOR3 theta, CoordTf::VECTOR3 scale);

    class addChar {
    public:
        char* str = nullptr;
        size_t size = 0;

        void addStr(char* str1, char* str2);

        ~addChar() {
            S_DELETE(str);
        }
    };

    void createTangent(int numMaterial, unsigned int* indexCntArr,
        void* vertexArr, unsigned int** indexArr, int structByteStride,
        int norBytePos, int tangentBytePos, CoordTf::VECTOR3 upVec);
}

class Vulkan2D;
class VulkanBasicPolygon;
class VulkanSkinMesh;

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

    VkPhysicalDevice getPhysicalDevice(int index = 0);

    VkSurfaceKHR getSurface();

    VkInstance getInstance();
};

class VulkanDevice final {

public:
    const static uint32_t numLightMax = 256;
    const static uint32_t numBoneMax = 256;
    const static uint32_t numTextureMax = 254;
    const static uint32_t numTexFileNamelenMax = 256;

    class BufferSet {
    protected:
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory mem = VK_NULL_HANDLE;
        VkDeviceSize Size = 0;

    public:
        VkDescriptorBufferInfo info = {};
        VkBuffer getBuffer()const { return buffer; }
        VkBuffer* getBufferAddress() { return &buffer; }
        VkDeviceMemory getMemory()const { return mem; }
        VkDeviceSize getSize()const { return Size; }

        void createUploadBuffer(VkDeviceSize size, VkBufferUsageFlags usage, void* allocateMemory_add_pNext);

        void createDefaultBuffer(VkDeviceSize size, VkBufferUsageFlags usage, void* allocateMemory_add_pNext);

        void memoryMap(void* pData);

        void* Map();

        void UnMap();

        void destroy();
    };

    class ImageSet {
    protected:
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory mem = VK_NULL_HANDLE;
        VkFormat format = {};

    public:
        VkDescriptorImageInfo info = {};
        VkImage getImage()const { return image; }
        VkImage* getImageAddress() { return &image; }
        VkDeviceMemory getMemory()const { return mem; }
        VkFormat getFormat()const { return format; }

        void barrierResource(uint32_t comBufindex, VkImageLayout dstImageLayout);

        void createImage(uint32_t width, uint32_t height, VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

        void createImageView(VkFormat format, VkImageAspectFlags mask,
            VkComponentMapping components = { VK_COMPONENT_SWIZZLE_IDENTITY });

        void destroy();
    };

    struct VkTexture {
        ImageSet image;
        uint32_t width = 0;
        uint32_t height = 0;

        void barrierResource(uint32_t comBufindex, VkImageLayout dstImageLayout) {
            image.barrierResource(comBufindex, dstImageLayout);
        }
        void createImage(uint32_t Width, uint32_t Height, VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {
            image.createImage(Width, Height, format, tiling, usage, properties);
            width = Width;
            height = Height;
        }
        void createImageView(VkFormat format, VkImageAspectFlags mask,
            VkComponentMapping components = { VK_COMPONENT_SWIZZLE_IDENTITY }) {
            image.createImageView(format, mask, components);
        }
        void destroy() {
            image.destroy();
        }
    };

    struct textureIdSet {
        int diffuseId = -1;
        char difUvName[256] = {};
        VulkanDevice::VkTexture difTex;
        int normalId = -1;
        char norUvName[256] = {};
        VulkanDevice::VkTexture norTex;
        int specularId = -1;
        char speUvName[256] = {};
        VulkanDevice::VkTexture speTex;
        void destroy() {
            difTex.destroy();
            norTex.destroy();
            speTex.destroy();
        }
    };

    struct Depth {
        VkFormat format;
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    };

    class swapchainBuffer {
    private:
        Depth depth;
        VkExtent2D wh = {};
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        std::unique_ptr<VkSurfaceFormatKHR[]> BackBufferFormat;
        uint32_t imageCount = 0;
        uint32_t currentFrameIndex = 0;
        std::unique_ptr<VkImage[]> images = nullptr;//スワップチェーンの画像表示に使う
        std::unique_ptr<VkImageView[]> views = nullptr;
        std::unique_ptr<VkFramebuffer[]> frameBuffer = nullptr;
        VkFence swFence = nullptr;
        bool firstswFence = false;
        VkFormat format = {};
        bool swapchainAlive = false;
        VkRenderPass renderPass = VK_NULL_HANDLE;

        void createswapchain(VkSurfaceKHR surface);
        void createDepth();
        void createFence();
        void createRenderPass(bool clearBackBuffer);
        void createFramebuffers();

    public:
        void destroySwapchain();
        void create(VkSurfaceKHR surface, bool clearBackBuffer);
        void acquireNextImageAndWait();
        void beginRenderPass(uint32_t comBufindex);
        const VkSurfaceFormatKHR getBackBufferFormat(uint32_t index) { return BackBufferFormat[index]; }
        const VkImage getCurrentImage() { return images[currentFrameIndex]; }
        const VkImage getImage(uint32_t index) { return images[index]; }
        const VkFence getFence() { return swFence; }
        const uint32_t getImageCount() { return imageCount; }
        const VkFramebuffer getFramebuffer() { return frameBuffer[currentFrameIndex]; }
        const VkSwapchainKHR* getSwapchain() { return &swapchain; }
        const VkFormat getFormat() { return format; }
        const uint32_t* getCurrentFrameIndex() { return &currentFrameIndex; }
        const VkFormat getDepthFormat() { return depth.format; }
        const VkExtent2D getSize() { return wh; }
        const VkRenderPass getRenderPass() { return renderPass; }
    };

    struct Vertex3D {
        float pos[3];
        float normal[3];
        float difUv[2];
        float speUv[2];
    };

private:
    friend Vulkan2D;
    friend VulkanBasicPolygon;
    friend VulkanSkinMesh;

    static VulkanDevice* DevicePointer;

    VkPhysicalDevice pDev = VK_NULL_HANDLE;//VulkanInstanceからポインタを受け取る
    VkDevice device = VK_NULL_HANDLE;
    VkQueue devQueue = VK_NULL_HANDLE;
    uint32_t queueFamilyIndex = 0xffffffff;
    VkPhysicalDeviceMemoryProperties memProps = {};
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkFence sFence = VK_NULL_HANDLE;
    VkSemaphore renderCompletedSem, presentCompletedSem;

    swapchainBuffer swBuf;
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    uint32_t commandBufferCount = 1;
    std::unique_ptr<VkCommandBuffer[]> commandBuffer = nullptr;

    CoordTf::MATRIX proj, view;
    CoordTf::VECTOR4 viewPos;
    const CoordTf::VECTOR3 upVec = { 0.0f,1.0f,0.0f };
    CoordTf::VECTOR4 lightPos[numLightMax];
    CoordTf::VECTOR4 lightColor[numLightMax];
    uint32_t numLight = 1;
    float attenuation1 = 1.0f;
    float attenuation2 = 0.001f;
    float attenuation3 = 0.001f;

    struct MatrixSet {
        CoordTf::MATRIX world;
        CoordTf::MATRIX mvp;
        CoordTf::MATRIX bone[numBoneMax];
    };

    struct Material {
        CoordTf::VECTOR4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
        CoordTf::VECTOR4 specular = { 0.0f, 0.0f, 0.0f, 0.0f };
        CoordTf::VECTOR4 ambient = { 0.0f, 0.0f, 0.0f, 0.0f };
        CoordTf::VECTOR4 viewPos;
        CoordTf::VECTOR4 lightPos[numLightMax];
        CoordTf::VECTOR4 lightColor[numLightMax];
        CoordTf::VECTOR4 numLight;//ライト数,減衰1,減衰2,減衰3
        CoordTf::VECTOR4 UvSwitch = {};//.x==0:そのまま, 1:切り替え
    };

    struct MatrixSet2D {
        CoordTf::VECTOR2 world;
    };

    struct Texture {
        unsigned char* byte = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        void setByte(unsigned char* inbyte) {
            byte = new unsigned char[width * 4 * height];
            memcpy(byte, inbyte, sizeof(unsigned char) * width * 4 * height);
        }
        void destroy() {
            delete[] byte;
            byte = nullptr;
            width = 0;
            height = 0;
        }
    };
    Texture texture[numTextureMax + 2];
    char textureNameList[numTextureMax][numTexFileNamelenMax];
    uint32_t numTexture = 0;

    VulkanDevice() {}
    VulkanDevice(const VulkanDevice& obj) {}   // コピーコンストラクタ禁止
    void operator=(const VulkanDevice& obj) {}// 代入演算子禁止

    VulkanDevice(VkPhysicalDevice pd, uint32_t numCommandBuffer, bool V_SYNC);

    ~VulkanDevice();

    void create(std::vector<const char*>* requiredExtensions, const void* pNext);

    void createCommandPool();

    void createSFence();

    void createSemaphore();

    void createCommandBuffers();

    void beginCommandWithFramebuffer(uint32_t comBufindex, VkFramebuffer fb);

    void submitCommands(uint32_t comBufindex, VkFence fence, bool useRender);

    VkResult waitForFence(VkFence fence);

    void present();

    void resetFence(VkFence fence);

    void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width,
        uint32_t height);

    void AllocateMemory(VkBufferUsageFlags usage, VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties,
        VkDeviceMemory& bufferMemory, void* add_pNext);

    auto createTextureImage(uint32_t comBufindex, Texture& inByte);

    void createTextureSampler(VkSampler& textureSampler);

    void destroyTexture();

    char* getNameFromPass(char* pass);

    void copyBuffer(uint32_t comBufindex, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void descriptorAndPipelineLayouts(bool useTexture, VkPipelineLayout& pipelineLayout,
        VkDescriptorSetLayout& descSetLayout);

    void descriptorAndPipelineLayouts2D(bool useTexture, VkPipelineLayout& pipelineLayout,
        VkDescriptorSetLayout& descSetLayout);

    void createDescriptorPool(bool useTexture, VkDescriptorPool& descPool);

    void createDescriptorPool2D(bool useTexture, VkDescriptorPool& descPool);

    VkPipelineCache createPipelineCache();

    VkPipeline createGraphicsPipelineVF(bool useAlpha,
        const VkPipelineShaderStageCreateInfo& vshader,
        const VkPipelineShaderStageCreateInfo& fshader,
        const VkVertexInputBindingDescription& bindDesc,
        const VkVertexInputAttributeDescription* attrDescs,
        uint32_t numAttr,
        const VkPipelineLayout& pLayout,
        const VkRenderPass renderPass,
        const VkPipelineCache& pCache);

public:
    static void InstanceCreate(VkPhysicalDevice pd, uint32_t numCommandBuffer = 1,
        bool V_SYNC = true);

    static VulkanDevice* GetInstance();
    static void DeleteInstance();

    VkPipelineShaderStageCreateInfo createShaderModule(const char* fname, char* shader, VkShaderStageFlagBits stage);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext);

    void createImage(uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags mask,
        VkComponentMapping components = { VK_COMPONENT_SWIZZLE_IDENTITY });

    void createVkTexture(VkTexture& tex, uint32_t comBufindex, Texture& inByte);

    void createTextureSet(uint32_t comBufindex, textureIdSet& texSet);

    void memoryMap(void* pData, VkDeviceMemory mem, VkDeviceSize size);

    void* Map(VkDeviceMemory mem);

    void UnMap(VkDeviceMemory mem);

    template<typename UNI>
    struct Uniform {
        BufferSet buf = {};
        UNI uni;
    };

    uint32_t upDescriptorSet(bool useTexture, VkTexture& difTexture, VkTexture& norTexture, VkTexture& speTexture,
        Uniform<MatrixSet>& uni, Uniform<Material>& material,
        VkDescriptorSet& descriptorSet, VkDescriptorPool& descPool,
        VkDescriptorSetLayout& descSetLayout);

    uint32_t upDescriptorSet2D(bool useTexture, VkTexture& texture, Uniform<MatrixSet2D>& uni,
        VkDescriptorSet& descriptorSet, VkDescriptorPool& descPool,
        VkDescriptorSetLayout& descSetLayout);

    void createUploadBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext);

    void createDefaultBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext);

    template<typename UNI>
    void createUniform(UNI& uni) {
        uni.buf.createUploadBuffer(sizeof(UNI), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, nullptr);
    }

    template<typename UNI>
    void updateUniform(UNI& uni) {
        uni.buf.memoryMap(&uni.uni);
    }

    template<typename T>
    auto createVertexBuffer(uint32_t comBufindex, T* ver, int num, bool typeIndex,
        void* allocateMemory_add_pNext, VkBufferUsageFlags* add_usage) {

        VkDeviceSize bufferSize = sizeof(T) * num;

        BufferSet stagingBuffer;
        stagingBuffer.createUploadBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, nullptr);

        stagingBuffer.memoryMap(ver);

        BufferSet vertexBuffer;
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (typeIndex)usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (add_usage)usage = usage | *add_usage;
        vertexBuffer.createDefaultBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, allocateMemory_add_pNext);

        copyBuffer(comBufindex, stagingBuffer.getBuffer(), vertexBuffer.getBuffer(), bufferSize);

        stagingBuffer.destroy();

        return vertexBuffer;
    }

    void createDevice(std::vector<const char*>* requiredExtensions = nullptr, const void* pNext = nullptr);

    void createSwapchain(VkSurfaceKHR surface, bool clearBackBuffer);

    void destroySwapchain();

    void GetTexture(uint32_t comBufindex, char* fileName, unsigned char* byteArr, uint32_t width,
        uint32_t height);

    int32_t getTextureNo(char* pass);

    void updateProjection(float AngleView = 45.0f, float Near = 1.0f, float Far = 100.0f);

    void updateView(CoordTf::VECTOR3 view, CoordTf::VECTOR3 gaze);

    void setNumLight(uint32_t num);

    void setLightAttenuation(float att1, float att2, float att3);

    void setLight(uint32_t index, CoordTf::VECTOR3 pos, CoordTf::VECTOR3 color);

    void beginCommandNextImage(uint32_t comBufindex);

    void barrierResource(uint32_t comBufindex, VkImage image, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout);

    void beginDraw(uint32_t comBufindex);

    void endDraw(uint32_t comBufindex);

    void endCommand(uint32_t comBufindex);

    void Present(uint32_t comBufindex);

    void beginCommand(uint32_t comBufindex);

    void submitCommandsDoNotRender(uint32_t comBufindex);

    VkDevice getDevice() { return device; }

    VkCommandBuffer getCommandBuffer(uint32_t comBufindex) { return commandBuffer[comBufindex]; }

    swapchainBuffer* getSwapchainObj() { return &swBuf; }

    void DeviceWaitIdle();

    CoordTf::MATRIX getProjection() { return proj; }
    CoordTf::MATRIX getCameraView() { return view; }
    CoordTf::VECTOR4 getCameraViewPos() { return viewPos; }
    const CoordTf::VECTOR3 getUpVec() { return upVec; }
};
#endif