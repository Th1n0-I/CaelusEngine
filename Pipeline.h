//
// Created by volts on 2026-11-10.
//

#ifndef CAELUSENGINE_PIPELINE_H
#define CAELUSENGINE_PIPELINE_H
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Math/Math.h"

namespace Caelus {
    struct PipelineDesc {
        VkShaderModule vert = VK_NULL_HANDLE;
        VkShaderModule frag = VK_NULL_HANDLE;
        uint32_t vertexStride = 0;
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        bool depthTest = true;
        bool depthWrite = true;
        bool alphaBlend = false;
        uint32_t pushConstantSize = 0;
        std::vector<VkDescriptorSetLayout> setLayouts;
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    };
    
    struct Consts {
        int res[2];
        float spin[2];
        float matrix[4][4];
        Math::Vector4 bodyPosRadius;
        Math::Vector4 sunPosEmissive;
        float E;
    };

    struct LineConsts {
        float viewProj[4][4];
        Math::Vector4 color;
    };
    VkShaderModule createShaderModule(const VkDevice& device, const std::vector<char>& code);

    bool createPipeline(const VkDevice& device, VkFormat format, const PipelineDesc& desc,
                        VkPipelineLayout& layout, VkPipeline& pipeline);
} // Caelus

#endif //CAELUSENGINE_PIPELINE_H
