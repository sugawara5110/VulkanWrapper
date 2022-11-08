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

namespace {
    VkRayTracingShaderGroupCreateInfoKHR CreateShaderGroupRayGeneration(uint32_t shaderIndex) {

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

    VkRayTracingShaderGroupCreateInfoKHR CreateShaderGroupMiss(uint32_t shaderIndex) {

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

    VkRayTracingShaderGroupCreateInfoKHR CreateShaderGroupHit(
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

void VulkanRendererRt::Init(std::vector<VulkanBasicPolygonRt::RtData*> r) {

    rt = r;

    for (int i = 0; i < rt.size(); i++) {
        vertexArr.push_back(rt[i]->vertexBuf.info);
        indexArr.push_back(rt[i]->indexBuf.info);
        for (int j = 0; j < rt[i]->instance.size(); j++) {
            textureDifArr.push_back(rt[i]->texId.difTex.image.info);
            textureNorArr.push_back(rt[i]->texId.norTex.image.info);
            textureSpeArr.push_back(rt[i]->texId.speTex.image.info);
            materialArr.push_back(rt[i]->mat.buf.info);
        }
    }

    // レイトレーシングするためTLASを準備します.
    CreateSceneTLAS();

    // レイトレーシング用の結果バッファを準備する.
    CreateRaytracedBuffer();

    // これから必要になる各種レイアウトの準備.
    CreateLayouts();

    // シーンパラメータ用UniformBufferを生成.
    VulkanDevice::GetInstance()->createUniform(m_sceneUBO);

    // レイトレーシングパイプラインを構築する.
    CreateRaytracePipeline();

    // シェーダーバインディングテーブルを構築する.
    CreateShaderBindingTable();

    // ディスクリプタの準備・書き込み.
    CreateDescriptorSets();

    using namespace CoordTf;
    m_sceneParam.dLightColor = { 1,1,1,1 };
    m_sceneParam.dDirection = { -0.2f, -1.0f, -1.0f, 0.0f };
    m_sceneParam.GlobalAmbientColor = { 0.25f,0.25f,0.25f,0.25 };
}

void VulkanRendererRt::destroy() {
    // GPU の処理が全て終わるまでを待機.
    vkDeviceWaitIdle(VulkanDevice::GetInstance()->getDevice());

    m_sceneUBO.buf.destroy();
    m_instancesBuffer.destroy();
    m_topLevelAS.destroy();

    m_raytracedImage.destroy();
    m_shaderBindingTable.destroy();

    auto device = VulkanDevice::GetInstance()->getDevice();
    vkDestroyPipeline(device, m_raytracePipeline, nullptr);
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_dsLayout, nullptr);
    vkFreeDescriptorSets(device, m_device->GetDescriptorPool(), 1, &m_descriptorSet);

    m_device->destroy();
    m_device.reset();
}

void VulkanRendererRt::Update() {
    using namespace CoordTf;
    VulkanDevice* dev = VulkanDevice::GetInstance();
    m_sceneParam.cameraPosition = dev->getCameraViewPos();

    MATRIX VP;
    MatrixMultiply(&VP, &dev->getCameraView(), &dev->getProjection());
    MatrixTranspose(&VP);
    MatrixInverse(&m_sceneParam.projectionToWorld, &VP);
}

void VulkanRendererRt::Render() {

    VulkanDevice* dev = VulkanDevice::GetInstance();
    dev->beginCommandNextImage(0);

    auto command = dev->getCommandBuffer(0);

    UpdateSceneTLAS(command);

    memcpy(&m_sceneUBO.uni, &m_sceneParam, sizeof(m_sceneParam));
    dev->updateUniform(m_sceneUBO);

    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_raytracePipeline);
    vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipelineLayout, 0,
        1, &m_descriptorSet, 0, nullptr);

    uint32_t width = dev->getSwapchainObj()->getSize().width;
    uint32_t height = dev->getSwapchainObj()->getSize().height;

    VkStridedDeviceAddressRegionKHR callable_shader_sbt_entry{};
    vkCmdTraceRaysKHR(
        command,
        &m_sbtInfo.rgen,
        &m_sbtInfo.miss,
        &m_sbtInfo.hit,
        &callable_shader_sbt_entry,
        width, height, 1
    );

    // レイトレーシング結果画像をバックバッファへコピー.
    VkImageCopy region{};
    region.extent = { width, height, 1 };
    region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };

    m_raytracedImage.barrierResource(0, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    dev->barrierResource(0,
        dev->getSwapchainObj()->getCurrentImage(),
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vkCmdCopyImage(command,
        m_raytracedImage.getImage(), m_raytracedImage.info.imageLayout,
        dev->getSwapchainObj()->getCurrentImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region);

    m_raytracedImage.barrierResource(0, VK_IMAGE_LAYOUT_GENERAL);

    dev->beginDraw(0);

    //通常のレンダリングする場合ここで処理/////////////////////

     // レンダーパスが終了するとバックバッファは
     // TRANSFER_DST_OPTIMAL->PRESENT_SRC_KHR へレイアウト変更が適用される.
    dev->endDraw(0);
    dev->endCommand(0);
    dev->Present(0);
}

void VulkanRendererRt::CreateSceneTLAS() {

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
    asGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    asGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    asGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
    asGeometry.geometry.instances.data = instanceDataDeviceAddress;

    VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfo{};
    asBuildRangeInfo.primitiveCount = uint32_t(asInstances.size());
    asBuildRangeInfo.primitiveOffset = 0;
    asBuildRangeInfo.firstVertex = 0;
    asBuildRangeInfo.transformOffset = 0;

    AccelerationStructure::Input tlasInput{};
    tlasInput.Geometry = { asGeometry };
    tlasInput.BuildRangeInfo = { asBuildRangeInfo };
    VkBuildAccelerationStructureFlagsKHR buildFlags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    m_topLevelAS.buildAS(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, tlasInput, buildFlags);

    m_topLevelAS.destroyScratchBuffer();
}

void VulkanRendererRt::UpdateSceneTLAS(VkCommandBuffer command) {

    std::vector<VkAccelerationStructureInstanceKHR> asInstances;

    for (int i = 0; i < rt.size(); i++) {
        for (int j = 0; j < rt[i]->instance.size(); j++) {
            asInstances.push_back(rt[i]->instance[j].vkInstance);
        }
    }

    m_instancesBuffer.memoryMap(asInstances.data());

    VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
    instanceDataDeviceAddress.deviceAddress = m_instancesBuffer.getDeviceAddress();

    VkAccelerationStructureGeometryKHR asGeometry{};
    asGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    asGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    asGeometry.flags = 0;
    asGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    asGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
    asGeometry.geometry.instances.data = instanceDataDeviceAddress;

    VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfo{};
    asBuildRangeInfo.primitiveCount = uint32_t(asInstances.size());
    asBuildRangeInfo.primitiveOffset = 0;
    asBuildRangeInfo.firstVertex = 0;
    asBuildRangeInfo.transformOffset = 0;

    AccelerationStructure::Input tlasInput{};
    tlasInput.Geometry = { asGeometry };
    tlasInput.BuildRangeInfo = { asBuildRangeInfo };

    VkBuildAccelerationStructureFlagsKHR buildFlags = 0;
    buildFlags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    m_topLevelAS.update(
        command,
        VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        tlasInput,
        buildFlags
    );
}

void VulkanRendererRt::CreateRaytracedBuffer() {

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

    // バッファの状態を変更しておく.
    auto command = dev->getCommandBuffer(0);
    dev->beginCommand(0);

    m_raytracedImage.barrierResource(0, VK_IMAGE_LAYOUT_GENERAL);

    dev->endCommand(0);
    dev->submitCommandsDoNotRender(0);
}

void VulkanRendererRt::CreateRaytracePipeline() {

    // レイトレーシングのシェーダーを読み込む.
    vkUtil::addChar ray[1];
    vkUtil::addChar clo[4];
    ray[0].addStr(Shader_common, Shader_raygen);
    clo[0].addStr(Shader_common, ShaderCalculateLighting);
    clo[1].addStr(clo[0].str, ShaderNormalTangent);
    clo[2].addStr(clo[1].str, Shader_hitCom);
    clo[3].addStr(clo[2].str, Shader_closesthit);
    auto rgsStage = VulkanDevice::GetInstance()->createShaderModule("raygen", ray[0].str, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
    auto missStage = VulkanDevice::GetInstance()->createShaderModule("miss", Shader_miss, VK_SHADER_STAGE_MISS_BIT_KHR);
    auto chitStage = VulkanDevice::GetInstance()->createShaderModule("closesthit", clo[3].str, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

    std::vector<VkPipelineShaderStageCreateInfo> stages = {
        rgsStage, missStage, chitStage
    };

    // stages 配列内での各シェーダーのインデックス.
    const int indexRaygen = 0;
    const int indexMiss = 1;
    const int indexClosestHit = 2;

    // シェーダーグループの生成.
    m_shaderGroups.resize(MaxShaderGroup);
    m_shaderGroups[GroupRayGenShader] = CreateShaderGroupRayGeneration(indexRaygen);
    m_shaderGroups[GroupMissShader] = CreateShaderGroupMiss(indexMiss);
    m_shaderGroups[GroupHitShader] = CreateShaderGroupHit(indexClosestHit);

    // レイトレーシングパイプラインの生成.
    VkRayTracingPipelineCreateInfoKHR rtPipelineCI{};

    rtPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    rtPipelineCI.stageCount = uint32_t(stages.size());
    rtPipelineCI.pStages = stages.data();
    rtPipelineCI.groupCount = uint32_t(m_shaderGroups.size());
    rtPipelineCI.pGroups = m_shaderGroups.data();
    rtPipelineCI.maxPipelineRayRecursionDepth = 1;
    rtPipelineCI.layout = m_pipelineLayout;
    vkCreateRayTracingPipelinesKHR(
        m_device->GetDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE,
        1, &rtPipelineCI, nullptr, &m_raytracePipeline);

    // 作り終えたのでシェーダーモジュールは解放してしまう.
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
    auto hitShaderEntrySize = Align(handleSize, handleAlignment);

    const auto raygenShaderCount = 1;
    const auto missShaderCount = 1;
    const auto hitShaderCount = 1;

    //各グループのサイズ
    const auto baseAlign = rtPipelineProps.shaderGroupBaseAlignment;
    auto regionRaygen = Align(raygenShaderEntrySize * raygenShaderCount, baseAlign);
    auto regionMiss = Align(missShaderEntrySize * missShaderCount, baseAlign);
    auto regionHit = Align(hitShaderEntrySize * hitShaderCount, baseAlign);

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{
  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
  nullptr,
    };
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    void* pNext = &memoryAllocateFlagsInfo;

    m_shaderBindingTable.createUploadBuffer(regionRaygen + regionMiss + regionHit,
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

    // RayGenerationシェーダーのエントリを書き込む.
    auto raygen = shaderHandleStorage.data() + handleSizeAligned * GroupRayGenShader;
    memcpy(dst, raygen, handleSize);
    dst += regionRaygen;
    m_sbtInfo.rgen.deviceAddress = deviceAddress;
    // Raygen は size=strideが必要.
    m_sbtInfo.rgen.stride = raygenShaderEntrySize;
    m_sbtInfo.rgen.size = m_sbtInfo.rgen.stride;

    // Missシェーダーのエントリを書き込む.
    auto miss = shaderHandleStorage.data() + handleSizeAligned * GroupMissShader;
    memcpy(dst, miss, handleSize);
    dst += regionMiss;
    m_sbtInfo.miss.deviceAddress = deviceAddress + regionRaygen;
    m_sbtInfo.miss.size = regionMiss;
    m_sbtInfo.miss.stride = missShaderEntrySize;

    //Hitシェーダーのエントリを書き込む.
    auto hit = shaderHandleStorage.data() + handleSizeAligned * GroupHitShader;
    memcpy(dst, hit, handleSize);
    m_sbtInfo.hit.deviceAddress = deviceAddress + regionRaygen + regionMiss;
    m_sbtInfo.hit.size = regionHit;
    m_sbtInfo.hit.stride = hitShaderEntrySize;

    m_shaderBindingTable.UnMap();
}

void VulkanRendererRt::CreateLayouts() {

    VkDescriptorSetLayoutBinding layoutAS{};
    layoutAS.binding = 0;
    layoutAS.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    layoutAS.descriptorCount = 1;
    layoutAS.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    VkDescriptorSetLayoutBinding layoutRtImage{};
    layoutRtImage.binding = 1;
    layoutRtImage.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutRtImage.descriptorCount = 1;
    layoutRtImage.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    VkDescriptorSetLayoutBinding layoutSceneUBO{};
    layoutSceneUBO.binding = 2;
    layoutSceneUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutSceneUBO.descriptorCount = 1;
    layoutSceneUBO.stageFlags = VK_SHADER_STAGE_ALL;

    VkDescriptorSetLayoutBinding layoutDifTex{};
    layoutDifTex.binding = 3;
    layoutDifTex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutDifTex.descriptorCount = (uint32_t)textureDifArr.size();
    layoutDifTex.stageFlags = VK_SHADER_STAGE_ALL;

    VkDescriptorSetLayoutBinding layoutNorTex{};
    layoutNorTex.binding = 4;
    layoutNorTex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutNorTex.descriptorCount = (uint32_t)textureNorArr.size();
    layoutNorTex.stageFlags = VK_SHADER_STAGE_ALL;

    VkDescriptorSetLayoutBinding layoutSpeTex{};
    layoutSpeTex.binding = 5;
    layoutSpeTex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutSpeTex.descriptorCount = (uint32_t)textureSpeArr.size();
    layoutSpeTex.stageFlags = VK_SHADER_STAGE_ALL;

    VkDescriptorSetLayoutBinding layoutVertex{};
    layoutVertex.binding = 6;
    layoutVertex.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutVertex.descriptorCount = (uint32_t)vertexArr.size();
    layoutVertex.stageFlags = VK_SHADER_STAGE_ALL;

    VkDescriptorSetLayoutBinding layoutIndex{};
    layoutIndex.binding = 7;
    layoutIndex.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutIndex.descriptorCount = (uint32_t)indexArr.size();
    layoutIndex.stageFlags = VK_SHADER_STAGE_ALL;

    VkDescriptorSetLayoutBinding layoutMaterialCB{};
    layoutMaterialCB.binding = 8;
    layoutMaterialCB.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutMaterialCB.descriptorCount = (uint32_t)materialArr.size();
    layoutMaterialCB.stageFlags = VK_SHADER_STAGE_ALL;

    std::vector<VkDescriptorSetLayoutBinding> bindings({
        layoutAS, layoutRtImage, layoutSceneUBO,layoutDifTex,layoutNorTex,layoutSpeTex,layoutVertex,layoutIndex,layoutMaterialCB });

    VkDescriptorSetLayoutCreateInfo dsLayoutCI{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
    };
    dsLayoutCI.bindingCount = static_cast<uint32_t>(bindings.size());
    dsLayoutCI.pBindings = bindings.data();
    vkCreateDescriptorSetLayout(
        m_device->GetDevice(), &dsLayoutCI, nullptr, &m_dsLayout);

    // Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutCI{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
    };
    pipelineLayoutCI.setLayoutCount = 1;
    pipelineLayoutCI.pSetLayouts = &m_dsLayout;
    vkCreatePipelineLayout(m_device->GetDevice(),
        &pipelineLayoutCI, nullptr, &m_pipelineLayout);
}

void VulkanRendererRt::CreateDescriptorSets() {

    m_descriptorSet = m_device->AllocateDescriptorSet(m_dsLayout);

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
    asWrite.dstSet = m_descriptorSet;
    asWrite.dstBinding = 0;
    asWrite.descriptorCount = 1;
    asWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

    VkWriteDescriptorSet imageWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    imageWrite.dstSet = m_descriptorSet;
    imageWrite.dstBinding = 1;
    imageWrite.descriptorCount = 1;
    imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imageWrite.pImageInfo = &m_raytracedImage.info;

    VkDescriptorBufferInfo sceneUboDescriptor{};
    sceneUboDescriptor = m_sceneUBO.buf.info;

    VkWriteDescriptorSet sceneUboWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    sceneUboWrite.dstSet = m_descriptorSet;
    sceneUboWrite.dstBinding = 2;
    sceneUboWrite.descriptorCount = 1;
    sceneUboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    sceneUboWrite.pBufferInfo = &sceneUboDescriptor;

    VkWriteDescriptorSet difTexImageWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    difTexImageWrite.dstSet = m_descriptorSet;
    difTexImageWrite.dstBinding = 3;
    difTexImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    difTexImageWrite.descriptorCount = (uint32_t)textureDifArr.size();
    difTexImageWrite.pImageInfo = textureDifArr.data();

    VkWriteDescriptorSet norTexImageWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    norTexImageWrite.dstSet = m_descriptorSet;
    norTexImageWrite.dstBinding = 4;
    norTexImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    norTexImageWrite.descriptorCount = (uint32_t)textureNorArr.size();
    norTexImageWrite.pImageInfo = textureNorArr.data();

    VkWriteDescriptorSet speTexImageWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    speTexImageWrite.dstSet = m_descriptorSet;
    speTexImageWrite.dstBinding = 5;
    speTexImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    speTexImageWrite.descriptorCount = (uint32_t)textureSpeArr.size();
    speTexImageWrite.pImageInfo = textureSpeArr.data();

    VkWriteDescriptorSet vertexWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    vertexWrite.dstSet = m_descriptorSet;
    vertexWrite.dstBinding = 6;
    vertexWrite.descriptorCount = (uint32_t)vertexArr.size();
    vertexWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    vertexWrite.pBufferInfo = vertexArr.data();

    VkWriteDescriptorSet indexWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    indexWrite.dstSet = m_descriptorSet;
    indexWrite.dstBinding = 7;
    indexWrite.descriptorCount = (uint32_t)indexArr.size();
    indexWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    indexWrite.pBufferInfo = indexArr.data();

    VkWriteDescriptorSet materialWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
    };
    materialWrite.dstSet = m_descriptorSet;
    materialWrite.dstBinding = 8;
    materialWrite.descriptorCount = (uint32_t)materialArr.size();
    materialWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    materialWrite.pBufferInfo = materialArr.data();

    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        asWrite, imageWrite, sceneUboWrite,difTexImageWrite,norTexImageWrite,speTexImageWrite,vertexWrite,indexWrite,materialWrite
    };
    vkUpdateDescriptorSets(
        m_device->GetDevice(),
        uint32_t(writeDescriptorSets.size()),
        writeDescriptorSets.data(),
        0,
        nullptr);
}


