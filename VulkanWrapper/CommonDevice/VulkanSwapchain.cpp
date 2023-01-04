//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanSwapchain.cpp                                    **//
//**                                                                                     **//
//*****************************************************************************************//

#include "VulkanSwapchain.h"

namespace {
    VulkanDevice* getDevice() {
        return VulkanDevice::GetInstance();
    }

    VkDevice getVkDevice() {
        return VulkanDevice::GetInstance()->getDevice();
    }

    VkCommandBuffer getCommand(uint32_t QueueIndex, uint32_t comBufindex) {
        return VulkanDevice::GetInstance()->getCommandObj(QueueIndex)->getCommandBuffer(comBufindex);
    }
}

VulkanSwapchain* VulkanSwapchain::swP = nullptr;

void VulkanSwapchain::InstanceCreate() {
    if (swP == nullptr)swP = new VulkanSwapchain();
}

VulkanSwapchain* VulkanSwapchain::GetInstance() {
    return swP;
}

void VulkanSwapchain::DeleteInstance() {
    if (swP != nullptr) {
        delete swP;
        swP = nullptr;
    }
}

VulkanSwapchain::~VulkanSwapchain() {
    if (swapchainAlive) {
        VkDevice d = getVkDevice();
        vkDestroySemaphore(d, presentCompletedSem, nullptr);
        vkDestroySemaphore(d, renderCompletedSem, nullptr);
        depth.destroy();
        vkDestroyRenderPass(d, renderPass, nullptr);
        vkDestroyFence(d, swFence, nullptr);
        renderPass = VK_NULL_HANDLE;
        swFence = VK_NULL_HANDLE;
        for (uint32_t i = 0; i < imageCount; i++) {
            vkDestroyImageView(d, views[i], nullptr);
            vkDestroyFramebuffer(d, frameBuffer[i], nullptr);
            views[i] = VK_NULL_HANDLE;
            frameBuffer[i] = VK_NULL_HANDLE;
        }
        vkDestroySwapchainKHR(d, swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
        swapchainAlive = false;
    }
}

void VulkanSwapchain::createSemaphore() {
    VkDevice d = getVkDevice();
    VkSemaphoreCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(d, &ci, nullptr, &renderCompletedSem);
    vkCreateSemaphore(d, &ci, nullptr, &presentCompletedSem);
}

void VulkanSwapchain::createswapchain(
    uint32_t QueueIndex, uint32_t comBufindex,
    VkPhysicalDevice pd, VkSurfaceKHR surface, bool V_SYNC) {

    if (V_SYNC) {
        presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }
    else {
        presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    VulkanDevice* dev = getDevice();

    VkSwapchainCreateInfoKHR scinfo{};
    //サーフェスの機能取得
    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface, &surfaceCaps);
    //サーフェスフォーマット数取得
    uint32_t surfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &surfaceFormatCount, nullptr);
    //サーフェスフォーマット取得
    BackBufferFormat = std::make_unique<VkSurfaceFormatKHR[]>(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &surfaceFormatCount, BackBufferFormat.get());
    //サーフェスでサポートされるプレゼンテーションモード数取得
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface, &presentModeCount, nullptr);
    //サーフェスでサポートされるプレゼンテーションモード取得
    auto presentModes = std::make_unique<VkPresentModeKHR[]>(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface, &presentModeCount, presentModes.get());

    for (uint32_t i = 0; i < surfaceFormatCount; i++) {
        auto c = BackBufferFormat[i];
#ifdef __ANDROID__
#else
        OutputDebugString(L"Supported Format Check...");
#endif
    }

    wh = surfaceCaps.currentExtent;

    scinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    scinfo.pNext = nullptr;
    scinfo.surface = surface;
    scinfo.minImageCount = surfaceCaps.minImageCount;
    scinfo.imageFormat = BackBufferFormat.get()->format;
    scinfo.imageColorSpace = BackBufferFormat.get()->colorSpace;
    scinfo.imageExtent = wh;
    scinfo.imageArrayLayers = surfaceCaps.maxImageArrayLayers;
    scinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    scinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    scinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    scinfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

#ifdef __ANDROID__
    scinfo.presentMode = presentModes[0];
#else
    scinfo.presentMode = presentMode;
#endif
    scinfo.clipped = VK_TRUE;
    VkDevice d = getVkDevice();
    //スワップチェーン生成
    auto res = vkCreateSwapchainKHR(d, &scinfo, nullptr, &swapchain);
    vkUtil::checkError(res);

    //ウインドウに直接表示する画像のオブジェクト生成
    res = vkGetSwapchainImagesKHR(d, swapchain, &imageCount,
        nullptr);//個数imageCount取得
    vkUtil::checkError(res);
    images = std::make_unique<VkImage[]>(imageCount);
    res = vkGetSwapchainImagesKHR(d, swapchain, &imageCount,
        images.get());//個数分生成
    vkUtil::checkError(res);

    VulkanDevice::CommandObj* com = dev->getCommandObj(QueueIndex);

    com->beginCommand(comBufindex);
    for (uint32_t i = 0; i < imageCount; ++i) {
        dev->barrierResource(QueueIndex, comBufindex, images[i],
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    com->endCommand(comBufindex);
    com->submitCommandsDoNotRender();

    //ビュー生成
    views = std::make_unique<VkImageView[]>(imageCount);
    format = scinfo.imageFormat;

    for (uint32_t i = 0; i < imageCount; i++) {
        views[i] = dev->createImageView(images[i], format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
             VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A });
    }
}

void VulkanSwapchain::createDepth(VkPhysicalDevice pd) {

    const VkFormat depth_format = VK_FORMAT_D32_SFLOAT;
    VkImageTiling tiling;
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(pd, depth_format, &props);
    if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        tiling = VK_IMAGE_TILING_LINEAR;
    }
    else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        tiling = VK_IMAGE_TILING_OPTIMAL;
    }
    else {
#ifdef __ANDROID__
#else
        OutputDebugString(L"depth_formatUnsupported.\n");
#endif
        exit(-1);
    }

    //深度Image生成
    depth.createImage(wh.width, wh.height, depth_format,
        tiling, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkImageAspectFlags depthMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    //view生成
    depth.createImageView(depth_format, depthMask,
        { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
         VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A });
}

void VulkanSwapchain::createFence() {
    VkFenceCreateInfo finfo{};
    finfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    auto res = vkCreateFence(getVkDevice(), &finfo, nullptr, &swFence);
    vkUtil::checkError(res);
    firstswFence = false;
}

void VulkanSwapchain::createRenderPass(bool clearBackBuffer) {

    VkAttachmentDescription attachmentDesc[2]{};
    attachmentDesc[0].format = format;
    attachmentDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDesc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDesc[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachmentDesc[1].format = depth.getFormat();
    attachmentDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDesc[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDesc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachmentDesc[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDesc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDesc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachmentDesc[1].flags = 0;

    if (!clearBackBuffer) {
        attachmentDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    VkAttachmentReference attachmentRef{};
    attachmentRef.attachment = 0;
    attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {};
    depth_reference.attachment = 1;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachmentRef;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depth_reference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachmentDesc;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    auto res = vkCreateRenderPass(getVkDevice(), &renderPassInfo, nullptr, &renderPass);
    vkUtil::checkError(res);
}

void VulkanSwapchain::createFramebuffers() {

    frameBuffer = std::make_unique<VkFramebuffer[]>(imageCount);

    VkImageView attachmentViews[2];
    attachmentViews[1] = depth.info.imageView;

    VkFramebufferCreateInfo fbinfo{};
    fbinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbinfo.pNext = nullptr;
    fbinfo.attachmentCount = 2;
    fbinfo.renderPass = renderPass;
    fbinfo.pAttachments = attachmentViews;
    fbinfo.width = wh.width;
    fbinfo.height = wh.height;
    fbinfo.layers = 1;
    fbinfo.flags = 0;

    for (uint32_t i = 0; i < imageCount; i++) {
        attachmentViews[0] = views[i];
        auto res = vkCreateFramebuffer(getVkDevice(), &fbinfo, nullptr, &frameBuffer[i]);
        vkUtil::checkError(res);
    }
}

void VulkanSwapchain::present(uint32_t QueueIndex) {
    VkPresentInfoKHR pinfo{};

    pinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pinfo.swapchainCount = 1;
    pinfo.pSwapchains = getSwapchain();
    pinfo.pImageIndices = getCurrentFrameIndex();
    pinfo.waitSemaphoreCount = 1;
    pinfo.pWaitSemaphores = &renderCompletedSem;

    auto res = vkQueuePresentKHR(getDevice()->getCommandObj(QueueIndex)->getQueue(), &pinfo);
    vkUtil::checkError(res);
}

void VulkanSwapchain::create(
    uint32_t QueueIndex, uint32_t comBufindex,
    VkPhysicalDevice pd, VkSurfaceKHR surface, bool clearBackBuffer, bool V_SYNC) {

    createSemaphore();
    createswapchain(QueueIndex, comBufindex, pd, surface, V_SYNC);
    createDepth(pd);
    createFence();
    createRenderPass(clearBackBuffer);
    createFramebuffers();
    swapchainAlive = true;
}

void VulkanSwapchain::acquireNextImageAndWait() {
    //vkAcquireNextImageKHR:命令はバックバッファのスワップを行い,次に描画されるべきImageのインデックスを返す
    auto res = vkAcquireNextImageKHR(getVkDevice(), swapchain,
        UINT64_MAX, presentCompletedSem, VK_NULL_HANDLE,
        &currentFrameIndex);
    vkUtil::checkError(res);
    if (!firstswFence) {
        firstswFence = true;
        return;
    }
    getDevice()->waitForFence(swFence);
}

void VulkanSwapchain::beginCommandNextImage(uint32_t QueueIndex, uint32_t comBufindex) {
    acquireNextImageAndWait();
    getDevice()->getCommandObj(QueueIndex)->beginCommand(comBufindex, getFramebuffer());
}

void VulkanSwapchain::beginDraw(uint32_t QueueIndex, uint32_t comBufindex) {
    static VkClearValue clearValue[2];
    clearValue[0].color.float32[0] = 0.0f;
    clearValue[0].color.float32[1] = 0.0f;
    clearValue[0].color.float32[2] = 0.0f;
    clearValue[0].color.float32[3] = 1.0f;
    clearValue[1].depthStencil.depth = 1.0f;
    clearValue[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo rpinfo{};
    rpinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpinfo.framebuffer = getFramebuffer();
    rpinfo.renderPass = renderPass;
    rpinfo.renderArea.extent.width = wh.width;
    rpinfo.renderArea.extent.height = wh.height;
    rpinfo.clearValueCount = 2;
    rpinfo.pClearValues = clearValue;

    vkCmdBeginRenderPass(getCommand(QueueIndex, comBufindex), &rpinfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanSwapchain::endDraw(uint32_t QueueIndex, uint32_t comBufindex) {
    vkCmdEndRenderPass(getCommand(QueueIndex, comBufindex));
}

void VulkanSwapchain::Present(uint32_t QueueIndex, uint32_t comBufindex) {

    getDevice()->getCommandObj(QueueIndex)->submitCommands(getFence(), true,
        1, &presentCompletedSem,
        1, &renderCompletedSem);

    present(QueueIndex);
}