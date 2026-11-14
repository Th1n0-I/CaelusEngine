//
// Created by volts on 2026-11-10.
//

#ifndef CAELUSENGINE_PIPELINE_H
#define CAELUSENGINE_PIPELINE_H
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Math/Math.h"

namespace Caelus {
    struct Consts {
        int res[2];
        float spin[2];
        float matrix[4][4];
        Math::Vector4 bodyPosRadius;
        Math::Vector4 sunPosEmissive;
    };

    VkShaderModule createShaderModule(const VkDevice& device, const std::vector<char>& code);
    bool createPipeline(const VkShaderModule& vertModule, const VkShaderModule& fragModule, const VkDevice& device,
                        const VkFormat format, VkPipelineLayout& layout, VkPipeline& pipeline,
                        VkPushConstantRange& range, VkDescriptorSetLayout descriptorSetLayout);
} // Caelus

#endif //CAELUSENGINE_PIPELINE_H
