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
        float rayleighScaleHeight = 0;
        float mieScaleHeight = 0;
        float mieBeta = 0;
        float mieG = 0;
        Math::Vector3 rayleighBeta = Math::Vector3(0, 0, 0);
    };
    
    struct AtmosphereConstants {
        Math::Vector3 BodyPosition; float Radius;
        Math::Vector3 Rayleigh; float FOV;
        Math::Vector3 SunPos; float E;
        Math::Vector2 res; Math::Vector2 camera;
        float atmosphereHeight;
        float scaleHeight;
        float mieScaleHeight;
        float mieG;
        float mieBeta;
    };

    Textures::Texture generateODLUT(int width, int height, AtmosphereParams params, Renderer& renderer);
} // Caelus

#endif //CAELUSENGINE_ATMOSPHERE_H
