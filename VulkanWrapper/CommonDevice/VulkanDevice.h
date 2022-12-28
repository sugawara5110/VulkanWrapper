//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanDevice.h                                         **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanDevice_Header
#define VulkanDevice_Header

#include "VulkanInstance.h"

class VulkanDevice final {

public:
    static void InstanceCreate(VkPhysicalDevice pd,
        uint32_t ApiVersion,
        uint32_t numCommandBuffer = 1,
        bool V_SYNC = false);

    static VulkanDevice* GetInstance();
    static void DeleteInstance();

    const char* GetDeviceName()const;
    VkDeviceSize GetStorageBufferAlignment()const;
    VkDeviceSize GetUniformBufferAlignment()const;

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

    template<class T>
    class Uniform {
    protected:
        BufferSet buf = {};
        uint32_t elementByteSize = 0;
        uint32_t elementCount = 0;
        uint8_t* MappedData = nullptr;

    public:
        Uniform(uint32_t ElementCount, VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, void* pNext = nullptr) {
            elementCount = ElementCount;
            uint32_t alignment = (uint32_t)VulkanDevice::GetInstance()->GetUniformBufferAlignment();
            elementByteSize = (uint32_t)vkUtil::Align(sizeof(T), alignment);
            uint32_t bSize = elementByteSize * elementCount;
            buf.createUploadBuffer(bSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usage, pNext);
            MappedData = static_cast<uint8_t*>(buf.Map());
        }

        ~Uniform() {
            buf.UnMap();
            buf.destroy();
        }

        BufferSet* getBufferSet() {
            return &buf;
        }

        void update(uint32_t elementIndex, T* data) {
            memcpy(&MappedData[elementIndex * elementByteSize], data, sizeof(T));
        }

        void updateArr(T* dataArr) {
            for (uint32_t i = 0; i < elementCount; i++) {
                update(i, &dataArr[i]);
            }
        }
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

        void barrierResource(uint32_t comBufindex, VkImageLayout dstImageLayout, VkImageAspectFlagBits mask);

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
            image.barrierResource(comBufindex, dstImageLayout, VK_IMAGE_ASPECT_COLOR_BIT);
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

    struct textureIdSetInput {
        int diffuseId = -1;
        int normalId = -1;
        int specularId = -1;
    };

    class swapchainBuffer {
    private:
        ImageSet depth;
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
        ImageSet* getDepthImageSet() { return &depth; }
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
    static VulkanDevice* DevicePointer;

    VkPhysicalDevice pDev = VK_NULL_HANDLE;//VulkanInstanceからポインタを受け取る
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue devQueue = VK_NULL_HANDLE;
    uint32_t queueFamilyIndex = 0xffffffff;
    VkPhysicalDeviceMemoryProperties memProps = {};
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkFence sFence = VK_NULL_HANDLE;
    VkSemaphore renderCompletedSem, presentCompletedSem;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    swapchainBuffer swBuf;
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    uint32_t commandBufferCount = 1;
    std::unique_ptr<VkCommandBuffer[]> commandBuffer = nullptr;

    CoordTf::MATRIX proj, view;
    CoordTf::VECTOR4 viewPos;
    CoordTf::VECTOR3 upVec = {};

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
    VulkanDevice(const VulkanDevice& obj) = delete;  //コピーコンストラクタ禁止
    void operator=(const VulkanDevice& obj) = delete;//代入演算子禁止

    VulkanDevice(VkPhysicalDevice pd, uint32_t numCommandBuffer, bool V_SYNC);

    ~VulkanDevice();

    void create(std::vector<const char*>* requiredExtensions, const void* pNext);

    void createCommandPool();

    void createSFence();

    void createSemaphore();

    void createCommandBuffers();

    bool createDescriptorPool(std::vector<VkDescriptorPoolSize>* add_poolSize, uint32_t maxDescriptorSets);

    void beginCommandWithFramebuffer(uint32_t comBufindex, VkFramebuffer fb);

    void submitCommands(uint32_t comBufindex, VkFence fence, bool useRender);

    VkResult waitForFence(VkFence fence);

    void present();

    void resetFence(VkFence fence);

    void AllocateMemory(VkBufferUsageFlags usage, VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties,
        VkDeviceMemory& bufferMemory, void* add_pNext);

    auto createTextureImage(uint32_t comBufindex, Texture& inByte);

    void destroyTexture();

    void copyBuffer(uint32_t comBufindex, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

public:
    void copyBufferToImage(uint32_t comBufindex,
        VkBuffer buffer,
        VkImage image, uint32_t width, uint32_t height,
        VkImageAspectFlagBits mask);

    void copyImageToBuffer(uint32_t comBufindex,
        VkImage image, uint32_t width, uint32_t height,
        VkBuffer buffer,
        VkImageAspectFlagBits mask);

    VkPipelineShaderStageCreateInfo createShaderModule(const char* fname, char* shader, VkShaderStageFlagBits stage);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext);

    void createImage(uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags mask,
        VkComponentMapping components = { VK_COMPONENT_SWIZZLE_IDENTITY });

    void createVkTexture(VkTexture& tex, uint32_t comBufindex, Texture& inByte);

    void createTextureSampler(VkSampler& textureSampler);

    void createTextureSet(uint32_t comBufindex, textureIdSet& texSet);

    void memoryMap(void* pData, VkDeviceMemory mem, VkDeviceSize size);

    void* Map(VkDeviceMemory mem);

    void UnMap(VkDeviceMemory mem);

    void createUploadBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext);

    void createDefaultBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext);

    template<typename T>
    auto createDefaultCopiedBuffer(uint32_t comBufindex, T* data, int num,
        void* allocateMemory_add_pNext, VkBufferUsageFlags* add_usage) {

        VkDeviceSize bufferSize = sizeof(T) * num;

        BufferSet stagingBuffer;
        stagingBuffer.createUploadBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, nullptr);

        stagingBuffer.memoryMap(data);

        BufferSet defaultBuffer;
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (add_usage)usage = usage | *add_usage;
        defaultBuffer.createDefaultBuffer(bufferSize, usage, allocateMemory_add_pNext);

        copyBuffer(comBufindex, stagingBuffer.getBuffer(), defaultBuffer.getBuffer(), bufferSize);

        stagingBuffer.destroy();

        return defaultBuffer;
    }

    template<typename T>
    auto createVertexBuffer(uint32_t comBufindex, T* ver, int num, bool typeIndex,
        void* allocateMemory_add_pNext, VkBufferUsageFlags* add_usage) {

        VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (typeIndex)usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (add_usage)usage = usage | *add_usage;

        return createDefaultCopiedBuffer(comBufindex, ver, num, allocateMemory_add_pNext, &usage);
    }

    void createDevice(
        std::vector<const char*>* requiredExtensions = nullptr, const void* pNext = nullptr,
        std::vector<VkDescriptorPoolSize>* add_poolSize = nullptr, uint32_t maxDescriptorSets = 0);

    void createSwapchain(VkSurfaceKHR surface, bool clearBackBuffer);

    void destroySwapchain();

    void GetTexture(uint32_t comBufindex, char* fileName, unsigned char* byteArr, uint32_t width,
        uint32_t height);

    int32_t getTextureNo(char* pass);

    void updateProjection(float AngleView = 45.0f, float Near = 1.0f, float Far = 100.0f);

    void updateView(CoordTf::VECTOR3 view, CoordTf::VECTOR3 gaze, CoordTf::VECTOR3 up = { 0.0f,1.0f,0.0f });

    void beginCommandNextImage(uint32_t comBufindex);

    void barrierResource(uint32_t comBufindex, VkImage image,
        VkImageLayout srcImageLayout, VkImageLayout dstImageLayout, VkImageAspectFlagBits mask);

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
    CoordTf::VECTOR3 getUpVec() { return upVec; }
    Texture getTexture(uint32_t index) { return texture[index]; }

    VkDescriptorPool GetDescriptorPool() const { return descriptorPool; }

    VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout dsLayout, const void* pNext = nullptr);

    void DeallocateDescriptorSet(VkDescriptorSet ds);
};

#endif
