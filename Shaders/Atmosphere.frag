#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Consts{
    vec4 BodyPositionRadius;
    vec4 FOVRayleigh;
    vec4 intensitySunPos;
    float res[2];
    float camera[2];
} pc;

layout(set = 0, binding = 0) uniform sampler2D sunRayLUT;

mat3 rotX(float a) { float c = cos(a), s = sin(a); return mat3(1,0,0,  0,c,s,  0,-s,c); }
mat3 rotY(float a) { float c = cos(a), s = sin(a); return mat3(c,0,-s, 0,1,0,  s,0,c); }



vec3 getRayDirection(){
    vec3 rayDir;
    vec2 ndc = (uv - 0.5) * 2.0;
    
    float f = tan(pc.FOVRayleigh.x * 0.5f);
    float aspect = pc.res[0] / pc.res[1];
    
    float deltaY = f * -ndc.y;
    float deltaX = f * aspect * ndc.x;
    
    vec3 newPos = vec3(deltaX, deltaY, -1.0);
    newPos = normalize(newPos);
    
    mat3 mat = rotY(pc.camera[1]) * rotX(-pc.camera[0]);
    rayDir = mat * newPos;
    
    return rayDir;
}

vec2 raySphereIntersect(vec3 rayDir, vec3 rayStart, float sphereRadius){
    vec3 sphereCenter = pc.BodyPositionRadius.xyz / 1000;
    rayStart /= 1000;
    sphereRadius /= 1000;
    rayDir = normalize(rayDir);
    float b = dot(2 * rayDir, (rayStart - sphereCenter));
    float c = dot(rayStart - sphereCenter, rayStart - sphereCenter) - sphereRadius * sphereRadius;
    float disc = b * b - 4 * c;
    if(disc <= 0) return vec2(0, 0);
    float t1 = (-b + sqrt(disc)) / 2 * 1000;
    float t2 = (-b - sqrt(disc)) / 2 * 1000;
    return vec2(min(t1, t2), max(t1, t2));
}

vec2 getDoubleSphereIntersect(vec2 big, vec2 small){
    
    if(big == vec2(0, 0)) return vec2(0, 0);
    if(small == vec2(0, 0)) {
        if(big.x < 0) return vec2(0, big.y);
        else return big;
    } else {
        if(small.y < 0) return vec2(0, big.y);
        if(big.x < 0) return vec2(0, small.x);
        else return vec2(big.x, small.x);
    }
}

float densityAtPoint(vec3 point){
    float heightAboveSurface = distance(point, pc.BodyPositionRadius.xyz) - pc.BodyPositionRadius.w;
    float height01 = heightAboveSurface / 8000;
    float localDensity = exp(-height01);
    return localDensity;
}

vec3 opticalDepth(vec3 rayOrigin, vec3 rayDir, float rayLength){
    vec3 denistySamplePos = rayOrigin;
    float stepSize = rayLength / 32;
    float opticalDepth = 0;
    
    for(int i = 0; i < 32; i++){
        float localDensity = densityAtPoint(denistySamplePos);
        opticalDepth += localDensity * stepSize;
        denistySamplePos += rayDir * stepSize;
    }
    return opticalDepth * pc.FOVRayleigh.yzw;
}

void main(){
    outColor = vec4(0, 0, 0, 0);
    vec3 raydir = getRayDirection();
    float planetRadius = pc.BodyPositionRadius.w;
    vec2 innerIntersect = raySphereIntersect(raydir, vec3(0, 0, 0), planetRadius);
    vec2 outerIntersect = raySphereIntersect(raydir, vec3(0, 0, 0), planetRadius + 100000);
    vec2 Intersect = getDoubleSphereIntersect(outerIntersect, innerIntersect);
    if(Intersect == vec2(0, 0)) return;
    
    float stepSize = (Intersect.y - Intersect.x) / 64;
    vec3 inScatteredLight = vec3(0, 0, 0);
    float sumTransmittance = 1;
    float viewRayOpticalDepth;
    for(int i = 0; i < 64; i++){
        vec3 pos;
        if (i * stepSize + stepSize / 2 > Intersect.y - Intersect.x) pos = raydir * Intersect.y;
        else pos = raydir * (i * stepSize + Intersect.x + stepSize / 2);
        vec3 sunDir = normalize(pc.intensitySunPos.yzw - pos);
        vec2 sunInnerIntersect = raySphereIntersect(sunDir, pos, planetRadius);
        vec2 sunOuterIntersect = raySphereIntersect(sunDir, pos, planetRadius + 100000);
        vec2 sunRayIntersect = getDoubleSphereIntersect(sunOuterIntersect, sunInnerIntersect);
        float sunRayLength = sunRayIntersect.y - sunRayIntersect.x;
        vec2 LUTuv = vec2((distance(pos, pc.BodyPositionRadius.xyz) - pc.BodyPositionRadius.w) / 100000, dot(normalize(pos - pc.BodyPositionRadius.xyz), sunDir) * 0.5 + 0.5);
        vec3 sunRayOpticalDepth = texture(sunRayLUT, LUTuv).r * pc.FOVRayleigh.yzw;
        float localDensity = densityAtPoint(pos);
        viewRayOpticalDepth += localDensity * stepSize;
        vec3 transmittance = exp(-(sunRayOpticalDepth + viewRayOpticalDepth * pc.FOVRayleigh.yzw));
        sumTransmittance *= exp(-stepSize * localDensity * pc.FOVRayleigh.y);
        if(sunInnerIntersect == vec2(0, 0) || sunInnerIntersect.x < 0) inScatteredLight += localDensity * transmittance * stepSize * pc.FOVRayleigh.yzw * pc.intensitySunPos.x;
    }
    outColor = vec4(inScatteredLight, 1 - sumTransmittance);
}