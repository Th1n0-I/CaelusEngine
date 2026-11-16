//
// Created by volts on 2026-11-16.
//

#ifndef CAELUSENGINE_ATMOSPHERE_H
#define CAELUSENGINE_ATMOSPHERE_H
#include "Renderer.h"
#include "Textures.h"
#include "Math/Math.h"

namespace Caelus {
    struct AtmosphereParams {
        float radius = 0;
        float atmosphereHeight = 0;
        float scaleHeight = 0;
        Math::Vector3 rayleighBeta = Math::Vector3(0, 0, 0);
    };

    Textures::Texture GenerateLUT(int width, int height, AtmosphereParams params, Renderer& renderer);
} // Caelus

#endif //CAELUSENGINE_ATMOSPHERE_H
