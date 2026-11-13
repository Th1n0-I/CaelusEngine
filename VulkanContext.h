//
// Created by volts on 2026-11-10.
//

#ifndef CAELUSENGINE_VULKANCONTEXT_H
#define CAELUSENGINE_VULKANCONTEXT_H

#include <vulkan/vulkan.h>

namespace Caelus {
    struct VulkanContext {
        VkInstance instance = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkPhysicalDevice gpu = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        uint32_t queueFamilyIndex = UINT32_MAX;
    };


    bool createInstance(VkInstance & instance);
    bool pickGpu(const VkInstance& instance, VkPhysicalDevice& gpu);
    bool findGraphicsQueueFamilyIndex(const VkPhysicalDevice& gpu, uint32_t& queueFamilyIndex,
                                      const VkSurfaceKHR& surface);
    bool createLogicalDevice(const uint32_t& queueFamilyIndex, const VkPhysicalDevice& gpu, VkDevice& device,
                             VkQueue& graphicsQueue);
} // caelus

#endif //CAELUSENGINE_VULKANCONTEXT_H
