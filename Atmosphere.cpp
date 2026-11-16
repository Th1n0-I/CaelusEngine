//
// Created by volts on 2026-11-16.
//

#include "Atmosphere.h"

#include <vector>

#include "Renderer.h"

using namespace Caelus::Math; 

namespace Caelus {
    static Vector2 raySphereIntersect(Vector3 rayDir, Vector3 rayStart, float sphereRadius){
        rayDir = normalize(rayDir);
        float b = dot(rayDir * 2, (rayStart ));
        float c = dot(rayStart, rayStart) - sphereRadius * sphereRadius;
        float disc = b * b - 4 * c;
        if(disc <= 0) return {0, 0};
        float t1 = (-b + sqrt(disc)) / 2;
        float t2 = (-b - sqrt(disc)) / 2;
        return {std::min(t1, t2), std::max(t1, t2)};
    }

    static float densityAtPoint(float height, AtmosphereParams params){
        float height01 = height / params.scaleHeight;
        float localDensity = expf(-std::max(height01, 0.0f));
        return localDensity;
    }

    static float GetOpticalDepth(float height, float cosine, AtmosphereParams params) {
        Vector3 startPos = {0, height + params.radius, 0};
        Vector3 direction = normalize(Vector3{sqrtf(1 - cosine * cosine), cosine, 0}) ;
    
        auto innerIntersect = raySphereIntersect(direction, startPos, params.radius);
        auto outerIntersect = raySphereIntersect(direction, startPos, params.radius + params.atmosphereHeight);
        float exit = outerIntersect.y;
    
    
        float stepSize = exit / 32;
        float opticalDepth = 0;
    
        for(int i = 0; i < 32; i++){
            float localDensity = densityAtPoint(startPos.length() - params.radius, params);
            opticalDepth += localDensity * stepSize;
            startPos += direction * stepSize;
        }
        return opticalDepth;
    }
    
    Textures::Texture GenerateLUT(int width, int height, AtmosphereParams params, Renderer& renderer) {
        std::vector<float> data;
        data.reserve(width * height);
        data.resize(width * height);
        for (int w = 0; w < width; w++) {
            for (int h = 0; h < height; h++) {
                float altitude = (w + 0.5) / width * params.atmosphereHeight;
                float cos = (h + 0.5) / height * 2 - 1;
                float value = GetOpticalDepth(altitude, cos, params);
                data[w + h * width] = value;
            }
        }
        return renderer.createTexture(data.data(), width, height, VK_FORMAT_R32_SFLOAT, 4, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    }
} // Caelus