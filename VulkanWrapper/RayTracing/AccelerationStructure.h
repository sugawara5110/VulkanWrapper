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

    void build(const VkAccelerationStructureBuildGeometryInfoKHR& BuildGeometryInfo,
        const std::vector<VkAccelerationStructureBuildRangeInfoKHR>& BuildRangeInfo);

public:
    void destroy();
    void destroyScratchBuffer();

    struct Input {
        std::vector<VkAccelerationStructureGeometryKHR> Geometry;
        std::vector<VkAccelerationStructureBuildRangeInfoKHR> BuildRangeInfo;
    };

    void buildAS(VkAccelerationStructureTypeKHR type,
        const Input& input,
        VkBuildAccelerationStructureFlagsKHR buildFlags);

    void update(
        VkCommandBuffer command,
        VkAccelerationStructureTypeKHR type,
        const Input& input,
        VkBuildAccelerationStructureFlagsKHR buildFlags = 0);

    VkAccelerationStructureKHR getHandle() const { return accelerationStructure.handle; }
    VkDeviceAddress getDeviceAddress() const { return accelerationStructure.deviceAddress; }
};

#endif