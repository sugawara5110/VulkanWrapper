//*****************************************************************************************//
//**                                                                                     **//
//**                                  VulkanPFN.cpp                                      **//
//**                                                                                     **//
//*****************************************************************************************//

#include "VulkanPFN.h"

#ifdef __ANDROID__
#include <dlfcn.h>

int VulkanPFN() {
    void* libvulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!libvulkan)
        return 0;

    vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(dlsym(libvulkan, "vkCreateInstance"));
    vkDestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(dlsym(libvulkan,
        "vkDestroyPipeline"));
    vkDestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(dlsym(libvulkan,
        "vkDestroyPipelineCache"));
    vkDestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(dlsym(libvulkan,
        "vkDestroyPipelineLayout"));
    vkMapMemory = reinterpret_cast<PFN_vkMapMemory>(dlsym(libvulkan, "vkMapMemory"));
    vkUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(dlsym(libvulkan, "vkUnmapMemory"));
    vkDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(dlsym(libvulkan, "vkDestroyBuffer"));
    vkFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(dlsym(libvulkan, "vkFreeMemory"));
    vkDestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(dlsym(libvulkan,
        "vkDestroyShaderModule"));
    vkCmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(dlsym(libvulkan,
        "vkCmdBindPipeline"));
    vkCmdSetViewport = reinterpret_cast<PFN_vkCmdSetViewport>(dlsym(libvulkan, "vkCmdSetViewport"));
    vkCmdSetScissor = reinterpret_cast<PFN_vkCmdSetScissor>(dlsym(libvulkan, "vkCmdSetScissor"));
    vkCmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(dlsym(libvulkan,
        "vkCmdBindVertexBuffers"));
    vkCmdDraw = reinterpret_cast<PFN_vkCmdDraw>(dlsym(libvulkan, "vkCmdDraw"));
    vkDestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(dlsym(
        libvulkan, "vkDestroyDescriptorSetLayout"));
    vkFreeDescriptorSets = reinterpret_cast<PFN_vkFreeDescriptorSets>(dlsym(libvulkan,
        "vkFreeDescriptorSets"));
    vkDestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(dlsym(libvulkan,
        "vkDestroyDescriptorPool"));
    vkCmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(dlsym(libvulkan,
        "vkCmdBindDescriptorSets"));
    vkCmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(dlsym(libvulkan,
        "vkCmdBindIndexBuffer"));
    vkCmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(dlsym(libvulkan, "vkCmdDrawIndexed"));
    vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(libvulkan,
        "vkGetInstanceProcAddr"));
    vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(dlsym(libvulkan,
        "vkEnumeratePhysicalDevices"));
    vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(dlsym(
        libvulkan, "vkGetPhysicalDeviceProperties"));
    vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(dlsym(
        libvulkan, "vkGetPhysicalDeviceMemoryProperties"));
    vkDestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(dlsym(libvulkan,
        "vkDestroySurfaceKHR"));
    vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(dlsym(libvulkan,
        "vkDestroyInstance"));
    vkFreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(dlsym(libvulkan,
        "vkFreeCommandBuffers"));
    vkDestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(dlsym(libvulkan,
        "vkDestroyImageView"));
    vkDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(dlsym(libvulkan, "vkDestroyImage"));
    vkDestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(dlsym(libvulkan,
        "vkDestroyRenderPass"));
    vkDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(dlsym(libvulkan, "vkDestroyFence"));
    vkDestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(dlsym(libvulkan,
        "vkDestroyFramebuffer"));
    vkDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(dlsym(libvulkan,
        "vkDestroySwapchainKHR"));
    vkDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(dlsym(libvulkan,
        "vkDestroySemaphore"));
    vkDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(dlsym(libvulkan,
        "vkDestroyCommandPool"));
    vkDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(dlsym(libvulkan, "vkDeviceWaitIdle"));
    vkDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(dlsym(libvulkan, "vkDestroyDevice"));
    vkGetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(dlsym(
        libvulkan, "vkGetPhysicalDeviceQueueFamilyProperties"));
    vkCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(dlsym(libvulkan, "vkCreateDevice"));
    vkGetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(dlsym(libvulkan, "vkGetDeviceQueue"));
    vkCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(dlsym(libvulkan,
        "vkCreateCommandPool"));
    vkCreateFence = reinterpret_cast<PFN_vkCreateFence>(dlsym(libvulkan, "vkCreateFence"));
    vkCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(dlsym(libvulkan,
        "vkCreateSemaphore"));
    vkGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(dlsym(
        libvulkan, "vkGetPhysicalDeviceSurfaceSupportKHR"));
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(dlsym(
        libvulkan, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
    vkGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(dlsym(
        libvulkan, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
    vkGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(dlsym(
        libvulkan, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
    vkCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(dlsym(libvulkan,
        "vkCreateSwapchainKHR"));
    vkGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(dlsym(libvulkan,
        "vkGetSwapchainImagesKHR"));
    vkGetPhysicalDeviceFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(dlsym(
        libvulkan, "vkGetPhysicalDeviceFormatProperties"));
    vkCreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(dlsym(libvulkan,
        "vkCreateRenderPass"));
    vkCreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(dlsym(libvulkan,
        "vkCreateFramebuffer"));
    vkAllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(dlsym(libvulkan,
        "vkAllocateCommandBuffers"));
    vkBeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(dlsym(libvulkan,
        "vkBeginCommandBuffer"));
    vkQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(dlsym(libvulkan, "vkQueueSubmit"));
    vkAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(dlsym(libvulkan,
        "vkAcquireNextImageKHR"));
    vkWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(dlsym(libvulkan, "vkWaitForFences"));
    vkQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(dlsym(libvulkan,
        "vkQueuePresentKHR"));
    vkResetFences = reinterpret_cast<PFN_vkResetFences>(dlsym(libvulkan, "vkResetFences"));
    vkCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(dlsym(libvulkan,
        "vkCmdPipelineBarrier"));
    vkCmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(dlsym(libvulkan,
        "vkCmdBeginRenderPass"));
    vkCmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(dlsym(libvulkan,
        "vkCmdCopyBufferToImage"));
    vkCreateImage = reinterpret_cast<PFN_vkCreateImage>(dlsym(libvulkan, "vkCreateImage"));
    vkGetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(dlsym(
        libvulkan, "vkGetImageMemoryRequirements"));
    vkAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(dlsym(libvulkan, "vkAllocateMemory"));
    vkBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(dlsym(libvulkan,
        "vkBindImageMemory"));
    vkEndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(dlsym(libvulkan,
        "vkEndCommandBuffer"));
    vkCreateImageView = reinterpret_cast<PFN_vkCreateImageView>(dlsym(libvulkan,
        "vkCreateImageView"));
    vkCreateSampler = reinterpret_cast<PFN_vkCreateSampler>(dlsym(libvulkan, "vkCreateSampler"));
    vkDestroySampler = reinterpret_cast<PFN_vkDestroySampler>(dlsym(libvulkan, "vkDestroySampler"));
    vkCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(dlsym(libvulkan, "vkCreateBuffer"));
    vkGetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(dlsym(
        libvulkan, "vkGetBufferMemoryRequirements"));
    vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(dlsym(libvulkan,
        "vkBindBufferMemory"));
    vkCmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(dlsym(libvulkan, "vkCmdCopyBuffer"));
    vkCreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(dlsym(libvulkan,
        "vkCreateDescriptorSetLayout"));
    vkCreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(dlsym(libvulkan,
        "vkCreatePipelineLayout"));
    vkCreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(dlsym(libvulkan,
        "vkCreateShaderModule"));
    vkCreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(dlsym(libvulkan,
        "vkCreateDescriptorPool"));
    vkAllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(dlsym(libvulkan,
        "vkAllocateDescriptorSets"));
    vkUpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(dlsym(libvulkan,
        "vkUpdateDescriptorSets"));
    vkCreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(dlsym(libvulkan,
        "vkCreatePipelineCache"));
    vkCreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(dlsym(libvulkan,
        "vkCreateGraphicsPipelines"));
    vkCmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(dlsym(libvulkan,
        "vkCmdEndRenderPass"));
    vkCreateAndroidSurfaceKHR = reinterpret_cast<PFN_vkCreateAndroidSurfaceKHR>(dlsym(libvulkan,
        "vkCreateAndroidSurfaceKHR"));
    return 1;
}

PFN_vkCreateInstance vkCreateInstance;
PFN_vkDestroyPipeline vkDestroyPipeline;
PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
PFN_vkMapMemory vkMapMemory;
PFN_vkUnmapMemory vkUnmapMemory;
PFN_vkDestroyBuffer vkDestroyBuffer;
PFN_vkFreeMemory vkFreeMemory;
PFN_vkDestroyShaderModule vkDestroyShaderModule;
PFN_vkCmdBindPipeline vkCmdBindPipeline;
PFN_vkCmdSetViewport vkCmdSetViewport;
PFN_vkCmdSetScissor vkCmdSetScissor;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
PFN_vkCmdDraw vkCmdDraw;
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
PFN_vkDestroyInstance vkDestroyInstance;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
PFN_vkDestroyImageView vkDestroyImageView;
PFN_vkDestroyImage vkDestroyImage;
PFN_vkDestroyRenderPass vkDestroyRenderPass;
PFN_vkDestroyFence vkDestroyFence;
PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
PFN_vkDestroySemaphore vkDestroySemaphore;
PFN_vkDestroyCommandPool vkDestroyCommandPool;
PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
PFN_vkDestroyDevice vkDestroyDevice;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkCreateDevice vkCreateDevice;
PFN_vkGetDeviceQueue vkGetDeviceQueue;
PFN_vkCreateCommandPool vkCreateCommandPool;
PFN_vkCreateFence vkCreateFence;
PFN_vkCreateSemaphore vkCreateSemaphore;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
PFN_vkCreateRenderPass vkCreateRenderPass;
PFN_vkCreateFramebuffer vkCreateFramebuffer;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
PFN_vkQueueSubmit vkQueueSubmit;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
PFN_vkWaitForFences vkWaitForFences;
PFN_vkQueuePresentKHR vkQueuePresentKHR;
PFN_vkResetFences vkResetFences;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
PFN_vkCreateImage vkCreateImage;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
PFN_vkAllocateMemory vkAllocateMemory;
PFN_vkBindImageMemory vkBindImageMemory;
PFN_vkEndCommandBuffer vkEndCommandBuffer;
PFN_vkCreateImageView vkCreateImageView;
PFN_vkCreateSampler vkCreateSampler;
PFN_vkDestroySampler vkDestroySampler;
PFN_vkCreateBuffer vkCreateBuffer;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
PFN_vkBindBufferMemory vkBindBufferMemory;
PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
PFN_vkCreateShaderModule vkCreateShaderModule;
PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
PFN_vkCreatePipelineCache vkCreatePipelineCache;
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;
#endif