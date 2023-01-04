//*****************************************************************************************//
//**                                                                                     **//
//**                            AccelerationStructure.h                                  **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef AccelerationStructure_Header
#define AccelerationStructure_Header

#include <memory>
#include <vector>
#include "VulkanDeviceRt.h"

class AccelerationStructure {

private:
    struct {
        VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
        VkDeviceAddress deviceAddress = 0;
        VkDeviceSize size = 0;
        BufferSetRt bufferResource;
    } accelerationStructure;

    BufferSetRt scratchBuffer;//構築用
    BufferSetRt updateBuffer;//更新用

    VkAccelerationStructureGeometryKHR Geometry;
    VkAccelerationStructureBuildRangeInfoKHR BuildRangeInfo;

    void build(uint32_t QueueIndex, uint32_t comIndex,
        VkAccelerationStructureBuildGeometryInfoKHR BuildGeometryInfo,
        VkAccelerationStructureBuildRangeInfoKHR BuildRangeInfo);

public:
    void destroy();
    void destroyScratchBuffer();

    void buildAS(uint32_t QueueIndex, uint32_t comIndex,
        VkAccelerationStructureTypeKHR type,
        VkAccelerationStructureGeometryKHR geometry,
        VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo,
        VkBuildAccelerationStructureFlagsKHR buildFlags);

    void update(uint32_t QueueIndex, uint32_t comIndex,
        VkAccelerationStructureTypeKHR type,
        VkBuildAccelerationStructureFlagsKHR buildFlags);

    VkAccelerationStructureKHR getHandle() const { return accelerationStructure.handle; }
    VkDeviceAddress getDeviceAddress() const { return accelerationStructure.deviceAddress; }
};

#endif