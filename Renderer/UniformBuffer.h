//
// Created by thino.ivarsson on 2026-11-23.
//

#ifndef CAELUSENGINE_UNIFORMBUFFER_H
#define CAELUSENGINE_UNIFORMBUFFER_H
#include <vulkan/vulkan_core.h>

namespace Caelus
{
    struct UniformBuffer
    {
        [[nodiscard]] VkDescriptorSetLayout& descriptorSetLayout() {return m_descriptorSetLayout;}
        [[nodiscard]] VkDescriptorSet& binding() {return m_descriptorSet;}
    private:
        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
    };

} // Caelus

#endif //CAELUSENGINE_UNIFORMBUFFER_H
