#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Consts{
    vec4 BodyPositionRadius;
    vec4 FOVRayleigh;
    vec4 absorbtionSunPos;
    float res[2];
    float camera[2];
} pc;

mat3 rotX(float a) { float c = cos(a), s = sin(a); return mat3(1,0,0,  0,c,s,  0,-s,c); }
mat3 rotY(float a) { float c = cos(a), s = sin(a); return mat3(c,0,-s, 0,1,0,  s,0,c); }

vec2 raySphereIntersect(vec3 rayDir, vec3 rayStart, vec3 sphereCenter, float sphereRadius){
    rayDir = normalize(rayDir);
    float b = dot(2 * rayDir, (rayStart - sphereCenter));
    float c = dot(rayStart - sphereCenter, rayStart - sphereCenter) - sphereRadius * sphereRadius;
    float disc = b * b - 4 * c;
    if(disc <= 0) vec2(0, 0);
    float t1 = (-b + sqrt(disc)) / 2;
    float t2 = (-b - sqrt(disc)) / 2;
    return vec2(min(t1, t2), max(t1, t2));
}

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

void main(){
    vec3 raydir = getRayDirection();
    vec2 innerIntersect = raySphereIntersect(raydir, vec3(0, 0, 0), vec3(pc.BodyPositionRadius[0], pc.BodyPositionRadius[1], pc.BodyPositionRadius[2]), pc.BodyPositionRadius[3]);
    vec2 outerIntersect = raySphereIntersect(raydir, vec3(0, 0, 0), vec3(pc.BodyPositionRadius[0], pc.BodyPositionRadius[1], pc.BodyPositionRadius[2]), pc.BodyPositionRadius[3] + 100000);
    if(outerIntersect == vec2(0, 0)) {
        outColor = vec4(getRayDirection() * 0.5 + 0.5, 1.0);
    } else if(innerIntersect == vec2(0, 0)){
        float opticalDepth = outerIntersect.y - outerIntersect.x;
        float transmittance = exp(-opticalDepth * pc.absorbtionSunPos[0]);
        outColor = vec4(0, 0, 0, 1 - transmittance);
    } else{
        float opticalDepth = innerIntersect.x - outerIntersect.x;
        float transmittance = exp(-opticalDepth * pc.absorbtionSunPos[0]);
        outColor = vec4(0, 0, 0, 1 - transmittance);
    }
    
}