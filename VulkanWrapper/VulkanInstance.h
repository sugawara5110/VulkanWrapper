//*****************************************************************************************//
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
#endif
#include "VulkanPFN.h"
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

    void createinstance(char* appName);

    void createDebugReportCallback();

    void createPhysicalDevice();

public:
    ~VulkanInstance();

    void createInstance(char* appName);

#ifdef __ANDROID__
    void createSurfaceAndroid(ANativeWindow* Window);
#else
    void createSurfaceHwnd(HWND hWnd);
#endif
    void destroySurface();

    VkPhysicalDevice getPhysicalDevice(int index = 0);

    VkSurfaceKHR getSurface();
};

class Device final {

private:
    friend Vulkan2D;
    friend VulkanBasicPolygon;
    friend VulkanSkinMesh;
    VkPhysicalDevice pDev = nullptr;//VulkanInstanceからポインタを受け取る
    VkDevice device = nullptr;
    VkQueue devQueue = nullptr;
    uint32_t queueFamilyIndex = 0xffffffff;
    VkPhysicalDeviceMemoryProperties memProps = {};
    VkCommandPool commandPool;
    VkFence sFence;
    std::unique_ptr<VkFence[]> swFence = nullptr;//現状0番のみ使用
    bool firstswFence = false;
    VkSemaphore renderCompletedSem, presentCompletedSem;

    struct swapchainBuffer {
        VkSwapchainKHR swapchain = {};
        uint32_t imageCount = 0;
        std::unique_ptr<VkImage[]> images = nullptr;//スワップチェーンの画像表示に使う
        std::unique_ptr<VkImageView[]> views = nullptr;
        std::unique_ptr<VkFramebuffer[]> frameBuffer = nullptr;
        VkFormat format = {};
    };
    bool swapchainAlive = false;
    swapchainBuffer swBuf;
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

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
    std::unique_ptr<VkCommandBuffer[]> commandBuffer = nullptr;
    uint32_t currentFrameIndex = 0;
    const static uint32_t numLightMax = 256;
#if CHANGE
    const static uint32_t numBoneMax = 64;
#else
    const static uint32_t numBoneMax = 256;
#endif
    const static uint32_t numTextureMax = 254;
    const static uint32_t numTexFileNamelenMax = 256;

    MATRIX proj, view;
    VECTOR4 viewPos;
    VECTOR4 lightPos[numLightMax];
    VECTOR4 lightColor[numLightMax];
    uint32_t numLight = 1;
    float attenuation1 = 1.0f;
    float attenuation2 = 0.001f;
    float attenuation3 = 0.001f;

    struct MatrixSet {
        MATRIX world;
        MATRIX mvp;
        MATRIX bone[numBoneMax];
    };

    struct Material {
        VECTOR4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
        VECTOR4 specular = { 0.0f, 0.0f, 0.0f, 0.0f };
        VECTOR4 ambient = { 0.0f, 0.0f, 0.0f, 0.0f };
        VECTOR4 viewPos;
        VECTOR4 lightPos[numLightMax];
        VECTOR4 lightColor[numLightMax];
        VECTOR4 numLight;//ライト数,減衰1,減衰2,減衰3
        VECTOR4 UvSwitch = {};//.x==0:そのまま, 1:切り替え
    };

    template<typename UNI>
    struct Uniform {
        VkBuffer vkBuf;
        VkDeviceMemory mem;
        VkDeviceSize memSize;
        UNI uni;
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
    Texture texture[numTextureMax + 2];
    uint32_t numTexture = 0;
    char textureNameList[numTextureMax][numTexFileNamelenMax];

    Device() {}

    void create();

    void createCommandPool();

    void createFence();

    void createSFence();

    void createSemaphore();

    void createswapchain(VkSurfaceKHR surface);

    void createDepth();

    void createCommonRenderPass();

    void createFramebuffers();

    void createCommandBuffers();

    void beginCommandWithFramebuffer(uint32_t comBufindex, VkFramebuffer fb);

    void submitCommands(uint32_t comBufindex, VkFence fence, bool useRender);

    void acquireNextImageAndWait(uint32_t& currentFrameIndex);

    VkResult waitForFence(VkFence fence);

    void present(uint32_t currentframeIndex);

    void resetFence(VkFence fence);

    void barrierResource(uint32_t comBufindex, VkImage image, VkImageLayout srcImageLayout,
        VkImageLayout dstImageLayout);

    void beginRenderPass(uint32_t comBufindex, uint32_t currentframeIndex);

    void
        copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width,
            uint32_t height);

    void createImage(uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory);

    auto createTextureImage(uint32_t comBufindex, unsigned char* byteArr, uint32_t width,
        uint32_t height);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags mask,
        VkComponentMapping components = { VK_COMPONENT_SWIZZLE_IDENTITY });

    void createTextureSampler(VkSampler& textureSampler);

    void destroyTexture();

    char* getNameFromPass(char* pass);

    //モデル毎(モデル側から呼ばれる)
    template<typename T>
    auto createVertexBuffer(uint32_t comBufindex, T* ver, int num, bool typeIndex) {

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

        copyBuffer(comBufindex, stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

        std::pair<VkBuffer, VkDeviceMemory> buf = std::make_pair(vertexBuffer, vertexBufferMemory);
        return buf;
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties, VkBuffer& buffer,
        VkDeviceMemory& bufferMemory, VkDeviceSize& memSize);

    void
        copyBuffer(uint32_t comBufindex, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    template<typename UNI>
    void createUniform(UNI& uni) {
        createBuffer(sizeof(UNI), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uni.vkBuf, uni.mem, uni.memSize);

        uni.info.buffer = uni.vkBuf;
        uni.info.offset = 0;
        uni.info.range = sizeof(UNI);
    }

    template<typename UNI>
    void updateUniform(UNI& uni) {
        uint8_t* pData;
        auto res = vkMapMemory(device, uni.mem, 0, uni.memSize, 0, (void**)&pData);
        checkError(res);
        memcpy(pData, &uni.uni, sizeof(UNI));
        vkUnmapMemory(device, uni.mem);
        uni.info.buffer = uni.vkBuf;
        uni.info.offset = 0;
        uni.info.range = sizeof(UNI);
    }

    void descriptorAndPipelineLayouts(bool useTexture, VkPipelineLayout& pipelineLayout,
        VkDescriptorSetLayout& descSetLayout);

    VkPipelineLayout createPipelineLayout2D();

    VkShaderModule createShaderModule(char* shader);

    void createDescriptorPool(bool useTexture, VkDescriptorPool& descPool);

    uint32_t
        upDescriptorSet(bool useTexture, Texture& difTexture, Texture& norTexture, Texture& speTexture,
            Uniform<MatrixSet>& uni, Uniform<Material>& material,
            VkDescriptorSet& descriptorSet, VkDescriptorPool& descPool,
            VkDescriptorSetLayout& descSetLayout);

    VkPipelineCache createPipelineCache();

    VkPipeline createGraphicsPipelineVF(bool useAlpha,
        const VkShaderModule& vshader,
        const VkShaderModule& fshader,
        const VkVertexInputBindingDescription& bindDesc,
        const VkVertexInputAttributeDescription* attrDescs,
        uint32_t numAttr,
        const VkPipelineLayout& pLayout,
        const VkRenderPass renderPass,
        const VkPipelineCache& pCache);

    //モデル毎
    void getTextureSub(uint32_t comBufindex, uint32_t texNo, unsigned char* byteArr, uint32_t width,
        uint32_t height);

public:
    Device(VkPhysicalDevice pd, uint32_t numCommandBuffer = 1,
        bool V_SYNC = true);

    ~Device();

    void createDevice();

    void createSwapchain(VkSurfaceKHR surface);

    void destroySwapchain();

    void GetTexture(uint32_t comBufindex, char* fileName, unsigned char* byteArr, uint32_t width,
        uint32_t height);

    int32_t getTextureNo(char* pass);

    void updateProjection(float AngleView = 45.0f, float Near = 1.0f, float Far = 100.0f);

    void updateView(VECTOR3 view, VECTOR3 gaze, VECTOR3 up);

    void setNumLight(uint32_t num);

    void setLightAttenuation(float att1, float att2, float att3);

    void setLight(uint32_t index, VECTOR3 pos, VECTOR3 color);

    void beginCommand(uint32_t comBufindex);

    void endCommand(uint32_t comBufindex);

    void Present(uint32_t comBufindex);
};
#endif
