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
        CoordTf::MATRIX prevViewProjection = {};
        CoordTf::MATRIX projectionToWorld = {};
        CoordTf::MATRIX ImageBasedLighting_Matrix = {};
        CoordTf::VECTOR4 cameraPosition = {};
        CoordTf::VECTOR4 emissivePosition[VulkanDevice::numLightMax] = {};//xyz:Pos, w:オンオフ
        CoordTf::VECTOR4 numEmissive = {};//.xのみ
        CoordTf::VECTOR4 GlobalAmbientColor = {};
        CoordTf::VECTOR4 emissiveNo[VulkanDevice::numLightMax] = {};//x:emissiveNo, y:OutlineSize
        CoordTf::VECTOR4 TMin_TMax = {};//x, y
        CoordTf::VECTOR4 frameReset_DepthRange_NorRange;//.x:フレームインデックスリセット(1.0でリセット), .y:深度レンジ, .z:法線レンジ
        CoordTf::VECTOR4 maxRecursion = {};//x:, y:maxNumInstance
        uint32_t traceMode;
        uint32_t SeedFrame;
        float IBL_size;
        bool useImageBasedLighting;
    };

    std::vector<VulkanBasicPolygonRt::RtData*> rt;
    BufferSetRt            m_instancesBuffer[VulkanBasicPolygonRt::numSwap] = {};
    AccelerationStructure  m_topLevelAS[VulkanBasicPolygonRt::numSwap] = {};
    const static int       numDescriptorSet = 6;
    VkDescriptorSetLayout  m_dsLayout[VulkanBasicPolygonRt::numSwap][numDescriptorSet] = {};
    VkDescriptorSet        m_descriptorSet[VulkanBasicPolygonRt::numSwap][numDescriptorSet] = {};
    VkPipelineLayout       m_pipelineLayout[VulkanBasicPolygonRt::numSwap] = {};
    VkPipeline             m_raytracePipeline[VulkanBasicPolygonRt::numSwap] = {};
    uint32_t frameInd;
    float frameReset;
    bool firstRenderer;

    VulkanDevice::ImageSet m_raytracedImage;
    VulkanDevice::ImageSet instanceIdMap;
    VulkanDevice::ImageSet depthMap;
    VulkanDevice::BufferSet depthMapUp;

    VulkanDevice::ImageSet prevDepthMap;
    VulkanDevice::ImageSet frameIndexMap;
    VulkanDevice::ImageSet normalMap;
    VulkanDevice::ImageSet prevNormalMap;

    VulkanDevice::ImageSet ImageBasedLighting;
    VulkanDevice::BufferSet upImageBasedLighting;

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
        CoordTf::VECTOR4 RefractiveIndex_roughness;
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

    void createImageBasedLightingTexture(uint32_t QueueIndex, uint32_t comIndex, char* FileName);

public:
    enum TestMode {
        NormalMap = 0,
        InstanceIdMap = 1,
        DepthMap = 2
    };

    enum TraceMode {
        ONE_RAY = 0,
        PathTracing = 1,
        NEE = 2
    };

    void TestModeOn(TestMode mode);

    void Init(uint32_t QueueIndex, uint32_t comIndex, std::vector<VulkanBasicPolygonRt::RtData*> rt,
        char* ImageBasedLightingTextureFileName = nullptr);

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

    void setGIparameter(TraceMode mode = ONE_RAY);
    void resetFrameIndex();
    void set_DepthRange_NorRange(float DepthRange, float NorRange);
    void useImageBasedLightingTexture(bool on);
    void setImageBasedLighting_size(float size);
    void setImageBasedLighting_Direction(CoordTf::VECTOR3 dir);
};

#endif