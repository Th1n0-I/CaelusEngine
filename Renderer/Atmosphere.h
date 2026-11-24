//
// Created by volts on 2026-11-16.
//

#ifndef CAELUSENGINE_ATMOSPHERE_H
#define CAELUSENGINE_ATMOSPHERE_H
#include "Renderer.h"
#include "Textures.h"
#include "../Math/Math.h"

namespace Caelus {
    struct AtmosphereParams {
        float radius = 0;
        float atmosphereHeight = 0;
        float rayleighScaleHeight = 0;
        float mieScaleHeight = 0;
        float mieG = 0;
        Math::Vector3 rayleighBeta = Math::Vector3(0, 0, 0);
        Math::Vector3 mieBeta = Math::Vector3(0, 0, 0);
        Math::Vector3 mieAbsorption = Math::Vector3(0, 0, 0);
        std::vector<float> m_data;
        int m_width;
        int m_height;
    };

    struct SingleAtmoParams
    {
        Math::Vector3 Rayleigh; float Radius;
        Math::Vector3 MieBeta; float mieG;
        Math::Vector3 mieAbsorption; float mieScaleHeight;
        float atmosphereHeight;
        float scaleHeight;
        float padding[2];
    };

    struct AtmoUboConsts
    {
        SingleAtmoParams atmospheres[8];
    };
    
    struct AtmosphereConstants {
        Math::Vector3 BodyPosition; float Radius;
        Math::Vector3 Rayleigh; float FOV;
        Math::Vector3 SunPos; float E;
        Math::Vector3 mieBeta; float mieG;
        Math::Vector3 mieAbsorption; float mieScaleHeight;
        Math::Vector2 res; Math::Vector2 camera;
        float atmosphereHeight;
        float scaleHeight;
    };

    Textures::Texture generateODLUT(int width, int height, AtmosphereParams& params, Renderer& renderer);
    Textures::Texture generateMultiScatterLUT(int width, int height, AtmosphereParams& params, Renderer& renderer);
    Textures::Texture generateGroundTransmittanceLUT(int width, AtmosphereParams& params, Renderer& renderer);
} // Caelus

#endif //CAELUSENGINE_ATMOSPHERE_H
