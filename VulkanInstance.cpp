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
	const char* extensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report" };
	const char* layers[] = { "VK_LAYER_LUNARG_standard_validation" };

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pApplicationName = appName;
	appInfo.apiVersion = VK_API_VERSION_1_0;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = std::size(extensions);
	instanceInfo.ppEnabledExtensionNames = extensions;
	instanceInfo.enabledLayerCount = std::size(layers);
	instanceInfo.ppEnabledLayerNames = layers;

	auto res = vkCreateInstance(&instanceInfo, nullptr, &instance);
	checkError(res);

	// load extensions
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

	auto res = _vkCreateDebugReportCallbackEXT(instance, &callbackInfo, nullptr, &debugReportCallback);
}

void VulkanInstance::createSurfaceHwnd(HWND hWnd) {

	VkWin32SurfaceCreateInfoKHR surfaceInfo{};

	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.hinstance = GetModuleHandle(nullptr);
	surfaceInfo.hwnd = hWnd;

	auto res = vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
	checkError(res);
}

void VulkanInstance::createPhysicalDevice() {

	auto res = vkEnumeratePhysicalDevices(instance, &adapterCount, nullptr);
	checkError(res);
	adapters = std::make_unique<VkPhysicalDevice[]>(adapterCount);
	res = vkEnumeratePhysicalDevices(instance, &adapterCount, adapters.get());
	checkError(res);
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
	createinstance(appName);
	createDebugReportCallback();
	createSurfaceHwnd(hWnd);
	createPhysicalDevice();
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
	for (int i = 0; i < imageCount; i++)vkFreeCommandBuffers(device, commandPool, imageCount, commandBuffer.get());
	for (int i = 0; i < imageCount; i++)vkDestroyFramebuffer(device, frameBuffer[i], nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	for (int i = 0; i < imageCount; i++)vkDestroyImageView(device, views[i], nullptr);
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroyFence(device, fence, nullptr);
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
}

void Device::create() {
	VkDeviceCreateInfo devInfo{};
	VkDeviceQueueCreateInfo queueInfo{};

	//グラフィックス用のデバイスキューのファミリー番号を取得
	uint32_t propertyCount;
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
	const char* extensions[] = { "VK_KHR_swapchain" };
	static float qPriorities[] = { 0.0f };
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueCount = 1;
	queueInfo.queueFamilyIndex = queueFamilyIndex;
	queueInfo.pQueuePriorities = qPriorities;

	devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	devInfo.queueCreateInfoCount = 1;
	devInfo.pQueueCreateInfos = &queueInfo;
	devInfo.enabledLayerCount = std::size(layers);
	devInfo.ppEnabledLayerNames = layers;
	devInfo.enabledExtensionCount = std::size(extensions);
	devInfo.ppEnabledExtensionNames = extensions;

	//デバイス生成
	auto res = vkCreateDevice(pDev, &devInfo, nullptr, &device);
	checkError(res);
	vkGetDeviceQueue(device, queueFamilyIndex, 0, &devQueue);
	vkGetPhysicalDeviceMemoryProperties(pDev, &memProps);
}

void Device::createCommandPool() {
	VkCommandPoolCreateInfo info{};

	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.queueFamilyIndex = queueFamilyIndex;
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	auto res = vkCreateCommandPool(device, &info, nullptr, &commandPool);
	checkError(res);
}

void Device::createSwapchain() {
	VkSwapchainCreateInfoKHR scinfo{};

	VkBool32 surfaceSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR(pDev, this->queueFamilyIndex, surface, &surfaceSupported);
	VkSurfaceCapabilitiesKHR surfaceCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDev, surface, &surfaceCaps);
	uint32_t surfaceFormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(pDev, surface, &surfaceFormatCount, nullptr);
	auto surfaceFormats = std::make_unique<VkSurfaceFormatKHR[]>(surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(pDev, surface, &surfaceFormatCount, surfaceFormats.get());
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(pDev, surface, &presentModeCount, nullptr);
	auto presentModes = std::make_unique<VkPresentModeKHR[]>(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(pDev, surface, &presentModeCount, presentModes.get());

	for (uint32_t i = 0; i < surfaceFormatCount; i++)
	{
		auto c = surfaceFormats[i];
		OutputDebugString(L"Supported Format Check...");
	}

	scinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
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

	auto res = vkCreateSwapchainKHR(device, &scinfo, nullptr, &swapchain);
	checkError(res);
}

void Device::retrieveImagesFromSwapchain() {
	auto res = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
	checkError(res);
	images = std::make_unique<VkImage[]>(imageCount);
	res = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.get());
	checkError(res);
}

void Device::createImageViews() {

	views = std::make_unique<VkImageView[]>(imageCount);

	for (uint32_t i = 0; i < imageCount; i++)
	{
		VkImageViewCreateInfo vinfo{};
		vinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		vinfo.image = images[i];
		vinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		vinfo.format = VK_FORMAT_B8G8R8A8_UNORM;
		vinfo.components = {
			VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A
		};
		vinfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		auto res = vkCreateImageView(device, &vinfo, nullptr, &views[i]);
		checkError(res);
	}
}

void Device::createCommonRenderPass() {
	VkAttachmentDescription attachmentDesc{};
	VkAttachmentReference attachmentRef{};

	attachmentDesc.format = VK_FORMAT_B8G8R8A8_UNORM;
	attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentRef.attachment = 0;
	attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	VkRenderPassCreateInfo renderPassInfo{};

	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &attachmentRef;
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &attachmentDesc;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	auto res = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
	checkError(res);
}

void Device::createFramebuffers() {

	frameBuffer = std::make_unique<VkFramebuffer[]>(imageCount);

	VkFramebufferCreateInfo fbinfo{};
	VkImageView attachmentViews[1];

	fbinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbinfo.attachmentCount = 1;
	fbinfo.renderPass = renderPass;
	fbinfo.pAttachments = attachmentViews;
	fbinfo.width = width;
	fbinfo.height = height;
	fbinfo.layers = 1;

	for (uint32_t i = 0; i < imageCount; i++)
	{
		attachmentViews[0] = views[i];

		auto res = vkCreateFramebuffer(device, &fbinfo, nullptr, &frameBuffer[i]);
		checkError(res);
	}
}

auto Device::createShaderModule(char* shader) {

	VkShaderModuleCreateInfo shaderInfo{};
	VkShaderModule mod;
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderInfo.codeSize = strlen(shader);
	shaderInfo.pCode = reinterpret_cast<uint32_t*>(shader);

	auto res = vkCreateShaderModule(device, &shaderInfo, nullptr, &mod);
	checkError(res);
	return mod;
}

auto Device::createPipelineLayout() {
	VkPipelineLayoutCreateInfo pLayoutInfo{};
	pLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	VkPipelineLayout pipelineLayout;
	auto res = vkCreatePipelineLayout(device, &pLayoutInfo, nullptr, &pipelineLayout);
	checkError(res);
	return pipelineLayout;
}

auto Device::createPipelineCache() {
	VkPipelineCacheCreateInfo cacheInfo{};
	VkPipelineCache pipelineCache;
	cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	auto res = vkCreatePipelineCache(device, &cacheInfo, nullptr, &pipelineCache);
	checkError(res);
	return pipelineCache;
}

auto Device::createGraphicsPipelineVF(
	const VkShaderModule& vshader, const VkShaderModule& fshader,
	const VkVertexInputBindingDescription& bindDesc, const VkVertexInputAttributeDescription* attrDescs, uint32_t numAttr,
	const VkPipelineLayout& pLayout, const VkRenderPass renderPass, const VkPipelineCache& pCache) {

	VkPipelineShaderStageCreateInfo stageInfo[2]{};
	VkPipelineVertexInputStateCreateInfo vinStateInfo{};
	VkPipelineInputAssemblyStateCreateInfo iaInfo{};
	VkPipelineViewportStateCreateInfo vpInfo{};
	VkPipelineRasterizationStateCreateInfo rasterizerStateInfo{};
	VkPipelineMultisampleStateCreateInfo msInfo{};
	VkPipelineColorBlendAttachmentState blendState{};
	VkPipelineColorBlendStateCreateInfo blendInfo{};
	VkPipelineDynamicStateCreateInfo dynamicInfo{};
	VkGraphicsPipelineCreateInfo gpInfo{};

	static VkViewport vports[] = { { 0.0f, 0.0f, width, height, 0.0f, 1.0f } };
	static VkRect2D scissors[] = { { { 0, 0 }, { width, height } } };
	static VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	stageInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stageInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stageInfo[0].module = vshader;
	stageInfo[1].module = fshader;
	stageInfo[0].pName = "main";
	stageInfo[1].pName = "main";
	vinStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vinStateInfo.vertexBindingDescriptionCount = 1;
	vinStateInfo.pVertexBindingDescriptions = &bindDesc;
	vinStateInfo.vertexAttributeDescriptionCount = numAttr;
	vinStateInfo.pVertexAttributeDescriptions = attrDescs;
	iaInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	iaInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	vpInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vpInfo.viewportCount = 1;
	vpInfo.pViewports = vports;
	vpInfo.scissorCount = 1;
	vpInfo.pScissors = scissors;
	rasterizerStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerStateInfo.depthClampEnable = VK_FALSE;
	rasterizerStateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerStateInfo.cullMode = VK_CULL_MODE_NONE;
	rasterizerStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerStateInfo.depthBiasEnable = VK_FALSE;
	rasterizerStateInfo.lineWidth = 1.0f;
	msInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	msInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	msInfo.sampleShadingEnable = VK_FALSE;
	msInfo.alphaToCoverageEnable = VK_FALSE;
	msInfo.alphaToOneEnable = VK_FALSE;
	blendState.blendEnable = VK_FALSE;
	blendState.colorWriteMask = VK_COLOR_COMPONENT_A_BIT
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT;
	blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendInfo.logicOpEnable = VK_FALSE;
	blendInfo.attachmentCount = 1;
	blendInfo.pAttachments = &blendState;
	dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicInfo.dynamicStateCount = std::size(dynamicStates);
	dynamicInfo.pDynamicStates = dynamicStates;
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
	gpInfo.layout = pLayout;
	gpInfo.renderPass = renderPass;
	gpInfo.subpass = 0;

	VkPipeline pipeline;
	auto res = vkCreateGraphicsPipelines(device, pCache, 1, &gpInfo, nullptr, &pipeline);
	checkError(res);
	return pipeline;
}

void Device::createCommandBuffers() {

	VkCommandBufferAllocateInfo cbAllocInfo{};

	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = commandPool;
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbAllocInfo.commandBufferCount = imageCount;

	commandBuffer = std::make_unique<VkCommandBuffer[]>(imageCount);
	auto res = vkAllocateCommandBuffers(device, &cbAllocInfo, commandBuffer.get());
	checkError(res);
}

void Device::createFence() {
	VkFenceCreateInfo finfo{};
	finfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	auto res = vkCreateFence(device, &finfo, nullptr, &fence);
	checkError(res);
}

void Device::submitCommandAndWait(uint32_t comBufindex) {
	static VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo sinfo{};

	sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sinfo.pWaitDstStageMask = &stageFlags;
	sinfo.commandBufferCount = 1;
	sinfo.pCommandBuffers = &commandBuffer[comBufindex];

	auto res = vkQueueSubmit(devQueue, 1, &sinfo, VK_NULL_HANDLE);
	checkError(res);
	res = vkQueueWaitIdle(devQueue);
	checkError(res);
}

void Device::acquireNextImageAndWait(uint32_t currentFrameIndex) {
	auto res = vkAcquireNextImageKHR(device, swapchain,
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

auto Device::waitForFence() {
	return vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
}

void Device::present(uint32_t currentframeIndex) {
	VkPresentInfoKHR pinfo{};

	pinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pinfo.swapchainCount = 1;
	pinfo.pSwapchains = &swapchain;
	pinfo.pImageIndices = &currentframeIndex;

	auto res = vkQueuePresentKHR(devQueue, &pinfo);
	checkError(res);
}

void Device::resetFence() {
	auto res = vkResetFences(device, 1, &fence);
	checkError(res);
}

void Device::initialImageLayouting(uint32_t comBufindex) {
	auto barriers = std::make_unique<VkImageMemoryBarrier[]>(imageCount);

	for (uint32_t i = 0; i < imageCount; i++)
	{
		VkImageMemoryBarrier barrier{};

		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.image = images[i];
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;
		barriers[i] = barrier;
	}

	vkCmdPipelineBarrier(commandBuffer[comBufindex], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0, 0, nullptr, 0, nullptr, imageCount, barriers.get());
}

void Device::beginCommandWithFramebuffer(uint32_t comBufindex, VkFramebuffer fb) {
	VkCommandBufferInheritanceInfo inhInfo{};
	VkCommandBufferBeginInfo beginInfo{};

	inhInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inhInfo.framebuffer = fb;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pInheritanceInfo = &inhInfo;

	vkBeginCommandBuffer(commandBuffer[comBufindex], &beginInfo);
}

void Device::barrierResource(uint32_t currentframeIndex, uint32_t comBufindex,
	VkPipelineStageFlags srcStageFlags,
	VkPipelineStageFlags dstStageFlags,
	VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
	VkImageLayout srcImageLayout, VkImageLayout dstImageLayout) {

	VkImageMemoryBarrier barrier{};

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = images[currentframeIndex];
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
	static VkClearValue clearValue
	{
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	VkRenderPassBeginInfo rpinfo{};

	rpinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpinfo.framebuffer = frameBuffer[currentframeIndex];
	rpinfo.renderPass = renderPass;
	rpinfo.renderArea.extent.width = width;
	rpinfo.renderArea.extent.height = height;
	rpinfo.clearValueCount = 1;
	rpinfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(commandBuffer[comBufindex], &rpinfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Device::createDevice() {
	create();
	createCommandPool();
	createFence();
	createSwapchain();
	retrieveImagesFromSwapchain();
	createImageViews();
	createCommonRenderPass();
	createFramebuffers();
	createCommandBuffers();
}

//2Dtest
struct VertexData {
	float pos[2];
	float color[4];
};

static VertexData verticesData[] = {
	{ { 0.0f, -0.75f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { -0.5f, 0.75f }, { 1.0f, 0.5f, 0.0f, 1.0f } },
	{ { 0.5f, 0.75f }, { 0.0f, 0.5f, 1.0f, 1.0f } }
};
std::pair<VkBuffer, VkDeviceMemory> vertices;
VkPipeline pipeline;
void Device::d2test() {

	static VkVertexInputBindingDescription bindDesc
	{
		0, sizeof(VertexData), VK_VERTEX_INPUT_RATE_VERTEX
	};
	static VkVertexInputAttributeDescription attrDescs[] =
	{
		{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 2 }
	};
	vertices = createVertexBuffer(verticesData, sizeof(verticesData));

	char* vsShader =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"

		"layout(location = 0) in vec2 pos;\n"
		"layout(location = 1) in vec4 color;\n"

		"layout(location = 0) out vec4 color_out;\n"
		"out gl_PerVertex{ vec4 gl_Position; };\n"

		"void main()\n"
		"{\n"
		"	gl_Position = vec4(pos, 0.0f, 1.0f);\n"
		"	color_out = color;\n"
		"}\n";

	char* fsShader =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout(location = 0) in vec4 color;\n"
		"layout(location = 0) out vec4 color_out;\n"
		"void main() { color_out = color; }\n";

	auto vs = createShaderModule(vsShader);
	auto fs = createShaderModule(fsShader);

	auto pLayout = createPipelineLayout();
	auto pCache = createPipelineCache();
	pipeline = createGraphicsPipelineVF(vs, fs, bindDesc, attrDescs, 2, pLayout, renderPass, pCache);

	beginCommandWithFramebuffer(0, VkFramebuffer());
	initialImageLayouting(0);
	auto res = vkEndCommandBuffer(commandBuffer[0]);
	checkError(res);
	submitCommandAndWait(0);

	// Acquire First
	acquireNextImageAndWait(currentFrameIndex);
}

void Device::d2testDraw() {
	static VkViewport vp = { 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f };
	static VkRect2D sc = { { 0, 0 }, { 640, 480 } };
	static VkDeviceSize offsets[] = { 0 };

	beginCommandWithFramebuffer(0, frameBuffer[currentFrameIndex]);
	barrierResource(0, currentFrameIndex,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	beginRenderPass(0, currentFrameIndex);
	vkCmdBindPipeline(commandBuffer[0], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdSetViewport(commandBuffer[0], 0, 1, &vp);
	vkCmdSetScissor(commandBuffer[0], 0, 1, &sc);
	vkCmdBindVertexBuffers(commandBuffer[0], 0, 1, &vertices.first, offsets);
	vkCmdDraw(commandBuffer[0], 3, 1, 0, 0);
	vkCmdEndRenderPass(commandBuffer[0]);
	/*Vulkan::barrierResource(commandBuffer[0], images.first[currentFrameIndex],
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);*/
	vkEndCommandBuffer(commandBuffer[0]);

	// Submit and Wait with Fences
	submitCommands(0);
	switch (waitForFence())
	{
	case VK_SUCCESS: present(currentFrameIndex); break;
	case VK_TIMEOUT: throw std::runtime_error("Command execution timed out."); break;
	default: OutputDebugString(L"waitForFence returns unknown value.\n");
	}
	resetFence();

	// Acquire next
	acquireNextImageAndWait(currentFrameIndex);
}