//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanRendererRt.h                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanRendererRt_Header
#define VulkanRendererRt_Header

#include "AccelerationStructure.h"
#include "VulkanBasicPolygonRt.h"

template<class T> T Align(T size, uint32_t align) {
    return (size + align - 1) & ~static_cast<T>((align - 1));
}

class VulkanRendererRt {

private:
    void CreateSceneTLAS();

    void CreateRaytracedBuffer();

    void CreateRaytracePipeline();

    void CreateShaderBindingTable();

    void CreateLayouts();

    void CreateDescriptorSets();

    void UpdateSceneTLAS(VkCommandBuffer Command);

    struct ShaderBindingTableInfo {
        VkStridedDeviceAddressRegionKHR rgen = { };
        VkStridedDeviceAddressRegionKHR miss = { };
        VkStridedDeviceAddressRegionKHR hit = { };
    };
    struct SceneParam {
        CoordTf::MATRIX projectionToWorld = {};
        CoordTf::VECTOR4 cameraPosition = {};
        CoordTf::VECTOR4 emissivePosition[VulkanDevice::numLightMax] = {};//xyz:Pos, w:オンオフ
        CoordTf::VECTOR4 numEmissive = {};//.xのみ
        CoordTf::VECTOR4 GlobalAmbientColor = {};
        CoordTf::VECTOR4 emissiveNo[VulkanDevice::numLightMax] = {};//.xのみ
        CoordTf::VECTOR4 dDirection = {};
        CoordTf::VECTOR4 dLightColor = {};
        CoordTf::VECTOR4 dLightst = {};//x:オンオフ
        CoordTf::VECTOR4 TMin_TMax = {};//x, y
        uint32_t maxRecursion = 0;
    };

    // ジオメトリ情報.
    std::vector<VulkanBasicPolygonRt::RtData*> rt;
    BufferSetRt  m_instancesBuffer;
    AccelerationStructure m_topLevelAS;
    VkDescriptorSetLayout m_dsLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VulkanDevice::ImageSet   m_raytracedImage;

    VkPipeline m_raytracePipeline;
    VkDescriptorSet m_descriptorSet;

    enum ShaderGroups {
        GroupRayGenShader = 0,
        GroupMissShader0 = 1,
        GroupMissShader1 = 2,
        GroupEmMissShader0 = 3,
        GroupEmMissShader1 = 4,
        GroupHitShader0 = 5,
        GroupHitShader1 = 6,
        GroupEmHitShader0 = 7,
        GroupEmHitShader1 = 8,
        MaxShaderGroup
    };
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_shaderGroups;
    BufferSetRt  m_shaderBindingTable;
    ShaderBindingTableInfo m_sbtInfo;

    std::vector<VkDescriptorImageInfo> textureDifArr;
    std::vector<VkDescriptorImageInfo> textureNorArr;
    std::vector<VkDescriptorImageInfo> textureSpeArr;
    std::vector<VkDescriptorBufferInfo> vertexArr;
    std::vector<VkDescriptorBufferInfo> indexArr;

    SceneParam m_sceneParam;
    VulkanDevice::Uniform<SceneParam> m_sceneUBO;
    std::vector<VulkanBasicPolygonRt::RtMaterial> materialArr;
    BufferSetRt materialUBO;

public:
    std::unique_ptr<VulkanDeviceRt> m_device;

    void Init(std::vector<VulkanBasicPolygonRt::RtData*> rt);
    void destroy();

    void Update();
    void Render();
};

#endif