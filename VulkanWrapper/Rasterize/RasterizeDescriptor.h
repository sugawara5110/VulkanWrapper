//*****************************************************************************************//
//**                                                                                     **//
//**                              RasterizeDescriptor.h                                  **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef RasterizeDescriptor_Header
#define RasterizeDescriptor_Header

#include "../CommonDevice/VulkanDevice.h"

class Vulkan2D;
class VulkanBasicPolygon;
class VulkanSkinMesh;

class RasterizeDescriptor {

private:
    friend Vulkan2D;
    friend VulkanBasicPolygon;
    friend VulkanSkinMesh;

    static RasterizeDescriptor* ptr;
    CoordTf::VECTOR4 lightPos[VulkanDevice::numLightMax];
    CoordTf::VECTOR4 lightColor[VulkanDevice::numLightMax];
    uint32_t numLight = 1;
    float attenuation1 = 1.0f;
    float attenuation2 = 0.001f;
    float attenuation3 = 0.001f;

    struct MatrixSet {
        CoordTf::MATRIX world;
        CoordTf::MATRIX mvp;
        CoordTf::MATRIX bone[VulkanDevice::numBoneMax];
    };

    struct Material {
        CoordTf::VECTOR4 diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
        CoordTf::VECTOR4 specular = { 0.0f, 0.0f, 0.0f, 0.0f };
        CoordTf::VECTOR4 ambient = { 0.0f, 0.0f, 0.0f, 0.0f };
        CoordTf::VECTOR4 viewPos;
        CoordTf::VECTOR4 lightPos[VulkanDevice::numLightMax];
        CoordTf::VECTOR4 lightColor[VulkanDevice::numLightMax];
        CoordTf::VECTOR4 numLight;//ライト数,減衰1,減衰2,減衰3
        CoordTf::VECTOR4 UvSwitch = {};//.x==0:そのまま, 1:切り替え
    };

    struct MatrixSet2D {
        CoordTf::VECTOR2 world;
    };

    RasterizeDescriptor() {}
    RasterizeDescriptor(const RasterizeDescriptor& obj) {}   // コピーコンストラクタ禁止
    void operator=(const RasterizeDescriptor& obj) {}// 代入演算子禁止

    void descriptorAndPipelineLayouts(bool useTexture, VkPipelineLayout& pipelineLayout,
        VkDescriptorSetLayout& descSetLayout);

    void descriptorAndPipelineLayouts2D(bool useTexture, VkPipelineLayout& pipelineLayout,
        VkDescriptorSetLayout& descSetLayout);

    void createDescriptorPool(bool useTexture, VkDescriptorPool& descPool);

    void createDescriptorPool2D(bool useTexture, VkDescriptorPool& descPool);

    VkPipelineCache createPipelineCache();

    VkPipeline createGraphicsPipelineVF(bool useAlpha,
        const VkPipelineShaderStageCreateInfo& vshader,
        const VkPipelineShaderStageCreateInfo& fshader,
        const VkVertexInputBindingDescription& bindDesc,
        const VkVertexInputAttributeDescription* attrDescs,
        uint32_t numAttr,
        const VkPipelineLayout& pLayout,
        const VkRenderPass renderPass,
        const VkPipelineCache& pCache);

public:
    static void InstanceCreate();
    static RasterizeDescriptor* GetInstance();
    static void DeleteInstance();

    uint32_t upDescriptorSet(bool useTexture,
        VulkanDevice::VkTexture& difTexture,
        VulkanDevice::VkTexture& norTexture,
        VulkanDevice::VkTexture& speTexture,
        VulkanDevice::Uniform<MatrixSet>& uni,
        VulkanDevice::Uniform<Material>& material,
        VkDescriptorSet& descriptorSet, VkDescriptorPool& descPool,
        VkDescriptorSetLayout& descSetLayout);

    uint32_t upDescriptorSet2D(bool useTexture, VulkanDevice::VkTexture& texture,
        VulkanDevice::Uniform<MatrixSet2D>& uni,
        VkDescriptorSet& descriptorSet, VkDescriptorPool& descPool,
        VkDescriptorSetLayout& descSetLayout);

    void setNumLight(uint32_t num);

    void setLightAttenuation(float att1, float att2, float att3);

    void setLight(uint32_t index, CoordTf::VECTOR3 pos, CoordTf::VECTOR3 color);
};

#endif