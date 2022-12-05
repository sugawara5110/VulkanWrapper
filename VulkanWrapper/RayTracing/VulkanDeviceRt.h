//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanDeviceRt.h                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanDeviceRt_Header
#define VulkanDeviceRt_Header

#include <cstdint>
#include <vector>
#include "../CommonDevice/VulkanDevice.h"

class VulkanDeviceRt;

class BufferSetRt : public VulkanDevice::BufferSet {

private:
    VkDeviceAddress deviceAddress = 0;

public:
    VkDeviceAddress getDeviceAddress()const { return deviceAddress; }
    VkDescriptorBufferInfo getDescriptor() const { return VkDescriptorBufferInfo{ buffer, 0, VK_WHOLE_SIZE }; }
    void createUploadBuffer(VkDeviceSize size, VkBufferUsageFlags usage, void* allocateMemory_add_pNext);
    void createDefaultBuffer(VkDeviceSize size, VkBufferUsageFlags usage, void* allocateMemory_add_pNext);

    template<typename T>
    void createVertexBuffer(uint32_t comBufindex, T* ver, int num, bool typeIndex,
        void* allocateMemory_add_pNext, VkBufferUsageFlags* add_usage) {

        VulkanDevice::BufferSet buf =
            VulkanDevice::GetInstance()->createVertexBuffer(comBufindex, ver, num, typeIndex, allocateMemory_add_pNext, add_usage);

        buffer = buf.getBuffer();
        mem = buf.getMemory();
        Size = buf.getSize();
        info = buf.info;
        deviceAddress = VulkanDeviceRt::getVulkanDeviceRt()->GetDeviceAddress(buffer);
    }
};

class VulkanDeviceRt {

private:
    static VulkanDeviceRt* pDeviceRt;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties physicalDeviceProperties;

public:
    static const uint32_t numHitShader = 4;

    static VulkanDeviceRt* getVulkanDeviceRt() { return pDeviceRt; };

    bool createDevice(VkInstance ins, VkPhysicalDevice phDev, uint32_t ApiVersion);
    void destroy();

    bool CreateSwapchain(VkSurfaceKHR surface, uint32_t width, uint32_t height);

    VkDevice GetDevice() const { return VulkanDevice::GetInstance()->getDevice(); }

    VkDescriptorPool GetDescriptorPool() const { return descriptorPool; }

    VkSurfaceFormatKHR GetBackBufferFormat() const {
        return VulkanDevice::GetInstance()->getSwapchainObj()->getBackBufferFormat(0);
    }

    uint32_t GetBackBufferCount() const { return VulkanDevice::GetInstance()->getSwapchainObj()->getImageCount(); }

    const char* GetDeviceName() const { return physicalDeviceProperties.deviceName; }

    VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout dsLayout, const void* pNext = nullptr);
    void DeallocateDescriptorSet(VkDescriptorSet ds);

    bool CreateDescriptorPool();

    uint64_t GetDeviceAddress(VkBuffer buffer);
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR GetRayTracingPipelineProperties();
    VkDeviceSize GetUniformBufferAlignment() const { return physicalDeviceProperties.limits.minUniformBufferOffsetAlignment; }
    VkDeviceSize GetStorageBufferAlignment() const { return physicalDeviceProperties.limits.minStorageBufferOffsetAlignment; }
};

#endif