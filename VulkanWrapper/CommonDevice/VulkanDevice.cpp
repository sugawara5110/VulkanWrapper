//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanDevice.cpp                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "VulkanDevice.h"

static uint32_t ApiVersion = VK_API_VERSION_1_0;
VulkanDevice* VulkanDevice::DevicePointer = nullptr;

void VulkanDevice::BufferSet::createUploadBuffer(VkDeviceSize size, VkBufferUsageFlags usage, void* allocateMemory_add_pNext) {
    DevicePointer->createUploadBuffer(size, usage, buffer, mem, allocateMemory_add_pNext);
    Size = size;
    info.range = VK_WHOLE_SIZE;
    info.buffer = buffer;
    info.offset = 0;
}

void VulkanDevice::BufferSet::createDefaultBuffer(VkDeviceSize size, VkBufferUsageFlags usage, void* allocateMemory_add_pNext) {
    DevicePointer->createDefaultBuffer(size, usage, buffer, mem, allocateMemory_add_pNext);
    Size = size;
    info.range = VK_WHOLE_SIZE;
    info.buffer = buffer;
    info.offset = 0;
}

void VulkanDevice::BufferSet::memoryMap(void* pData) {
    memcpy(Map(), pData, (size_t)Size);
    UnMap();
}

void* VulkanDevice::BufferSet::Map() {
    return DevicePointer->Map(mem);
}

void VulkanDevice::BufferSet::UnMap() {
    DevicePointer->UnMap(mem);
}

void VulkanDevice::BufferSet::destroy() {
    VkDevice d = DevicePointer->device;
    _vkDestroyBuffer(d, buffer, nullptr);
    _vkFreeMemory(d, mem, nullptr);
    buffer = VK_NULL_HANDLE;
    mem = VK_NULL_HANDLE;
}

void VulkanDevice::ImageSet::barrierResource(
    uint32_t QueueIndex, uint32_t comBufindex, VkImageLayout dstImageLayout, VkImageAspectFlagBits mask) {
    DevicePointer->barrierResource(QueueIndex, comBufindex, image, info.imageLayout, dstImageLayout, mask);
    info.imageLayout = dstImageLayout;
}

void VulkanDevice::ImageSet::createImage(uint32_t width, uint32_t height, VkFormat format,
    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
    VkImageLayout initialLayout) {

    Width = width;
    Height = height;

    DevicePointer->createImage(width, height, format, tiling, usage, properties,
        image, mem, initialLayout);

    info.imageLayout = initialLayout;
    this->format = format;
}

void VulkanDevice::ImageSet::createImageView(VkFormat format, VkImageAspectFlags mask,
    VkComponentMapping components) {

    info.imageView = DevicePointer->createImageView(image, format, mask, components);
    this->format = format;
}

void VulkanDevice::ImageSet::destroy() {
    VkDevice d = DevicePointer->device;
    _vkDestroyImageView(d, info.imageView, nullptr);
    _vkDestroySampler(d, info.sampler, nullptr);
    _vkDestroyImage(d, image, nullptr);
    _vkFreeMemory(d, mem, nullptr);
    info.imageView = VK_NULL_HANDLE;
    info.sampler = VK_NULL_HANDLE;
    image = VK_NULL_HANDLE;
    mem = VK_NULL_HANDLE;
}

void VulkanDevice::CommandObj::createCommandPool() {
    VkDevice d = VulkanDevice::GetInstance()->getDevice();
    VkCommandPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.queueFamilyIndex = queueFamilyIndex;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    //コマンドプールの作成:コマンドバッファーメモリが割り当てられるオブジェクト
    auto res = _vkCreateCommandPool(d, &info, nullptr, &commandPool);
    vkUtil::checkError(res);
}

void VulkanDevice::CommandObj::createFence() {
    VkDevice d = VulkanDevice::GetInstance()->getDevice();
    VkFenceCreateInfo finfo{};
    finfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    auto res = _vkCreateFence(d, &finfo, nullptr, &fence);
    vkUtil::checkError(res);
}

void VulkanDevice::CommandObj::createCommandBuffers() {
    VkDevice d = VulkanDevice::GetInstance()->getDevice();
    VkCommandBufferAllocateInfo cbAllocInfo{};
    cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbAllocInfo.commandPool = commandPool;
    cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbAllocInfo.commandBufferCount = (uint32_t)commandBuffer.size();
    //コマンドバッファの作成
    auto res = _vkAllocateCommandBuffers(d, &cbAllocInfo, commandBuffer.data());
    vkUtil::checkError(res);
    temp = std::make_unique<VkCommandBuffer[]>(commandBuffer.size());
    status = std::make_unique<Status[]>(commandBuffer.size());
    for (size_t i = 0; i < commandBuffer.size(); i++) {
        status[i] = OPEN;
    }
}

void VulkanDevice::CommandObj::create(uint32_t numCommand) {
    commandBuffer.resize(numCommand);
    createCommandPool();
    createFence();
    createCommandBuffers();
}

void VulkanDevice::CommandObj::destroy() {
    VkDevice d = VulkanDevice::GetInstance()->getDevice();
    _vkFreeCommandBuffers(d, commandPool, (uint32_t)commandBuffer.size(), commandBuffer.data());
    _vkDestroyFence(d, fence, nullptr);
    _vkDestroyCommandPool(d, commandPool, nullptr);
}

uint32_t VulkanDevice::CommandObj::getQueueFamilyIndex() {
    return queueFamilyIndex;
}

VkQueue VulkanDevice::CommandObj::getQueue() {
    return devQueue;
}

void VulkanDevice::CommandObj::beginCommand(uint32_t comBufindex, VkFramebuffer fb) {
    VkCommandBufferInheritanceInfo inhInfo{};
    VkCommandBufferBeginInfo beginInfo{};

    inhInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inhInfo.framebuffer = fb;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pInheritanceInfo = &inhInfo;
    //コマンド記録開始
    _vkBeginCommandBuffer(commandBuffer[comBufindex], &beginInfo);
}

void VulkanDevice::CommandObj::endCommand(uint32_t comBufindex) {
    _vkEndCommandBuffer(commandBuffer[comBufindex]);
    status[comBufindex] = CLOSE;
}

void VulkanDevice::CommandObj::submitCommands(VkFence fence,
    uint32_t waitSemaphoreCount, VkSemaphore* WaitSemaphores,
    uint32_t signalSemaphoreCount, VkSemaphore* SignalSemaphores) {

    uint32_t comCnt = 0;
    for (size_t i = 0; i < commandBuffer.size(); i++) {
        if (status[i] == CLOSE) {
            temp[comCnt++] = commandBuffer[i];
        }
        status[i] = OPEN;
    }

    VkSubmitInfo sinfo{};
    static const VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sinfo.commandBufferCount = comCnt;
    sinfo.pCommandBuffers = temp.get();
    sinfo.pWaitDstStageMask = &waitStageMask;
    sinfo.waitSemaphoreCount = waitSemaphoreCount;
    sinfo.pWaitSemaphores = WaitSemaphores;
    sinfo.signalSemaphoreCount = signalSemaphoreCount;
    sinfo.pSignalSemaphores = SignalSemaphores;

    auto res = _vkQueueSubmit(devQueue, 1, &sinfo, fence);
    vkUtil::checkError(res);
}

void VulkanDevice::CommandObj::submitCommandsAndWait() {
    submitCommands(fence,
        0, nullptr,
        0, nullptr);
    VulkanDevice::GetInstance()->waitForFence(fence);
    VulkanDevice::GetInstance()->resetFence(fence);
}

void VulkanDevice::CommandObj::submitCommands() {
    submitCommands(fence,
        0, nullptr,
        0, nullptr);
}

void VulkanDevice::CommandObj::Wait() {
    VulkanDevice::GetInstance()->waitForFence(fence);
    VulkanDevice::GetInstance()->resetFence(fence);
}

VkCommandBuffer VulkanDevice::CommandObj::getCommandBuffer(uint32_t comBufindex) {
    return commandBuffer[comBufindex];
}

VulkanDevice::VulkanDevice(VkPhysicalDevice pd, uint32_t numCommandBuffer, uint32_t numGraphicsQueue, uint32_t numComputeQueue) {
    pDev = pd;
    _vkGetPhysicalDeviceProperties(pDev, &physicalDeviceProperties);
    commandBufferCount = numCommandBuffer;
    NumGraphicsQueue = numGraphicsQueue;
    NumComputeQueue = numComputeQueue;
    NumAllQueue = NumGraphicsQueue + NumComputeQueue;
}

VulkanDevice::~VulkanDevice() {
    _vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    destroyTexture();
    for (uint32_t i = 0; i < NumAllQueue; i++) {
        commandObj[i].destroy();
    }
    _vkDeviceWaitIdle(device);
    _vkDestroyDevice(device, nullptr);
}

void VulkanDevice::create(std::vector<const char*>* requiredExtensions, const void* pNext) {
    VkDeviceCreateInfo devInfo{};
    VkDeviceQueueCreateInfo queueInfo[2] = {};

    const uint32_t uintMax = 0xffffffff;

    uint32_t queueFamilyIndexGRAPHICS = uintMax;
    uint32_t cntGr = 0;
    uint32_t queueFamilyIndexCOMPUTE = uintMax;
    uint32_t cntCo = 0;

    //デバイスキューのファミリー番号を取得
    uint32_t propertyCount;
    //nullptr指定でプロパティ数取得
    _vkGetPhysicalDeviceQueueFamilyProperties(pDev, &propertyCount, nullptr);
    auto properties = std::make_unique<VkQueueFamilyProperties[]>(propertyCount);
    _vkGetPhysicalDeviceQueueFamilyProperties(pDev, &propertyCount, properties.get());

    for (uint32_t i = 0; i < propertyCount; i++) {
        if ((properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            queueFamilyIndexGRAPHICS = i;
            cntGr = properties[i].queueCount;
            break;
        }
    }
    for (uint32_t i = 0; i < propertyCount; i++) {
        if ((properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 &&
            (properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) {
            queueFamilyIndexCOMPUTE = i;
            cntCo = properties[i].queueCount;
            break;
        }
    }

    if (queueFamilyIndexGRAPHICS == uintMax) {
        throw std::runtime_error("No Graphics queues available on current device.");
    }

    const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
    std::vector<const char*> extensions = { "VK_KHR_swapchain" };//スワップチェーンで必須

    if (requiredExtensions) {
        for (auto& e : (*requiredExtensions)) {
            extensions.push_back(e);
        }
    }

    if (NumGraphicsQueue <= 0) {
        throw std::runtime_error("graphicsQueue.size() is less than or equal to 0.");
    }

    if (NumGraphicsQueue > cntGr) {
        throw std::runtime_error("graphicsQueue.size() is too large.");
    }
    if (NumComputeQueue > cntCo) {
        throw std::runtime_error("computeQueue.size() is too large.");
    }

    float* qPrioritiesGr = NEW float[NumGraphicsQueue];//優先度配列 max1.0f
    float step = 1.0f / (float)NumGraphicsQueue;
    float cnt = 1.0f;
    for (size_t i = 0; i < NumGraphicsQueue; i++) {
        qPrioritiesGr[i] = cnt;
        cnt -= step;
    }

    float* qPrioritiesCo = nullptr;
    if (NumComputeQueue > 0) {
        qPrioritiesCo = NEW float[NumComputeQueue];//優先度配列 max1.0f
        float step = 1.0f / (float)NumComputeQueue;
        float cnt = 1.0f;
        for (size_t i = 0; i < NumComputeQueue; i++) {
            qPrioritiesCo[i] = cnt;
            cnt -= step;
        }
    }

    queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo[0].pNext = nullptr;
    queueInfo[0].queueCount = (uint32_t)NumGraphicsQueue;
    queueInfo[0].queueFamilyIndex = queueFamilyIndexGRAPHICS;
    queueInfo[0].pQueuePriorities = qPrioritiesGr;

    queueInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo[1].pNext = nullptr;
    queueInfo[1].queueCount = (uint32_t)NumComputeQueue;
    queueInfo[1].queueFamilyIndex = queueFamilyIndexCOMPUTE;
    queueInfo[1].pQueuePriorities = qPrioritiesCo;

    uint32_t InfoCount = 1;
    if (NumComputeQueue > 0)InfoCount = 2;

    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.pNext = pNext;
    devInfo.queueCreateInfoCount = InfoCount;
    devInfo.pQueueCreateInfos = queueInfo;
    devInfo.enabledLayerCount = (uint32_t)COUNTOF(layers);
    devInfo.ppEnabledLayerNames = layers;
    devInfo.enabledExtensionCount = (uint32_t)extensions.size();
    devInfo.ppEnabledExtensionNames = extensions.data();
    devInfo.pEnabledFeatures = nullptr;

    //論理デバイス生成,キューも生成される
    auto res = _vkCreateDevice(pDev, &devInfo, nullptr, &device);
    vkUtil::checkError(res);
    //キュー取得
    commandObj.resize(NumAllQueue);
    for (uint32_t i = 0; i < NumGraphicsQueue; i++) {
        CommandObj* Gr = &commandObj[i];
        _vkGetDeviceQueue(device, queueFamilyIndexGRAPHICS, i, &Gr->devQueue);
        Gr->queueFamilyIndex = queueFamilyIndexGRAPHICS;
    }
    for (uint32_t i = 0; i < NumComputeQueue; i++) {
        CommandObj* Co = &commandObj[i + NumGraphicsQueue];
        _vkGetDeviceQueue(device, queueFamilyIndexCOMPUTE, i, &Co->devQueue);
        Co->queueFamilyIndex = queueFamilyIndexCOMPUTE;
    }
    //VRAMプロパティ取得:頂点バッファ取得時使用
    _vkGetPhysicalDeviceMemoryProperties(pDev, &memProps);

    vkUtil::ARR_DELETE(qPrioritiesGr);
    vkUtil::ARR_DELETE(qPrioritiesCo);
}

VkResult VulkanDevice::waitForFence(VkFence fence) {
    //コマンドの処理が指定数(ここでは1)終わるまで待つ
    //条件が満たされた場合待ち解除でVK_SUCCESSを返す
    //条件が満たされない,かつ
    //タイムアウト(ここではUINT64_MAX)に達した場合,待ち解除でVK_TIMEOUTを返す
    return _vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
}

void VulkanDevice::resetFence(VkFence fence) {
    //指定数(ここでは1)のフェンスをリセット
    auto res = _vkResetFences(device, 1, &fence);
    vkUtil::checkError(res);
}

void VulkanDevice::barrierResource(uint32_t QueueIndex, uint32_t comBufindex, VkImage image,
    VkImageLayout srcImageLayout, VkImageLayout dstImageLayout, VkImageAspectFlagBits mask) {

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = srcImageLayout;
    barrier.newLayout = dstImageLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = mask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    switch (srcImageLayout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        barrier.srcAccessMask = 0;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        break;
    };

    switch (dstImageLayout) {
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        break;
    }

    _vkCmdPipelineBarrier(commandObj[QueueIndex].commandBuffer[comBufindex], srcStage, dstStage,
        0, 0, nullptr,
        0, nullptr,
        1, &barrier);
}

void VulkanDevice::copyBufferToImage(
    uint32_t QueueIndex,
    uint32_t comBufindex,
    VkBuffer buffer,
    VkImage image, uint32_t width, uint32_t height,
    VkImageAspectFlagBits mask) {

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = mask;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
            width,
            height,
            1
    };

    _vkCmdCopyBufferToImage(commandObj[QueueIndex].commandBuffer[comBufindex],
        buffer,
        image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region);
}

void VulkanDevice::copyImageToBuffer(
    uint32_t QueueIndex,
    uint32_t comBufindex,
    VkImage image, uint32_t width, uint32_t height,
    VkBuffer buffer,
    VkImageAspectFlagBits mask) {

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = mask;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
            width,
            height,
            1
    };

    _vkCmdCopyImageToBuffer(commandObj[QueueIndex].commandBuffer[comBufindex],
        image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        buffer, 1, &region);
}

void VulkanDevice::createImage(uint32_t width, uint32_t height, VkFormat format,
    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
    VkImage& image, VkDeviceMemory& imageMemory, VkImageLayout initialLayout) {

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = initialLayout;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto res = _vkCreateImage(device, &imageInfo, nullptr, &image);
    vkUtil::checkError(res);

    VkMemoryRequirements memRequirements;
    _vkGetImageMemoryRequirements(device, image, &memRequirements);

    AllocateMemory(usage, memRequirements, properties, imageMemory, nullptr);

    res = _vkBindImageMemory(device, image, imageMemory, 0);
    vkUtil::checkError(res);
}

auto VulkanDevice::createTextureImage(uint32_t QueueIndex, uint32_t comBufindex, Texture& inByte, BufferSet& stagingBuffer) {

    VkDeviceSize imageSize = inByte.width * inByte.height * 4;

    if (!inByte.byte) {
        throw std::runtime_error("failed to load texture image!");
    }

    stagingBuffer.createUploadBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, nullptr);

    ImageSet texture;
    texture.createImage(inByte.width, inByte.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    updateTextureImage(QueueIndex, comBufindex, inByte, stagingBuffer, texture);

    return texture;
}

void VulkanDevice::updateTextureImage(uint32_t QueueIndex, uint32_t comBufindex, Texture& inByte, BufferSet& stagingBuffer, ImageSet& DefaultBuffer) {

    if (!inByte.byte) {
        throw std::runtime_error("failed to load texture image!");
    }

    stagingBuffer.memoryMap(inByte.byte);

    CommandObj* com = &commandObj[QueueIndex];

    com->beginCommand(comBufindex);

    DefaultBuffer.barrierResource(QueueIndex, comBufindex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(QueueIndex, comBufindex, stagingBuffer.getBuffer(), DefaultBuffer.getImage(),
        static_cast<uint32_t>(inByte.width), static_cast<uint32_t>(inByte.height), VK_IMAGE_ASPECT_COLOR_BIT);

    DefaultBuffer.barrierResource(QueueIndex, comBufindex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    com->endCommand(comBufindex);
    com->submitCommandsAndWait();
}

VkImageView VulkanDevice::createImageView(VkImage image, VkFormat format,
    VkImageAspectFlags mask, VkComponentMapping components) {

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components.r = components.r;
    viewInfo.components.g = components.g;
    viewInfo.components.b = components.b;
    viewInfo.components.a = components.a;
    viewInfo.subresourceRange.aspectMask = mask;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VkResult res = _vkCreateImageView(device, &viewInfo, nullptr, &imageView);
    vkUtil::checkError(res);
    return imageView;
}

void VulkanDevice::createTextureSampler(VkSampler& textureSampler) {

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    //samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;//VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXTの場合はこれにする
    //samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;//VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXTの場合はこれにする
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    //samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;//VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXTの場合はこれにする
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.flags = VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT;
    auto res = _vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler);
    vkUtil::checkError(res);
}

void VulkanDevice::destroyTexture() {
    for (uint32_t i = 0; i < numTexture; i++) {
        texture[i].destroy();
    }
    texture[numTextureMax].destroy();
    texture[numTextureMax + 1].destroy();
}

void VulkanDevice::AllocateMemory(VkBufferUsageFlags usage, VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties,
    VkDeviceMemory& bufferMemory, void* add_pNext) {

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    allocInfo.pNext = add_pNext;

    auto res = _vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
    vkUtil::checkError(res);
}

void VulkanDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
    VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext) {

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto res = _vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
    vkUtil::checkError(res);

    VkMemoryRequirements memRequirements;
    _vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    AllocateMemory(usage, memRequirements, properties, bufferMemory, allocateMemory_add_pNext);

    _vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanDevice::createUploadBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
    VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext) {

    createBuffer(size, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        buffer, bufferMemory, allocateMemory_add_pNext);
}

void VulkanDevice::createDefaultBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
    VkBuffer& buffer, VkDeviceMemory& bufferMemory, void* allocateMemory_add_pNext) {

    createBuffer(size, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        buffer, bufferMemory, allocateMemory_add_pNext);
}

void VulkanDevice::copyBuffer(uint32_t QueueIndex, uint32_t comBufindex, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

    CommandObj* com = &commandObj[QueueIndex];
    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    _vkCmdCopyBuffer(com->commandBuffer[comBufindex], srcBuffer, dstBuffer, 1, &copyRegion);
}

void VulkanDevice::copyImage(
    uint32_t QueueIndex, uint32_t comBufindex,
    VkImage srcImage, VkImageLayout srcImageLayout,
    VkImage dstImage, VkImageLayout dstImageLayout,
    uint32_t width, uint32_t height,
    VkImageAspectFlagBits mask) {

    VkImageSubresourceLayers Subresource = {};
    Subresource.aspectMask = mask;
    Subresource.mipLevel = 0;
    Subresource.baseArrayLayer = 0;
    Subresource.layerCount = 1;
    VkOffset3D Offset = { 0, 0, 0 };
    VkExtent3D extent = { width,height,1 };

    VkImageCopy region = {};
    region.srcSubresource = Subresource;
    region.srcOffset = Offset;
    region.dstSubresource = Subresource;
    region.dstOffset = Offset;
    region.extent = extent;

    CommandObj* com = &commandObj[QueueIndex];

    _vkCmdCopyImage(com->commandBuffer[comBufindex],
        srcImage, srcImageLayout, dstImage, dstImageLayout,
        1, &region);
}

uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {

    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to findMemoryType");
}

void* VulkanDevice::Map(VkDeviceMemory mem) {
    void* mdata;
    auto res = _vkMapMemory(device, mem, 0, VK_WHOLE_SIZE, 0, &mdata);
    vkUtil::checkError(res);
    return mdata;
}

void VulkanDevice::UnMap(VkDeviceMemory mem) {
    _vkUnmapMemory(device, mem);
}

void VulkanDevice::memoryMap(void* pData, VkDeviceMemory mem, VkDeviceSize size) {
    memcpy(Map(mem), pData, (size_t)size);
    UnMap(mem);
}

static std::vector<uint32_t> CompileShader(const char* fname, char* source, VkShaderStageFlags stage) {

    shaderc_env_version env_version = shaderc_env_version_vulkan_1_0;
    switch (ApiVersion) {
    case VK_API_VERSION_1_0:
        env_version = shaderc_env_version_vulkan_1_0;
        break;
    case VK_API_VERSION_1_1:
        env_version = shaderc_env_version_vulkan_1_1;
        break;
    case VK_API_VERSION_1_2:
        env_version = shaderc_env_version_vulkan_1_2;
        break;
    case VK_API_VERSION_1_3:
        env_version = shaderc_env_version_vulkan_1_3;
        break;
    }

    shaderc::Compiler compiler;
    auto sourceText = std::string(source, strlen(source));
    // コンパイラに設定するオプションの指定.
    shaderc::CompileOptions options{};
    options.SetTargetEnvironment(
        shaderc_target_env_vulkan, env_version);
#if _DEBUG
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
    options.SetGenerateDebugInfo();
#else
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif
    // シェーダーの種類.
    shaderc_shader_kind kind;
    switch (stage) {
    case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        kind = shaderc_raygen_shader;
        break;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        kind = shaderc_closesthit_shader;
        break;
    case VK_SHADER_STAGE_MISS_BIT_KHR:
        kind = shaderc_miss_shader;
        break;
    case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        kind = shaderc_anyhit_shader;
        break;
    case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        kind = shaderc_intersection_shader;
        break;
    case VK_SHADER_STAGE_VERTEX_BIT:
        kind = shaderc_vertex_shader;
        break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
        kind = shaderc_fragment_shader;
        break;
    case VK_SHADER_STAGE_COMPUTE_BIT:
        kind = shaderc_compute_shader;
        break;
    }
    // コンパイルの実行.
    auto result = compiler.CompileGlslToSpv(
        sourceText, kind, fname, options);
    auto status = result.GetCompilationStatus();
    std::vector<uint32_t> compiled;
    if (status == shaderc_compilation_status_success) {
        // コンパイルに成功. SPIR-Vコードを取得.
        compiled.assign(result.cbegin(), result.cend());
    }
    else {
        auto errMessage = result.GetErrorMessage();
#ifdef __ANDROID__
#else
        OutputDebugStringA(errMessage.c_str());
#endif
    }
    return compiled;
}

VkPipelineShaderStageCreateInfo VulkanDevice::createShaderModule(const char* fname, char* shader, VkShaderStageFlagBits stage) {

    std::vector<uint32_t> spv = CompileShader(fname, shader, stage);

    VkShaderModuleCreateInfo shaderInfo{};
    VkShaderModule mod;
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = spv.size() * sizeof(uint32_t);
    shaderInfo.pCode = spv.data();

    auto res = _vkCreateShaderModule(device, &shaderInfo, nullptr, &mod);
    vkUtil::checkError(res);

    VkPipelineShaderStageCreateInfo stageInfo = {};

    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = stage;
    stageInfo.module = mod;
    stageInfo.pName = "main";

    return stageInfo;
}

void VulkanDevice::createDevice(
    std::vector<const char*>* requiredExtensions, const void* pNext,
    std::vector<VkDescriptorPoolSize>* add_poolSize, uint32_t maxDescriptorSets) {

    create(requiredExtensions, pNext);
    for (size_t i = 0; i < commandObj.size(); i++) {
        commandObj[i].create(commandBufferCount);
    }
    createDescriptorPool(add_poolSize, maxDescriptorSets);

    //ダミーテクスチャ生成(テクスチャーが無い場合に代わりに入れる)
    unsigned char* dummyNor = NEW unsigned char[64 * 4 * 64];
    unsigned char* dummyDifSpe = NEW unsigned char[64 * 4 * 64];
    unsigned char nor[4] = { 128,128,255,0 };
    unsigned char difspe[4] = { 255,255,255,255 };
    for (int i = 0; i < 64 * 4 * 64; i += 4) {
        memcpy(&dummyNor[i], nor, sizeof(unsigned char) * 4);
        memcpy(&dummyDifSpe[i], difspe, sizeof(unsigned char) * 4);
    }

    texture[numTextureMax].width = 64;
    texture[numTextureMax].height = 64;
    texture[numTextureMax].setByte(dummyNor);
    texture[numTextureMax + 1].width = 64;
    texture[numTextureMax + 1].height = 64;
    texture[numTextureMax + 1].setByte(dummyDifSpe);
    vkUtil::ARR_DELETE(dummyNor);
    vkUtil::ARR_DELETE(dummyDifSpe);
}

void VulkanDevice::createVkTexture(ImageSet& tex, uint32_t QueueIndex, uint32_t comBufindex, Texture& inByte, BufferSet& stagingBuffer) {
    tex = createTextureImage(QueueIndex, comBufindex, inByte, stagingBuffer);
    tex.createImageView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    createTextureSampler(tex.info.sampler);
}

void VulkanDevice::createTextureSet(uint32_t QueueIndex, uint32_t comIndex, textureIdSet& texSet) {
    VulkanDevice* d = VulkanDevice::GetInstance();
    if (texSet.diffuseId < 0) {
        VulkanDevice::Texture& tex = d->texture[d->numTextureMax + 1];
        d->createVkTexture(texSet.difTex, QueueIndex, comIndex,
            tex, texSet.updifTex);
        if (tex.use_movie) {
            tex.defArr.push_back(&texSet.difTex);
            tex.upArr.push_back(&texSet.updifTex);
        }
    }
    else {
        VulkanDevice::Texture& tex = d->texture[texSet.diffuseId];
        d->createVkTexture(texSet.difTex, QueueIndex, comIndex,
            tex, texSet.updifTex);
        if (tex.use_movie) {
            tex.defArr.push_back(&texSet.difTex);
            tex.upArr.push_back(&texSet.updifTex);
        }
    }

    if (texSet.normalId < 0) {
        BufferSet bs;
        d->createVkTexture(texSet.norTex, QueueIndex, comIndex,
            d->texture[d->numTextureMax], bs);
        bs.destroy();
    }
    else {
        BufferSet bs;
        d->createVkTexture(texSet.norTex, QueueIndex, comIndex,
            d->texture[texSet.normalId], bs);
        bs.destroy();
    }

    if (texSet.specularId < 0) {
        BufferSet bs;
        d->createVkTexture(texSet.speTex, QueueIndex, comIndex,
            d->texture[d->numTextureMax + 1], bs);
        bs.destroy();
    }
    else {
        BufferSet bs;
        d->createVkTexture(texSet.speTex, QueueIndex, comIndex,
            d->texture[texSet.specularId], bs);
        bs.destroy();
    }
}

void VulkanDevice::GetTexture(
    char* fileName, unsigned char* byteArr,
    uint32_t width, uint32_t height, bool use_movie) {
    //ファイル名登録
    char* filename = vkUtil::getNameFromPath(fileName);
    if (strlen(filename) >= (size_t)numTexFileNamelenMax)
        throw std::runtime_error("The file name limit has been.");
    strcpy(textureNameList[numTexture], filename);

    //テクスチャ登録
    texture[numTexture].width = width;
    texture[numTexture].height = height;
    texture[numTexture].setByte(byteArr);
    texture[numTexture].use_movie = use_movie;
    numTexture++;
    if (numTexture >= numTextureMax)
        throw std::runtime_error("The file limit has been.");
}

void VulkanDevice::updateTexture(uint32_t QueueIndex, uint32_t comBufindex, char* fileName, unsigned char* frame) {
    int32_t ind = getTextureNo(fileName);
    Texture tex = getTexture(ind);
    VkDeviceSize imageSize = tex.width * tex.height * 4;
    memcpy(tex.byte, frame, sizeof(unsigned char) * imageSize);
    if (tex.use_movie) {
        for (size_t i = 0; i < tex.upArr.size(); i++) {
            updateTextureImage(QueueIndex, comBufindex, tex, *tex.upArr[i], *tex.defArr[i]);
        }
    }
}

int32_t VulkanDevice::getTextureNo(char* path) {
    if (!path)return -1;
    for (uint32_t i = 0; i < numTexture; i++) {
        size_t len1 = strlen(textureNameList[i]);
        size_t len2 = strlen(path);

        if (len1 == len2 && !strcmp(textureNameList[i], path))return i;
    }
    return -1;
}

void VulkanDevice::updateProjection(VkExtent2D wh, float AngleView, float Near, float Far) {
    MatrixPerspectiveFovLH(&proj, AngleView, (float)wh.width / (float)wh.height, Near, Far);
}

void VulkanDevice::updateView(CoordTf::VECTOR3 vi, CoordTf::VECTOR3 gaze, CoordTf::VECTOR3 up) {
    MatrixLookAtLH(&view, vi, gaze, up);
    upVec = up;
    viewPos.as(vi.x, vi.y, vi.z, 0.0f);
}

void VulkanDevice::DeviceWaitIdle() {
    _vkDeviceWaitIdle(device);
}

void VulkanDevice::InstanceCreate(
    VkPhysicalDevice pd,
    uint32_t apiVersion,
    uint32_t numCommandBuffer,
    uint32_t numGraphicsQueue,
    uint32_t numComputeQueue) {

    if (DevicePointer == nullptr)DevicePointer = NEW VulkanDevice(pd, numCommandBuffer, numGraphicsQueue, numComputeQueue);
    ApiVersion = apiVersion;
}

VulkanDevice* VulkanDevice::GetInstance() {
    return DevicePointer;
}

void VulkanDevice::DeleteInstance() {
    if (DevicePointer != nullptr) {
        delete DevicePointer;
        DevicePointer = nullptr;
    }
}

const char* VulkanDevice::GetDeviceName() const {
    return physicalDeviceProperties.deviceName;
}

VkDeviceSize VulkanDevice::GetUniformBufferAlignment() const {
    return physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
}

VkDeviceSize VulkanDevice::GetStorageBufferAlignment() const {
    return physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;
}

bool VulkanDevice::createDescriptorPool(std::vector<VkDescriptorPoolSize>* add_poolSize, uint32_t maxDescriptorSets) {

    VulkanDevice* vkDev = VulkanDevice::GetInstance();
    VkResult result;
    VkDescriptorPoolSize poolSize[] = {
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 100 },
    };
    VkDescriptorPoolCreateInfo descPoolCI{
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      nullptr,  VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
      500, // maxDescriptorSets数
      COUNTOF(poolSize), poolSize
    };

    if (add_poolSize) {
        descPoolCI.poolSizeCount = (uint32_t)(*add_poolSize).size();
        descPoolCI.pPoolSizes = (*add_poolSize).data();
    }
    if (maxDescriptorSets > 0) {
        descPoolCI.maxSets = maxDescriptorSets;
    }

    result = _vkCreateDescriptorPool(vkDev->getDevice(), &descPoolCI, nullptr, &descriptorPool);
    return result == VK_SUCCESS;
}

VkDescriptorSet VulkanDevice::AllocateDescriptorSet(VkDescriptorSetLayout dsLayout, const void* pNext) {

    VulkanDevice* vkDev = VulkanDevice::GetInstance();
    VkDescriptorSetAllocateInfo dsAI{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr
    };
    dsAI.descriptorPool = descriptorPool;
    dsAI.pSetLayouts = &dsLayout;
    dsAI.descriptorSetCount = 1;
    dsAI.pNext = pNext;
    VkDescriptorSet ds{};
    auto r = _vkAllocateDescriptorSets(vkDev->getDevice(), &dsAI, &ds);
    vkUtil::checkError(r);
    return ds;
}

void VulkanDevice::DeallocateDescriptorSet(VkDescriptorSet ds) {

    VulkanDevice* vkDev = VulkanDevice::GetInstance();
    _vkFreeDescriptorSets(vkDev->getDevice(), descriptorPool, 1, &ds);
}