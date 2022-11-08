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

bool VulkanDeviceRt::createDevice(VkInstance ins, VkPhysicalDevice phDev) {
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

    VulkanDevice::InstanceCreate(phDev, 2, false);
    VulkanDevice* vkDev = VulkanDevice::GetInstance();

    const void* pNext = &physicalDeviceFeatures2;
    vkDev->createDevice(&requiredExtensions, pNext);

    CreateDescriptorPool();

    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

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
        vkDeviceWaitIdle(vkDev->getDevice());
    }

    if (descriptorPool) {
        vkDestroyDescriptorPool(vkDev->getDevice(), descriptorPool, nullptr);
    }
    VulkanDevice::DeleteInstance();
}

bool VulkanDeviceRt::CreateSwapchain(VkSurfaceKHR surface, uint32_t width, uint32_t height) {

    VulkanDevice* vkDev = VulkanDevice::GetInstance();
    vkDev->createSwapchain(surface, false);

    uint32_t imageCount = vkDev->getSwapchainObj()->getImageCount();

    auto command = vkDev->getCommandBuffer(0);
    vkDev->beginCommand(0);
    for (uint32_t i = 0; i < imageCount; ++i) {
        vkDev->barrierResource(0, vkDev->getSwapchainObj()->getImage(i), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }
    vkDev->endCommand(0);
    vkDev->submitCommandsDoNotRender(0);

    return true;
}

uint64_t VulkanDeviceRt::GetDeviceAddress(VkBuffer buffer) {

    VulkanDevice* vkDev = VulkanDevice::GetInstance();
    VkBufferDeviceAddressInfo bufferDeviceInfo{
        VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr
    };
    bufferDeviceInfo.buffer = buffer;
    return vkGetBufferDeviceAddress(vkDev->getDevice(), &bufferDeviceInfo);
}

bool VulkanDeviceRt::CreateDescriptorPool() {

    VulkanDevice* vkDev = VulkanDevice::GetInstance();
    VkResult result;
    VkDescriptorPoolSize poolSize[] = {
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
      { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 100 },
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
    };
    VkDescriptorPoolCreateInfo descPoolCI{
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      nullptr,  VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
      100, // maxSets
      _countof(poolSize), poolSize,
    };
    result = vkCreateDescriptorPool(vkDev->getDevice(), &descPoolCI, nullptr, &descriptorPool);
    return result == VK_SUCCESS;
}

VkDescriptorSet VulkanDeviceRt::AllocateDescriptorSet(VkDescriptorSetLayout dsLayout, const void* pNext) {

    VulkanDevice* vkDev = VulkanDevice::GetInstance();
    VkDescriptorSetAllocateInfo dsAI{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr
    };
    dsAI.descriptorPool = descriptorPool;
    dsAI.pSetLayouts = &dsLayout;
    dsAI.descriptorSetCount = 1;
    dsAI.pNext = pNext;
    VkDescriptorSet ds{};
    auto r = vkAllocateDescriptorSets(vkDev->getDevice(), &dsAI, &ds);
    vkUtil::checkError(r);
    return ds;
}

void VulkanDeviceRt::DeallocateDescriptorSet(VkDescriptorSet ds) {

    VulkanDevice* vkDev = VulkanDevice::GetInstance();
    vkFreeDescriptorSets(vkDev->getDevice(), descriptorPool, 1, &ds);
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
