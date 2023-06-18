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
#include <vulkan/vulkan.h>//1.3.224.1

int VulkanPFN();
extern PFN_vkCreateInstance _vkCreateInstance;
extern PFN_vkDestroyPipeline _vkDestroyPipeline;
extern PFN_vkDestroyPipelineCache _vkDestroyPipelineCache;
extern PFN_vkDestroyPipelineLayout _vkDestroyPipelineLayout;
extern PFN_vkMapMemory _vkMapMemory;
extern PFN_vkUnmapMemory _vkUnmapMemory;
extern PFN_vkDestroyBuffer _vkDestroyBuffer;
extern PFN_vkFreeMemory _vkFreeMemory;
extern PFN_vkDestroyShaderModule _vkDestroyShaderModule;
extern PFN_vkCmdBindPipeline _vkCmdBindPipeline;
extern PFN_vkCmdSetViewport _vkCmdSetViewport;
extern PFN_vkCmdSetScissor _vkCmdSetScissor;
extern PFN_vkCmdBindVertexBuffers _vkCmdBindVertexBuffers;
extern PFN_vkCmdDraw _vkCmdDraw;
extern PFN_vkDestroyDescriptorSetLayout _vkDestroyDescriptorSetLayout;
extern PFN_vkFreeDescriptorSets _vkFreeDescriptorSets;
extern PFN_vkDestroyDescriptorPool _vkDestroyDescriptorPool;
extern PFN_vkCmdBindDescriptorSets _vkCmdBindDescriptorSets;
extern PFN_vkCmdBindIndexBuffer _vkCmdBindIndexBuffer;
extern PFN_vkCmdDrawIndexed _vkCmdDrawIndexed;
extern PFN_vkGetInstanceProcAddr _vkGetInstanceProcAddr;
extern PFN_vkEnumeratePhysicalDevices _vkEnumeratePhysicalDevices;
extern PFN_vkGetPhysicalDeviceProperties _vkGetPhysicalDeviceProperties;
extern PFN_vkGetPhysicalDeviceMemoryProperties _vkGetPhysicalDeviceMemoryProperties;
extern PFN_vkDestroySurfaceKHR _vkDestroySurfaceKHR;
extern PFN_vkDestroyInstance _vkDestroyInstance;
extern PFN_vkFreeCommandBuffers _vkFreeCommandBuffers;
extern PFN_vkDestroyImageView _vkDestroyImageView;
extern PFN_vkDestroyImage _vkDestroyImage;
extern PFN_vkDestroyRenderPass _vkDestroyRenderPass;
extern PFN_vkDestroyFence _vkDestroyFence;
extern PFN_vkDestroyFramebuffer _vkDestroyFramebuffer;
extern PFN_vkDestroySwapchainKHR _vkDestroySwapchainKHR;
extern PFN_vkDestroySemaphore _vkDestroySemaphore;
extern PFN_vkDestroyCommandPool _vkDestroyCommandPool;
extern PFN_vkDeviceWaitIdle _vkDeviceWaitIdle;
extern PFN_vkDestroyDevice _vkDestroyDevice;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties _vkGetPhysicalDeviceQueueFamilyProperties;
extern PFN_vkCreateDevice _vkCreateDevice;
extern PFN_vkGetDeviceQueue _vkGetDeviceQueue;
extern PFN_vkCreateCommandPool _vkCreateCommandPool;
extern PFN_vkCreateFence _vkCreateFence;
extern PFN_vkCreateSemaphore _vkCreateSemaphore;
extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR _vkGetPhysicalDeviceSurfaceSupportKHR;
extern PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR _vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR _vkGetPhysicalDeviceSurfaceFormatsKHR;
extern PFN_vkGetPhysicalDeviceSurfacePresentModesKHR _vkGetPhysicalDeviceSurfacePresentModesKHR;
extern PFN_vkCreateSwapchainKHR _vkCreateSwapchainKHR;
extern PFN_vkGetSwapchainImagesKHR _vkGetSwapchainImagesKHR;
extern PFN_vkGetPhysicalDeviceFormatProperties _vkGetPhysicalDeviceFormatProperties;
extern PFN_vkCreateRenderPass _vkCreateRenderPass;
extern PFN_vkCreateFramebuffer _vkCreateFramebuffer;
extern PFN_vkAllocateCommandBuffers _vkAllocateCommandBuffers;
extern PFN_vkBeginCommandBuffer _vkBeginCommandBuffer;
extern PFN_vkQueueSubmit _vkQueueSubmit;
extern PFN_vkAcquireNextImageKHR _vkAcquireNextImageKHR;
extern PFN_vkWaitForFences _vkWaitForFences;
extern PFN_vkQueuePresentKHR _vkQueuePresentKHR;
extern PFN_vkResetFences _vkResetFences;
extern PFN_vkCmdPipelineBarrier _vkCmdPipelineBarrier;
extern PFN_vkCmdBeginRenderPass _vkCmdBeginRenderPass;
extern PFN_vkCmdCopyBufferToImage _vkCmdCopyBufferToImage;
extern PFN_vkCreateImage _vkCreateImage;
extern PFN_vkGetImageMemoryRequirements _vkGetImageMemoryRequirements;
extern PFN_vkAllocateMemory _vkAllocateMemory;
extern PFN_vkBindImageMemory _vkBindImageMemory;
extern PFN_vkEndCommandBuffer _vkEndCommandBuffer;
extern PFN_vkCreateImageView _vkCreateImageView;
extern PFN_vkCreateSampler _vkCreateSampler;
extern PFN_vkDestroySampler _vkDestroySampler;
extern PFN_vkCreateBuffer _vkCreateBuffer;
extern PFN_vkGetBufferMemoryRequirements _vkGetBufferMemoryRequirements;
extern PFN_vkBindBufferMemory _vkBindBufferMemory;
extern PFN_vkCmdCopyBuffer _vkCmdCopyBuffer;
extern PFN_vkCreateDescriptorSetLayout _vkCreateDescriptorSetLayout;
extern PFN_vkCreatePipelineLayout _vkCreatePipelineLayout;
extern PFN_vkCreateShaderModule _vkCreateShaderModule;
extern PFN_vkCreateDescriptorPool _vkCreateDescriptorPool;
extern PFN_vkAllocateDescriptorSets _vkAllocateDescriptorSets;
extern PFN_vkUpdateDescriptorSets _vkUpdateDescriptorSets;
extern PFN_vkCreatePipelineCache _vkCreatePipelineCache;
extern PFN_vkCreateGraphicsPipelines _vkCreateGraphicsPipelines;
extern PFN_vkCmdEndRenderPass _vkCmdEndRenderPass;
extern PFN_vkCmdCopyImageToBuffer _vkCmdCopyImageToBuffer;
extern PFN_vkCreateComputePipelines _vkCreateComputePipelines;
extern PFN_vkCmdDispatch _vkCmdDispatch;
extern PFN_vkCmdCopyImage _vkCmdCopyImage;
extern PFN_vkGetBufferDeviceAddress _vkGetBufferDeviceAddress;
extern PFN_vkGetPhysicalDeviceFeatures _vkGetPhysicalDeviceFeatures;
extern PFN_vkGetDeviceProcAddr _vkGetDeviceProcAddr;
extern PFN_vkGetPhysicalDeviceProperties2 _vkGetPhysicalDeviceProperties2;

#ifdef __ANDROID__
#include <vulkan/vulkan_android.h>
extern PFN_vkCreateAndroidSurfaceKHR _vkCreateAndroidSurfaceKHR;
#endif

#endif