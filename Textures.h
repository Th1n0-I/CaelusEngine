//
// Created by volts on 2026-11-14.
//

#ifndef CAELUSENGINE_TEXTURES_H
#define CAELUSENGINE_TEXTURES_H
#include <vulkan/vulkan_core.h>

namespace Caelus {
    namespace Textures {
        struct Texture {
            VkImage image = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;
            VkImageView view = VK_NULL_HANDLE;
            uint32_t width = 0, height = 0;
            VkSampler sampler = VK_NULL_HANDLE;
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        };
    } // Textures
} // Caelus

#endif //CAELUSENGINE_TEXTURES_H

