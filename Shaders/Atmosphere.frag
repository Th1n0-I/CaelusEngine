#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Consts{
    vec3 BodyPosition; float Radius;
    vec3 Rayleigh; float FOV;
    vec3 SunPos; float E;
    vec3 mieBeta; float mieG;
    vec3 mieAbsortption; float mieScaleHeight;
    vec2 res; vec2 camera;
    float atmosphereHeight;
    float scaleHeight;
} pc;

#include "Eclipse.glsl"

layout(set = 0, binding = 0) uniform sampler2D sunRayLUT;
layout(set = 1, binding = 0) uniform sampler2D multiscatterLUT;

mat3 rotX(float a) { float c = cos(a), s = sin(a); return mat3(1,0,0,  0,c,s,  0,-s,c); }
mat3 rotY(float a) { float c = cos(a), s = sin(a); return mat3(c,0,-s, 0,1,0,  s,0,c); }



vec3 getRayDirection(){
    vec3 rayDir;
    vec2 ndc = (uv - 0.5) * 2.0;
    
    float f = tan(pc.FOV * 0.5f);
    float aspect = pc.res.x / pc.res.y;
    
    float deltaY = f * -ndc.y;
    float deltaX = f * aspect * ndc.x;
    
    vec3 newPos = vec3(deltaX, deltaY, -1.0);
    newPos = normalize(newPos);
    
    mat3 mat = rotY(pc.camera.y) * rotX(-pc.camera.x);
    rayDir = mat * newPos;
    
    return rayDir;
}

vec2 raySphereIntersect(vec3 rayDir, vec3 rayStart, float sphereRadius){
    vec3 sphereCenter = pc.BodyPosition;
    rayDir = normalize(rayDir);
    float b = dot(2 * rayDir, (rayStart - sphereCenter));
    float c = dot(rayStart - sphereCenter, rayStart - sphereCenter) - sphereRadius * sphereRadius;
    float disc = b * b - 4 * c;
    if(disc <= 0) return vec2(0, 0);
    float t1 = (-b + sqrt(disc)) / 2;
    float t2 = (-b - sqrt(disc)) / 2;
    return vec2(min(t1, t2), max(t1, t2));
}

vec2 getDoubleSphereIntersect(vec2 big, vec2 small){
    
    if(big == vec2(0, 0)) return vec2(0, 0);
    if(big.y <= 0) return vec2(0, 0);
    if(small == vec2(0, 0)) {
        if(big.x < 0) return vec2(0, big.y);
        else return big;
    } else {
        if(small.y < 0) return vec2(0, big.y);
        if(big.x < 0) return vec2(0, small.x);
        else return vec2(big.x, small.x);
    }
}

vec2 densityAtPoint(vec3 point){
    float heightAboveSurface = distance(point, pc.BodyPosition) - pc.Radius;
    float height01 = heightAboveSurface / pc.scaleHeight;
    float mieHeight01 = heightAboveSurface / pc.mieScaleHeight;
    float localDensity = exp(-height01);
    float mieLocalDensity = exp(-mieHeight01);
    return vec2(localDensity, mieLocalDensity);
}

float phaseRayleigh(float cosTheta){
    return 3.0 / (16.0 * 3.1415926535) * (1.0 + cosTheta * cosTheta);
}

float henyeyGreenstein(float cosTheta, float g){
    return (1.0 / (4.0 * 3.1415926535)) * ((1 - g * g) / pow(1.0 + g * g - 2.0 * g * cosTheta, 1.5));
}

float avg(vec3 v){
    return (v.x + v.y + v.z) / 3;
}

void main(){
    outColor = vec4(0, 0, 0, 1);
    vec3 mieExtinction = pc.mieBeta + pc.mieAbsortption;
    
    //-Setup-//
    vec3 raydir = getRayDirection();
    vec2 innerIntersect = raySphereIntersect(raydir, vec3(0, 0, 0), pc.Radius);
    vec2 outerIntersect = raySphereIntersect(raydir, vec3(0, 0, 0), pc.Radius + pc.atmosphereHeight);
    vec2 Intersect = getDoubleSphereIntersect(outerIntersect, innerIntersect);
    if(Intersect == vec2(0, 0)) return;
    
    float stepSize = (Intersect.y - Intersect.x) / 64;
    
    //-Fields-//
    vec3 inScatteredR = vec3(0);
    vec3 inScatteredM = vec3(0);
    vec3 inScatteredMS = vec3(0);
    float sumTransmittance = 1;
    vec2 viewRayOpticalDepth = vec2(0);
    
    for(int i = 0; i < 64; i++){
        // Position and direction to sun
        vec3 pos = raydir * (i * stepSize + Intersect.x + stepSize / 2);
        vec3 sunDir = normalize(pc.SunPos - pos);
        
        vec2 localDensity = densityAtPoint(pos);
        
        vec3 sigmaT = pc.Rayleigh * localDensity.x + mieExtinction * localDensity.y;
        vec3 stepLen = (1.0 - exp(-sigmaT * stepSize)) / max(sigmaT, vec3(1e-8));
        
        vec2 LUTuv = vec2((distance(pos, pc.BodyPosition) - pc.Radius) / pc.atmosphereHeight, dot(normalize(pos - pc.BodyPosition), sunDir) * 0.5 + 0.5);
        vec3 multiscatter = textureLod(multiscatterLUT, LUTuv, 0.0).rgb;
        
        vec3 viewTransmittance = exp(-(pc.Rayleigh * viewRayOpticalDepth.x + mieExtinction * viewRayOpticalDepth.y));
        vec3 scattering = pc.Rayleigh * localDensity.x + pc.mieBeta * localDensity.y;
        
        float eclipseTerm = getEclipseShadowAmount(pos);
        
        inScatteredMS += scattering * viewTransmittance * multiscatter * stepLen * eclipseTerm;

        viewRayOpticalDepth += localDensity * stepSize;
        sumTransmittance *= avg(exp(-sigmaT * stepSize));
        
        // Early skip on shadowed
        vec2 sunInnerIntersect = raySphereIntersect(sunDir, pos, pc.Radius);
        if(sunInnerIntersect != vec2(0,0) && sunInnerIntersect.x >= 0) continue;
        
        //LUT
        vec2 sunOD = textureLod(sunRayLUT, LUTuv, 0.0).rg;
        
        //How much light gets absorbed.
        vec3 sunTransmittance = exp(-(pc.Rayleigh * sunOD.r + mieExtinction * sunOD.g)) * eclipseTerm;
        vec3 transmittance = viewTransmittance * sunTransmittance;
        
        if(isnan(sumTransmittance) || isinf(sumTransmittance)){
            outColor = vec4(1, 0, 1, 0);
            return;
        }
        
        inScatteredM += localDensity.y * transmittance * stepLen;
        inScatteredR += localDensity.x * transmittance * stepLen;
    }
    
    float cosTheta = dot(raydir, normalize(pc.SunPos));
    float phaseR = phaseRayleigh(cosTheta);
    float phaseM = henyeyGreenstein(cosTheta, pc.mieG);
    vec3 rayleigh = inScatteredR * phaseR * pc.Rayleigh * pc.E;
    vec3 mie = inScatteredM * phaseM * pc.mieBeta * pc.E;
    vec3 multi = inScatteredMS * pc.E; 
    
    outColor = vec4(rayleigh + mie + multi, sumTransmittance);
}