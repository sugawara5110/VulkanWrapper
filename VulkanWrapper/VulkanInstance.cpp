//*****************************************************************************************//
//**                                                                                     **//
//**                              VulkanInstance.cpp                                     **//
//**                                                                                     **//
//*****************************************************************************************//

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
	instanceInfo.enabledExtensionCount = std::size(extensions);
	instanceInfo.ppEnabledExtensionNames = extensions;
	instanceInfo.enabledLayerCount = std::size(layers);
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
	for (int i = 0; i < commandBufferCount; i++)vkFreeCommandBuffers(device, commandPool, commandBufferCount, commandBuffer.get());
	vkDestroyImageView(device, depth.view, nullptr);
	vkDestroyImage(device, depth.image, nullptr);
	vkFreeMemory(device, depth.mem, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	for (int i = 0; i < swBuf.imageCount; i++) {
		vkDestroyFramebuffer(device, swBuf.frameBuffer[i], nullptr);
		vkDestroyImageView(device, swBuf.views[i], nullptr);
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

	//グラフィックス用のデバイスキューのファミリー番号を取得
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
	devInfo.enabledLayerCount = std::size(layers);
	devInfo.ppEnabledLayerNames = layers;
	devInfo.enabledExtensionCount = std::size(extensions);
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
	//おそらくポストエフェクトはこのオブジェクトから画像取得して処理をする,VkImageはテクスチャ的なもの？
	res = vkGetSwapchainImagesKHR(device, swBuf.swapchain, &swBuf.imageCount, nullptr);//個数imageCount取得
	checkError(res);
	swBuf.images = std::make_unique<VkImage[]>(swBuf.imageCount);
	res = vkGetSwapchainImagesKHR(device, swBuf.swapchain, &swBuf.imageCount, swBuf.images.get());//個数分生成
	checkError(res);

	//ビュー生成
	swBuf.views = std::make_unique<VkImageView[]>(swBuf.imageCount);

	for (uint32_t i = 0; i < swBuf.imageCount; i++)
	{
		VkImageViewCreateInfo vinfo{};
		vinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		vinfo.image = swBuf.images[i];
		vinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		vinfo.format = VK_FORMAT_B8G8R8A8_UNORM;
		vinfo.components = {
			VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A
		};
		vinfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		auto res = vkCreateImageView(device, &vinfo, nullptr, &swBuf.views[i]);
		checkError(res);
	}
}

void Device::createDepth() {
	VkResult res;
	VkImageCreateInfo image_info = {};

	const VkFormat depth_format = VK_FORMAT_D16_UNORM;
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(pDev, depth_format, &props);
	if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		image_info.tiling = VK_IMAGE_TILING_LINEAR;
	}
	else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	}
	else {
		OutputDebugString(L"depth_formatUnsupported.\n");
		exit(-1);
	}

	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = nullptr;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = depth_format;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = NULL;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_info.flags = 0;

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = nullptr;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = nullptr;
	view_info.image = VK_NULL_HANDLE;
	view_info.format = depth_format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.flags = 0;

	if (depth_format == VK_FORMAT_D16_UNORM_S8_UINT || depth_format == VK_FORMAT_D24_UNORM_S8_UINT ||
		depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
		view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	VkMemoryRequirements mem_reqs;

	res = vkCreateImage(device, &image_info, nullptr, &depth.image);
	checkError(res);

	vkGetImageMemoryRequirements(device, depth.image, &mem_reqs);

	mem_alloc.allocationSize = mem_reqs.size;

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
		if ((memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			mem_alloc.memoryTypeIndex = i;
			break;
		}
	}
	if (mem_alloc.memoryTypeIndex == UINT32_MAX) throw std::runtime_error("No found available heap.");

	res = vkAllocateMemory(device, &mem_alloc, nullptr, &depth.mem);
	checkError(res);

	res = vkBindImageMemory(device, depth.image, depth.mem, 0);
	checkError(res);

	view_info.image = depth.image;
	res = vkCreateImageView(device, &view_info, nullptr, &depth.view);
	checkError(res);
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

void Device::submitCommandAndWait(uint32_t comBufindex) {
	static VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo sinfo{};

	sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sinfo.pWaitDstStageMask = &stageFlags;
	sinfo.commandBufferCount = 1;
	sinfo.pCommandBuffers = &commandBuffer[comBufindex];
	//コマンドをキューに送信
	auto res = vkQueueSubmit(devQueue, 1, &sinfo, VK_NULL_HANDLE);
	checkError(res);
	//フェンスをキューに送信し,そのフェンスがシグナルを受け取るまで待ち
	res = vkQueueWaitIdle(devQueue);
	checkError(res);
}

void Device::acquireNextImageAndWait(uint32_t& currentFrameIndex) {
	auto res = vkAcquireNextImageKHR(device, swBuf.swapchain,
		UINT64_MAX, VK_NULL_HANDLE, fence, &currentFrameIndex);
	checkError(res);
	res = vkWaitForFences(device, 1, &fence, VK_FALSE, UINT64_MAX);
	checkError(res);
	res = vkResetFences(device, 1, &fence);
	checkError(res);
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

void Device::barrierResource(uint32_t currentframeIndex, uint32_t comBufindex,
	VkPipelineStageFlags srcStageFlags,
	VkPipelineStageFlags dstStageFlags,
	VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
	VkImageLayout srcImageLayout, VkImageLayout dstImageLayout) {

	VkImageMemoryBarrier barrier{};

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = swBuf.images[currentframeIndex];
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	barrier.oldLayout = srcImageLayout;
	barrier.newLayout = dstImageLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	vkCmdPipelineBarrier(commandBuffer[comBufindex], srcStageFlags, dstStageFlags, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Device::beginRenderPass(uint32_t currentframeIndex, uint32_t comBufindex) {
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

void Device::createUniform(Uniform& uni) {
	VkResult res;

	MATRIX dummy;
	MatrixIdentity(&dummy);

	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buf_info.size = sizeof(dummy.m);
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = nullptr;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;
	res = vkCreateBuffer(device, &buf_info, nullptr, &uni.vkBuf);
	checkError(res);

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device, uni.vkBuf, &mem_reqs);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.memoryTypeIndex = 0;
	allocInfo.allocationSize = mem_reqs.size;

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
	{
		if ((memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
		{
			allocInfo.memoryTypeIndex = i;//メモリタイプインデックス検索
			break;
		}
	}
	if (allocInfo.memoryTypeIndex == UINT32_MAX) throw std::runtime_error("No mappable, coherent memory");

	res = vkAllocateMemory(device, &allocInfo, NULL, &uni.mem);
	checkError(res);

	uint8_t* pData;
	res = vkMapMemory(device, uni.mem, 0, mem_reqs.size, 0, (void**)& pData);
	uni.memSize = mem_reqs.size;
	checkError(res);

	memcpy(pData, &uni.mvp, sizeof(uni.mvp));

	vkUnmapMemory(device, uni.mem);
	uni.info.buffer = uni.vkBuf;
	uni.info.offset = 0;
	uni.info.range = sizeof(uni.mvp);

	res = vkBindBufferMemory(device, uni.vkBuf, uni.mem, 0);
	checkError(res);
}

void Device::updateUniform(Uniform& uni, MATRIX move) {

	MATRIX vm;
	MatrixMultiply(&vm, &move, &view);
	MatrixMultiply(&uni.mvp, &vm, &proj);

	uint8_t* pData;
	auto res = vkMapMemory(device, uni.mem, 0, uni.memSize, 0, (void**)& pData);

	checkError(res);

	memcpy(pData, &uni.mvp, sizeof(uni.mvp));

	vkUnmapMemory(device, uni.mem);
	uni.info.buffer = uni.vkBuf;
	uni.info.offset = 0;
	uni.info.range = sizeof(uni.mvp);
}

void Device::descriptorAndPipelineLayouts(VkPipelineLayout& pipelineLayout, VkDescriptorSetLayout& descSetLayout) {
	VkDescriptorSetLayoutBinding layout_bindings[2];
	layout_bindings[0].binding = 0;
	layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_bindings[0].descriptorCount = 1;
	layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layout_bindings[0].pImmutableSamplers = nullptr;

	/*if (use_texture) {
		layout_bindings[1].binding = 1;
		layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layout_bindings[1].descriptorCount = 1;
		layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layout_bindings[1].pImmutableSamplers = NULL;
	}*/

	/* Next take layout bindings and use them to create a descriptor set layout
	 */
	VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
	descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout.pNext = NULL;
	descriptor_layout.flags = 0;
	descriptor_layout.bindingCount = 1;//use_texture ? 2 : 1;
	descriptor_layout.pBindings = layout_bindings;

	VkResult res;

	res = vkCreateDescriptorSetLayout(device, &descriptor_layout, nullptr, &descSetLayout);
	checkError(res);

	/* Now use the descriptor layout to create a pipeline layout */
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

void Device::createDescriptorPool(VkDescriptorPool& descPool) {
	VkResult res;
	VkDescriptorPoolSize type_count[2];
	type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	type_count[0].descriptorCount = 1;
	/*if (use_texture) {
		type_count[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		type_count[1].descriptorCount = 1;
	}*/

	VkDescriptorPoolCreateInfo descriptor_pool = {};
	descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool.pNext = NULL;
	descriptor_pool.maxSets = 1;
	descriptor_pool.poolSizeCount = 1;//use_texture ? 2 : 1;
	descriptor_pool.pPoolSizes = type_count;

	res = vkCreateDescriptorPool(device, &descriptor_pool, nullptr, &descPool);
	checkError(res);
}

void Device::upDescriptorSet(Uniform& uni,
	VkDescriptorSet& descriptorSet,
	VkDescriptorPool& descPool,
	VkDescriptorSetLayout& descSetLayout)
{
	VkResult res;

	VkDescriptorSetAllocateInfo alloc_info[1];
	alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info[0].pNext = NULL;
	alloc_info[0].descriptorPool = descPool;
	alloc_info[0].descriptorSetCount = 1;
	alloc_info[0].pSetLayouts = &descSetLayout;

	res = vkAllocateDescriptorSets(device, alloc_info, &descriptorSet);
	checkError(res);

	VkWriteDescriptorSet writes[2];

	writes[0] = {};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].pNext = nullptr;
	writes[0].dstSet = descriptorSet;
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo = &uni.info;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;

	/*if (use_texture) {
		writes[1] = {};
		writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[1].dstSet = info.desc_set[0];
		writes[1].dstBinding = 1;
		writes[1].descriptorCount = 1;
		writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[1].pImageInfo = &info.texture_data.image_info;
		writes[1].dstArrayElement = 0;
	}*/

	vkUpdateDescriptorSets(device, 1, writes, 0, nullptr);
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

	static VkViewport vports[] = { { 0.0f, 0.0f, width, height, 0.0f, 1.0f } };
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
	dynamicInfo.dynamicStateCount = std::size(dynamicStates);
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
	gpInfo.stageCount = std::size(stageInfo);
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
	submitCommandAndWait(0);
	acquireNextImageAndWait(currentFrameIndex);
}

void Device::updateProjection(float AngleView, float Near, float Far) {
	MatrixPerspectiveFovLH(&proj, AngleView, (float)width / (float)height, Near, Far);
}

void Device::updateView(VECTOR3 vi, VECTOR3 gaze, VECTOR3 up) {
	MatrixLookAtLH(&view,
		vi.x, vi.y, vi.z,
		gaze.x, gaze.y, gaze.z,
		up.x, up.y, up.z);
}

void Device::beginCommand(uint32_t comBufindex) {
	beginCommandWithFramebuffer(comBufindex, swBuf.frameBuffer[currentFrameIndex]);

	barrierResource(currentFrameIndex, comBufindex,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	beginRenderPass(currentFrameIndex, comBufindex);
}

void Device::endCommand(uint32_t comBufindex) {
	vkCmdEndRenderPass(commandBuffer[comBufindex]);
	/*barrierResource(currentFrameIndex, comBufindex,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);*/

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
	acquireNextImageAndWait(currentFrameIndex);
}