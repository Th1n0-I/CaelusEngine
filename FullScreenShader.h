//
// Created by volts on 2026-11-14.
//

#ifndef CAELUSENGINE_FULLSCREENSHADER_H
#define CAELUSENGINE_FULLSCREENSHADER_H
#include "Pipeline.h"

namespace Caelus {
    struct FullScreenShader {
        VkPipeline pipeline;
        VkPipelineLayout layout;
        uint32_t pushSize;
        const void* pushData = nullptr;
    };
} // Caelus

#endif //CAELUSENGINE_FULLSCREENSHADER_H
