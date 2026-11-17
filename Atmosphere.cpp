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

    static Vector2 densityAtPoint(float height, AtmosphereParams params){
        float rayleighHeight01 = height / params.rayleighScaleHeight;
        float mieHeight01 = height / params.mieScaleHeight;
        float rayleighDensity = expf(-std::max(rayleighHeight01, 0.0f));
        float mieDensity = expf(-std::max(mieHeight01, 0.0f));
        return {rayleighDensity, mieDensity} ;
    }

    static Vector2 GetOpticalDepth(float height, float cosine, AtmosphereParams params) {
        Vector3 startPos = {0, height + params.radius, 0};
        Vector3 direction = normalize(Vector3{sqrtf(1 - cosine * cosine), cosine, 0}) ;
    
        auto innerIntersect = raySphereIntersect(direction, startPos, params.radius);
        auto outerIntersect = raySphereIntersect(direction, startPos, params.radius + params.atmosphereHeight);
        float exit = outerIntersect.y;
    
    
        float stepSize = exit / 256;
        float rayleighOpticalDepth = 0;
        float mieOpticalDepth = 0;
        
        startPos += direction * stepSize * 0.5;
    
        for(int i = 0; i < 256; i++){
            Vector2 localDensity = densityAtPoint(startPos.length() - params.radius, params);
            rayleighOpticalDepth += localDensity.x * stepSize;
            mieOpticalDepth += localDensity.y * stepSize;
            startPos += direction * stepSize;
        }
        return {rayleighOpticalDepth, mieOpticalDepth} ;
    }
    
    Textures::Texture GenerateLUT(int width, int height, AtmosphereParams params, Renderer& renderer) {
        std::vector<float> data;
        data.reserve(width * height * 2);
        data.resize(width * height * 2);
        for (int w = 0; w < width; w++) {
            for (int h = 0; h < height; h++) {
                float altitude = (w + 0.5) / width * params.atmosphereHeight;
                float cos = (h + 0.5) / height * 2 - 1;
                Vector2 value = GetOpticalDepth(altitude, cos, params);
                data[(w + h * width) * 2] = value.x;
                data[(w + h * width) * 2+ 1] = value.y;
            }
        }
        return renderer.createTexture(data.data(), width, height, VK_FORMAT_R32G32_SFLOAT, 8, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    }
} // Caelus