//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanRendererRt.cpp                                     **//
//**                                                                                     **//
//*****************************************************************************************//

#include "VulkanRendererRt.h"
#include "../CommonDevice/VulkanSwapchain.h"
#include "../CommonDevice/SharedShader.h"
#include "Shader/Shader_common.h"
#include "Shader/ShaderNormalTangent.h"
#include "Shader/Shader_hitCom.h"
#include "Shader/Shader_closesthit.h"
#include "Shader/Shader_miss.h"
#include "Shader/Shader_raygen.h"
#include "Shader/Shader_anyHit.h"
#include "Shader/Shader_emissiveHit.h"
#include "Shader/Shader_emissiveMiss.h"
#include "Shader/Shader_traceRay.h"
#include "Shader/Shader_traceRay_OneRay.h"
#include "Shader/Shader_traceRay_PathTracing.h"
#include "Shader/Shader_closesthit_NormalMapTest.h"
#include "Shader/Shader_raygen_In.h"
#include "Shader/Shader_raygenInstanceIdMapTest.h"
#include "Shader/Shader_raygenDepthMapTest.h"
#include "Shader/Shader_hitCom_PathTracing.h"

namespace {
    VkRayTracingShaderGroupCreateInfoKHR createShaderGroupRayGeneration(uint32_t shaderIndex) {

        VkRayTracingShaderGroupCreateInfoKHR shaderGroupCI{
            VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR
        };
        shaderGroupCI.generalShader = shaderIndex;
        shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroupCI.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroupCI.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;
        return shaderGroupCI;
    }

    VkRayTracingShaderGroupCreateInfoKHR createShaderGroupMiss(uint32_t shaderIndex) {

        VkRayTracingShaderGroupCreateInfoKHR shaderGroupCI{
            VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR
        };
        shaderGroupCI.generalShader = shaderIndex;
        shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroupCI.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroupCI.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;
        return shaderGroupCI;
    }

    VkRayTracingShaderGroupCreateInfoKHR createShaderGroupHit(
        uint32_t closestHitShaderIndex,
        uint32_t anyHitShaderIndex = VK_SHADER_UNUSED_KHR,
        uint32_t intersectionShaderIndex = VK_SHADER_UNUSED_KHR) {

        VkRayTracingShaderGroupCreateInfoKHR shaderGroupCI{
            VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR
        };
        shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;

        if (intersectionShaderIndex != VK_SHADER_UNUSED_KHR) {
            shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        }

        shaderGroupCI.generalShader = VK_SHADER_UNUSED_KHR;
        shaderGroupCI.closestHitShader = closestHitShaderIndex;
        shaderGroupCI.anyHitShader = anyHitShaderIndex;
        shaderGroupCI.intersectionShader = intersectionShaderIndex;
        return shaderGroupCI;
    }
}

void VulkanRendererRt::Init(uint32_t QueueIndex, uint32_t comIndex, std::vector<VulkanBasicPolygonRt::RtData*> r,
    char* ImageBasedLightingTextureFileName) {

    m_sceneUBO = NEW VulkanDevice::Uniform<SceneParam>(1);

    rt = r;

    int emissiveCnt = 0;
    int instanceCnt = 0;
    for (int i = 0; i < rt.size(); i++) {
        VulkanBasicPolygonRt::RtData* Rt = rt[i];
        for (int j = 0; j < Rt->instance.size(); j++) {
            textureDifArr.push_back(Rt->texId.difTex.info);
            textureNorArr.push_back(Rt->texId.norTex.info);
            textureSpeArr.push_back(Rt->texId.speTex.info);
            Material m = {};
            VulkanBasicPolygonRt::Instance& ins = Rt->instance[j];
            memcpy(&m.vDiffuse, &Rt->mat, sizeof(VulkanBasicPolygonRt::RtMaterial));
            memcpy(&m.lightst, &ins.lightst, sizeof(CoordTf::VECTOR4));
            memcpy(&m.mvp, &ins.mvp, sizeof(CoordTf::MATRIX));
            memcpy(&m.addColor, &ins.addColor, sizeof(CoordTf::VECTOR4));
            materialArr.push_back(m);

            if (Rt->mat.MaterialType.x == EMISSIVE) {

                CoordTf::VECTOR4 v4{
                ins.world._41,
                ins.world._42,
                ins.world._43,
                ins.lightOn
                };

                m_sceneParam.emissivePosition[emissiveCnt] = v4;
                m_sceneParam.emissiveNo[emissiveCnt].x = (float)instanceCnt;
                m_sceneParam.emissiveNo[emissiveCnt].y = ins.OutlineSize;

                emissiveCnt++;
            }
            instanceCnt++;
        }
    }
    m_sceneParam.numEmissive.x = (float)emissiveCnt;

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{
  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
  nullptr,
    };
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    void* pNext = &memoryAllocateFlagsInfo;
    materialUBO = NEW VulkanDevice::Uniform<Material>((uint32_t)materialArr.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, pNext);
    materialUBO->updateArr(materialArr.data());

    CreateTLAS(QueueIndex, comIndex);

    CreateRaytracedBuffer(QueueIndex, comIndex);

    createImageBasedLightingTexture(QueueIndex, comIndex, ImageBasedLightingTextureFileName);

    CreateLayouts();

    CreateRaytracePipeline();

    CreateShaderBindingTable();

    CreateDescriptorSets();

    CoordTf::MatrixIdentity(&m_sceneParam.ImageBasedLighting_Matrix);
    m_sceneParam.GlobalAmbientColor = { 0.01f,0.01f,0.01f,0.0f };
    m_sceneParam.TMin_TMax.as(0.1f, 1000.0f, 0.0f, 0.0f);
    m_sceneParam.frameReset_DepthRange_NorRange = { 0.0f,0.0001f,0.999f,0.0f };
    m_sceneParam.maxRecursion.x = 1;
    m_sceneParam.maxRecursion.y = (float)instanceCnt;
    m_sceneParam.traceMode = 0;
    m_sceneParam.SeedFrame = 0;
    m_sceneParam.IBL_size = 5000.0f;
    m_sceneParam.useImageBasedLighting = false;
    frameInd = 0;
    frameReset = 0.0f;
    firstRenderer = false;
}

void VulkanRendererRt::destroy() {
    // GPU の処理が全て終わるまでを待機.
    _vkDeviceWaitIdle(VulkanDevice::GetInstance()->getDevice());

    vkUtil::S_DELETE(m_sceneUBO);
    vkUtil::S_DELETE(materialUBO);

    frameIndexMap.destroy();
    normalMap.destroy();
    prevDepthMap.destroy();
    prevNormalMap.destroy();
    ImageBasedLighting.destroy();
    upImageBasedLighting.destroy();

    instanceIdMap.destroy();
    depthMap.destroy();
    depthMapUp.destroy();
    m_raytracedImage.destroy();

    for (uint32_t j = 0; j < VulkanBasicPolygonRt::numSwap; j++) {
        m_instancesBuffer[j].destroy();
        m_topLevelAS[j].destroy();
        m_shaderBindingTable[j].destroy();

        auto device = VulkanDevice::GetInstance()->getDevice();
        _vkDestroyPipeline(device, m_raytracePipeline[j], nullptr);
        _vkDestroyPipelineLayout(device, m_pipelineLayout[j], nullptr);

        VulkanDevice* dev = VulkanDevice::GetInstance();

        for (int i = 0; i < numDescriptorSet; i++) {
            _vkDestroyDescriptorSetLayout(device, m_dsLayout[j][i], nullptr);
            _vkFreeDescriptorSets(device, dev->GetDescriptorPool(), 1, &m_descriptorSet[j][i]);
        }
    }
}

void VulkanRendererRt::setTMin_TMax(float min, float max) {
    m_sceneParam.TMin_TMax.x = min;
    m_sceneParam.TMin_TMax.y = max;
}

void VulkanRendererRt::setGlobalAmbientColor(CoordTf::VECTOR3 Color) {
    m_sceneParam.GlobalAmbientColor = { Color.x,Color.y,Color.z,0.0f };
}

void VulkanRendererRt::Update(int maxRecursion) {
    using namespace CoordTf;
    VulkanDevice* dev = VulkanDevice::GetInstance();
    m_sceneParam.cameraPosition = dev->getCameraViewPos();
    m_sceneParam.maxRecursion.x = (float)maxRecursion;
    m_sceneParam.frameReset_DepthRange_NorRange.x = frameReset;
    frameReset = 0.0f;

    MATRIX VP = dev->getCameraView() * dev->getProjection();
    MatrixTranspose(&VP);
    MatrixInverse(&m_sceneParam.prevViewProjection, &m_sceneParam.projectionToWorld);
    MatrixInverse(&m_sceneParam.projectionToWorld, &VP);

    int emissiveCnt = 0;
    int instanceCnt = 0;
    for (int i = 0; i < rt.size(); i++) {
        VulkanBasicPolygonRt::RtData* Rt = rt[i];
        for (int j = 0; j < Rt->instance.size(); j++) {

            Material m = {};
            VulkanBasicPolygonRt::Instance& ins = Rt->instance[j];
            memcpy(&m.vDiffuse, &Rt->mat, sizeof(VulkanBasicPolygonRt::RtMaterial));
            memcpy(&m.lightst, &ins.lightst, sizeof(CoordTf::VECTOR4));
            memcpy(&m.mvp, &ins.mvp, sizeof(CoordTf::MATRIX));
            memcpy(&m.addColor, &ins.addColor, sizeof(CoordTf::VECTOR4));
            memcpy(&materialArr[instanceCnt], &m, sizeof(Material));

            if (Rt->mat.MaterialType.x == (float)EMISSIVE) {

                CoordTf::VECTOR4 v4{
                ins.world._41,
                ins.world._42,
                ins.world._43,
                ins.lightOn
                };

                m_sceneParam.emissivePosition[emissiveCnt] = v4;
                m_sceneParam.emissiveNo[emissiveCnt].x = (float)instanceCnt;
                m_sceneParam.emissiveNo[emissiveCnt].y = ins.OutlineSize;

                emissiveCnt++;
            }
            instanceCnt++;
        }
    }
    m_sceneParam.numEmissive.x = (float)emissiveCnt;
}

void VulkanRendererRt::DepthMapWrite(uint32_t QueueIndex, uint32_t comIndex) {

    VulkanDevice* dev = VulkanDevice::GetInstance();
    VulkanSwapchain* sw = VulkanSwapchain::GetInstance();

    uint32_t width = sw->getSize().width;
    uint32_t height = sw->getSize().height;

    sw->getDepthImageSet()->barrierResource(QueueIndex, comIndex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_ASPECT_DEPTH_BIT);

    dev->copyBufferToImage(QueueIndex, comIndex, depthMapUp.getBuffer(),
        sw->getDepthImageSet()->getImage(), width, height, VK_IMAGE_ASPECT_DEPTH_BIT);

    sw->getDepthImageSet()->barrierResource(QueueIndex, comIndex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanRendererRt::DepthMapUpdate(uint32_t QueueIndex, uint32_t comIndex) {

    VulkanDevice* dev = VulkanDevice::GetInstance();
    VulkanSwapchain* sw = VulkanSwapchain::GetInstance();

    uint32_t width = sw->getSize().width;
    uint32_t height = sw->getSize().height;

    depthMap.barrierResource(QueueIndex, comIndex, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    dev->copyImageToBuffer(QueueIndex, comIndex, depthMap.getImage(), width, height,
        depthMapUp.getBuffer());

    depthMap.barrierResource(QueueIndex, comIndex, VK_IMAGE_LAYOUT_GENERAL);

    DepthMapWrite(QueueIndex, comIndex);
}

void VulkanRendererRt::Render(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex, bool depthUpdate) {

    VulkanDevice* dev = VulkanDevice::GetInstance();
    VulkanSwapchain* sw = VulkanSwapchain::GetInstance();
    VulkanDevice::CommandObj* com = dev->getCommandObj(QueueIndex);
    auto command = com->getCommandBuffer(comIndex);

    if (!firstRenderer) {
        m_sceneParam.frameReset_DepthRange_NorRange.x = 1.0f;//フレームインデックスバッファ初期化
        firstRenderer = true;
    }

    m_sceneUBO->update(0, &m_sceneParam);
    materialUBO->updateArr(materialArr.data());

    _vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_raytracePipeline[swapIndex]);
    _vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipelineLayout[swapIndex], 0,
        numDescriptorSet, m_descriptorSet[swapIndex], 0, nullptr);

    uint32_t width = sw->getSize().width;
    uint32_t height = sw->getSize().height;

    VkStridedDeviceAddressRegionKHR callable_shader_sbt_entry{};
    vkCmdTraceRaysKHR(
        command,
        &m_sbtInfo[swapIndex].rgen,
        &m_sbtInfo[swapIndex].miss,
        &m_sbtInfo[swapIndex].hit,
        &callable_shader_sbt_entry,
        width, height, 1);

    // レイトレーシング結果画像をバックバッファへコピー.
    m_raytracedImage.barrierResource(QueueIndex, comIndex,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    dev->barrierResource(QueueIndex, comIndex,
        sw->getCurrentImage(),
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

    dev->copyImage(
        QueueIndex, comIndex,
        m_raytracedImage.getImage(), m_raytracedImage.info.imageLayout,
        sw->getCurrentImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        width, height,
        VK_IMAGE_ASPECT_COLOR_BIT);

    m_raytracedImage.barrierResource(QueueIndex, comIndex,
        VK_IMAGE_LAYOUT_GENERAL);

    prevDepthMap.barrierResource(QueueIndex, comIndex,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    prevNormalMap.barrierResource(QueueIndex, comIndex,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    depthMap.barrierResource(QueueIndex, comIndex,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    normalMap.barrierResource(QueueIndex, comIndex,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    dev->copyImage(
        QueueIndex, comIndex,
        depthMap.getImage(), depthMap.info.imageLayout,
        prevDepthMap.getImage(), prevDepthMap.info.imageLayout,
        width, height,
        VK_IMAGE_ASPECT_COLOR_BIT);

    dev->copyImage(
        QueueIndex, comIndex,
        normalMap.getImage(), normalMap.info.imageLayout,
        prevNormalMap.getImage(), prevNormalMap.info.imageLayout,
        width, height,
        VK_IMAGE_ASPECT_COLOR_BIT);

    prevDepthMap.barrierResource(QueueIndex, comIndex,
        VK_IMAGE_LAYOUT_GENERAL);

    prevNormalMap.barrierResource(QueueIndex, comIndex,
        VK_IMAGE_LAYOUT_GENERAL);

    depthMap.barrierResource(QueueIndex, comIndex,
        VK_IMAGE_LAYOUT_GENERAL);

    normalMap.barrierResource(QueueIndex, comIndex,
        VK_IMAGE_LAYOUT_GENERAL);

    if (depthUpdate) {
        DepthMapUpdate(QueueIndex, comIndex);
    }

    if (INT_MAX <= frameInd++) {
        frameInd = 0;
        resetFrameIndex();
    }

    if (INT_MAX <= m_sceneParam.SeedFrame++) {
        m_sceneParam.SeedFrame = 0;
    }
}

void VulkanRendererRt::CreateTLAS(uint32_t QueueIndex, uint32_t comIndex) {

    auto tlas = [this](uint32_t sIndex, uint32_t QueueIndex, uint32_t comIndex) {

        std::vector<VkAccelerationStructureInstanceKHR> asInstances;

        for (int i = 0; i < rt.size(); i++) {
            for (int j = 0; j < rt[i]->instance.size(); j++) {
                asInstances.push_back(rt[i]->instance[j].vkInstance[sIndex]);
            }
        }

        auto instancesBufferSize = sizeof(VkAccelerationStructureInstanceKHR) * asInstances.size();
        auto usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
      nullptr,
        };
        memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        void* pNext = &memoryAllocateFlagsInfo;

        m_instancesBuffer[sIndex].createUploadBuffer(instancesBufferSize, usage, pNext);

        m_instancesBuffer[sIndex].memoryMap(asInstances.data());

        VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
        instanceDataDeviceAddress.deviceAddress = m_instancesBuffer[sIndex].getDeviceAddress();

        VkAccelerationStructureGeometryKHR asGeometry{};
        asGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        asGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        asGeometry.flags = 0;//AnyHitを使う場合0
        asGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        asGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
        asGeometry.geometry.instances.data = instanceDataDeviceAddress;

        VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfo{};
        asBuildRangeInfo.primitiveCount = uint32_t(asInstances.size());
        asBuildRangeInfo.primitiveOffset = 0;
        asBuildRangeInfo.firstVertex = 0;
        asBuildRangeInfo.transformOffset = 0;

        VkBuildAccelerationStructureFlagsKHR buildFlags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
        m_topLevelAS[sIndex].buildAS(QueueIndex, comIndex, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
            asGeometry,
            asBuildRangeInfo,
            buildFlags);

        m_topLevelAS[sIndex].destroyScratchBuffer();
    };

    for (uint32_t i = 0; i < VulkanBasicPolygonRt::numSwap; i++) {
        tlas(i, QueueIndex, comIndex);
    }
}

void VulkanRendererRt::UpdateTLAS(uint32_t swapIndex, uint32_t QueueIndex, uint32_t comIndex) {

    std::vector<VkAccelerationStructureInstanceKHR> asInstances;

    for (int i = 0; i < rt.size(); i++) {
        for (int j = 0; j < rt[i]->instance.size(); j++) {
            asInstances.push_back(rt[i]->instance[j].vkInstance[swapIndex]);
        }
    }

    m_instancesBuffer[swapIndex].memoryMap(asInstances.data());

    VkBuildAccelerationStructureFlagsKHR buildFlags = 0;
    buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    m_topLevelAS[swapIndex].update(
        QueueIndex,
        comIndex,
        VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        buildFlags);
}

void VulkanRendererRt::CreateRaytracedBuffer(uint32_t QueueIndex, uint32_t comIndex) {

    VulkanDevice* dev = VulkanDevice::GetInstance();
    VulkanDeviceRt* devRt = VulkanDeviceRt::getVulkanDeviceRt();
    VulkanSwapchain* sw = VulkanSwapchain::GetInstance();

    // バックバッファと同じフォーマットで作成する.
    auto format = sw->getBackBufferFormat(0).format;

    uint32_t width = sw->getSize().width;
    uint32_t height = sw->getSize().height;

    VkImageUsageFlags usage =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_STORAGE_BIT;
    VkMemoryPropertyFlags devMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_raytracedImage.createImage(width, height, format,
        VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);

    m_raytracedImage.createImageView(format);

    VkFormat mapFormat = VK_FORMAT_R32_SFLOAT;

    instanceIdMap.createImage(width, height, mapFormat,
        VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);

    instanceIdMap.createImageView(mapFormat);

    depthMap.createImage(width, height, mapFormat,
        VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);

    depthMap.createImageView(mapFormat);

    VkDeviceSize imageSize = width * height * 4;
    VkImageUsageFlags usageUp =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    depthMapUp.createUploadBuffer(imageSize, usageUp, nullptr);

    prevDepthMap.createImage(width, height, mapFormat,
        VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);

    prevDepthMap.createImageView(mapFormat);

    frameIndexMap.createImage(width, height, mapFormat,
        VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);

    frameIndexMap.createImageView(mapFormat);

    normalMap.createImage(width, height, format,
        VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);

    normalMap.createImageView(format);

    prevNormalMap.createImage(width, height, format,
        VK_IMAGE_TILING_OPTIMAL, usage, devMemProps);

    prevNormalMap.createImageView(format);

    VulkanDevice::CommandObj* com = dev->getCommandObj(QueueIndex);

    // バッファの状態を変更しておく.
    auto command = com->getCommandBuffer(comIndex);
    com->beginCommand(comIndex);

    m_raytracedImage.barrierResource(QueueIndex, comIndex, VK_IMAGE_LAYOUT_GENERAL);
    instanceIdMap.barrierResource(QueueIndex, comIndex, VK_IMAGE_LAYOUT_GENERAL);
    depthMap.barrierResource(QueueIndex, comIndex, VK_IMAGE_LAYOUT_GENERAL);

    prevDepthMap.barrierResource(QueueIndex, comIndex, VK_IMAGE_LAYOUT_GENERAL);
    frameIndexMap.barrierResource(QueueIndex, comIndex, VK_IMAGE_LAYOUT_GENERAL);
    normalMap.barrierResource(QueueIndex, comIndex, VK_IMAGE_LAYOUT_GENERAL);
    prevNormalMap.barrierResource(QueueIndex, comIndex, VK_IMAGE_LAYOUT_GENERAL);

    com->endCommand(comIndex);
    com->submitCommandsAndWait();
}

void VulkanRendererRt::CreateRaytracePipeline() {

    vkUtil::addChar traceRay = {};
    vkUtil::addChar hitcom = {};
    vkUtil::addChar ray[3] = {};
    vkUtil::addChar clo[6] = {};
    vkUtil::addChar emHit[1] = {};
    vkUtil::addChar aHit[1] = {};
    vkUtil::addChar mis[1] = {};
    vkUtil::addChar emis[1] = {};

    int numMaterial = (int)materialArr.size();

    char replace[255] = {};
    snprintf(replace, sizeof(replace), "%d", numMaterial);

    char* Shader_common_R = vkUtil::changeStr(Shader_common, "replace_NUM_MAT_CB", replace, 1);

    hitcom.addStr(Shader_hitCom, Shader_hitCom_PathTracing);

    traceRay.addStr(Shader_traceRay_OneRay, Shader_traceRay_PathTracing);

    ray[0].addStr(Shader_common_R, Shader_traceRay);
    ray[1].addStr(ray[0].str, Shader_raygen_In);
    if (testMode[InstanceIdMap]) {
        ray[2].addStr(ray[1].str, Shader_raygenInstanceIdMapTest);
    }
    if (testMode[DepthMap]) {
        ray[2].addStr(ray[1].str, Shader_raygenDepthMapTest);
    }
    if (!testMode[InstanceIdMap] && !testMode[DepthMap]) {
        ray[2].addStr(ray[1].str, Shader_raygen);
    }

    clo[0].addStr(Shader_common_R, SharedShader::getShaderCalculateLighting());
    clo[1].addStr(clo[0].str, ShaderNormalTangent);
    clo[2].addStr(clo[1].str, hitcom.str);
    clo[3].addStr(clo[2].str, Shader_traceRay);
    clo[4].addStr(clo[3].str, traceRay.str);
    if (testMode[NormalMap]) {
        clo[5].addStr(clo[4].str, Shader_closesthit_NormalMapTest);
    }
    else {
        clo[5].addStr(clo[4].str, Shader_closesthit);
    }

    emHit[0].addStr(clo[2].str, Shader_emissiveHit);
    aHit[0].addStr(clo[2].str, Shader_anyHit);
    mis[0].addStr(Shader_common_R, Shader_miss);
    emis[0].addStr(Shader_common_R, Shader_emissiveMiss);

    VulkanDevice* dev = VulkanDevice::GetInstance();
    auto rgsStage = dev->createShaderModule("raygen", ray[2].str, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
    auto missStage = dev->createShaderModule("miss", mis[0].str, VK_SHADER_STAGE_MISS_BIT_KHR);
    auto chitStage = dev->createShaderModule("closesthit", clo[5].str, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    auto emMissStage = dev->createShaderModule("emMiss", emis[0].str, VK_SHADER_STAGE_MISS_BIT_KHR);
    auto emHitStage = dev->createShaderModule("emhit", emHit[0].str, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    auto aHitStage = dev->createShaderModule("ahit", aHit[0].str, VK_SHADER_STAGE_ANY_HIT_BIT_KHR);

    vkUtil::ARR_DELETE(Shader_common_R);

    std::vector<VkPipelineShaderStageCreateInfo> stages = {
        rgsStage,
        missStage,
        emMissStage,
        chitStage,
        emHitStage,
        aHitStage
    };

    // stages 配列内での各シェーダーのインデックス.
    const int indexRaygen = 0;
    const int indexMiss = 1;
    const int indexEmMiss = 2;
    const int indexClosestHit = 3;
    const int indexEmHit = 4;
    const int indexAHit = 5;

    // シェーダーグループの生成.
    m_shaderGroups.resize(MaxShaderGroup);
    m_shaderGroups[GroupRayGenShader] = createShaderGroupRayGeneration(indexRaygen);
    m_shaderGroups[GroupMissShader] = createShaderGroupMiss(indexMiss);
    m_shaderGroups[GroupEmMissShader] = createShaderGroupMiss(indexEmMiss);
    m_shaderGroups[GroupHitShader] = createShaderGroupHit(indexClosestHit, indexAHit);
    m_shaderGroups[GroupEmHitShader] = createShaderGroupHit(indexEmHit, indexAHit);

    VulkanDeviceRt* devRt = VulkanDeviceRt::getVulkanDeviceRt();

    // レイトレーシングパイプラインの生成.
    for (uint32_t i = 0; i < VulkanBasicPolygonRt::numSwap; i++) {
        VkRayTracingPipelineCreateInfoKHR rtPipelineCI{};

        rtPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        rtPipelineCI.stageCount = uint32_t(stages.size());
        rtPipelineCI.pStages = stages.data();
        rtPipelineCI.groupCount = uint32_t(m_shaderGroups.size());
        rtPipelineCI.pGroups = m_shaderGroups.data();
        rtPipelineCI.maxPipelineRayRecursionDepth = devRt->GetRayTracingPipelineProperties().maxRayRecursionDepth;
        rtPipelineCI.layout = m_pipelineLayout[i];
        vkCreateRayTracingPipelinesKHR(
            devRt->GetDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE,
            1, &rtPipelineCI, nullptr, &m_raytracePipeline[i]);
    }
    //シェーダーモジュール解放
    for (auto& v : stages) {
        _vkDestroyShaderModule(
            devRt->GetDevice(), v.module, nullptr);
    }
}

void VulkanRendererRt::CreateShaderBindingTable() {

    auto csb = [this](uint32_t sIndex) {

        VulkanDeviceRt* devRt = VulkanDeviceRt::getVulkanDeviceRt();

        auto memProps = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        auto usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        const auto rtPipelineProps = devRt->GetRayTracingPipelineProperties();

        //各エントリサイズ shaderGroupHandleAlignment にアライメント
        const auto handleSize = rtPipelineProps.shaderGroupHandleSize;
        const auto handleAlignment = rtPipelineProps.shaderGroupHandleAlignment;
        auto raygenShaderEntrySize = vkUtil::Align(handleSize, handleAlignment);
        auto missShaderEntrySize = vkUtil::Align(handleSize, handleAlignment);
        auto hitShaderEntrySize = vkUtil::Align(handleSize + sizeof(uint64_t) * 2, handleAlignment);//IndexBufferアドレスとVertexBufferアドレス込み

        const auto raygenShaderCount = 1;
        const auto missShaderCount = 2;
        const auto hitShaderCount = rt.size();

        //各グループのサイズ
        const auto baseAlign = rtPipelineProps.shaderGroupBaseAlignment;
        auto RaygenGroupSize = vkUtil::Align(raygenShaderEntrySize * raygenShaderCount, baseAlign);
        auto MissGroupSize = vkUtil::Align(missShaderEntrySize * missShaderCount, baseAlign);
        auto HitGroupSize = vkUtil::Align(hitShaderEntrySize * hitShaderCount, baseAlign);

        VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{
          VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
          nullptr,
        };
        memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        void* pNext = &memoryAllocateFlagsInfo;

        m_shaderBindingTable[sIndex].createUploadBuffer(RaygenGroupSize + MissGroupSize + HitGroupSize,
            usage, pNext);

        //パイプラインのShaderGroupハンドルを取得
        auto handleSizeAligned = vkUtil::Align(handleSize, handleAlignment);
        auto handleStorageSize = m_shaderGroups.size() * handleSizeAligned;
        std::vector<uint8_t> shaderHandleStorage(handleStorageSize);
        vkGetRayTracingShaderGroupHandlesKHR(devRt->GetDevice(),
            m_raytracePipeline[sIndex],
            0, uint32_t(m_shaderGroups.size()),
            shaderHandleStorage.size(), shaderHandleStorage.data());

        auto device = VulkanDevice::GetInstance()->getDevice();
        auto deviceAddress = m_shaderBindingTable[sIndex].getDeviceAddress();

        void* p = m_shaderBindingTable[sIndex].Map();
        auto dst = static_cast<uint8_t*>(p);

        //RayGeneration
        auto raygen = shaderHandleStorage.data() + handleSizeAligned * GroupRayGenShader;
        memcpy(dst, raygen, handleSize);
        dst += RaygenGroupSize;
        ShaderBindingTableInfo& sbtInfo = m_sbtInfo[sIndex];
        sbtInfo.rgen.deviceAddress = deviceAddress;
        //Raygen は size=strideが必要.
        sbtInfo.rgen.stride = raygenShaderEntrySize;
        sbtInfo.rgen.size = sbtInfo.rgen.stride;

        //Miss
        auto miss = shaderHandleStorage.data() + handleSizeAligned * GroupMissShader;
        auto dstM = dst;
        memcpy(dstM, miss, handleSize);//miss
        dstM += missShaderEntrySize;
        miss += handleSizeAligned;
        memcpy(dstM, miss, handleSize);//emMiss
        dst += MissGroupSize;
        sbtInfo.miss.deviceAddress = deviceAddress + RaygenGroupSize;
        sbtInfo.miss.size = MissGroupSize;
        sbtInfo.miss.stride = missShaderEntrySize;

        //Hit
        //emHitでは頂点データ未使用だがstride揃える関係でとりあえず入れてる
        auto hit = shaderHandleStorage.data() + handleSizeAligned * GroupHitShader;
        auto dstH = dst;

        for (size_t i = 0; i < rt.size(); i++) {
            rt[i]->hitShaderIndex = (uint32_t)i;
            uint32_t stride = handleSizeAligned;
            if (rt[i]->mat.MaterialType.x != EMISSIVE)stride = 0;
            writeSBTDataAndHit(sIndex, rt[i], dstH, hitShaderEntrySize, hit, stride, handleSize);
            dstH += hitShaderEntrySize;
        }

        sbtInfo.hit.deviceAddress = deviceAddress + RaygenGroupSize + MissGroupSize;
        sbtInfo.hit.size = HitGroupSize;
        sbtInfo.hit.stride = hitShaderEntrySize;

        m_shaderBindingTable[sIndex].UnMap();

        };

    for (uint32_t i = 0; i < VulkanBasicPolygonRt::numSwap; i++) {
        csb(i);
    }
}

void VulkanRendererRt::writeSBTDataAndHit(
    uint32_t SwapIndex,
    VulkanBasicPolygonRt::RtData* rt,
    void* dst, uint64_t hitShaderEntrySize,
    void* hit, uint32_t handleSizeAligned, uint32_t hitHandleSize) {

    auto Hit = static_cast<uint8_t*>(hit);
    auto p = static_cast<uint8_t*>(dst);

    //hitShader
    memcpy(p, Hit + handleSizeAligned, hitHandleSize);
    p += hitHandleSize;

    //IndexBuffer
    uint64_t deviceAddr = 0;
    deviceAddr = rt->indexBuf.getDeviceAddress();
    memcpy(p, &deviceAddr, sizeof(deviceAddr));
    p += sizeof(deviceAddr);

    //VertexBuffer
    deviceAddr = rt->vertexBuf[SwapIndex]->getDeviceAddress();
    memcpy(p, &deviceAddr, sizeof(deviceAddr));
}

void VulkanRendererRt::CreateLayouts() {

    std::vector<VkDescriptorSetLayoutBinding> set[numDescriptorSet];

    VkDescriptorSetLayoutBinding layoutAS{};
    layoutAS.binding = 0;
    layoutAS.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    layoutAS.descriptorCount = 1;
    layoutAS.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

    set[0].push_back(layoutAS);

    VkDescriptorSetLayoutBinding layoutRtImage{};
    layoutRtImage.binding = 1;
    layoutRtImage.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutRtImage.descriptorCount = 1;
    layoutRtImage.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    set[0].push_back(layoutRtImage);

    VkDescriptorSetLayoutBinding layoutSceneUBO{};
    layoutSceneUBO.binding = 2;
    layoutSceneUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutSceneUBO.descriptorCount = 1;
    layoutSceneUBO.stageFlags = VK_SHADER_STAGE_ALL;

    set[0].push_back(layoutSceneUBO);

    VkDescriptorSetLayoutBinding layoutInstanceIdMap{};
    layoutInstanceIdMap.binding = 3;
    layoutInstanceIdMap.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutInstanceIdMap.descriptorCount = 1;
    layoutInstanceIdMap.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    set[0].push_back(layoutInstanceIdMap);

    VkDescriptorSetLayoutBinding layoutDepthMap{};
    layoutDepthMap.binding = 4;
    layoutDepthMap.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutDepthMap.descriptorCount = 1;
    layoutDepthMap.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    set[0].push_back(layoutDepthMap);

    VkDescriptorSetLayoutBinding layoutDifTex{};
    layoutDifTex.binding = 0;
    layoutDifTex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutDifTex.descriptorCount = (uint32_t)textureDifArr.size();
    layoutDifTex.stageFlags = VK_SHADER_STAGE_ALL;

    set[1].push_back(layoutDifTex);

    VkDescriptorSetLayoutBinding layoutNorTex{};
    layoutNorTex.binding = 0;
    layoutNorTex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutNorTex.descriptorCount = (uint32_t)textureNorArr.size();
    layoutNorTex.stageFlags = VK_SHADER_STAGE_ALL;

    set[2].push_back(layoutNorTex);

    VkDescriptorSetLayoutBinding layoutSpeTex{};
    layoutSpeTex.binding = 0;
    layoutSpeTex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutSpeTex.descriptorCount = (uint32_t)textureSpeArr.size();
    layoutSpeTex.stageFlags = VK_SHADER_STAGE_ALL;

    set[3].push_back(layoutSpeTex);

    VkDescriptorSetLayoutBinding layoutMaterialCB{};
    layoutMaterialCB.binding = 0;
    layoutMaterialCB.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutMaterialCB.descriptorCount = 1;
    layoutMaterialCB.stageFlags = VK_SHADER_STAGE_ALL;

    set[4].push_back(layoutMaterialCB);

    VkDescriptorSetLayoutBinding layoutPrevDepthMap{};
    layoutPrevDepthMap.binding = 0;
    layoutPrevDepthMap.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutPrevDepthMap.descriptorCount = 1;
    layoutPrevDepthMap.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    set[5].push_back(layoutPrevDepthMap);

    VkDescriptorSetLayoutBinding layoutFrameIndexMap{};
    layoutFrameIndexMap.binding = 1;
    layoutFrameIndexMap.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutFrameIndexMap.descriptorCount = 1;
    layoutFrameIndexMap.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    set[5].push_back(layoutFrameIndexMap);

    VkDescriptorSetLayoutBinding layoutNormalMap{};
    layoutNormalMap.binding = 2;
    layoutNormalMap.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutNormalMap.descriptorCount = 1;
    layoutNormalMap.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    set[5].push_back(layoutNormalMap);

    VkDescriptorSetLayoutBinding layoutPrevNormalMap{};
    layoutPrevNormalMap.binding = 3;
    layoutPrevNormalMap.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutPrevNormalMap.descriptorCount = 1;
    layoutPrevNormalMap.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    set[5].push_back(layoutPrevNormalMap);

    VkDescriptorSetLayoutBinding layoutImageBasedLighting{};
    layoutImageBasedLighting.binding = 4;
    layoutImageBasedLighting.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutImageBasedLighting.descriptorCount = 1;
    layoutImageBasedLighting.stageFlags = VK_SHADER_STAGE_ALL;

    set[5].push_back(layoutImageBasedLighting);

    auto cl = [this, set](uint32_t sIndex) {

        VkDescriptorSetLayoutCreateInfo dsLayout[numDescriptorSet] = {};

        VulkanDevice* dev = VulkanDevice::GetInstance();

        for (int i = 0; i < numDescriptorSet; i++) {
            dsLayout[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            dsLayout[i].bindingCount = static_cast<uint32_t>(set[i].size());
            dsLayout[i].pBindings = set[i].data();

            _vkCreateDescriptorSetLayout(
                dev->getDevice(), &dsLayout[i], nullptr, &m_dsLayout[sIndex][i]);

            m_descriptorSet[sIndex][i] = dev->AllocateDescriptorSet(m_dsLayout[sIndex][i]);
        }

        VkPipelineLayoutCreateInfo pipelineLayoutCI{
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
        };
        pipelineLayoutCI.setLayoutCount = numDescriptorSet;
        pipelineLayoutCI.pSetLayouts = m_dsLayout[sIndex];
        _vkCreatePipelineLayout(dev->getDevice(),
            &pipelineLayoutCI, nullptr, &m_pipelineLayout[sIndex]);
        };

    for (uint32_t i = 0; i < VulkanBasicPolygonRt::numSwap; i++) {
        cl(i);
    }
}

void VulkanRendererRt::CreateDescriptorSets() {

    auto cds = [this](uint32_t sIndex) {

        std::vector<VkAccelerationStructureKHR> asHandles = {
            m_topLevelAS[sIndex].getHandle()
        };

        VkWriteDescriptorSetAccelerationStructureKHR asDescriptor{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR
        };
        asDescriptor.accelerationStructureCount = (uint32_t)asHandles.size();
        asDescriptor.pAccelerationStructures = asHandles.data();

        VkWriteDescriptorSet asWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        asWrite.pNext = &asDescriptor;
        asWrite.dstSet = m_descriptorSet[sIndex][0];
        asWrite.dstBinding = 0;
        asWrite.descriptorCount = 1;
        asWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

        VkWriteDescriptorSet imageWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        imageWrite.dstSet = m_descriptorSet[sIndex][0];
        imageWrite.dstBinding = 1;
        imageWrite.descriptorCount = 1;
        imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        imageWrite.pImageInfo = &m_raytracedImage.info;

        VkWriteDescriptorSet sceneUboWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        sceneUboWrite.dstSet = m_descriptorSet[sIndex][0];
        sceneUboWrite.dstBinding = 2;
        sceneUboWrite.descriptorCount = 1;
        sceneUboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        sceneUboWrite.pBufferInfo = &m_sceneUBO->getBufferSet()->info;

        VkWriteDescriptorSet InstanceIdMapWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        InstanceIdMapWrite.dstSet = m_descriptorSet[sIndex][0];
        InstanceIdMapWrite.dstBinding = 3;
        InstanceIdMapWrite.descriptorCount = 1;
        InstanceIdMapWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        InstanceIdMapWrite.pImageInfo = &instanceIdMap.info;

        VkWriteDescriptorSet depthWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        depthWrite.dstSet = m_descriptorSet[sIndex][0];
        depthWrite.dstBinding = 4;
        depthWrite.descriptorCount = 1;
        depthWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        depthWrite.pImageInfo = &depthMap.info;

        VkWriteDescriptorSet difTexImageWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        difTexImageWrite.dstSet = m_descriptorSet[sIndex][1];
        difTexImageWrite.dstBinding = 0;
        difTexImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        difTexImageWrite.descriptorCount = (uint32_t)textureDifArr.size();
        difTexImageWrite.pImageInfo = textureDifArr.data();

        VkWriteDescriptorSet norTexImageWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        norTexImageWrite.dstSet = m_descriptorSet[sIndex][2];
        norTexImageWrite.dstBinding = 0;
        norTexImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        norTexImageWrite.descriptorCount = (uint32_t)textureNorArr.size();
        norTexImageWrite.pImageInfo = textureNorArr.data();

        VkWriteDescriptorSet speTexImageWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        speTexImageWrite.dstSet = m_descriptorSet[sIndex][3];
        speTexImageWrite.dstBinding = 0;
        speTexImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        speTexImageWrite.descriptorCount = (uint32_t)textureSpeArr.size();
        speTexImageWrite.pImageInfo = textureSpeArr.data();

        VkWriteDescriptorSet materialWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        materialWrite.dstSet = m_descriptorSet[sIndex][4];
        materialWrite.dstBinding = 0;
        materialWrite.descriptorCount = 1;
        materialWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        materialWrite.pBufferInfo = &materialUBO->getBufferSet()->info;

        VkWriteDescriptorSet prevDepthWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        prevDepthWrite.dstSet = m_descriptorSet[sIndex][5];
        prevDepthWrite.dstBinding = 0;
        prevDepthWrite.descriptorCount = 1;
        prevDepthWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        prevDepthWrite.pImageInfo = &prevDepthMap.info;

        VkWriteDescriptorSet frameIndexWrite{
           VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        frameIndexWrite.dstSet = m_descriptorSet[sIndex][5];
        frameIndexWrite.dstBinding = 1;
        frameIndexWrite.descriptorCount = 1;
        frameIndexWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        frameIndexWrite.pImageInfo = &frameIndexMap.info;

        VkWriteDescriptorSet normalMapWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        normalMapWrite.dstSet = m_descriptorSet[sIndex][5];
        normalMapWrite.dstBinding = 2;
        normalMapWrite.descriptorCount = 1;
        normalMapWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        normalMapWrite.pImageInfo = &normalMap.info;

        VkWriteDescriptorSet prevNormalMapWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        prevNormalMapWrite.dstSet = m_descriptorSet[sIndex][5];
        prevNormalMapWrite.dstBinding = 3;
        prevNormalMapWrite.descriptorCount = 1;
        prevNormalMapWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        prevNormalMapWrite.pImageInfo = &prevNormalMap.info;

        VkWriteDescriptorSet ImageBasedLightingWrite{
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
        };
        ImageBasedLightingWrite.dstSet = m_descriptorSet[sIndex][5];
        ImageBasedLightingWrite.dstBinding = 4;
        ImageBasedLightingWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        ImageBasedLightingWrite.descriptorCount = 1;
        ImageBasedLightingWrite.pImageInfo = &ImageBasedLighting.info;

        std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
            asWrite, imageWrite, sceneUboWrite, InstanceIdMapWrite, depthWrite,
            difTexImageWrite,
            norTexImageWrite,
            speTexImageWrite,
            materialWrite,
            prevDepthWrite, frameIndexWrite, normalMapWrite, prevNormalMapWrite, ImageBasedLightingWrite
        };

        VulkanDeviceRt* devRt = VulkanDeviceRt::getVulkanDeviceRt();

        _vkUpdateDescriptorSets(
            devRt->GetDevice(),
            uint32_t(writeDescriptorSets.size()),
            writeDescriptorSets.data(),
            0,
            nullptr);
        };

    for (uint32_t i = 0; i < VulkanBasicPolygonRt::numSwap; i++) {
        cds(i);
    }
}

void VulkanRendererRt::TestModeOn(TestMode mode) {
    testMode[mode] = true;
}

void VulkanRendererRt::setGIparameter(TraceMode mode) {
    m_sceneParam.traceMode = mode;
}

void VulkanRendererRt::resetFrameIndex() {
    frameReset = 1.0f;
}

void VulkanRendererRt::set_DepthRange_NorRange(float DepthRange, float NorRange) {
    m_sceneParam.frameReset_DepthRange_NorRange.y = DepthRange;
    m_sceneParam.frameReset_DepthRange_NorRange.z = NorRange;
}

void VulkanRendererRt::useImageBasedLightingTexture(bool on) {
    m_sceneParam.useImageBasedLighting = on;
}

void VulkanRendererRt::setImageBasedLighting_size(float size) {
    m_sceneParam.IBL_size = size;
}

void VulkanRendererRt::setImageBasedLighting_Direction(CoordTf::VECTOR3 dir) {
    using namespace CoordTf;
    MATRIX rotZ, rotY, rotX;
    MatrixRotationZ(&rotZ, dir.z);
    MatrixRotationY(&rotY, dir.y);
    MatrixRotationX(&rotX, dir.x);
    m_sceneParam.ImageBasedLighting_Matrix = rotZ * rotY * rotX;
}

void VulkanRendererRt::createImageBasedLightingTexture(uint32_t QueueIndex, uint32_t comIndex, char* FileName) {
    VulkanDevice* d = VulkanDevice::GetInstance();
    int32_t tInd = d->getTextureNo(FileName);
    if (tInd < 0)tInd = d->numTextureMax + 1;

    VulkanDevice::Texture tex = d->getTexture(tInd);

    d->createVkTexture(ImageBasedLighting, QueueIndex, comIndex, tex, upImageBasedLighting);
    if (tex.use_movie) {
        tex.defArr.push_back(&ImageBasedLighting);
        tex.upArr.push_back(&upImageBasedLighting);
    }
}