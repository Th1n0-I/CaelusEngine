//
// Created by volts on 2026-11-10.
//

#ifndef CAELUSENGINE_BUFFER_H
#define CAELUSENGINE_BUFFER_H
#include <vulkan/vulkan_core.h>

#include "VulkanContext.h"

namespace Caelus
{
    //? GPU buffer plus memory.
    struct Buffer
    {
        VkBuffer handle = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDeviceSize size = 0;
    };
    
    uint32_t findMemoryType(const VkPhysicalDevice &gpu, const uint32_t typeFilter, VkMemoryPropertyFlags properties);
    bool allocateBuffer(const VulkanContext& ctx, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, Buffer& out);
    void destroyBuffer(VkDevice device, Buffer& buffer);
} // Caelus

#endif //CAELUSENGINE_BUFFER_H
