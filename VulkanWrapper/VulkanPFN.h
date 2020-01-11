//*****************************************************************************************//
//**                                                                                     **//
//**                                  VulkanPFN.h                                        **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef VulkanPFN_Header
#define VulkanPFN_Header
#ifdef __ANDROID__
#define VK_NO_PROTOTYPES 1
#endif
#include <vulkan/vulkan.h>
#ifdef __ANDROID__
#include <vulkan/vulkan_android.h>

int VulkanPFN();
extern PFN_vkCreateInstance vkCreateInstance;
extern PFN_vkDestroyPipeline vkDestroyPipeline;
extern PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
extern PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
extern PFN_vkMapMemory vkMapMemory;
extern PFN_vkUnmapMemory vkUnmapMemory;
extern PFN_vkDestroyBuffer vkDestroyBuffer;
extern PFN_vkFreeMemory vkFreeMemory;
extern PFN_vkDestroyShaderModule vkDestroyShaderModule;
extern PFN_vkCmdBindPipeline vkCmdBindPipeline;
extern PFN_vkCmdSetViewport vkCmdSetViewport;
extern PFN_vkCmdSetScissor vkCmdSetScissor;
extern PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
extern PFN_vkCmdDraw vkCmdDraw;
extern PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
extern PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
extern PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
extern PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
extern PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
extern PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
extern PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
extern PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
extern PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
extern PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
extern PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
extern PFN_vkDestroyInstance vkDestroyInstance;
extern PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
extern PFN_vkDestroyImageView vkDestroyImageView;
extern PFN_vkDestroyImage vkDestroyImage;
extern PFN_vkDestroyRenderPass vkDestroyRenderPass;
extern PFN_vkDestroyFence vkDestroyFence;
extern PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
extern PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
extern PFN_vkDestroySemaphore vkDestroySemaphore;
extern PFN_vkDestroyCommandPool vkDestroyCommandPool;
extern PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
extern PFN_vkDestroyDevice vkDestroyDevice;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
extern PFN_vkCreateDevice vkCreateDevice;
extern PFN_vkGetDeviceQueue vkGetDeviceQueue;
extern PFN_vkCreateCommandPool vkCreateCommandPool;
extern PFN_vkCreateFence vkCreateFence;
extern PFN_vkCreateSemaphore vkCreateSemaphore;
extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
extern PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
extern PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
extern PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
extern PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
extern PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
extern PFN_vkCreateRenderPass vkCreateRenderPass;
extern PFN_vkCreateFramebuffer vkCreateFramebuffer;
extern PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
extern PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
extern PFN_vkQueueSubmit vkQueueSubmit;
extern PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
extern PFN_vkWaitForFences vkWaitForFences;
extern PFN_vkQueuePresentKHR vkQueuePresentKHR;
extern PFN_vkResetFences vkResetFences;
extern PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
extern PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
extern PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
extern PFN_vkCreateImage vkCreateImage;
extern PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
extern PFN_vkAllocateMemory vkAllocateMemory;
extern PFN_vkBindImageMemory vkBindImageMemory;
extern PFN_vkEndCommandBuffer vkEndCommandBuffer;
extern PFN_vkCreateImageView vkCreateImageView;
extern PFN_vkCreateSampler vkCreateSampler;
extern PFN_vkDestroySampler vkDestroySampler;
extern PFN_vkCreateBuffer vkCreateBuffer;
extern PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
extern PFN_vkBindBufferMemory vkBindBufferMemory;
extern PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
extern PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
extern PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
extern PFN_vkCreateShaderModule vkCreateShaderModule;
extern PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
extern PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
extern PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
extern PFN_vkCreatePipelineCache vkCreatePipelineCache;
extern PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
extern PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
extern PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;
#endif
#endif