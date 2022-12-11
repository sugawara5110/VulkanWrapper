//*****************************************************************************************//
//**                                                                                     **//
//**                            VulkanRendererRt.cpp                                     **//
//**                                                                                     **//
//*****************************************************************************************//

#include "VulkanRendererRt.h"
#include "Shader/Shader_common.h"
#include "Shader/ShaderCalculateLighting.h"
#include "Shader/ShaderNormalTangent.h"
#include "Shader/Shader_hitCom.h"
#include "Shader/Shader_closesthit.h"
#include "Shader/Shader_miss.h"
#include "Shader/Shader_raygen.h"
#include "Shader/Shader_anyHit.h"
#include "Shader/Shader_emissiveHit.h"
#include "Shader/Shader_emissiveMiss.h"
#include "Shader/Shader_traceRay.h"
#include "Shader/Shader_location_Index.h"
#include "Shader/Shader_closesthit_NormalMapTest.h"
#include "Shader/Shader_raygen_In.h"
#include "Shader/Shader_raygenInstanceIdMapTest.h"
#include "Shader/Shader_raygenDepthMapTest.h"

namespace {
    char* changeStr(char* srcStr, char* target, char* replacement) {
        uint32_t ln = (uint32_t)strlen(srcStr);
        uint32_t taln = (uint32_t)strlen(target);
        uint32_t reln = (uint32_t)strlen(replacement);
        uint32_t retln = ln - taln + reln;

        char* ret = new char[retln + 1];
        int retCnt = 0;
        for (uint32_t i = 0; i < ln; i++) {
            if (!strncmp(&srcStr[i], target, sizeof(char) * taln)) {
                memcpy(&ret[retCnt], replacement, sizeof(char) * reln);
                retCnt += reln;
                i += (taln - 1);
            }
            else {
                ret[retCnt] = srcStr[i];
                retCnt++;
            }
        }
        ret[retln] = '\0';
        return ret;
    }

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

void VulkanRendererRt::Init(uint32_t comIndex, std::vector<VulkanBasicPolygonRt::RtData*> r) {

    VulkanDevice::GetInstance()->createUniform(m_sceneUBO);

    rt = r;

    int emissiveCnt = 0;
    int instanceCnt = 0;
    for (int i = 0; i < rt.size(); i++) {
        for (int j = 0; j < rt[i]->instance.size(); j++) {
            textureDifArr.push_back(rt[i]->texId.difTex.image.info);
            textureNorArr.push_back(rt[i]->texId.norTex.image.info);
            textureSpeArr.push_back(rt[i]->texId.speTex.image.info);
            Material m = {};
            memcpy(&m, &rt[i]->mat, sizeof(VulkanBasicPolygonRt::RtMaterial));
            memcpy(&m.lightst, &rt[i]->instance[j].lightst, sizeof(CoordTf::VECTOR4));
            memcpy(&m.mvp, &rt[i]->instance[j].mvp, sizeof(CoordTf::MATRIX));
            materialArr.push_back(m);

            if (rt[i]->mat.MaterialType.x == (float)EMISSIVE) {

                CoordTf::VECTOR4 v4{
                rt[i]->instance[j].world._41,
                rt[i]->instance[j].world._42,
                rt[i]->instance[j].world._43,
                rt[i]->instance[j].lightOn
                };

                memcpy(&m_sceneParam.emissivePosition[emissiveCnt],
                    &v4,
                    sizeof(CoordTf::VECTOR4));
                m_sceneParam.emissiveNo[emissiveCnt].x = (float)instanceCnt;
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
    materialUBO.createUploadBuffer(materialArr.size() * sizeof(Material),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, pNext);
    materialUBO.memoryMap(materialArr.data());

    CreateTLAS(comIndex);

    CreateRaytracedBuffer(comIndex);

    CreateLayouts();

    CreateRaytracePipeline();

    CreateShaderBindingTable();

    CreateDescriptorSets();

    m_sceneParam.dLightColor = { 1.0f,1.0f,1.0f,0.0f };
    m_sceneParam.dDirection = { -0.2f, -1.0f, -1.0f, 0.0f };
    m_sceneParam.GlobalAmbientColor = { 0.01f,0.01f,0.01f,0.0f };
    m_sceneParam.dLightst.x = 0.0f;//off
    m_sceneParam.TMin_TMax.as(0.1f, 1000.0f, 0.0f, 0.0f);
    m_sceneParam.maxRecursion.x = 1;
    m_sceneParam.maxRecursion.y = (float)instanceCnt;
}

void VulkanRendererRt::destroy() {
    // GPU の処理が全て終わるまでを待機.
    vkDeviceWaitIdle(VulkanDevice::GetInstance()->getDevice());

    m_sceneUBO.buf.destroy();
    materialUBO.destroy();
    m_instancesBuffer.destroy();
    m_topLevelAS.destroy();

    m_raytracedImage.destroy();
    m_shaderBindingTable.destroy();
    instanceIdMap.destroy();
    depthMap.destroy();

    auto device = VulkanDevice::GetInstance()->getDevice();
    vkDestroyPipeline(device, m_raytracePipeline, nullptr);
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);

    for (int i = 0; i < numDescriptorSet; i++) {
        vkDestroyDescriptorSetLayout(device, m_dsLayout[i], nullptr);
        vkFreeDescriptorSets(device, m_device->GetDescriptorPool(), 1, &m_descriptorSet[i]);
    }

    m_device->destroy();
    m_device.reset();
}

void VulkanRendererRt::setDirectionLight(bool on, CoordTf::VECTOR3 Color, CoordTf::VECTOR3 Direction) {
    m_sceneParam.dLightColor = { Color.x,Color.y,Color.z,0.0f };
    m_sceneParam.dDirection = { Direction.x, Direction.y, Direction.z, 0.0f };
    m_sceneParam.dLightst.x = 0.0f;
    if (on)m_sceneParam.dLightst.x = 1.0f;
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

    MATRIX VP = dev->getCameraView() * dev->getProjection();
    MatrixTranspose(&VP);
    MatrixInverse(&m_sceneParam.projectionToWorld, &VP);

    int emissiveCnt = 0;
    int instanceCnt = 0;
    for (int i = 0; i < rt.size(); i++) {
        for (int j = 0; j < rt[i]->instance.size(); j++) {

            Material m = {};
            memcpy(&m, &rt[i]->mat, sizeof(VulkanBasicPolygonRt::RtMaterial));
            memcpy(&m.lightst, &rt[i]->instance[j].lightst, sizeof(CoordTf::VECTOR4));
            memcpy(&m.mvp, &rt[i]->instance[j].mvp, sizeof(CoordTf::MATRIX));
            memcpy(&materialArr[instanceCnt], &m, sizeof(Material));

            if (rt[i]->mat.MaterialType.x == (float)EMISSIVE) {

                CoordTf::VECTOR4 v4{
                rt[i]->instance[j].world._41,
                rt[i]->instance[j].world._42,
                rt[i]->instance[j].world._43,
                rt[i]->instance[j].lightOn
                };

                memcpy(&m_sceneParam.emissivePosition[emissiveCnt],
                    &v4,
                    sizeof(CoordTf::VECTOR4));
                m_sceneParam.emissiveNo[emissiveCnt].x = (float)instanceCnt;
                emissiveCnt++;
            }
            instanceCnt++;
        }
    }
    m_sceneParam.numEmissive.x = (float)emissiveCnt;
}

void VulkanRendererRt::Render(uint32_t comIndex) {

    VulkanDevice* dev = VulkanDevice::GetInstance();
    auto command = dev->getCommandBuffer(comIndex);

    UpdateTLAS(comIndex);

    memcpy(&m_sceneUBO.uni, &m_sceneParam, sizeof(m_sceneParam));
    dev->updateUniform(m_sceneUBO);
    materialUBO.memoryMap(materialArr.data());

    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_raytracePipeline);
    vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipelineLayout, 0,
        numDescriptorSet, m_descriptorSet, 0, nullptr);

    uint32_t width = dev->getSwapchainObj()->getSize().width;
    uint32_t height = dev->getSwapchainObj()->getSize().height;

    VkStridedDeviceAddressRegionKHR callable_shader_sbt_entry{};
    vkCmdTraceRaysKHR(
        command,
        &m_sbtInfo.rgen,
        &m_sbtInfo.miss,
        &m_sbtInfo.hit,
        &callable_shader_sbt_entry,
        width, height, 1);

    // レイトレーシング結果画像をバックバッファへコピー.
    VkImageCopy region{};
    region.extent = { width, height, 1 };
    region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };

    m_raytracedImage.barrierResource(comIndex, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    dev->barrierResource(comIndex,
        dev->getSwapchainObj()->getCurrentImage(),
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vkCmdCopyImage(command,
        m_raytracedImage.getImage(), m_raytracedImage.info.imageLayout,
        dev->getSwapchainObj()->getCurrentImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region);

    m_raytracedImage.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL);
}

void VulkanRendererRt::CreateTLAS(uint32_t comIndex) {

    std::vector<VkAccelerationStructureInstanceKHR> asInstances;

    for (int i = 0; i < rt.size(); i++) {
        for (int j = 0; j < rt[i]->instance.size(); j++) {
            asInstances.push_back(rt[i]->instance[j].vkInstance);
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


    m_instancesBuffer.createUploadBuffer(instancesBufferSize, usage, pNext);

    m_instancesBuffer.memoryMap(asInstances.data());

    VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
    instanceDataDeviceAddress.deviceAddress = m_instancesBuffer.getDeviceAddress();

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
    m_topLevelAS.buildAS(comIndex, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        asGeometry,
        asBuildRangeInfo,
        buildFlags);

    m_topLevelAS.destroyScratchBuffer();
}

void VulkanRendererRt::UpdateTLAS(uint32_t comIndex) {

    std::vector<VkAccelerationStructureInstanceKHR> asInstances;

    for (int i = 0; i < rt.size(); i++) {
        for (int j = 0; j < rt[i]->instance.size(); j++) {
            asInstances.push_back(rt[i]->instance[j].vkInstance);
        }
    }

    m_instancesBuffer.memoryMap(asInstances.data());

    VkBuildAccelerationStructureFlagsKHR buildFlags = 0;
    buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    m_topLevelAS.update(
        comIndex,
        VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        buildFlags);
}

void VulkanRendererRt::CreateRaytracedBuffer(uint32_t comIndex) {

    VulkanDevice* dev = VulkanDevice::GetInstance();
    // バックバッファと同じフォーマットで作成する.
    auto format = m_device->GetBackBufferFormat().format;

    uint32_t width = dev->getSwapchainObj()->getSize().width;
    uint32_t height = dev->getSwapchainObj()->getSize().height;

    VkImageUsageFlags usage =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_STORAGE_BIT;
    VkMemoryPropertyFlags devMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_raytracedImage.createImage(width, height, format,
        VK_IMAGE_TILING_OPTIMAL, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT, devMemProps);

    m_raytracedImage.createImageView(format,
        VK_IMAGE_ASPECT_COLOR_BIT);

    VkFormat mapFormat = VK_FORMAT_R32_SFLOAT;

    instanceIdMap.createImage(width, height, mapFormat,
        VK_IMAGE_TILING_OPTIMAL, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT, devMemProps);

    instanceIdMap.createImageView(mapFormat,
        VK_IMAGE_ASPECT_COLOR_BIT);

    depthMap.createImage(width, height, mapFormat,
        VK_IMAGE_TILING_OPTIMAL, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT, devMemProps);

    depthMap.createImageView(mapFormat,
        VK_IMAGE_ASPECT_COLOR_BIT);

    // バッファの状態を変更しておく.
    auto command = dev->getCommandBuffer(comIndex);
    dev->beginCommand(comIndex);

    m_raytracedImage.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL);
    instanceIdMap.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL);
    depthMap.barrierResource(comIndex, VK_IMAGE_LAYOUT_GENERAL);

    dev->endCommand(comIndex);
    dev->submitCommandsDoNotRender(comIndex);
}

void VulkanRendererRt::CreateRaytracePipeline() {

    vkUtil::addChar ray[2];
    vkUtil::addChar miss0[1];
    vkUtil::addChar miss1[1];
    vkUtil::addChar clo0[6];
    vkUtil::addChar clo1[3];
    vkUtil::addChar emMiss0[1];
    vkUtil::addChar emMiss1[1];
    vkUtil::addChar emHit0[2];
    vkUtil::addChar emHit1[2];
    vkUtil::addChar aHit[1];

    int numMaterial = (int)materialArr.size();

    char replace[255] = {};
    snprintf(replace, sizeof(replace), "%d", numMaterial);

    char* Shader_common_R = changeStr(Shader_common, "replace_NUM_MAT_CB", replace);

    ray[0].addStr(Shader_common_R, Shader_raygen_In);
    if (testMode[InstanceIdMap]) {
        ray[1].addStr(ray[0].str, Shader_raygenInstanceIdMapTest);
    }
    if (testMode[DepthMap]) {
        ray[1].addStr(ray[0].str, Shader_raygenDepthMapTest);
    }
    if (!testMode[InstanceIdMap] && !testMode[DepthMap]) {
        ray[1].addStr(ray[0].str, Shader_raygen);
    }

    miss0[0].addStr(Shader_location_Index_In_0_Miss, Shader_miss);
    miss1[0].addStr(Shader_location_Index_In_1_Miss, Shader_miss);

    clo0[0].addStr(Shader_common_R, ShaderCalculateLighting);
    clo0[1].addStr(clo0[0].str, ShaderNormalTangent);
    clo0[2].addStr(clo0[1].str, Shader_hitCom);
    clo0[3].addStr(clo0[2].str, Shader_location_Index_Clo_0);
    clo0[4].addStr(clo0[3].str, Shader_traceRay);
    if (testMode[NormalMap]) {
        clo0[5].addStr(clo0[4].str, Shader_closesthit_NormalMapTest);
    }
    else {
        clo0[5].addStr(clo0[4].str, Shader_closesthit);
    }

    clo1[0].addStr(clo0[2].str, Shader_location_Index_Clo_1);
    clo1[1].addStr(clo1[0].str, Shader_traceRay);
    clo1[2].addStr(clo1[1].str, Shader_closesthit);

    emMiss0[0].addStr(Shader_location_Index_In_1_Miss, Shader_emissiveMiss);
    emMiss1[0].addStr(Shader_location_Index_In_0_Miss, Shader_emissiveMiss);

    emHit0[0].addStr(clo0[2].str, Shader_location_Index_In_1);
    emHit0[1].addStr(emHit0[0].str, Shader_emissiveHit);

    emHit1[0].addStr(clo0[2].str, Shader_location_Index_In_0);
    emHit1[1].addStr(emHit1[0].str, Shader_emissiveHit);

    aHit[0].addStr(clo0[2].str, Shader_anyHit);

    VulkanDevice* dev = VulkanDevice::GetInstance();
    auto rgsStage = dev->createShaderModule("raygen", ray[1].str, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
    auto miss0Stage = dev->createShaderModule("miss0", miss0[0].str, VK_SHADER_STAGE_MISS_BIT_KHR);
    auto miss1Stage = dev->createShaderModule("miss1", miss1[0].str, VK_SHADER_STAGE_MISS_BIT_KHR);
    auto chit0Stage = dev->createShaderModule("closesthit0", clo0[5].str, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    auto chit1Stage = dev->createShaderModule("closesthit1", clo1[2].str, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    auto emMiss0Stage = dev->createShaderModule("emMiss0", emMiss0[0].str, VK_SHADER_STAGE_MISS_BIT_KHR);
    auto emMiss1Stage = dev->createShaderModule("emMiss1", emMiss1[0].str, VK_SHADER_STAGE_MISS_BIT_KHR);
    auto emHit0Stage = dev->createShaderModule("emhit0", emHit0[1].str, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    auto emHit1Stage = dev->createShaderModule("emhit1", emHit1[1].str, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    auto aHitStage = dev->createShaderModule("ahit", aHit[0].str, VK_SHADER_STAGE_ANY_HIT_BIT_KHR);

    vkUtil::ARR_DELETE(Shader_common_R);

    std::vector<VkPipelineShaderStageCreateInfo> stages = {
        rgsStage,
        miss0Stage, miss1Stage,
        emMiss0Stage, emMiss1Stage,
        chit0Stage, chit1Stage,
        emHit0Stage, emHit1Stage,
        aHitStage
    };

    // stages 配列内での各シェーダーのインデックス.
    const int indexRaygen = 0;
    const int indexMiss0 = 1;
    const int indexMiss1 = 2;
    const int indexEmMiss0 = 3;
    const int indexEmMiss1 = 4;
    const int indexClosestHit0 = 5;
    const int indexClosestHit1 = 6;
    const int indexEmHit0 = 7;
    const int indexEmHit1 = 8;
    const int indexAHit = 9;

    // シェーダーグループの生成.
    m_shaderGroups.resize(MaxShaderGroup);
    m_shaderGroups[GroupRayGenShader] = createShaderGroupRayGeneration(indexRaygen);
    m_shaderGroups[GroupMissShader0] = createShaderGroupMiss(indexMiss0);
    m_shaderGroups[GroupMissShader1] = createShaderGroupMiss(indexMiss1);
    m_shaderGroups[GroupEmMissShader0] = createShaderGroupMiss(indexEmMiss0);
    m_shaderGroups[GroupEmMissShader1] = createShaderGroupMiss(indexEmMiss1);
    m_shaderGroups[GroupHitShader0] = createShaderGroupHit(indexClosestHit0, indexAHit);
    m_shaderGroups[GroupHitShader1] = createShaderGroupHit(indexClosestHit1, indexAHit);
    m_shaderGroups[GroupEmHitShader0] = createShaderGroupHit(indexEmHit0, indexAHit);
    m_shaderGroups[GroupEmHitShader1] = createShaderGroupHit(indexEmHit1, indexAHit);

    // レイトレーシングパイプラインの生成.
    VkRayTracingPipelineCreateInfoKHR rtPipelineCI{};

    rtPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    rtPipelineCI.stageCount = uint32_t(stages.size());
    rtPipelineCI.pStages = stages.data();
    rtPipelineCI.groupCount = uint32_t(m_shaderGroups.size());
    rtPipelineCI.pGroups = m_shaderGroups.data();
    rtPipelineCI.maxPipelineRayRecursionDepth = m_device->GetRayTracingPipelineProperties().maxRayRecursionDepth;
    rtPipelineCI.layout = m_pipelineLayout;
    vkCreateRayTracingPipelinesKHR(
        m_device->GetDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE,
        1, &rtPipelineCI, nullptr, &m_raytracePipeline);

    //シェーダーモジュール解放
    for (auto& v : stages) {
        vkDestroyShaderModule(
            m_device->GetDevice(), v.module, nullptr);
    }
}

void VulkanRendererRt::CreateShaderBindingTable() {

    auto memProps = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    auto usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    const auto rtPipelineProps = m_device->GetRayTracingPipelineProperties();

    //各エントリサイズ shaderGroupHandleAlignment にアライメント
    const auto handleSize = rtPipelineProps.shaderGroupHandleSize;
    const auto handleAlignment = rtPipelineProps.shaderGroupHandleAlignment;
    auto raygenShaderEntrySize = Align(handleSize, handleAlignment);
    auto missShaderEntrySize = Align(handleSize, handleAlignment);
    auto hitShaderEntrySize = Align(handleSize + sizeof(uint64_t) * 2, handleAlignment);//IndexBufferアドレスとVertexBufferアドレス込み

    const uint32_t numHitShader = VulkanDeviceRt::numHitShader;

    const auto raygenShaderCount = 1;
    const auto missShaderCount = numHitShader;
    const auto hitShaderCount = numHitShader * rt.size();

    //各グループのサイズ
    const auto baseAlign = rtPipelineProps.shaderGroupBaseAlignment;
    auto RaygenGroupSize = Align(raygenShaderEntrySize * raygenShaderCount, baseAlign);
    auto MissGroupSize = Align(missShaderEntrySize * missShaderCount, baseAlign);
    auto HitGroupSize = Align(hitShaderEntrySize * hitShaderCount, baseAlign);

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{
  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
  nullptr,
    };
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    void* pNext = &memoryAllocateFlagsInfo;

    m_shaderBindingTable.createUploadBuffer(RaygenGroupSize + MissGroupSize + HitGroupSize,
        usage, pNext);

    //パイプラインのShaderGroupハンドルを取得
    auto handleSizeAligned = Align(handleSize, handleAlignment);
    auto handleStorageSize = m_shaderGroups.size() * handleSizeAligned;
    std::vector<uint8_t> shaderHandleStorage(handleStorageSize);
    vkGetRayTracingShaderGroupHandlesKHR(m_device->GetDevice(),
        m_raytracePipeline,
        0, uint32_t(m_shaderGroups.size()),
        shaderHandleStorage.size(), shaderHandleStorage.data());

    auto device = VulkanDevice::GetInstance()->getDevice();
    auto deviceAddress = m_shaderBindingTable.getDeviceAddress();

    void* p = m_shaderBindingTable.Map();
    auto dst = static_cast<uint8_t*>(p);

    //RayGeneration
    auto raygen = shaderHandleStorage.data() + handleSizeAligned * GroupRayGenShader;
    memcpy(dst, raygen, handleSize);
    dst += RaygenGroupSize;
    m_sbtInfo.rgen.deviceAddress = deviceAddress;
    //Raygen は size=strideが必要.
    m_sbtInfo.rgen.stride = raygenShaderEntrySize;
    m_sbtInfo.rgen.size = m_sbtInfo.rgen.stride;

    //Miss
    auto miss = shaderHandleStorage.data() + handleSizeAligned * GroupMissShader0;
    auto dstM = dst;
    memcpy(dstM, miss, handleSize);//miss0
    dstM += missShaderEntrySize;
    miss += handleSizeAligned;
    memcpy(dstM, miss, handleSize);//miss1
    dstM += missShaderEntrySize;
    miss += handleSizeAligned;
    memcpy(dstM, miss, handleSize);//emMiss0
    dstM += missShaderEntrySize;
    miss += handleSizeAligned;
    memcpy(dstM, miss, handleSize);//emMiss1
    dst += MissGroupSize;
    m_sbtInfo.miss.deviceAddress = deviceAddress + RaygenGroupSize;
    m_sbtInfo.miss.size = MissGroupSize;
    m_sbtInfo.miss.stride = missShaderEntrySize;

    //Hit
    //emHitでは頂点データ未使用だがstride揃える関係でとりあえず入れてる
    auto hit = shaderHandleStorage.data() + handleSizeAligned * GroupHitShader0;
    auto dstH = dst;

    for (size_t i = 0; i < rt.size(); i++) {
        rt[i]->hitShaderIndex = numHitShader * (uint32_t)i;
        writeSBTDataAndHit(rt[i], dstH, hitShaderEntrySize, hit, handleSizeAligned, handleSize);
        dstH += (hitShaderEntrySize * numHitShader);
    }

    m_sbtInfo.hit.deviceAddress = deviceAddress + RaygenGroupSize + MissGroupSize;
    m_sbtInfo.hit.size = HitGroupSize;
    m_sbtInfo.hit.stride = hitShaderEntrySize;

    m_shaderBindingTable.UnMap();
}

void VulkanRendererRt::writeSBTDataAndHit(VulkanBasicPolygonRt::RtData* rt,
    void* dst, uint64_t hitShaderEntrySize,
    void* hit, uint32_t handleSizeAligned, uint32_t hitHandleSize) {

    auto Dst = static_cast<uint8_t*>(dst);
    auto Hit = static_cast<uint8_t*>(hit);

    for (int i = 0; i < VulkanDeviceRt::numHitShader; i++) {

        auto p = static_cast<uint8_t*>(Dst);

        //hitShader
        memcpy(p, Hit, hitHandleSize);
        p += hitHandleSize;

        //IndexBuffer
        uint64_t deviceAddr = 0;
        deviceAddr = rt->indexBuf.getDeviceAddress();
        memcpy(p, &deviceAddr, sizeof(deviceAddr));
        p += sizeof(deviceAddr);

        //VertexBuffer
        deviceAddr = rt->vertexBuf->getDeviceAddress();
        memcpy(p, &deviceAddr, sizeof(deviceAddr));
        p += sizeof(deviceAddr);

        Dst += hitShaderEntrySize;
        Hit += handleSizeAligned;
    }
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

    VkDescriptorSetLayoutCreateInfo dsLayout[numDescriptorSet] = {};

    for (int i = 0; i < numDescriptorSet; i++) {
        dsLayout[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dsLayout[i].bindingCount = static_cast<uint32_t>(set[i].size());
        dsLayout[i].pBindings = set[i].data();

        vkCreateDescriptorSetLayout(
            m_device->GetDevice(), &dsLayout[i], nullptr, &m_dsLayout[i]);

        m_descriptorSet[i] = m_device->AllocateDescriptorSet(m_dsLayout[i]);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCI{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
    };
    pipelineLayoutCI.setLayoutCount = numDescriptorSet;
    pipelineLayoutCI.pSetLayouts = m_dsLayout;
    vkCreatePipelineLayout(m_device->GetDevice(),
        &pipelineLayoutCI, nullptr, &m_pipelineLayout);
}

void VulkanRendererRt::CreateDescriptorSets() {

    std::vector<VkAccelerationStructureKHR> asHandles = {
        m_topLevelAS.getHandle()
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
    asWrite.dstSet = m_descriptorSet[0];
    asWrite.dstBinding = 0;
    asWrite.descriptorCount = 1;
    asWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

    VkWriteDescriptorSet imageWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    imageWrite.dstSet = m_descriptorSet[0];
    imageWrite.dstBinding = 1;
    imageWrite.descriptorCount = 1;
    imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imageWrite.pImageInfo = &m_raytracedImage.info;

    VkWriteDescriptorSet sceneUboWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    sceneUboWrite.dstSet = m_descriptorSet[0];
    sceneUboWrite.dstBinding = 2;
    sceneUboWrite.descriptorCount = 1;
    sceneUboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    sceneUboWrite.pBufferInfo = &m_sceneUBO.buf.info;

    VkWriteDescriptorSet InstanceIdMapWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    InstanceIdMapWrite.dstSet = m_descriptorSet[0];
    InstanceIdMapWrite.dstBinding = 3;
    InstanceIdMapWrite.descriptorCount = 1;
    InstanceIdMapWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    InstanceIdMapWrite.pImageInfo = &instanceIdMap.info;

    VkWriteDescriptorSet depthWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    depthWrite.dstSet = m_descriptorSet[0];
    depthWrite.dstBinding = 4;
    depthWrite.descriptorCount = 1;
    depthWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    depthWrite.pImageInfo = &depthMap.info;

    VkWriteDescriptorSet difTexImageWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    difTexImageWrite.dstSet = m_descriptorSet[1];
    difTexImageWrite.dstBinding = 0;
    difTexImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    difTexImageWrite.descriptorCount = (uint32_t)textureDifArr.size();
    difTexImageWrite.pImageInfo = textureDifArr.data();

    VkWriteDescriptorSet norTexImageWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    norTexImageWrite.dstSet = m_descriptorSet[2];
    norTexImageWrite.dstBinding = 0;
    norTexImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    norTexImageWrite.descriptorCount = (uint32_t)textureNorArr.size();
    norTexImageWrite.pImageInfo = textureNorArr.data();

    VkWriteDescriptorSet speTexImageWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    speTexImageWrite.dstSet = m_descriptorSet[3];
    speTexImageWrite.dstBinding = 0;
    speTexImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    speTexImageWrite.descriptorCount = (uint32_t)textureSpeArr.size();
    speTexImageWrite.pImageInfo = textureSpeArr.data();

    VkWriteDescriptorSet materialWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    materialWrite.dstSet = m_descriptorSet[4];
    materialWrite.dstBinding = 0;
    materialWrite.descriptorCount = 1;
    materialWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    materialWrite.pBufferInfo = &materialUBO.info;

    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        asWrite, imageWrite, sceneUboWrite, InstanceIdMapWrite, depthWrite,
        difTexImageWrite,
        norTexImageWrite,
        speTexImageWrite,
        materialWrite
    };
    vkUpdateDescriptorSets(
        m_device->GetDevice(),
        uint32_t(writeDescriptorSets.size()),
        writeDescriptorSets.data(),
        0,
        nullptr);
}

void VulkanRendererRt::TestModeOn(TestMode mode) {
    testMode[mode] = true;
}
