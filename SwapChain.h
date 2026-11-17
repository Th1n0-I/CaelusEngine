//
// Created by volts on 2026-11-10.
//

#ifndef CAELUSENGINE_SWAPCHAIN_H
#define CAELUSENGINE_SWAPCHAIN_H
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Caelus {
    struct SwapChain {
        VkSwapchainKHR handle = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkExtent2D extent = {};
        std::vector<VkImage> images = {};
        std::vector<VkImageView> views = {};
        VkImage depthImage = VK_NULL_HANDLE;
        VkDeviceMemory depthMemory = VK_NULL_HANDLE;
        VkImageView depthView = VK_NULL_HANDLE;
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
        VkImage HDRImage = VK_NULL_HANDLE;
        VkDeviceMemory HDRMemory = VK_NULL_HANDLE;
        VkImageView HDRView = VK_NULL_HANDLE;
        VkFormat HDRFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    };

    bool createSwapChain(const VkPhysicalDevice& gpu, const VkSurfaceKHR& surface, const VkDevice& device,
                         SwapChain& out);
} // Caelus

#endif //CAELUSENGINE_SWAPCHAIN_H
