//*****************************************************************************************//
//**                                                                                     **//
//**                            AccelerationStructure.cpp                                **//
//**                                                                                     **//
//*****************************************************************************************//

#include "AccelerationStructure.h"

void AccelerationStructure::destroy() {
    scratchBuffer.destroy();
    updateBuffer.destroy();
    vkDestroyAccelerationStructureKHR(VulkanDevice::GetInstance()->getDevice(), accelerationStructure.handle, nullptr);
    accelerationStructure.bufferResource.destroy();
}

void AccelerationStructure::destroyScratchBuffer() {
    scratchBuffer.destroy();
}

void AccelerationStructure::buildAS(
    VkAccelerationStructureTypeKHR type,
    const Input& input,
    VkBuildAccelerationStructureFlagsKHR buildFlags) {

    auto device = VulkanDevice::GetInstance()->getDevice();

    VkAccelerationStructureBuildGeometryInfoKHR asBuildGeometryInfo{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR
    };
    asBuildGeometryInfo.type = type;
    asBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    asBuildGeometryInfo.flags = buildFlags;

    asBuildGeometryInfo.geometryCount = uint32_t(input.Geometry.size());
    asBuildGeometryInfo.pGeometries = input.Geometry.data();

    VkAccelerationStructureBuildSizesInfoKHR asBuildSizesInfo{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
    };
    std::vector<uint32_t> numPrimitives;
    numPrimitives.reserve(input.BuildRangeInfo.size());
    for (int i = 0; i < input.BuildRangeInfo.size(); i++) {
        numPrimitives.push_back(input.BuildRangeInfo[i].primitiveCount);
    }

    vkGetAccelerationStructureBuildSizesKHR(
        device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &asBuildGeometryInfo,
        numPrimitives.data(),
        &asBuildSizesInfo
    );

    VkBufferUsageFlags asUsage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{
  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, nullptr, };

    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

    void* pNext = &memoryAllocateFlagsInfo;

    accelerationStructure.bufferResource.createDefaultBuffer(asBuildSizesInfo.accelerationStructureSize, asUsage, pNext);

    VkAccelerationStructureCreateInfoKHR asCreateInfo{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR
    };
    asCreateInfo.buffer = accelerationStructure.bufferResource.getBuffer();
    asCreateInfo.size = accelerationStructure.size;
    asCreateInfo.type = asBuildGeometryInfo.type;
    vkCreateAccelerationStructureKHR(
        device, &asCreateInfo, nullptr, &accelerationStructure.handle);

    VkAccelerationStructureDeviceAddressInfoKHR asDeviceAddressInfo{};
    asDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    asDeviceAddressInfo.accelerationStructure = accelerationStructure.handle;
    accelerationStructure.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(
        device, &asDeviceAddressInfo);

    if (asBuildSizesInfo.buildScratchSize > 0) {
        scratchBuffer.createDefaultBuffer(asBuildSizesInfo.buildScratchSize,
            asUsage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, pNext);
    }

    if (asBuildSizesInfo.updateScratchSize > 0) {
        updateBuffer.createDefaultBuffer(asBuildSizesInfo.updateScratchSize,
            asUsage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, pNext);
    }

    asBuildGeometryInfo.dstAccelerationStructure = accelerationStructure.handle;
    asBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.getDeviceAddress();
    build(asBuildGeometryInfo, input.BuildRangeInfo);
}

void AccelerationStructure::update(VkCommandBuffer command,
    VkAccelerationStructureTypeKHR type,
    const Input& input,
    VkBuildAccelerationStructureFlagsKHR buildFlags) {

    VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR
    };
    accelerationStructureBuildGeometryInfo.type = type;
    accelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
    accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | buildFlags;

    accelerationStructureBuildGeometryInfo.geometryCount = uint32_t(input.Geometry.size());
    accelerationStructureBuildGeometryInfo.pGeometries = input.Geometry.data();

    accelerationStructureBuildGeometryInfo.dstAccelerationStructure = accelerationStructure.handle;
    accelerationStructureBuildGeometryInfo.srcAccelerationStructure = accelerationStructure.handle;
    accelerationStructureBuildGeometryInfo.scratchData.deviceAddress = updateBuffer.getDeviceAddress();

    std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> asBuildRangeInfoPtrs;
    for (auto& v : input.BuildRangeInfo) {
        asBuildRangeInfoPtrs.push_back(&v);
    }
    vkCmdBuildAccelerationStructuresKHR(
        command, 1, &accelerationStructureBuildGeometryInfo, asBuildRangeInfoPtrs.data()
    );

    VkMemoryBarrier barrier{
        VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    };
    barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    vkCmdPipelineBarrier(
        command,
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        0, 1, &barrier,
        0, nullptr,
        0, nullptr
    );
}

void AccelerationStructure::build(const VkAccelerationStructureBuildGeometryInfoKHR& asBuildGeometryInfo,
    const std::vector<VkAccelerationStructureBuildRangeInfoKHR>& asBuildRangeInfo) {

    std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> asBuildRangeInfoPtrs;
    for (auto& v : asBuildRangeInfo) {
        asBuildRangeInfoPtrs.push_back(&v);
    }

    VulkanDevice* dev = VulkanDevice::GetInstance();
    auto command = dev->getCommandBuffer(0);
    dev->beginCommand(0);

    vkCmdBuildAccelerationStructuresKHR(
        command, 1, &asBuildGeometryInfo, asBuildRangeInfoPtrs.data()
    );

    VkMemoryBarrier barrier{
        VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    };
    barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    vkCmdPipelineBarrier(
        command,
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        0, 1, &barrier,
        0, nullptr,
        0, nullptr
    );
    dev->endCommand(0);
    dev->submitCommandsDoNotRender(0);
}

