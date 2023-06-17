//*****************************************************************************************//
//**                                                                                     **//
//**                                  VulkanPFN.cpp                                      **//
//**                                                                                     **//
//*****************************************************************************************//

#include "VulkanPFN.h"

#ifdef __ANDROID__
#include <dlfcn.h>
#endif

int VulkanPFN() {

#ifdef __ANDROID__
    void* libvulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!libvulkan)
        return 0;

    _vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(dlsym(libvulkan, "vkCreateInstance"));
    _vkDestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(dlsym(libvulkan,
        "vkDestroyPipeline"));
    _vkDestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(dlsym(libvulkan,
        "vkDestroyPipelineCache"));
    _vkDestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(dlsym(libvulkan,
        "vkDestroyPipelineLayout"));
    _vkMapMemory = reinterpret_cast<PFN_vkMapMemory>(dlsym(libvulkan, "vkMapMemory"));
    _vkUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(dlsym(libvulkan, "vkUnmapMemory"));
    _vkDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(dlsym(libvulkan, "vkDestroyBuffer"));
    _vkFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(dlsym(libvulkan, "vkFreeMemory"));
    _vkDestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(dlsym(libvulkan,
        "vkDestroyShaderModule"));
    _vkCmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(dlsym(libvulkan,
        "vkCmdBindPipeline"));
    _vkCmdSetViewport = reinterpret_cast<PFN_vkCmdSetViewport>(dlsym(libvulkan, "vkCmdSetViewport"));
    _vkCmdSetScissor = reinterpret_cast<PFN_vkCmdSetScissor>(dlsym(libvulkan, "vkCmdSetScissor"));
    _vkCmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(dlsym(libvulkan,
        "vkCmdBindVertexBuffers"));
    _vkCmdDraw = reinterpret_cast<PFN_vkCmdDraw>(dlsym(libvulkan, "vkCmdDraw"));
    _vkDestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(dlsym(
        libvulkan, "vkDestroyDescriptorSetLayout"));
    _vkFreeDescriptorSets = reinterpret_cast<PFN_vkFreeDescriptorSets>(dlsym(libvulkan,
        "vkFreeDescriptorSets"));
    _vkDestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(dlsym(libvulkan,
        "vkDestroyDescriptorPool"));
    _vkCmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(dlsym(libvulkan,
        "vkCmdBindDescriptorSets"));
    _vkCmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(dlsym(libvulkan,
        "vkCmdBindIndexBuffer"));
    _vkCmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(dlsym(libvulkan, "vkCmdDrawIndexed"));
    _vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(libvulkan,
        "vkGetInstanceProcAddr"));
    _vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(dlsym(libvulkan,
        "vkEnumeratePhysicalDevices"));
    _vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(dlsym(
        libvulkan, "vkGetPhysicalDeviceProperties"));
    _vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(dlsym(
        libvulkan, "vkGetPhysicalDeviceMemoryProperties"));
    _vkDestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(dlsym(libvulkan,
        "vkDestroySurfaceKHR"));
    _vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(dlsym(libvulkan,
        "vkDestroyInstance"));
    _vkFreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(dlsym(libvulkan,
        "vkFreeCommandBuffers"));
    _vkDestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(dlsym(libvulkan,
        "vkDestroyImageView"));
    _vkDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(dlsym(libvulkan, "vkDestroyImage"));
    _vkDestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(dlsym(libvulkan,
        "vkDestroyRenderPass"));
    _vkDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(dlsym(libvulkan, "vkDestroyFence"));
    _vkDestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(dlsym(libvulkan,
        "vkDestroyFramebuffer"));
    _vkDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(dlsym(libvulkan,
        "vkDestroySwapchainKHR"));
    _vkDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(dlsym(libvulkan,
        "vkDestroySemaphore"));
    _vkDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(dlsym(libvulkan,
        "vkDestroyCommandPool"));
    _vkDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(dlsym(libvulkan, "vkDeviceWaitIdle"));
    _vkDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(dlsym(libvulkan, "vkDestroyDevice"));
    _vkGetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(dlsym(
        libvulkan, "vkGetPhysicalDeviceQueueFamilyProperties"));
    _vkCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(dlsym(libvulkan, "vkCreateDevice"));
    _vkGetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(dlsym(libvulkan, "vkGetDeviceQueue"));
    _vkCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(dlsym(libvulkan,
        "vkCreateCommandPool"));
    _vkCreateFence = reinterpret_cast<PFN_vkCreateFence>(dlsym(libvulkan, "vkCreateFence"));
    _vkCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(dlsym(libvulkan,
        "vkCreateSemaphore"));
    _vkGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(dlsym(
        libvulkan, "vkGetPhysicalDeviceSurfaceSupportKHR"));
    _vkGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(dlsym(
        libvulkan, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
    _vkGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(dlsym(
        libvulkan, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
    _vkGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(dlsym(
        libvulkan, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
    _vkCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(dlsym(libvulkan,
        "vkCreateSwapchainKHR"));
    _vkGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(dlsym(libvulkan,
        "vkGetSwapchainImagesKHR"));
    _vkGetPhysicalDeviceFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(dlsym(
        libvulkan, "vkGetPhysicalDeviceFormatProperties"));
    _vkCreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(dlsym(libvulkan,
        "vkCreateRenderPass"));
    _vkCreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(dlsym(libvulkan,
        "vkCreateFramebuffer"));
    _vkAllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(dlsym(libvulkan,
        "vkAllocateCommandBuffers"));
    _vkBeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(dlsym(libvulkan,
        "vkBeginCommandBuffer"));
    _vkQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(dlsym(libvulkan, "vkQueueSubmit"));
    _vkAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(dlsym(libvulkan,
        "vkAcquireNextImageKHR"));
    _vkWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(dlsym(libvulkan, "vkWaitForFences"));
    _vkQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(dlsym(libvulkan,
        "vkQueuePresentKHR"));
    _vkResetFences = reinterpret_cast<PFN_vkResetFences>(dlsym(libvulkan, "vkResetFences"));
    _vkCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(dlsym(libvulkan,
        "vkCmdPipelineBarrier"));
    _vkCmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(dlsym(libvulkan,
        "vkCmdBeginRenderPass"));
    _vkCmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(dlsym(libvulkan,
        "vkCmdCopyBufferToImage"));
    _vkCreateImage = reinterpret_cast<PFN_vkCreateImage>(dlsym(libvulkan, "vkCreateImage"));
    _vkGetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(dlsym(
        libvulkan, "vkGetImageMemoryRequirements"));
    _vkAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(dlsym(libvulkan, "vkAllocateMemory"));
    _vkBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(dlsym(libvulkan,
        "vkBindImageMemory"));
    _vkEndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(dlsym(libvulkan,
        "vkEndCommandBuffer"));
    _vkCreateImageView = reinterpret_cast<PFN_vkCreateImageView>(dlsym(libvulkan,
        "vkCreateImageView"));
    _vkCreateSampler = reinterpret_cast<PFN_vkCreateSampler>(dlsym(libvulkan, "vkCreateSampler"));
    _vkDestroySampler = reinterpret_cast<PFN_vkDestroySampler>(dlsym(libvulkan, "vkDestroySampler"));
    _vkCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(dlsym(libvulkan, "vkCreateBuffer"));
    _vkGetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(dlsym(
        libvulkan, "vkGetBufferMemoryRequirements"));
    _vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(dlsym(libvulkan,
        "vkBindBufferMemory"));
    _vkCmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(dlsym(libvulkan, "vkCmdCopyBuffer"));
    _vkCreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(dlsym(libvulkan,
        "vkCreateDescriptorSetLayout"));
    _vkCreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(dlsym(libvulkan,
        "vkCreatePipelineLayout"));
    _vkCreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(dlsym(libvulkan,
        "vkCreateShaderModule"));
    _vkCreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(dlsym(libvulkan,
        "vkCreateDescriptorPool"));
    _vkAllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(dlsym(libvulkan,
        "vkAllocateDescriptorSets"));
    _vkUpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(dlsym(libvulkan,
        "vkUpdateDescriptorSets"));
    _vkCreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(dlsym(libvulkan,
        "vkCreatePipelineCache"));
    _vkCreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(dlsym(libvulkan,
        "vkCreateGraphicsPipelines"));
    _vkCmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(dlsym(libvulkan,
        "vkCmdEndRenderPass"));
    _vkCmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(dlsym(libvulkan,
        "vkCmdCopyImageToBuffer"));
    _vkCreateComputePipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(dlsym(libvulkan,
        "vkCreateComputePipelines"));
    _vkCmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(dlsym(libvulkan,
        "vkCmdDispatch"));
    _vkCmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(dlsym(libvulkan,
        "vkCmdCopyImage"));

    _vkCreateAndroidSurfaceKHR = reinterpret_cast<PFN_vkCreateAndroidSurfaceKHR>(dlsym(libvulkan,
        "vkCreateAndroidSurfaceKHR"));
#else
    _vkCreateInstance = vkCreateInstance;
    _vkDestroyPipeline = vkDestroyPipeline;
    _vkDestroyPipelineCache = vkDestroyPipelineCache;
    _vkDestroyPipelineLayout = vkDestroyPipelineLayout;
    _vkMapMemory = vkMapMemory;
    _vkUnmapMemory = vkUnmapMemory;
    _vkDestroyBuffer = vkDestroyBuffer;
    _vkFreeMemory = vkFreeMemory;
    _vkDestroyShaderModule = vkDestroyShaderModule;
    _vkCmdBindPipeline = vkCmdBindPipeline;
    _vkCmdSetViewport = vkCmdSetViewport;
    _vkCmdSetScissor = vkCmdSetScissor;
    _vkCmdBindVertexBuffers = vkCmdBindVertexBuffers;
    _vkCmdDraw = vkCmdDraw;
    _vkDestroyDescriptorSetLayout = vkDestroyDescriptorSetLayout;
    _vkFreeDescriptorSets = vkFreeDescriptorSets;
    _vkDestroyDescriptorPool = vkDestroyDescriptorPool;
    _vkCmdBindDescriptorSets = vkCmdBindDescriptorSets;
    _vkCmdBindIndexBuffer = vkCmdBindIndexBuffer;
    _vkCmdDrawIndexed = vkCmdDrawIndexed;
    _vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    _vkEnumeratePhysicalDevices = vkEnumeratePhysicalDevices;
    _vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    _vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    _vkDestroySurfaceKHR = vkDestroySurfaceKHR;
    _vkDestroyInstance = vkDestroyInstance;
    _vkFreeCommandBuffers = vkFreeCommandBuffers;
    _vkDestroyImageView = vkDestroyImageView;
    _vkDestroyImage = vkDestroyImage;
    _vkDestroyRenderPass = vkDestroyRenderPass;
    _vkDestroyFence = vkDestroyFence;
    _vkDestroyFramebuffer = vkDestroyFramebuffer;
    _vkDestroySwapchainKHR = vkDestroySwapchainKHR;
    _vkDestroySemaphore = vkDestroySemaphore;
    _vkDestroyCommandPool = vkDestroyCommandPool;
    _vkDeviceWaitIdle = vkDeviceWaitIdle;
    _vkDestroyDevice = vkDestroyDevice;
    _vkGetPhysicalDeviceQueueFamilyProperties = vkGetPhysicalDeviceQueueFamilyProperties;
    _vkCreateDevice = vkCreateDevice;
    _vkGetDeviceQueue = vkGetDeviceQueue;
    _vkCreateCommandPool = vkCreateCommandPool;
    _vkCreateFence = vkCreateFence;
    _vkCreateSemaphore = vkCreateSemaphore;
    _vkGetPhysicalDeviceSurfaceSupportKHR = vkGetPhysicalDeviceSurfaceSupportKHR;
    _vkGetPhysicalDeviceSurfaceCapabilitiesKHR = vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    _vkGetPhysicalDeviceSurfaceFormatsKHR = vkGetPhysicalDeviceSurfaceFormatsKHR;
    _vkGetPhysicalDeviceSurfacePresentModesKHR = vkGetPhysicalDeviceSurfacePresentModesKHR;
    _vkCreateSwapchainKHR = vkCreateSwapchainKHR;
    _vkGetSwapchainImagesKHR = vkGetSwapchainImagesKHR;
    _vkGetPhysicalDeviceFormatProperties = vkGetPhysicalDeviceFormatProperties;
    _vkCreateRenderPass = vkCreateRenderPass;
    _vkCreateFramebuffer = vkCreateFramebuffer;
    _vkAllocateCommandBuffers = vkAllocateCommandBuffers;
    _vkBeginCommandBuffer = vkBeginCommandBuffer;
    _vkQueueSubmit = vkQueueSubmit;
    _vkAcquireNextImageKHR = vkAcquireNextImageKHR;
    _vkWaitForFences = vkWaitForFences;
    _vkQueuePresentKHR = vkQueuePresentKHR;
    _vkResetFences = vkResetFences;
    _vkCmdPipelineBarrier = vkCmdPipelineBarrier;
    _vkCmdBeginRenderPass = vkCmdBeginRenderPass;
    _vkCmdCopyBufferToImage = vkCmdCopyBufferToImage;
    _vkCreateImage = vkCreateImage;
    _vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    _vkAllocateMemory = vkAllocateMemory;
    _vkBindImageMemory = vkBindImageMemory;
    _vkEndCommandBuffer = vkEndCommandBuffer;
    _vkCreateImageView = vkCreateImageView;
    _vkCreateSampler = vkCreateSampler;
    _vkDestroySampler = vkDestroySampler;
    _vkCreateBuffer = vkCreateBuffer;
    _vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    _vkBindBufferMemory = vkBindBufferMemory;
    _vkCmdCopyBuffer = vkCmdCopyBuffer;
    _vkCreateDescriptorSetLayout = vkCreateDescriptorSetLayout;
    _vkCreatePipelineLayout = vkCreatePipelineLayout;
    _vkCreateShaderModule = vkCreateShaderModule;
    _vkCreateDescriptorPool = vkCreateDescriptorPool;
    _vkAllocateDescriptorSets = vkAllocateDescriptorSets;
    _vkUpdateDescriptorSets = vkUpdateDescriptorSets;
    _vkCreatePipelineCache = vkCreatePipelineCache;
    _vkCreateGraphicsPipelines = vkCreateGraphicsPipelines;
    _vkCmdEndRenderPass = vkCmdEndRenderPass;
    _vkCmdCopyImageToBuffer = vkCmdCopyImageToBuffer;
    _vkCreateComputePipelines = vkCreateComputePipelines;
    _vkCmdDispatch = vkCmdDispatch;
    _vkCmdCopyImage = vkCmdCopyImage;
#endif
    return 1;
}

PFN_vkCreateInstance _vkCreateInstance;
PFN_vkDestroyPipeline _vkDestroyPipeline;
PFN_vkDestroyPipelineCache _vkDestroyPipelineCache;
PFN_vkDestroyPipelineLayout _vkDestroyPipelineLayout;
PFN_vkMapMemory _vkMapMemory;
PFN_vkUnmapMemory _vkUnmapMemory;
PFN_vkDestroyBuffer _vkDestroyBuffer;
PFN_vkFreeMemory _vkFreeMemory;
PFN_vkDestroyShaderModule _vkDestroyShaderModule;
PFN_vkCmdBindPipeline _vkCmdBindPipeline;
PFN_vkCmdSetViewport _vkCmdSetViewport;
PFN_vkCmdSetScissor _vkCmdSetScissor;
PFN_vkCmdBindVertexBuffers _vkCmdBindVertexBuffers;
PFN_vkCmdDraw _vkCmdDraw;
PFN_vkDestroyDescriptorSetLayout _vkDestroyDescriptorSetLayout;
PFN_vkFreeDescriptorSets _vkFreeDescriptorSets;
PFN_vkDestroyDescriptorPool _vkDestroyDescriptorPool;
PFN_vkCmdBindDescriptorSets _vkCmdBindDescriptorSets;
PFN_vkCmdBindIndexBuffer _vkCmdBindIndexBuffer;
PFN_vkCmdDrawIndexed _vkCmdDrawIndexed;
PFN_vkGetInstanceProcAddr _vkGetInstanceProcAddr;
PFN_vkEnumeratePhysicalDevices _vkEnumeratePhysicalDevices;
PFN_vkGetPhysicalDeviceProperties _vkGetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceMemoryProperties _vkGetPhysicalDeviceMemoryProperties;
PFN_vkDestroySurfaceKHR _vkDestroySurfaceKHR;
PFN_vkDestroyInstance _vkDestroyInstance;
PFN_vkFreeCommandBuffers _vkFreeCommandBuffers;
PFN_vkDestroyImageView _vkDestroyImageView;
PFN_vkDestroyImage _vkDestroyImage;
PFN_vkDestroyRenderPass _vkDestroyRenderPass;
PFN_vkDestroyFence _vkDestroyFence;
PFN_vkDestroyFramebuffer _vkDestroyFramebuffer;
PFN_vkDestroySwapchainKHR _vkDestroySwapchainKHR;
PFN_vkDestroySemaphore _vkDestroySemaphore;
PFN_vkDestroyCommandPool _vkDestroyCommandPool;
PFN_vkDeviceWaitIdle _vkDeviceWaitIdle;
PFN_vkDestroyDevice _vkDestroyDevice;
PFN_vkGetPhysicalDeviceQueueFamilyProperties _vkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkCreateDevice _vkCreateDevice;
PFN_vkGetDeviceQueue _vkGetDeviceQueue;
PFN_vkCreateCommandPool _vkCreateCommandPool;
PFN_vkCreateFence _vkCreateFence;
PFN_vkCreateSemaphore _vkCreateSemaphore;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR _vkGetPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR _vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR _vkGetPhysicalDeviceSurfaceFormatsKHR;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR _vkGetPhysicalDeviceSurfacePresentModesKHR;
PFN_vkCreateSwapchainKHR _vkCreateSwapchainKHR;
PFN_vkGetSwapchainImagesKHR _vkGetSwapchainImagesKHR;
PFN_vkGetPhysicalDeviceFormatProperties _vkGetPhysicalDeviceFormatProperties;
PFN_vkCreateRenderPass _vkCreateRenderPass;
PFN_vkCreateFramebuffer _vkCreateFramebuffer;
PFN_vkAllocateCommandBuffers _vkAllocateCommandBuffers;
PFN_vkBeginCommandBuffer _vkBeginCommandBuffer;
PFN_vkQueueSubmit _vkQueueSubmit;
PFN_vkAcquireNextImageKHR _vkAcquireNextImageKHR;
PFN_vkWaitForFences _vkWaitForFences;
PFN_vkQueuePresentKHR _vkQueuePresentKHR;
PFN_vkResetFences _vkResetFences;
PFN_vkCmdPipelineBarrier _vkCmdPipelineBarrier;
PFN_vkCmdBeginRenderPass _vkCmdBeginRenderPass;
PFN_vkCmdCopyBufferToImage _vkCmdCopyBufferToImage;
PFN_vkCreateImage _vkCreateImage;
PFN_vkGetImageMemoryRequirements _vkGetImageMemoryRequirements;
PFN_vkAllocateMemory _vkAllocateMemory;
PFN_vkBindImageMemory _vkBindImageMemory;
PFN_vkEndCommandBuffer _vkEndCommandBuffer;
PFN_vkCreateImageView _vkCreateImageView;
PFN_vkCreateSampler _vkCreateSampler;
PFN_vkDestroySampler _vkDestroySampler;
PFN_vkCreateBuffer _vkCreateBuffer;
PFN_vkGetBufferMemoryRequirements _vkGetBufferMemoryRequirements;
PFN_vkBindBufferMemory _vkBindBufferMemory;
PFN_vkCmdCopyBuffer _vkCmdCopyBuffer;
PFN_vkCreateDescriptorSetLayout _vkCreateDescriptorSetLayout;
PFN_vkCreatePipelineLayout _vkCreatePipelineLayout;
PFN_vkCreateShaderModule _vkCreateShaderModule;
PFN_vkCreateDescriptorPool _vkCreateDescriptorPool;
PFN_vkAllocateDescriptorSets _vkAllocateDescriptorSets;
PFN_vkUpdateDescriptorSets _vkUpdateDescriptorSets;
PFN_vkCreatePipelineCache _vkCreatePipelineCache;
PFN_vkCreateGraphicsPipelines _vkCreateGraphicsPipelines;
PFN_vkCmdEndRenderPass _vkCmdEndRenderPass;
PFN_vkCmdCopyImageToBuffer _vkCmdCopyImageToBuffer;
PFN_vkCreateComputePipelines _vkCreateComputePipelines;
PFN_vkCmdDispatch _vkCmdDispatch;
PFN_vkCmdCopyImage _vkCmdCopyImage;

#ifdef __ANDROID__
PFN_vkCreateAndroidSurfaceKHR _vkCreateAndroidSurfaceKHR;
#endif
