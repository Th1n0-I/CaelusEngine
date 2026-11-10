//
// Created by volts on 2026-11-10.
//

#ifndef CAELUSENGINE_PIPELINE_H
#define CAELUSENGINE_PIPELINE_H
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Caelus
{
    VkShaderModule createShaderModule(const VkDevice &device, const std::vector<char> &code);
    bool createPipeline(const VkShaderModule& vertModule, const VkShaderModule& fragModule, const VkDevice& device, const VkFormat format, VkPipelineLayout& layout, VkPipeline& pipeline, VkPushConstantRange& range);
} // Caelus

#endif //CAELUSENGINE_PIPELINE_H
