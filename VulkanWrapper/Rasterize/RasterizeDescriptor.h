//*****************************************************************************************//
//**                                                                                     **//
//**                              RasterizeDescriptor.h                                  **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef RasterizeDescriptor_Header
#define RasterizeDescriptor_Header

#include "../CommonDevice/VulkanSwapchain.h"

class Vulkan2D;
class VulkanBasicPolygon;
class VulkanSkinMesh;

class RasterizeDescriptor {

private:
    friend Vulkan2D;
    friend VulkanBasicPolygon;
    friend VulkanSkinMesh;

    const static uint32_t numSwap = 2;

    static RasterizeDescriptor* ptr;
    static uint32_t numMaxLight;
    uint32_t numLight = 1;
    float attenuation1 = 1.0f;
    float attenuation2 = 0.001f;
    float attenuation3 = 0.001f;

    static const int numDescriptorSet = 4;

    struct ViewProjection {
        CoordTf::MATRIX viewProjection = {};
    };

    struct Instancing {
        CoordTf::MATRIX world;
        CoordTf::VECTOR4 pXpYmXmY;
        CoordTf::VECTOR4 addCol;
        CoordTf::VECTOR4 d2;
        CoordTf::VECTOR4 d3;
    };

    struct MatrixSet_bone {
        CoordTf::MATRIX bone;
    };

    struct Material {
        CoordTf::VECTOR4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
        CoordTf::VECTOR4 specular = { 0.2f, 0.2f, 0.2f, 0.0f };
        CoordTf::VECTOR4 ambient = { 0.0f, 0.0f, 0.0f, 0.0f };
        CoordTf::VECTOR4 viewPos = {};
        CoordTf::VECTOR4 numLight;//ライト数,減衰1,減衰2,減衰3
        CoordTf::VECTOR4 UvSwitch = {};//.x==0:そのまま, 1:切り替え
        CoordTf::VECTOR4 d1;
        CoordTf::VECTOR4 d2;
    };

    struct Light {
        CoordTf::VECTOR4 lightPos = {};
        CoordTf::VECTOR4 lightColor = {};
        CoordTf::VECTOR4 d1;
        CoordTf::VECTOR4 d2;
    };

    VulkanDevice::Uniform<RasterizeDescriptor::Light>* uniformLight[numSwap] = {};
    std::vector<RasterizeDescriptor::Light> matsetLight;

    RasterizeDescriptor();
    RasterizeDescriptor(const RasterizeDescriptor& obj) {}   // コピーコンストラクタ禁止
    void operator=(const RasterizeDescriptor& obj) {}// 代入演算子禁止

    void descriptorAndPipelineLayouts(VkPipelineLayout& pipelineLayout,
        VkDescriptorSetLayout* descSetLayoutArr);

    void descriptorAndPipelineLayouts2D(bool useTexture, VkPipelineLayout& pipelineLayout,
        VkDescriptorSetLayout& descSetLayout);

    VkPipelineCache createPipelineCache();

    VkPipeline createGraphicsPipelineVF(bool useAlpha, bool blending,
        const VkPipelineShaderStageCreateInfo& vshader,
        const VkPipelineShaderStageCreateInfo& fshader,
        const VkVertexInputBindingDescription& bindDesc,
        const VkVertexInputAttributeDescription* attrDescs,
        uint32_t numAttr,
        const VkPipelineLayout& pLayout,
        const VkRenderPass renderPass,
        const VkPipelineCache& pCache);

public:
    static void InstanceCreate(uint32_t NumMaxLight = 256);
    static RasterizeDescriptor* GetInstance();
    static void DeleteInstance();

    ~RasterizeDescriptor();

    void upDescriptorSet(
        VulkanDevice::ImageSet& difTexture,
        VulkanDevice::ImageSet& norTexture,
        VulkanDevice::ImageSet& speTexture,
        VulkanDevice::Uniform<ViewProjection>* vp,
        VulkanDevice::Uniform<Instancing>* uni,
        VulkanDevice::Uniform<MatrixSet_bone>* uni_bone,
        VulkanDevice::Uniform<Material>* material,
        VkDescriptorSet* descriptorSet,
        VkDescriptorSetLayout* descSetLayout,
        uint32_t SwapIndex);

    void upDescriptorSet2D(bool useTexture,
        VulkanDevice::ImageSet& texture,
        VulkanDevice::Uniform<Instancing>* uni,
        VkDescriptorSet& descriptorSet,
        VkDescriptorSetLayout& descSetLayout);

    void setNumLight(uint32_t num);

    void setLightAttenuation(float att1, float att2, float att3);

    void setLight(uint32_t swapIndex, uint32_t LightIndex, CoordTf::VECTOR3 pos, CoordTf::VECTOR3 color);
};

#endif