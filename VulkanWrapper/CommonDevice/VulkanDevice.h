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
    static void InstanceCreate(
        VkPhysicalDevice pd,
        uint32_t ApiVersion,
        uint32_t numCommandBuffer = 1,
        uint32_t numGraphicsQueue = 1,
        uint32_t numComputeQueue = 0);

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
        uint32_t Width = 0;
        uint32_t Height = 0;
        VkDescriptorImageInfo info = {};
        VkImage getImage()const { return image; }
        VkImage* getImageAddress() { return &image; }
        VkDeviceMemory getMemory()const { return mem; }
        VkFormat getFormat()const { return format; }

        void barrierResource(uint32_t QueueIndex, uint32_t comBufindex,
            VkImageLayout dstImageLayout, VkImageAspectFlagBits mask = VK_IMAGE_ASPECT_COLOR_BIT);

        void createImage(uint32_t width, uint32_t height, VkFormat format,
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);

        void createImageView(VkFormat format, VkImageAspectFlags mask = VK_IMAGE_ASPECT_COLOR_BIT,
            VkComponentMapping components = { VK_COMPONENT_SWIZZLE_IDENTITY });

        void destroy();
    };

    struct textureIdSet {
        int diffuseId = -1;
        char difUvName[256] = {};
        ImageSet difTex;
        int normalId = -1;
        char norUvName[256] = {};
        ImageSet norTex;
        int specularId = -1;
        char speUvName[256] = {};
        ImageSet speTex;
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

    struct Vertex3D {
        float pos[3];
        float normal[3];
        float difUv[2];
        float speUv[2];
    };

    class CommandObj {
    private:
        friend VulkanDevice;
        VkQueue devQueue = VK_NULL_HANDLE;
        uint32_t queueFamilyIndex = 0xffffffff;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> commandBuffer = {};
        VkFence fence = VK_NULL_HANDLE;

        void createCommandPool();
        void createFence();
        void createCommandBuffers();

        void create(uint32_t numCommand);
        void destroy();

    public:
        uint32_t getQueueFamilyIndex();

        VkQueue getQueue();

        void beginCommand(uint32_t comBufindex, VkFramebuffer fb = VkFramebuffer());

        void endCommand(uint32_t comBufindex);

        void submitCommands(VkFence fence, bool useRender,
            uint32_t waitSemaphoreCount, VkSemaphore* WaitSemaphores,
            uint32_t signalSemaphoreCount, VkSemaphore* SignalSemaphores);

        void submitCommandsDoNotRender();

        VkCommandBuffer getCommandBuffer(uint32_t comBufindex);
    };

private:
    static VulkanDevice* DevicePointer;

    VkPhysicalDevice pDev = VK_NULL_HANDLE;//VulkanInstanceからポインタを受け取る
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDeviceMemoryProperties memProps = {};
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    uint32_t commandBufferCount = 1;
    uint32_t NumGraphicsQueue = 0;
    uint32_t NumComputeQueue = 0;
    uint32_t NumAllQueue = 0;
    std::vector<CommandObj> commandObj = {};
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

    VulkanDevice(VkPhysicalDevice pd, uint32_t numCommandBuffer, uint32_t numGraphicsQueue, uint32_t numComputeQueue);

    ~VulkanDevice();

    void create(std::vector<const char*>* requiredExtensions, const void* pNext);

    bool createDescriptorPool(std::vector<VkDescriptorPoolSize>* add_poolSize, uint32_t maxDescriptorSets);

    void resetFence(VkFence fence);

    void AllocateMemory(VkBufferUsageFlags usage, VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties,
        VkDeviceMemory& bufferMemory, void* add_pNext);

    auto createTextureImage(uint32_t QueueIndex, uint32_t comBufindex, Texture& inByte);

    void destroyTexture();

    void copyBuffer(uint32_t QueueIndex, uint32_t comBufindex, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

public:
    VkResult waitForFence(VkFence fence);

    void copyBufferToImage(
        uint32_t QueueIndex, uint32_t comBufindex,
        VkBuffer buffer,
        VkImage image, uint32_t width, uint32_t height,
        VkImageAspectFlagBits mask = VK_IMAGE_ASPECT_COLOR_BIT);

    void copyImageToBuffer(
        uint32_t QueueIndex, uint32_t comBufindex,
        VkImage image, uint32_t width, uint32_t height,
        VkBuffer buffer,
        VkImageAspectFlagBits mask = VK_IMAGE_ASPECT_COLOR_BIT);

    VkPipelineShaderStageCreateInfo createShaderModule(const char* fname, char* shader, VkShaderStageFlagBits stage);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext);

    void createImage(uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory, VkImageLayout initialLayout);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags mask,
        VkComponentMapping components = { VK_COMPONENT_SWIZZLE_IDENTITY });

    void createVkTexture(ImageSet& tex, uint32_t QueueIndex, uint32_t comBufindex, Texture& inByte);

    void createTextureSampler(VkSampler& textureSampler);

    void createTextureSet(uint32_t QueueIndex, uint32_t comBufindex, textureIdSet& texSet);

    void memoryMap(void* pData, VkDeviceMemory mem, VkDeviceSize size);

    void* Map(VkDeviceMemory mem);

    void UnMap(VkDeviceMemory mem);

    void createUploadBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext);

    void createDefaultBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext);

    template<typename T>
    auto createDefaultCopiedBuffer(uint32_t QueueIndex, uint32_t comBufindex, T* data, int num,
        void* allocateMemory_add_pNext, VkBufferUsageFlags* add_usage) {

        VkDeviceSize bufferSize = sizeof(T) * num;

        BufferSet stagingBuffer;
        stagingBuffer.createUploadBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, nullptr);

        stagingBuffer.memoryMap(data);

        BufferSet defaultBuffer;
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (add_usage)usage = usage | *add_usage;
        defaultBuffer.createDefaultBuffer(bufferSize, usage, allocateMemory_add_pNext);

        copyBuffer(QueueIndex, comBufindex, stagingBuffer.getBuffer(), defaultBuffer.getBuffer(), bufferSize);

        stagingBuffer.destroy();

        return defaultBuffer;
    }

    template<typename T>
    auto createVertexBuffer(uint32_t QueueIndex, uint32_t comBufindex, T* ver, int num, bool typeIndex,
        void* allocateMemory_add_pNext, VkBufferUsageFlags* add_usage) {

        VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (typeIndex)usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (add_usage)usage = usage | *add_usage;

        return createDefaultCopiedBuffer(QueueIndex, comBufindex, ver, num, allocateMemory_add_pNext, &usage);
    }

    void createDevice(
        std::vector<const char*>* requiredExtensions = nullptr, const void* pNext = nullptr,
        std::vector<VkDescriptorPoolSize>* add_poolSize = nullptr, uint32_t maxDescriptorSets = 0);

    void GetTexture(
        char* fileName,
        unsigned char* byteArr,
        uint32_t width,
        uint32_t height);

    int32_t getTextureNo(char* pass);

    void updateProjection(VkExtent2D wh, float AngleView = 45.0f, float Near = 1.0f, float Far = 100.0f);

    void updateView(CoordTf::VECTOR3 view, CoordTf::VECTOR3 gaze, CoordTf::VECTOR3 up = { 0.0f,1.0f,0.0f });

    void barrierResource(uint32_t QueueIndex, uint32_t comBufindex, VkImage image,
        VkImageLayout srcImageLayout, VkImageLayout dstImageLayout, VkImageAspectFlagBits mask);

    VkDevice getDevice() { return device; }

    void DeviceWaitIdle();

    CoordTf::MATRIX getProjection() { return proj; }
    CoordTf::MATRIX getCameraView() { return view; }
    CoordTf::VECTOR4 getCameraViewPos() { return viewPos; }
    CoordTf::VECTOR3 getUpVec() { return upVec; }
    Texture getTexture(uint32_t index) { return texture[index]; }

    VkDescriptorPool GetDescriptorPool() const { return descriptorPool; }

    VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout dsLayout, const void* pNext = nullptr);

    void DeallocateDescriptorSet(VkDescriptorSet ds);

    CommandObj* getCommandObj(uint32_t index) { return &commandObj[index]; }
};

#endif
