﻿//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanInstance.cpp                                     **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "VulkanInstance.h"

void checkError(VkResult res) { if (res != VK_SUCCESS) throw std::runtime_error(std::to_string(res).c_str()); }

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT, uint64_t object,
	size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
	// OutputDebugString(L"Message Code: "); OutputDebugString(std::to_wstring(messageCode).c_str()); OutputDebugString(L"\n");
	OutputDebugString(L"Vulkan DebugCall: "); OutputDebugStringA(pMessage); OutputDebugString(L"\n");
	return VK_FALSE;
}

void VulkanInstance::createinstance(char* appName) {

	VkInstanceCreateInfo instanceInfo{};
	VkApplicationInfo appInfo{};
	//"VK_KHR_surface", "VK_KHR_win32_surface":Windows環境で必須, "VK_EXT_debug_report":検証レイヤで必須
	const char* extensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report" };
	const char* layers[] = { "VK_LAYER_LUNARG_standard_validation" };//検証レイヤ

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pApplicationName = appName;
	appInfo.pEngineName = appName;
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = (uint32_t)std::size(extensions);
	instanceInfo.ppEnabledExtensionNames = extensions;
	instanceInfo.enabledLayerCount = (uint32_t)std::size(layers);
	instanceInfo.ppEnabledLayerNames = layers;

	//Vulkanインスタンス生成
	auto res = vkCreateInstance(&instanceInfo, nullptr, &instance);
	checkError(res);

	//デバック
	_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
	_vkDebugReportMessageEXT = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT"));
	_vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
}

void VulkanInstance::createDebugReportCallback() {

	VkDebugReportCallbackCreateInfoEXT callbackInfo{};

	callbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callbackInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
	callbackInfo.pfnCallback = &debugCallback;
	//デバック有効化
	auto res = _vkCreateDebugReportCallbackEXT(instance, &callbackInfo, nullptr, &debugReportCallback);
	checkError(res);
}

void VulkanInstance::createSurfaceHwnd(HWND hWnd) {

	VkWin32SurfaceCreateInfoKHR surfaceInfo{};

	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.hinstance = GetModuleHandle(nullptr);
	surfaceInfo.hwnd = hWnd;
	//Windows用のサーフェース生成
	auto res = vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
	checkError(res);
}

void VulkanInstance::createPhysicalDevice() {

	//物理デバイス出力先にnullptrを指定:adapterCountに物理デバイス個数出力
	auto res = vkEnumeratePhysicalDevices(instance, &adapterCount, nullptr);
	checkError(res);
	adapters = std::make_unique<VkPhysicalDevice[]>(adapterCount);
	//個数分の物理デバイス出力
	res = vkEnumeratePhysicalDevices(instance, &adapterCount, adapters.get());
	checkError(res);
	//物理デバイスのプロパティ情報出力
	OutputDebugString(L"=== Physical Device Enumeration ===\n");
	for (uint32_t i = 0; i < adapterCount; i++) {
		static VkPhysicalDeviceProperties props;
		static VkPhysicalDeviceMemoryProperties memProps;
		vkGetPhysicalDeviceProperties(adapters[i], &props);
		vkGetPhysicalDeviceMemoryProperties(adapters[i], &memProps);

		OutputDebugString(L"#"); OutputDebugString(std::to_wstring(i).c_str()); OutputDebugString(L": \n");
		OutputDebugString(L"  Name: "); OutputDebugStringA(props.deviceName); OutputDebugString(L"\n");
		OutputDebugString(L"  API Version: "); OutputDebugString(std::to_wstring(props.apiVersion).c_str()); OutputDebugString(L"\n");
	}
}

VulkanInstance::~VulkanInstance() {
	vkDestroySurfaceKHR(instance, surface, nullptr);
	_vkDestroyDebugReportCallbackEXT(instance, debugReportCallback, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void VulkanInstance::createInstance(HWND hWnd, char* appName) {
	createinstance(appName);//インスタンス生成
	createDebugReportCallback();//デバック
	createSurfaceHwnd(hWnd);//ウインドウズ用サーフェス生成
	createPhysicalDevice();//物理デバイス生成
}

VkPhysicalDevice VulkanInstance::getPhysicalDevice(int index) {
	return adapters[index];
}

VkSurfaceKHR VulkanInstance::getSurface() {
	return surface;
}

Device::Device(VkPhysicalDevice pd, VkSurfaceKHR surfa, uint32_t wid, uint32_t hei) {
	pDev = pd;
	surface = surfa;
	width = wid;
	height = hei;
}

Device::~Device() {
	destroyTexture();
	for (uint32_t i = 0; i < commandBufferCount; i++)vkFreeCommandBuffers(device, commandPool, commandBufferCount, commandBuffer.get());
	vkDestroyImageView(device, depth.view, nullptr);
	vkDestroyImage(device, depth.image, nullptr);
	vkFreeMemory(device, depth.mem, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	for (uint32_t i = 0; i < swBuf.imageCount; i++) {
		vkDestroyImageView(device, swBuf.views[i], nullptr);
		vkDestroyFramebuffer(device, swBuf.frameBuffer[i], nullptr);
	}
	vkDestroySwapchainKHR(device, swBuf.swapchain, nullptr);
	vkDestroyFence(device, fence, nullptr);
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDeviceWaitIdle(device);
	vkDestroyDevice(device, nullptr);
}

void Device::create() {
	VkDeviceCreateInfo devInfo{};
	VkDeviceQueueCreateInfo queueInfo{};

	//グラフィックス用のデバイスキューのファミリー番号を取得:VK_QUEUE_GRAPHICS_BIT
	uint32_t propertyCount;
	//nullptr指定でプロパティ数取得
	vkGetPhysicalDeviceQueueFamilyProperties(pDev, &propertyCount, nullptr);
	auto properties = std::make_unique<VkQueueFamilyProperties[]>(propertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(pDev, &propertyCount, properties.get());
	for (uint32_t i = 0; i < propertyCount; i++)
	{
		if ((properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			queueFamilyIndex = i;
			break;
		}
	}
	if (queueFamilyIndex == 0xffffffff) throw std::runtime_error("No Graphics queues available on current device.");

	const char* layers[] = { "VK_LAYER_LUNARG_standard_validation" };
	const char* extensions[] = { "VK_KHR_swapchain" };//スワップチェーンで必須
	static float qPriorities[] = { 0.0f };
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pNext = nullptr;
	queueInfo.queueCount = 1;
	queueInfo.queueFamilyIndex = queueFamilyIndex;
	queueInfo.pQueuePriorities = qPriorities;

	devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	devInfo.pNext = nullptr;
	devInfo.queueCreateInfoCount = 1;
	devInfo.pQueueCreateInfos = &queueInfo;
	devInfo.enabledLayerCount = (uint32_t)std::size(layers);
	devInfo.ppEnabledLayerNames = layers;
	devInfo.enabledExtensionCount = (uint32_t)std::size(extensions);
	devInfo.ppEnabledExtensionNames = extensions;
	devInfo.pEnabledFeatures = nullptr;

	//論理デバイス生成,キューも生成される
	auto res = vkCreateDevice(pDev, &devInfo, nullptr, &device);
	checkError(res);
	//キュー取得
	vkGetDeviceQueue(device, queueFamilyIndex, 0, &devQueue);
	//VRAMプロパティ取得:頂点バッファ取得時使用
	vkGetPhysicalDeviceMemoryProperties(pDev, &memProps);
}

void Device::createCommandPool() {
	VkCommandPoolCreateInfo info{};

	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;
	info.queueFamilyIndex = queueFamilyIndex;
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	//コマンドプールの作成:コマンドバッファーメモリが割り当てられるオブジェクト
	auto res = vkCreateCommandPool(device, &info, nullptr, &commandPool);
	checkError(res);
}

void Device::createFence() {
	VkFenceCreateInfo finfo{};
	finfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	auto res = vkCreateFence(device, &finfo, nullptr, &fence);
	checkError(res);
}

void Device::createSwapchain() {

	VkSwapchainCreateInfoKHR scinfo{};
	//デバイスが,スワップチェーンをサポートしているか確認
	VkBool32 surfaceSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR(pDev, queueFamilyIndex, surface, &surfaceSupported);
	//サーフェスの機能取得
	VkSurfaceCapabilitiesKHR surfaceCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDev, surface, &surfaceCaps);
	//サーフェスフォーマット数取得
	uint32_t surfaceFormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(pDev, surface, &surfaceFormatCount, nullptr);
	//サーフェスフォーマット取得
	auto surfaceFormats = std::make_unique<VkSurfaceFormatKHR[]>(surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(pDev, surface, &surfaceFormatCount, surfaceFormats.get());
	//サーフェスでサポートされるプレゼンテーションモード数取得
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(pDev, surface, &presentModeCount, nullptr);
	//サーフェスでサポートされるプレゼンテーションモード取得
	auto presentModes = std::make_unique<VkPresentModeKHR[]>(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(pDev, surface, &presentModeCount, presentModes.get());

	for (uint32_t i = 0; i < surfaceFormatCount; i++)
	{
		auto c = surfaceFormats[i];
		OutputDebugString(L"Supported Format Check...");
	}

	scinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	scinfo.pNext = nullptr;
	scinfo.surface = surface;
	scinfo.minImageCount = 2;
	scinfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	scinfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	scinfo.imageExtent.width = width;
	scinfo.imageExtent.height = height;
	scinfo.imageArrayLayers = 1;
	scinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	scinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	scinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	scinfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	scinfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	scinfo.clipped = VK_TRUE;
	//スワップチェーン生成
	auto res = vkCreateSwapchainKHR(device, &scinfo, nullptr, &swBuf.swapchain);
	checkError(res);

	//ウインドウに直接表示する画像のオブジェクト生成
	res = vkGetSwapchainImagesKHR(device, swBuf.swapchain, &swBuf.imageCount, nullptr);//個数imageCount取得
	checkError(res);
	swBuf.images = std::make_unique<VkImage[]>(swBuf.imageCount);
	res = vkGetSwapchainImagesKHR(device, swBuf.swapchain, &swBuf.imageCount, swBuf.images.get());//個数分生成
	checkError(res);

	//ビュー生成
	swBuf.views = std::make_unique<VkImageView[]>(swBuf.imageCount);

	for (uint32_t i = 0; i < swBuf.imageCount; i++) {
		swBuf.views[i] = createImageView(swBuf.images[i], VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,
			{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A });
	}
}

void Device::createDepth() {

	const VkFormat depth_format = VK_FORMAT_D16_UNORM;
	VkImageTiling tiling;
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(pDev, depth_format, &props);
	if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		tiling = VK_IMAGE_TILING_LINEAR;
	}
	else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		tiling = VK_IMAGE_TILING_OPTIMAL;
	}
	else {
		OutputDebugString(L"depth_formatUnsupported.\n");
		exit(-1);
	}

	//深度Image生成
	createImage(width, height, depth_format,
		tiling, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depth.image, depth.mem);

	VkImageAspectFlags depthMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	if (depth_format == VK_FORMAT_D16_UNORM_S8_UINT || depth_format == VK_FORMAT_D24_UNORM_S8_UINT ||
		depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
		depthMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	//view生成
	depth.view = createImageView(depth.image, depth_format, depthMask,
		{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A });
	depth.format = depth_format;
}

void Device::createCommonRenderPass() {

	VkAttachmentDescription attachmentDesc[2]{};
	attachmentDesc[0].format = VK_FORMAT_B8G8R8A8_UNORM;
	attachmentDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDesc[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachmentDesc[1].format = depth.format;
	attachmentDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDesc[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachmentDesc[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDesc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDesc[1].flags = 0;

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

	auto res = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
	checkError(res);
}

void Device::createFramebuffers() {

	swBuf.frameBuffer = std::make_unique<VkFramebuffer[]>(swBuf.imageCount);

	VkImageView attachmentViews[2];
	attachmentViews[1] = depth.view;

	VkFramebufferCreateInfo fbinfo{};
	fbinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbinfo.pNext = nullptr;
	fbinfo.attachmentCount = 2;
	fbinfo.renderPass = renderPass;
	fbinfo.pAttachments = attachmentViews;
	fbinfo.width = width;
	fbinfo.height = height;
	fbinfo.layers = 1;
	fbinfo.flags = 0;
	//1度のDrawCallで描画するバッファをまとめたオブジェクトの生成
	for (uint32_t i = 0; i < swBuf.imageCount; i++) {
		attachmentViews[0] = swBuf.views[i];
		auto res = vkCreateFramebuffer(device, &fbinfo, nullptr, &swBuf.frameBuffer[i]);
		checkError(res);
	}
}

void Device::createCommandBuffers() {

	VkCommandBufferAllocateInfo cbAllocInfo{};

	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = commandPool;
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbAllocInfo.commandBufferCount = commandBufferCount;
	//コマンドバッファの作成
	commandBuffer = std::make_unique<VkCommandBuffer[]>(commandBufferCount);
	auto res = vkAllocateCommandBuffers(device, &cbAllocInfo, commandBuffer.get());
	checkError(res);
}

void Device::initialImageLayouting(uint32_t comBufindex) {
	auto barriers = std::make_unique<VkImageMemoryBarrier[]>(swBuf.imageCount);

	for (uint32_t i = 0; i < swBuf.imageCount; i++)
	{
		VkImageMemoryBarrier barrier{};

		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.image = swBuf.images[i];
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;
		barriers[i] = barrier;
	}

	vkCmdPipelineBarrier(commandBuffer[comBufindex], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0, 0, nullptr, 0, nullptr, swBuf.imageCount, barriers.get());
}

void Device::beginCommandWithFramebuffer(uint32_t comBufindex, VkFramebuffer fb) {
	VkCommandBufferInheritanceInfo inhInfo{};
	VkCommandBufferBeginInfo beginInfo{};

	inhInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inhInfo.framebuffer = fb;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pInheritanceInfo = &inhInfo;
	//コマンド記録開始
	vkBeginCommandBuffer(commandBuffer[comBufindex], &beginInfo);
}

void Device::submitCommands(uint32_t comBufindex) {
	VkSubmitInfo sinfo{};
	static const VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sinfo.commandBufferCount = 1;
	sinfo.pCommandBuffers = &commandBuffer[comBufindex];
	sinfo.pWaitDstStageMask = &waitStageMask;
	auto res = vkQueueSubmit(devQueue, 1, &sinfo, fence);
	checkError(res);
}

void Device::acquireNextImageAndWait(uint32_t& currentFrameIndex) {
	//vkAcquireNextImageKHR:命令はバックバッファのスワップを行い,次に描画されるべきImageのインデックスを返す
	auto res = vkAcquireNextImageKHR(device, swBuf.swapchain,
		UINT64_MAX, VK_NULL_HANDLE, fence, &currentFrameIndex);
	checkError(res);
	res = vkWaitForFences(device, 1, &fence, VK_FALSE, UINT64_MAX);
	checkError(res);
	res = vkResetFences(device, 1, &fence);
	checkError(res);
}

VkResult Device::waitForFence() {
	//コマンドの処理が指定数(ここでは1)終わるまで待つ
	//条件が満たされた場合待ち解除でVK_SUCCESSを返す
	//条件が満たされない,かつ
	//タイムアウト(ここではUINT64_MAX)に達した場合,待ち解除でVK_TIMEOUTを返す
	return vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
}

void Device::present(uint32_t currentframeIndex) {
	VkPresentInfoKHR pinfo{};

	pinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pinfo.swapchainCount = 1;
	pinfo.pSwapchains = &swBuf.swapchain;
	pinfo.pImageIndices = &currentframeIndex;

	auto res = vkQueuePresentKHR(devQueue, &pinfo);
	checkError(res);
}

void Device::resetFence() {
	//指定数(ここでは1)のフェンスをリセット
	auto res = vkResetFences(device, 1, &fence);
	checkError(res);
}

void Device::barrierResource(uint32_t comBufindex, VkImage image,
	VkImageLayout srcImageLayout, VkImageLayout dstImageLayout) {

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = srcImageLayout;
	barrier.newLayout = dstImageLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	if (srcImageLayout == VK_IMAGE_LAYOUT_UNDEFINED && dstImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (srcImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && dstImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (srcImageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && dstImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(commandBuffer[comBufindex], srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Device::beginRenderPass(uint32_t comBufindex, uint32_t currentframeIndex) {
	static VkClearValue clearValue[2];
	clearValue[0].color.float32[0] = 0.0f;
	clearValue[0].color.float32[1] = 0.0f;
	clearValue[0].color.float32[2] = 0.0f;
	clearValue[0].color.float32[3] = 1.0f;
	clearValue[1].depthStencil.depth = 1.0f;
	clearValue[1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo rpinfo{};
	rpinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpinfo.framebuffer = swBuf.frameBuffer[currentframeIndex];
	rpinfo.renderPass = renderPass;
	rpinfo.renderArea.extent.width = width;
	rpinfo.renderArea.extent.height = height;
	rpinfo.clearValueCount = 2;
	rpinfo.pClearValues = clearValue;

	vkCmdBeginRenderPass(commandBuffer[comBufindex], &rpinfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Device::copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void Device::createImage(uint32_t width, uint32_t height, VkFormat format,
	VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
	VkImage& image, VkDeviceMemory& imageMemory) {

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
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	auto res = vkCreateImage(device, &imageInfo, nullptr, &image);
	checkError(res);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	res = vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory);
	checkError(res);

	res = vkBindImageMemory(device, image, imageMemory, 0);
	checkError(res);
}

auto Device::createTextureImage(unsigned char* byteArr, uint32_t width, uint32_t height) {

	VkDeviceSize imageSize = width * height * 4;

	if (!byteArr) {
		throw std::runtime_error("failed to load texture image!");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkDeviceSize size;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory, size);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, byteArr, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	Texture texture;
	createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		texture.vkIma, texture.mem);

	beginCommandWithFramebuffer(0, swBuf.frameBuffer[currentFrameIndex]);
	barrierResource(0, texture.vkIma, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(commandBuffer[0], stagingBuffer, texture.vkIma, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	barrierResource(0, texture.vkIma, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	vkEndCommandBuffer(commandBuffer[0]);
	submitCommands(0);
	waitForFence();
	resetFence();

	texture.info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	texture.height = height;
	texture.width = width;
	texture.memSize = imageSize;

	return texture;
}

VkImageView Device::createImageView(VkImage image, VkFormat format,
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
	VkResult res = vkCreateImageView(device, &viewInfo, nullptr, &imageView);
	checkError(res);
	return imageView;
}

void Device::createTextureSampler(VkSampler& textureSampler) {

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	samplerInfo.flags = VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT;
	auto res = vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler);
	checkError(res);
}

void Device::destroyTexture() {
	vkDestroySampler(device, textureSampler, nullptr);
	for (uint32_t i = 0; i < numTexture; i++) {
		vkDestroyImageView(device, texture[i].info.imageView, nullptr);
		vkDestroyImage(device, texture[i].vkIma, nullptr);
		vkFreeMemory(device, texture[i].mem, nullptr);
	}
}

char* Device::getNameFromPass(char* pass) {

	uint32_t len = (uint32_t)strlen(pass);
	pass += len;//終端文字を指している

	for (uint32_t i = 0; i < len; i++) {
		pass--;
		if (*pass == '\\' || *pass == '/') {
			pass++;
			break;
		}
	}
	return pass;//ポインタ操作してるので返り値を使用させる
}

void Device::createUniform(UniformSet& uni, UniformSetMaterial& material) {

	createBuffer(sizeof(Uniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uni.vkBuf, uni.mem, uni.memSize);

	uni.info.buffer = uni.vkBuf;
	uni.info.offset = 0;
	uni.info.range = sizeof(Uniform);

	createBuffer(sizeof(UniformMaterial), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, material.vkBuf, material.mem, material.memSize);

	material.info.buffer = material.vkBuf;
	material.info.offset = 0;
	material.info.range = sizeof(UniformMaterial);
}

void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize& memSize) {

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	auto res = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
	checkError(res);

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	memSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	res = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
	checkError(res);

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void Device::copyBuffer(uint32_t comBufindex, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

	beginCommandWithFramebuffer(comBufindex, VkFramebuffer());

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer[comBufindex], srcBuffer, dstBuffer, 1, &copyRegion);

	auto res = vkEndCommandBuffer(commandBuffer[comBufindex]);
	checkError(res);
	submitCommands(comBufindex);
	res = vkWaitForFences(device, 1, &fence, VK_FALSE, UINT64_MAX);
	checkError(res);
	res = vkResetFences(device, 1, &fence);
	checkError(res);
}

uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(pDev, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to findMemoryType");
}

void Device::updateUniform(UniformSet& uni, MATRIX move, UniformSetMaterial& material) {

	MATRIX vm;
	MatrixMultiply(&vm, &move, &view);
	MatrixMultiply(&uni.uni.mvp, &vm, &proj);
	uni.uni.world = move;

	uint8_t* pData;
	auto res = vkMapMemory(device, uni.mem, 0, uni.memSize, 0, (void**)& pData);
	checkError(res);
	memcpy(pData, &uni.uni, sizeof(Uniform));
	vkUnmapMemory(device, uni.mem);
	uni.info.buffer = uni.vkBuf;
	uni.info.offset = 0;
	uni.info.range = sizeof(Uniform);

	material.uni.viewPos.as(viewPos.x, viewPos.y, viewPos.z, 0.0f);
	memcpy(material.uni.lightPos, lightPos, sizeof(VECTOR4) * numLight);
	memcpy(material.uni.lightColor, lightColor, sizeof(VECTOR4) * numLight);
	material.uni.numLight.x = (float)numLight;
	material.uni.numLight.y = attenuation1;
	material.uni.numLight.z = attenuation2;
	material.uni.numLight.w = attenuation3;

	uint8_t* pData2;
	res = vkMapMemory(device, material.mem, 0, material.memSize, 0, (void**)& pData2);
	checkError(res);
	memcpy(pData2, &material.uni, sizeof(UniformMaterial));
	vkUnmapMemory(device, material.mem);
	material.info.buffer = material.vkBuf;
	material.info.offset = 0;
	material.info.range = sizeof(UniformMaterial);
}

void Device::descriptorAndPipelineLayouts(bool useTexture, VkPipelineLayout& pipelineLayout, VkDescriptorSetLayout& descSetLayout) {
	VkDescriptorSetLayoutBinding layout_bindings[4];
	layout_bindings[0].binding = 0;
	layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_bindings[0].descriptorCount = 1;
	layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layout_bindings[0].pImmutableSamplers = nullptr;

	if (useTexture) {

		layout_bindings[1].binding = 1;
		layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layout_bindings[1].descriptorCount = 1;
		layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layout_bindings[1].pImmutableSamplers = nullptr;

		layout_bindings[2].binding = 2;
		layout_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layout_bindings[2].descriptorCount = 1;
		layout_bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layout_bindings[2].pImmutableSamplers = nullptr;

		layout_bindings[3].binding = 3;
		layout_bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layout_bindings[3].descriptorCount = 1;
		layout_bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layout_bindings[3].pImmutableSamplers = nullptr;
	}

	VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
	descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout.pNext = NULL;
	descriptor_layout.flags = 0;
	descriptor_layout.bindingCount = useTexture ? 4 : 1;
	descriptor_layout.pBindings = layout_bindings;

	VkResult res;

	res = vkCreateDescriptorSetLayout(device, &descriptor_layout, nullptr, &descSetLayout);
	checkError(res);

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = nullptr;
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	pPipelineLayoutCreateInfo.setLayoutCount = 1;
	pPipelineLayoutCreateInfo.pSetLayouts = &descSetLayout;

	res = vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	checkError(res);
}

VkPipelineLayout Device::createPipelineLayout2D() {
	VkPipelineLayoutCreateInfo pLayoutInfo{};
	pLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	VkPipelineLayout pipelineLayout;
	auto res = vkCreatePipelineLayout(device, &pLayoutInfo, nullptr, &pipelineLayout);
	checkError(res);
	return pipelineLayout;
}

VkShaderModule Device::createShaderModule(char* shader) {

	VkShaderModuleCreateInfo shaderInfo{};
	VkShaderModule mod;
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderInfo.codeSize = strlen(shader);
	shaderInfo.pCode = reinterpret_cast<uint32_t*>(shader);

	auto res = vkCreateShaderModule(device, &shaderInfo, nullptr, &mod);
	checkError(res);
	return mod;
}

void Device::createDescriptorPool(bool useTexture, VkDescriptorPool& descPool) {
	VkResult res;
	VkDescriptorPoolSize type_count[4];
	type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	type_count[0].descriptorCount = 1;
	if (useTexture) {
		type_count[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		type_count[1].descriptorCount = 1;

		type_count[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		type_count[2].descriptorCount = 1;

		type_count[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		type_count[3].descriptorCount = 1;
	}

	VkDescriptorPoolCreateInfo descriptor_pool = {};
	descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool.pNext = nullptr;
	descriptor_pool.maxSets = 1;
	descriptor_pool.poolSizeCount = useTexture ? 4 : 1;
	descriptor_pool.pPoolSizes = type_count;

	res = vkCreateDescriptorPool(device, &descriptor_pool, nullptr, &descPool);
	checkError(res);
}

void Device::upDescriptorSet(bool useTexture, Texture difTexture, Texture norTexture, UniformSet& uni, UniformSetMaterial& material,
	VkDescriptorSet& descriptorSet, VkDescriptorPool& descPool, VkDescriptorSetLayout& descSetLayout) {

	VkResult res;

	VkDescriptorSetAllocateInfo alloc_info[1];
	alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info[0].pNext = NULL;
	alloc_info[0].descriptorPool = descPool;
	alloc_info[0].descriptorSetCount = 1;
	alloc_info[0].pSetLayouts = &descSetLayout;

	res = vkAllocateDescriptorSets(device, alloc_info, &descriptorSet);
	checkError(res);

	VkWriteDescriptorSet writes[4];

	writes[0] = {};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].pNext = nullptr;
	writes[0].dstSet = descriptorSet;
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo = &uni.info;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;

	if (useTexture) {
		writes[1] = {};
		writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[1].dstSet = descriptorSet;
		writes[1].dstBinding = 1;
		writes[1].descriptorCount = 1;
		writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[1].pImageInfo = &difTexture.info;
		writes[1].dstArrayElement = 0;

		writes[2] = {};
		writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[2].dstSet = descriptorSet;
		writes[2].dstBinding = 2;
		writes[2].descriptorCount = 1;
		writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[2].pImageInfo = &norTexture.info;
		writes[2].dstArrayElement = 0;

		writes[3] = {};
		writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[3].pNext = nullptr;
		writes[3].dstSet = descriptorSet;
		writes[3].descriptorCount = 1;
		writes[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writes[3].pBufferInfo = &material.info;
		writes[3].dstArrayElement = 0;
		writes[3].dstBinding = 3;
	}

	vkUpdateDescriptorSets(device, 4, writes, 0, nullptr);
}

VkPipelineCache Device::createPipelineCache() {
	VkPipelineCacheCreateInfo cacheInfo{};
	VkPipelineCache pipelineCache;
	cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	auto res = vkCreatePipelineCache(device, &cacheInfo, nullptr, &pipelineCache);
	checkError(res);
	return pipelineCache;
}

VkPipeline Device::createGraphicsPipelineVF(
	const VkShaderModule& vshader, const VkShaderModule& fshader,
	const VkVertexInputBindingDescription& bindDesc, const VkVertexInputAttributeDescription* attrDescs, uint32_t numAttr,
	const VkPipelineLayout& pLayout, const VkRenderPass renderPass, const VkPipelineCache& pCache) {

	static VkViewport vports[] = { { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f } };
	static VkRect2D scissors[] = { { { 0, 0 }, { width, height } } };
	static VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineShaderStageCreateInfo stageInfo[2]{};
	stageInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stageInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stageInfo[0].module = vshader;
	stageInfo[1].module = fshader;
	stageInfo[0].pName = "main";
	stageInfo[1].pName = "main";

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

	VkPipelineColorBlendAttachmentState blendState{};
	blendState.blendEnable = VK_FALSE;
	blendState.colorWriteMask = VK_COLOR_COMPONENT_A_BIT
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT;

	VkPipelineColorBlendStateCreateInfo blendInfo{};
	blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendInfo.logicOpEnable = VK_FALSE;
	blendInfo.attachmentCount = 1;
	blendInfo.pAttachments = &blendState;

	VkPipelineDynamicStateCreateInfo dynamicInfo{};
	dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicInfo.dynamicStateCount = (uint32_t)std::size(dynamicStates);
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
	gpInfo.stageCount = (uint32_t)std::size(stageInfo);
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
	auto res = vkCreateGraphicsPipelines(device, pCache, 1, &gpInfo, nullptr, &pipeline);
	checkError(res);
	return pipeline;
}

void Device::createDevice() {
	create();
	createCommandPool();
	createFence();
	createSwapchain();
	createDepth();
	createCommonRenderPass();
	createFramebuffers();
	createCommandBuffers();

	//バリア初期化
	beginCommandWithFramebuffer(0, VkFramebuffer());
	initialImageLayouting(0);
	//コマンドの記録終了
	auto res = vkEndCommandBuffer(commandBuffer[0]);
	checkError(res);
	submitCommands(0);
	res = vkWaitForFences(device, 1, &fence, VK_FALSE, UINT64_MAX);
	checkError(res);
	res = vkResetFences(device, 1, &fence);
	checkError(res);
}

void Device::GetTexture(char* fileName, unsigned char* byteArr, uint32_t width, uint32_t height) {
	char* filename = getNameFromPass(fileName);
	if (strlen(filename) >= (size_t)numTexFileNamelenMax)
		throw std::runtime_error("The file name limit has been.");
	strcpy(textureNameList[numTexture], filename);
	texture[numTexture] = createTextureImage(byteArr, width, height);
	texture[numTexture].info.imageView = createImageView(texture[numTexture].vkIma,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	createTextureSampler(texture[numTexture].info.sampler);
	numTexture++;
	if (numTexture >= numTextureMax)
		throw std::runtime_error("The file limit has been.");
}

int32_t Device::getTextureNum(char* pass) {
	for (uint32_t i = 0; i < numTexture; i++) {
		size_t len1 = strlen(textureNameList[i]);
		size_t len2 = strlen(pass);

		if (len1 == len2 && !strcmp(textureNameList[i], pass))return i;
	}
	throw std::runtime_error("File name does not exist.");
	return -1;
}

void Device::updateProjection(float AngleView, float Near, float Far) {
	MatrixPerspectiveFovLH(&proj, AngleView, (float)width / (float)height, Near, Far);
}

void Device::updateView(VECTOR3 vi, VECTOR3 gaze, VECTOR3 up) {
	MatrixLookAtLH(&view,
		vi.x, vi.y, vi.z,
		gaze.x, gaze.y, gaze.z,
		up.x, up.y, up.z);
	viewPos.as(vi.x, vi.y, vi.z, 0.0f);
}

void Device::setNumLight(uint32_t num) {
	numLight = num;
}

void Device::setLightAttenuation(float att1, float att2, float att3) {
	attenuation1 = att1;
	attenuation2 = att2;
	attenuation3 = att3;
}

void Device::setLight(uint32_t index, VECTOR3 pos, VECTOR4 color) {
	lightPos[index].as(pos.x, pos.y, pos.z, 0.0f);
	lightColor[index].as(color.x, color.y, color.z, color.w);
}

void Device::beginCommand(uint32_t comBufindex) {
	acquireNextImageAndWait(currentFrameIndex);
	beginCommandWithFramebuffer(comBufindex, swBuf.frameBuffer[currentFrameIndex]);

	barrierResource(comBufindex, swBuf.images[currentFrameIndex],
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	beginRenderPass(comBufindex, currentFrameIndex);
}

void Device::endCommand(uint32_t comBufindex) {
	vkCmdEndRenderPass(commandBuffer[comBufindex]);
	vkEndCommandBuffer(commandBuffer[comBufindex]);
}

void Device::waitFence(uint32_t comBufindex) {
	submitCommands(comBufindex);
	switch (waitForFence())
	{
	case VK_SUCCESS: present(currentFrameIndex); break;
	case VK_TIMEOUT: throw std::runtime_error("Command execution timed out."); break;
	default: OutputDebugString(L"waitForFence returns unknown value.\n");
	}
	resetFence();
}