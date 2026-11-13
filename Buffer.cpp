//
// Created by volts on 2026-11-10.
//

#include "Buffer.h"

#include <cstdio>

#include "VulkanContext.h"

namespace Caelus {
    uint32_t findMemoryType(const VkPhysicalDevice& gpu, const uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            if ((typeFilter & (1u << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) return i;
        printf("Failed to find suitable memory type!\n");
        return 0;
    }

    bool allocateBuffer(const VulkanContext& ctx, VkDeviceSize size, VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags props, Buffer& out) {
        out = {};
        out.size = size;

        VkBufferCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        ci.size = size;
        ci.usage = usage;
        ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(ctx.device, &ci, nullptr, &out.handle) != VK_SUCCESS) {
            printf("Failed to create buffer!");
            return false;
        }

        VkMemoryRequirements req;
        vkGetBufferMemoryRequirements(ctx.device, out.handle, &req);

        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = findMemoryType(ctx.gpu, req.memoryTypeBits, props);
        if (vkAllocateMemory(ctx.device, &ai, nullptr, &out.memory) != VK_SUCCESS) {
            printf("Failed to allocate memory!");
            vkDestroyBuffer(ctx.device, out.handle, nullptr);
            out = {};
            return false;
        }

        vkBindBufferMemory(ctx.device, out.handle, out.memory, 0);
        return true;
    }

    void destroyBuffer(VkDevice device, Buffer& buffer) {
        vkDestroyBuffer(device, buffer.handle, nullptr);
        vkFreeMemory(device, buffer.memory, nullptr);
        buffer = {};
    }
} // Caelus