//
// Created by volts on 2026-11-16.
//

#include "Atmosphere.h"

#include <algorithm>
#include <numbers>
#include <vector>

#include "Renderer.h"

using namespace Caelus::Math; 

namespace Caelus {
    struct multiScatter
    {
        Vector3 secondBounce{0, 0, 0};
        Vector3 echoAmount{0, 0, 0};
        
        multiScatter operator/(const float& b) const {return {secondBounce / b, echoAmount / b}; }
        
        multiScatter& operator+=(const multiScatter& b) {
            secondBounce += b.secondBounce;
            echoAmount += b.echoAmount;
            return *this;
        };
    };
    
    


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

    static Vector2 densityAtPoint(float height, AtmosphereParams& params){
        float rayleighHeight01 = height / params.rayleighScaleHeight;
        float mieHeight01 = height / params.mieScaleHeight;
        float rayleighDensity = expf(-std::max(rayleighHeight01, 0.0f));
        float mieDensity = expf(-std::max(mieHeight01, 0.0f));
        return {rayleighDensity, mieDensity} ;
    }

    static Vector2 GetOpticalDepth(float height, float cosine, AtmosphereParams& params) {
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

    static std::vector<float> generateODData(int width, int height, AtmosphereParams& params) {
        std::vector<float> data;
        data.reserve(width * height * 2);
        data.resize(width * height * 2);
        params.m_data.reserve(width * height * 2);
        params.m_data.resize(width * height * 2);
        params.m_height = height;
        params.m_width = width;
        for (int w = 0; w < width; w++) {
            for (int h = 0; h < height; h++) {
                float altitude = (w + 0.5) / width * params.atmosphereHeight;
                float cos = (h + 0.5) / height * 2 - 1;
                Vector2 value = GetOpticalDepth(altitude, cos, params);
                data[(w + h * width) * 2] = value.x;
                data[(w + h * width) * 2+ 1] = value.y;
            }
        }
        params.m_data = data;
        return data;
    }

    Textures::Texture generateODLUT(int width, int height, AtmosphereParams& params, Renderer& renderer) {
        const std::vector<float> data = generateODData(width, height, params);
        return renderer.createTexture(data.data(), width, height, VK_FORMAT_R32G32_SFLOAT, 8, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    }
    
    Vector2 sampleODLUT(float alt, float cosine, AtmosphereParams& params) {
        float w = (alt / params.atmosphereHeight) * params.m_width - 0.5;
        float h = (cosine + 1) / 2 * params.m_height - 0.5;
        
        int iw = std::clamp((int)round(w), 0, params.m_width - 1);
        int ih = std::clamp((int)round(h), 0, params.m_height - 1);
        
        return {params.m_data[(iw + ih * params.m_width) * 2], params.m_data[(iw + ih * params.m_width) * 2 + 1]};
    }

    multiScatter getRayScatter(float height, float cosine, Vector3 direction, AtmosphereParams& params)
    {
        multiScatter ms{};
        Vector3 startPos = {0, params.radius + height, 0};
        
        Vector3 sunDir = {sqrtf(1 - cosine * cosine), cosine, 0};

        Vector2 groundIntersect = raySphereIntersect(direction, startPos, params.radius);
        Vector2 shellIntersect = raySphereIntersect(direction, startPos, params.radius + params.atmosphereHeight);
        
        float rayLength;
        if (groundIntersect == Vector2{0, 0} || groundIntersect.x < 0) rayLength = shellIntersect.y;
        
        else rayLength = groundIntersect.x;
        float stepSize = rayLength / 32;
        startPos -= direction * stepSize * 0.5;
        
        Vector2 viewOpticalDepth{};
        Vector3 f{};
        Vector3 L{};
        for (int i = 0; i < 32; i++)
        {
            startPos += direction * stepSize;
            float alt = startPos.length() - params.radius;
            Vector2 localDensity = densityAtPoint(alt, params);
            
            viewOpticalDepth += localDensity * stepSize;
            Vector3 transmittance = exp(-(params.rayleighBeta * viewOpticalDepth.x + params.mieBeta * 1.11 * viewOpticalDepth.y));
            Vector3 scatter = params.rayleighBeta * localDensity.x + params.mieBeta * localDensity.y;
            f += scatter * transmittance * stepSize;
            
            Vector2 shadowCheck = raySphereIntersect(sunDir, startPos, params.radius);
            if (shadowCheck.y > 0) continue;
            
            float sunCosine = dot(normalize(startPos), sunDir);
            Vector2 sunOD = sampleODLUT(alt, sunCosine, params);
            Vector3 TSun = exp(-(params.rayleighBeta * sunOD.x + params.mieBeta * 1.11 * sunOD.y));
            
            L += scatter * transmittance * TSun * stepSize / (std::numbers::pi * 4);
        }
        ms.secondBounce = L;
        ms.echoAmount = f;
        return ms;
    }

    multiScatter getMultiScatterAtPoint(float height, float cos, AtmosphereParams& params)
    {

        multiScatter gatheredScatter{};
        for (int bandValue = 0; bandValue < 8; bandValue++)
        {
            for (int ringValue = 0; ringValue < 8; ringValue++)
            {
                float angle = (((float)ringValue + 0.5) * (1.0 / 8)) * 2 * std::numbers::pi;
                float x = cosf(angle);
                float z = sinf(angle);
                float y = 1 - (((float)bandValue + 0.5) * (2.0 / 8));
                x *= sqrtf(1 - y * y);
                z *= sqrtf(1 - y * y);
                Vector3 direction = {x, y, z};
                gatheredScatter += getRayScatter(height, cos, direction, params);
            }
        }
        return gatheredScatter / 64;
    }

    Textures::Texture generateMultiScatterLUT(int width, int height, AtmosphereParams& params, Renderer& renderer)
    {
        std::vector<float> data;
        data.reserve(width * height * 4);
        data.resize(width * height * 4);
        for (int w = 0; w < width; w++)
        {
            for (int h = 0; h < height; h++)
            {
                float altitude = (w + 0.5) / width * params.atmosphereHeight;
                float cos = (h + 0.5) / height * 2 - 1;
                
                multiScatter ms = getMultiScatterAtPoint(altitude, cos, params);
                
                Vector3 psi = (ms.secondBounce / -(ms.echoAmount - 1)); // psi from Hillaire's paper
                
                data[(w + h * width) * 4 + 0] = psi.x;
                data[(w + h * width) * 4 + 1] = psi.y;
                data[(w + h * width) * 4 + 2] = psi.z;
                data[(w + h * width) * 4 + 3] = 1;
            }
        }
        return renderer.createTexture(data.data(), width, height, VK_FORMAT_R32G32B32A32_SFLOAT, 16, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    }
} // Caelus