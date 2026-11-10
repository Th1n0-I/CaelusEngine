//
// Created by volts on 2026-11-10.
//

#include "SwapChain.h"

#include "Buffer.h"
#include <cstdio>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Caelus
{
    
    static bool createDepthBuffer(const VkPhysicalDevice &gpu, const VkDevice &device, SwapChain &swapChain) {
        VkImageCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ci.imageType = VK_IMAGE_TYPE_2D;
        ci.format = swapChain.depthFormat;
        ci.extent = {.width = swapChain.extent.width, .height = swapChain.extent.height, .depth = 1};
        ci.mipLevels = 1;
        ci.arrayLayers = 1;
        ci.samples = VK_SAMPLE_COUNT_1_BIT;
        ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (vkCreateImage(device, &ci, nullptr, &swapChain.depthImage) != VK_SUCCESS) {
            printf("Failed to create depth image!");
            return false;
        }

        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(device, swapChain.depthImage, &req);

        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = findMemoryType(gpu, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device, &ai, nullptr, &swapChain.depthMemory) != VK_SUCCESS) {
            printf("Failed to allocate depth memory!");
            return false;
        }

        vkBindImageMemory(device, swapChain.depthImage, swapChain.depthMemory, 0);

        VkImageViewCreateInfo ici{};
        ici.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ici.image = swapChain.depthImage;
        ici.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ici.format = swapChain.depthFormat;
        ici.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};

        if (vkCreateImageView(device, &ici, nullptr, &swapChain.depthView) != VK_SUCCESS) {
            printf("Failed to create depth view!");
            return false;
        }
        printf("Successfully created depth buffer!");
        return true;
    }
    
    bool createSwapChain(const VkPhysicalDevice &gpu, const VkSurfaceKHR &surface, const VkDevice &device, SwapChain &out) {
        //! GET INFORMATION ABOUT THE SWAPCHAIN
        // Get some information Idk
        VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &caps);

        // Which pixel format it supports, call twice like in pickGpu()
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, &formats[0]);

        // Get the correct format
        VkSurfaceFormatKHR chosenFormat = formats[0];
        for (const auto &format: formats) {
            if (format.format == VK_FORMAT_R8G8B8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                chosenFormat = format;
                break;
            }
        }

        // How many images to keep in buffer so the cpu doesnt have to wait
        uint32_t imageCount = caps.minImageCount + 1;
        if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
            imageCount = caps.maxImageCount;

        const VkExtent2D extent = caps.currentExtent;

        //! CREATE THE SWAPCHAIN

        VkSwapchainCreateInfoKHR ci{};
        ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        ci.surface = surface;
        ci.minImageCount = imageCount;
        ci.imageFormat = chosenFormat.format;
        ci.imageColorSpace = chosenFormat.colorSpace;
        ci.imageExtent = extent;
        ci.imageArrayLayers = 1;
        ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ci.preTransform = caps.currentTransform;
        ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Dont blend the window with the desktop
        ci.presentMode = VK_PRESENT_MODE_FIFO_KHR; // VSync
        ci.clipped = VK_TRUE;
        ci.oldSwapchain = VK_NULL_HANDLE;


        if (vkCreateSwapchainKHR(device, &ci, nullptr, &out.handle) != VK_SUCCESS) {
            printf("Failed to create swapChain!\n");
            return false;
        }
        out.format = chosenFormat.format;
        out.extent = extent;
        printf("Successfully created swapchain! width: %u height: %u \n", extent.width, extent.height);

        //! CREATE IMAGES AND VIEWS

        // Get the amount of images, same double call as always
        uint32_t imgCount = 0;
        vkGetSwapchainImagesKHR(device, out.handle, &imgCount, nullptr);

        out.images.resize(imgCount);
        out.views.resize(imgCount);
        vkGetSwapchainImagesKHR(device, out.handle, &imgCount, out.images.data());

        // Create one image view per image
        for (uint32_t i = 0; i < imgCount; i++) {
            VkImageViewCreateInfo ici{};
            ici.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            ici.image = out.images[i];
            ici.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ici.format = out.format;
            ici.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            // Color data as opposed to something like a depth buffer
            ici.subresourceRange.baseMipLevel = 0;
            ici.subresourceRange.levelCount = 1;
            ici.subresourceRange.baseArrayLayer = 0;
            ici.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &ici, nullptr, &out.views[i]) != VK_SUCCESS) {
                printf("Failed to create image view %u!\n", i);
                return false;
            }
        }
        printf("Successfully created %u image views!\n", imgCount);

        if (!createDepthBuffer(gpu, device, out)) return false;

        return true;
    }
} // Caelus