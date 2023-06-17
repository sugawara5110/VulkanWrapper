//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanDeviceRt.cpp                                     **//
//**                                                                                     **//
//*****************************************************************************************//

#include "VulkanDeviceRt.h"

VulkanDeviceRt* VulkanDeviceRt::pDeviceRt = nullptr;

void BufferSetRt::createUploadBuffer(VkDeviceSize size, VkBufferUsageFlags usage, void* allocateMemory_add_pNext) {
    VulkanDevice::BufferSet::createUploadBuffer(size, usage, allocateMemory_add_pNext);
    deviceAddress = VulkanDeviceRt::getVulkanDeviceRt()->GetDeviceAddress(buffer);
}

void BufferSetRt::createDefaultBuffer(VkDeviceSize size, VkBufferUsageFlags usage, void* allocateMemory_add_pNext) {
    VulkanDevice::BufferSet::createDefaultBuffer(size, usage, allocateMemory_add_pNext);
    deviceAddress = VulkanDeviceRt::getVulkanDeviceRt()->GetDeviceAddress(buffer);
}

bool VulkanDeviceRt::createDevice(VkInstance ins, VkPhysicalDevice phDev, uint32_t ApiVersion,
    uint32_t numCommandBuffer,
    uint32_t numGraphicsQueue,
    uint32_t numComputeQueue,
    std::vector<VkDescriptorPoolSize>* add_poolSize, uint32_t maxDescriptorSets) {

    pDeviceRt = this;
    physicalDevice = phDev;
    std::vector<const char*> requiredExtensions = {
        VK_KHR_MAINTENANCE3_EXTENSION_NAME,
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,

        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    };

    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR enabledBufferDeviceAddressFeatures{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, nullptr,
    };
    enabledBufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, nullptr,
    };
    enabledRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
    enabledRayTracingPipelineFeatures.pNext = &enabledBufferDeviceAddressFeatures;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerataionStuctureFeatures{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, nullptr,
    };
    enabledAccelerataionStuctureFeatures.accelerationStructure = VK_TRUE;
    enabledAccelerataionStuctureFeatures.pNext = &enabledRayTracingPipelineFeatures;

    VkPhysicalDeviceDescriptorIndexingFeatures enabledDescriptorIndexingFeatures{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES
    };
    enabledDescriptorIndexingFeatures.pNext = &enabledAccelerataionStuctureFeatures;
    enabledDescriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing = VK_TRUE;
    enabledDescriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    enabledDescriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
    enabledDescriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
    enabledDescriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;

    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, nullptr,
    };
    physicalDeviceFeatures2.pNext = &enabledDescriptorIndexingFeatures;
    physicalDeviceFeatures2.features = features;

    VulkanDevice::InstanceCreate(phDev, ApiVersion,
        numCommandBuffer, numGraphicsQueue, numComputeQueue);

    VulkanDevice* vkDev = VulkanDevice::GetInstance();

    const void* pNext = &physicalDeviceFeatures2;
    vkDev->createDevice(&requiredExtensions, pNext, add_poolSize, maxDescriptorSets);

    load_VK_EXTENSIONS(
        ins,
        vkGetInstanceProcAddr,
        vkDev->getDevice(),
        vkGetDeviceProcAddr
    );
    return true;
}

void VulkanDeviceRt::destroy() {

    VulkanDevice* vkDev = VulkanDevice::GetInstance();
    if (vkDev->getDevice()) {
        _vkDeviceWaitIdle(vkDev->getDevice());
    }

    VulkanDevice::DeleteInstance();
}

uint64_t VulkanDeviceRt::GetDeviceAddress(VkBuffer buffer) {

    VulkanDevice* vkDev = VulkanDevice::GetInstance();
    VkBufferDeviceAddressInfo bufferDeviceInfo{
        VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr
    };
    bufferDeviceInfo.buffer = buffer;
    return vkGetBufferDeviceAddress(vkDev->getDevice(), &bufferDeviceInfo);
}

VkPhysicalDeviceRayTracingPipelinePropertiesKHR VulkanDeviceRt::GetRayTracingPipelineProperties() {

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR physDevRtPipelineProps{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR
    };
    VkPhysicalDeviceProperties2 physDevProps2{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
    };
    physDevProps2.pNext = &physDevRtPipelineProps;
    vkGetPhysicalDeviceProperties2(physicalDevice, &physDevProps2);
    return physDevRtPipelineProps;
}
