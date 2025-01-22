//*****************************************************************************************//
//**                                                                                     **//
//**                              RasterizeDescriptor.cpp                                **//
//**                                                                                     **//
//*****************************************************************************************//

#include "RasterizeDescriptor.h"

RasterizeDescriptor* RasterizeDescriptor::ptr = nullptr;

void RasterizeDescriptor::InstanceCreate() {
    if (ptr == nullptr)ptr = NEW RasterizeDescriptor();
}

RasterizeDescriptor* RasterizeDescriptor::GetInstance() {
    return ptr;
}

void RasterizeDescriptor::DeleteInstance() {
    if (ptr != nullptr) {
        delete ptr;
        ptr = nullptr;
    }
}

void RasterizeDescriptor::descriptorAndPipelineLayouts(
    VkPipelineLayout& pipelineLayout,
    VkDescriptorSetLayout* descSetLayoutArr) {

    std::vector<VkDescriptorSetLayoutBinding> set[numDescriptorSet];

    VkDescriptorSetLayoutBinding bufferMat = {};
    bufferMat.binding = 0;
    bufferMat.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMat.descriptorCount = 1;
    bufferMat.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bufferMat.pImmutableSamplers = nullptr;

    set[0].push_back(bufferMat);

    VkDescriptorSetLayoutBinding bufferMat_bone = {};
    bufferMat_bone.binding = 0;
    bufferMat_bone.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMat_bone.descriptorCount = 1;
    bufferMat_bone.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bufferMat_bone.pImmutableSamplers = nullptr;

    set[1].push_back(bufferMat_bone);

    VkDescriptorSetLayoutBinding texSampler = {};
    texSampler.binding = 0;
    texSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texSampler.descriptorCount = 1;
    texSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    texSampler.pImmutableSamplers = nullptr;

    set[2].push_back(texSampler);

    VkDescriptorSetLayoutBinding norSampler = {};
    norSampler.binding = 1;
    norSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    norSampler.descriptorCount = 1;
    norSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    norSampler.pImmutableSamplers = nullptr;

    set[2].push_back(norSampler);

    VkDescriptorSetLayoutBinding speSampler = {};
    speSampler.binding = 2;
    speSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    speSampler.descriptorCount = 1;
    speSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    speSampler.pImmutableSamplers = nullptr;

    set[2].push_back(speSampler);

    VkDescriptorSetLayoutBinding bufferMaterial = {};
    bufferMaterial.binding = 0;
    bufferMaterial.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMaterial.descriptorCount = 1;
    bufferMaterial.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bufferMaterial.pImmutableSamplers = nullptr;

    set[3].push_back(bufferMaterial);

    VkDescriptorSetLayoutCreateInfo descriptor_layout[numDescriptorSet] = {};

    VkResult res;

    for (int i = 0; i < numDescriptorSet; i++) {
        VkDescriptorSetLayoutCreateInfo& layout = descriptor_layout[i];
        layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout.pNext = NULL;
        layout.flags = 0;
        layout.bindingCount = static_cast<uint32_t>(set[i].size());
        layout.pBindings = set[i].data();

        res = _vkCreateDescriptorSetLayout(
            VulkanDevice::GetInstance()->getDevice(), &layout, nullptr, &descSetLayoutArr[i]);
        vkUtil::checkError(res);
    }

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = nullptr;
    pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = numDescriptorSet;
    pPipelineLayoutCreateInfo.pSetLayouts = descSetLayoutArr;

    res = _vkCreatePipelineLayout(VulkanDevice::GetInstance()->getDevice(), &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    vkUtil::checkError(res);
}

void  RasterizeDescriptor::descriptorAndPipelineLayouts2D(bool useTexture, VkPipelineLayout& pipelineLayout, VkDescriptorSetLayout& descSetLayout) {
    VkDescriptorSetLayoutBinding layout_bindings[2];
    uint32_t bCnt = 0;
    VkDescriptorSetLayoutBinding& bufferMat = layout_bindings[bCnt];
    bufferMat.binding = bCnt++;
    bufferMat.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMat.descriptorCount = 1;
    bufferMat.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bufferMat.pImmutableSamplers = nullptr;

    if (useTexture) {
        VkDescriptorSetLayoutBinding& texSampler = layout_bindings[bCnt];
        texSampler.binding = bCnt++;
        texSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texSampler.descriptorCount = 1;
        texSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        texSampler.pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext = NULL;
    descriptor_layout.flags = 0;
    descriptor_layout.bindingCount = bCnt;
    descriptor_layout.pBindings = layout_bindings;

    VkResult res;

    res = _vkCreateDescriptorSetLayout(VulkanDevice::GetInstance()->getDevice(), &descriptor_layout, nullptr, &descSetLayout);
    vkUtil::checkError(res);

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = nullptr;
    pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &descSetLayout;

    res = _vkCreatePipelineLayout(VulkanDevice::GetInstance()->getDevice(), &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    vkUtil::checkError(res);
}

void RasterizeDescriptor::upDescriptorSet(
    VulkanDevice::ImageSet& difTexture,
    VulkanDevice::ImageSet& norTexture,
    VulkanDevice::ImageSet& speTexture,
    VulkanDevice::Uniform<MatrixSet>* uni,
    VulkanDevice::Uniform<MatrixSet_bone>* uni_bone,
    VulkanDevice::Uniform<Material>* material,
    VkDescriptorSet* descriptorSet,
    VkDescriptorSetLayout* descSetLayout) {

    VkResult res;

    VkDescriptorSetAllocateInfo alloc_info[1];
    alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info[0].pNext = nullptr;
    alloc_info[0].descriptorPool = VulkanDevice::GetInstance()->GetDescriptorPool();
    alloc_info[0].descriptorSetCount = numDescriptorSet;
    alloc_info[0].pSetLayouts = descSetLayout;

    res = _vkAllocateDescriptorSets(VulkanDevice::GetInstance()->getDevice(), alloc_info, descriptorSet);
    vkUtil::checkError(res);

    const uint32_t num_writes = 6;
    VkWriteDescriptorSet writes[num_writes] = {};

    VkWriteDescriptorSet& bufferMat = writes[0];
    bufferMat = {};
    bufferMat.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    bufferMat.pNext = nullptr;
    bufferMat.dstSet = descriptorSet[0];
    bufferMat.descriptorCount = 1;
    bufferMat.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMat.pBufferInfo = &uni->getBufferSet()->info;
    bufferMat.dstArrayElement = 0;
    bufferMat.dstBinding = 0;

    VkWriteDescriptorSet& bufferMat_bone = writes[1];
    bufferMat_bone = {};
    bufferMat_bone.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    bufferMat_bone.pNext = nullptr;
    bufferMat_bone.dstSet = descriptorSet[1];
    bufferMat_bone.descriptorCount = 1;
    bufferMat_bone.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMat_bone.pBufferInfo = &uni_bone->getBufferSet()->info;
    bufferMat_bone.dstArrayElement = 0;
    bufferMat_bone.dstBinding = 0;

    VkWriteDescriptorSet& texSampler = writes[2];
    texSampler = {};
    texSampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    texSampler.dstSet = descriptorSet[2];
    texSampler.dstBinding = 0;
    texSampler.descriptorCount = 1;
    texSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texSampler.pImageInfo = &difTexture.info;
    texSampler.dstArrayElement = 0;

    VkWriteDescriptorSet& norSampler = writes[3];
    norSampler = {};
    norSampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    norSampler.dstSet = descriptorSet[2];
    norSampler.dstBinding = 1;
    norSampler.descriptorCount = 1;
    norSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    norSampler.pImageInfo = &norTexture.info;
    norSampler.dstArrayElement = 0;

    VkWriteDescriptorSet& speSampler = writes[4];
    speSampler = {};
    speSampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    speSampler.dstSet = descriptorSet[2];
    speSampler.dstBinding = 2;
    speSampler.descriptorCount = 1;
    speSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    speSampler.pImageInfo = &speTexture.info;
    speSampler.dstArrayElement = 0;

    VkWriteDescriptorSet& bufferMaterial = writes[5];
    bufferMaterial = {};
    bufferMaterial.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    bufferMaterial.pNext = nullptr;
    bufferMaterial.dstSet = descriptorSet[3];
    bufferMaterial.descriptorCount = 1;
    bufferMaterial.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMaterial.pBufferInfo = &material->getBufferSet()->info;
    bufferMaterial.dstArrayElement = 0;
    bufferMaterial.dstBinding = 0;

    _vkUpdateDescriptorSets(VulkanDevice::GetInstance()->getDevice(), num_writes, writes, 0, nullptr);
}

void RasterizeDescriptor::upDescriptorSet2D(bool useTexture,
    VulkanDevice::ImageSet& texture,
    VulkanDevice::Uniform<MatrixSet2D>* uni,
    VkDescriptorSet& descriptorSet,
    VkDescriptorSetLayout& descSetLayout) {

    VkResult res;

    VkDescriptorSetAllocateInfo alloc_info[1];
    alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info[0].pNext = nullptr;
    alloc_info[0].descriptorPool = VulkanDevice::GetInstance()->GetDescriptorPool();
    alloc_info[0].descriptorSetCount = 1;
    alloc_info[0].pSetLayouts = &descSetLayout;

    res = _vkAllocateDescriptorSets(VulkanDevice::GetInstance()->getDevice(), alloc_info, &descriptorSet);
    vkUtil::checkError(res);

    VkWriteDescriptorSet writes[2];
    uint32_t bCnt = 0;
    VkWriteDescriptorSet& bufferMat = writes[bCnt];
    bufferMat = {};
    bufferMat.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    bufferMat.pNext = nullptr;
    bufferMat.dstSet = descriptorSet;
    bufferMat.descriptorCount = 1;
    bufferMat.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferMat.pBufferInfo = &uni->getBufferSet()->info;
    bufferMat.dstArrayElement = 0;
    bufferMat.dstBinding = bCnt++;

    if (useTexture) {
        VkWriteDescriptorSet& texSampler = writes[bCnt];
        texSampler = {};
        texSampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        texSampler.dstSet = descriptorSet;
        texSampler.dstBinding = bCnt++;
        texSampler.descriptorCount = 1;
        texSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texSampler.pImageInfo = &texture.info;
        texSampler.dstArrayElement = 0;
    }

    _vkUpdateDescriptorSets(VulkanDevice::GetInstance()->getDevice(), bCnt, writes, 0, nullptr);
}

VkPipelineCache RasterizeDescriptor::createPipelineCache() {
    VkPipelineCacheCreateInfo cacheInfo{};
    VkPipelineCache pipelineCache;
    cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    auto res = _vkCreatePipelineCache(VulkanDevice::GetInstance()->getDevice(), &cacheInfo, nullptr, &pipelineCache);
    vkUtil::checkError(res);
    return pipelineCache;
}

VkPipeline RasterizeDescriptor::createGraphicsPipelineVF(bool useAlpha,
    const VkPipelineShaderStageCreateInfo& vshader, const VkPipelineShaderStageCreateInfo& fshader,
    const VkVertexInputBindingDescription& bindDesc, const VkVertexInputAttributeDescription* attrDescs, uint32_t numAttr,
    const VkPipelineLayout& pLayout, const VkRenderPass renderPass, const VkPipelineCache& pCache) {

    VulkanSwapchain* sw = VulkanSwapchain::GetInstance();
    uint32_t width = sw->getSize().width;
    uint32_t height = sw->getSize().height;

    static VkViewport vports[] = { {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f} };
    static VkRect2D scissors[] = { {{0, 0}, {width,height}} };
    static VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineShaderStageCreateInfo stageInfo[2]{};
    stageInfo[0] = vshader;
    stageInfo[1] = fshader;

    VkPipelineVertexInputStateCreateInfo vinStateInfo{};
    vinStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vinStateInfo.vertexBindingDescriptionCount = 1;
    vinStateInfo.pVertexBindingDescriptions = &bindDesc;
    vinStateInfo.vertexAttributeDescriptionCount = numAttr;
    vinStateInfo.pVertexAttributeDescriptions = attrDescs;

    VkPipelineInputAssemblyStateCreateInfo iaInfo{};
    iaInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    iaInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo vpInfo{};
    vpInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vpInfo.viewportCount = 1;
    vpInfo.pViewports = vports;
    vpInfo.scissorCount = 1;
    vpInfo.pScissors = scissors;

    VkPipelineRasterizationStateCreateInfo rasterizerStateInfo{};
    rasterizerStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerStateInfo.depthClampEnable = VK_FALSE;
    rasterizerStateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerStateInfo.cullMode = VK_CULL_MODE_NONE;
    rasterizerStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerStateInfo.depthBiasEnable = VK_FALSE;
    rasterizerStateInfo.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo msInfo{};
    msInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    msInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    msInfo.sampleShadingEnable = VK_FALSE;
    msInfo.alphaToCoverageEnable = VK_FALSE;
    msInfo.alphaToOneEnable = VK_FALSE;
    if (useAlpha) {
        msInfo.alphaToCoverageEnable = VK_TRUE;
    }

    VkPipelineColorBlendAttachmentState blendState{};
    blendState.blendEnable = VK_FALSE;
    blendState.colorWriteMask = VK_COLOR_COMPONENT_A_BIT
        | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_R_BIT;

    VkPipelineColorBlendStateCreateInfo blendInfo{};
    blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendInfo.logicOpEnable = VK_FALSE;
    blendInfo.attachmentCount = 1;
    blendInfo.pAttachments = &blendState;

    VkPipelineDynamicStateCreateInfo dynamicInfo{};
    dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicInfo.dynamicStateCount = (uint32_t)COUNTOF(dynamicStates);
    dynamicInfo.pDynamicStates = dynamicStates;

    VkPipelineDepthStencilStateCreateInfo ds;
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.pNext = NULL;
    ds.flags = 0;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.stencilTestEnable = VK_FALSE;
    ds.back.failOp = VK_STENCIL_OP_KEEP;
    ds.back.passOp = VK_STENCIL_OP_KEEP;
    ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
    ds.back.compareMask = 0;
    ds.back.reference = 0;
    ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
    ds.back.writeMask = 0;
    ds.minDepthBounds = 0;
    ds.maxDepthBounds = 0;
    ds.stencilTestEnable = VK_FALSE;
    ds.front = ds.back;

    VkGraphicsPipelineCreateInfo gpInfo{};
    gpInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gpInfo.stageCount = (uint32_t)COUNTOF(stageInfo);
    gpInfo.pStages = stageInfo;
    gpInfo.pVertexInputState = &vinStateInfo;
    gpInfo.pInputAssemblyState = &iaInfo;
    gpInfo.pViewportState = &vpInfo;
    gpInfo.pRasterizationState = &rasterizerStateInfo;
    gpInfo.pMultisampleState = &msInfo;
    gpInfo.pColorBlendState = &blendInfo;
    gpInfo.pDynamicState = &dynamicInfo;
    gpInfo.pDepthStencilState = &ds;
    gpInfo.layout = pLayout;
    gpInfo.renderPass = renderPass;
    gpInfo.subpass = 0;

    VkPipeline pipeline;
    auto res = _vkCreateGraphicsPipelines(VulkanDevice::GetInstance()->getDevice(), pCache, 1, &gpInfo, nullptr, &pipeline);
    vkUtil::checkError(res);
    return pipeline;
}

void RasterizeDescriptor::setNumLight(uint32_t num) {
    numLight = num;
}

void RasterizeDescriptor::setLightAttenuation(float att1, float att2, float att3) {
    attenuation1 = att1;
    attenuation2 = att2;
    attenuation3 = att3;
}

void RasterizeDescriptor::setLight(uint32_t index, CoordTf::VECTOR3 pos, CoordTf::VECTOR3 color) {
    lightPos[index].as(pos.x, pos.y, pos.z, 0.0f);
    lightColor[index].as(color.x, color.y, color.z, 1.0f);
}