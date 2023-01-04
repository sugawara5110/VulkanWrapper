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
        CoordTf::VECTOR4 emissivePosition[VulkanDevice::numLightMax] = {};//xyz:Pos, w:オンオフ
        CoordTf::VECTOR4 numEmissive = {};//.xのみ
        CoordTf::VECTOR4 GlobalAmbientColor = {};
        CoordTf::VECTOR4 emissiveNo[VulkanDevice::numLightMax] = {};//.xのみ
        CoordTf::VECTOR4 dDirection = {};
        CoordTf::VECTOR4 dLightColor = {};
        CoordTf::VECTOR4 dLightst = {};//x:オンオフ
        CoordTf::VECTOR4 TMin_TMax = {};//x, y
        CoordTf::VECTOR4 maxRecursion = {};//x:, y:maxNumInstance
    };

    std::vector<VulkanBasicPolygonRt::RtData*> rt;
    BufferSetRt  m_instancesBuffer;
    AccelerationStructure m_topLevelAS;
    const static int numDescriptorSet = 5;
    VkDescriptorSetLayout m_dsLayout[numDescriptorSet] = {};
    VkDescriptorSet m_descriptorSet[numDescriptorSet] = {};
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VulkanDevice::ImageSet   m_raytracedImage;
    VkPipeline m_raytracePipeline;
    VulkanDevice::ImageSet instanceIdMap;
    VulkanDevice::ImageSet depthMap;
    VulkanDevice::BufferSet depthMapUp;

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

    SceneParam m_sceneParam;
    VulkanDevice::Uniform<SceneParam>* m_sceneUBO = nullptr;

    struct Material {
        CoordTf::VECTOR4 vDiffuse;
        CoordTf::VECTOR4 vSpeculer;
        CoordTf::VECTOR4 vAmbient;
        CoordTf::VECTOR4 shininess;
        CoordTf::VECTOR4 RefractiveIndex;
        CoordTf::VECTOR4 useAlpha;
        CoordTf::VECTOR4 MaterialType;
        CoordTf::VECTOR4 lightst;
        CoordTf::MATRIX mvp;
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

    void UpdateTLAS(uint32_t QueueIndex, uint32_t comIndex);

    void writeSBTDataAndHit(VulkanBasicPolygonRt::RtData* rt,
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

    void setDirectionLight(bool on, CoordTf::VECTOR3 Color = { 1.0f,1.0f,1.0f }, CoordTf::VECTOR3 Direction = { -0.2f, -1.0f, -1.0f });
    void setTMin_TMax(float min, float max);
    void setGlobalAmbientColor(CoordTf::VECTOR3 Color);

    void Update(int maxRecursion);
    void Render(uint32_t QueueIndex, uint32_t comIndex, bool depthUpdate = false);

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