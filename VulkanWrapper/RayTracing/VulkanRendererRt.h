//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanRendererRt.h                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanRendererRt_Header
#define VulkanRendererRt_Header

#include "AccelerationStructure.h"
#include "VulkanBasicPolygonRt.h"

class VulkanRendererRt {

private:
    struct ShaderBindingTableInfo {
        VkStridedDeviceAddressRegionKHR rgen = { };
        VkStridedDeviceAddressRegionKHR miss = { };
        VkStridedDeviceAddressRegionKHR hit = { };
    };
    struct SceneParam {
        CoordTf::MATRIX projectionToWorld = {};
        CoordTf::VECTOR4 cameraPosition = {};
        CoordTf::VECTOR4 emissivePosition[VulkanDevice::numLightMax] = {};//xyz:Pos, w:ƒIƒ“ƒIƒt
        CoordTf::VECTOR4 numEmissive = {};//.x‚Ì‚Ý
        CoordTf::VECTOR4 GlobalAmbientColor = {};
        CoordTf::VECTOR4 emissiveNo[VulkanDevice::numLightMax] = {};//.x‚Ì‚Ý
        CoordTf::VECTOR4 TMin_TMax = {};//x, y
        CoordTf::VECTOR4 maxRecursion = {};//x:, y:maxNumInstance
    };

    std::vector<VulkanBasicPolygonRt::RtData*> rt;
    BufferSetRt            m_instancesBuffer[VulkanBasicPolygonRt::numSwap] = {};
    AccelerationStructure  m_topLevelAS[VulkanBasicPolygonRt::numSwap] = {};
    const static int       numDescriptorSet = 5;
    VkDescriptorSetLayout  m_dsLayout[VulkanBasicPolygonRt::numSwap][numDescriptorSet] = {};
    VkDescriptorSet        m_descriptorSet[VulkanBasicPolygonRt::numSwap][numDescriptorSet] = {};
    VkPipelineLayout       m_pipelineLayout[VulkanBasicPolygonRt::numSwap] = {};
    VkPipeline             m_raytracePipeline[VulkanBasicPolygonRt::numSwap] = {};

    VulkanDevice::ImageSet m_raytracedImage;
    VulkanDevice::ImageSet instanceIdMap;
    VulkanDevice::ImageSet depthMap;
    VulkanDevice::BufferSet depthMapUp;

    enum ShaderGroups {
        GroupRayGenShader = 0,
        GroupMissShader = 1,
        GroupEmMissShader = 2,
        GroupHitShader = 3,
        GroupEmHitShader = 4,
        MaxShaderGroup
    };
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_shaderGroups;
    BufferSetRt  m_shaderBindingTable[VulkanBasicPolygonRt::numSwap] = {};
    ShaderBindingTableInfo m_sbtInfo[VulkanBasicPolygonRt::numSwap] = {};

    std::vector<VkDescriptorImageInfo> textureDifArr;
    std::vector<VkDescriptorImageInfo> textureNorArr;
    std::vector<VkDescriptorImageInfo> textureSpeArr;

    SceneParam m_sceneParam;
    VulkanDevice::Uniform<SceneParam>* m_sceneUBO = nullptr;

    struct Material {
        CoordTf::MATRIX mvp;
        CoordTf::VECTOR4 vDiffuse;
        CoordTf::VECTOR4 vSpeculer;
        CoordTf::VECTOR4 vAmbient;
        CoordTf::VECTOR4 shininess;
        CoordTf::VECTOR4 RefractiveIndex;
        CoordTf::VECTOR4 MaterialType;
        CoordTf::VECTOR4 addColor;
        CoordTf::VECTOR4 lightst;
    };
    std::vector<Material> materialArr;
    VulkanDevice::Uniform<Material>* materialUBO = nullptr;

    bool testMode[3] = { false,false,false };

    void CreateTLAS(uint32_t QueueIndex, uint32_t comIndex);

    void CreateRaytracedBuffer(uint32_t QueueIndex, uint32_t comIndex);

    void CreateRaytracePipeline();

    void CreateShaderBindingTable();

    void CreateLayouts();

    void CreateDescriptorSets();

    void writeSBTDataAndHit(
        uint32_t SwapIndex,
        VulkanBasicPolygonRt::RtData* rt,
        void* dst, uint64_t hitShaderEntrySize,
        void* hit, uint32_t handleSizeAligned, uint32_t hitHandleSize);

    void DepthMapWrite(uint32_t QueueIndex, uint32_t comIndex);

    void DepthMapUpdate(uint32_t QueueIndex, uint32_t comIndex);

public:
    enum TestMode {
        NormalMap = 0,
        InstanceIdMap = 1,
        DepthMap = 2
    };

    void TestModeOn(TestMode mode);

    void Init(uint32_t QueueIndex, uint32_t comIndex, std::vector<VulkanBasicPolygonRt::RtData*> rt);
    void destroy();

    void setTMin_TMax(float min, float max);
    void setGlobalAmbientColor(CoordTf::VECTOR3 Color);

    void Update(int maxRecursion);
    void UpdateTLAS(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex);
    void Render(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex, bool depthUpdate = false);

    VulkanDevice::ImageSet* getInstanceIdMap() {
        return &instanceIdMap;
    }

    VulkanDevice::ImageSet* getDepthMap() {
        return &depthMap;
    }

    VulkanDevice::ImageSet* getRenderedImage() {
        return &m_raytracedImage;
    }
};

#endif