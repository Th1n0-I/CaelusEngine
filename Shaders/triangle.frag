#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragPos;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Consts{
    int res[2];
    float pad_[2];
    mat4 matrix;
    vec4 bodyPosRadius;
    vec4 sunPosEmissive;
} pc;

layout(set = 0, binding = 0) uniform sampler2D starMap;

void main(){
    if(pc.sunPosEmissive.w > 1.5) {
        vec3 d = normalize(fragPos);
        float u = 0.5 + atan(d.z, d.x) / (2.0 * 3.1415926535);
        float v = 0.5 - asin(d.y) / 3.1415926535;
        outColor = vec4(texture(starMap, vec2(u, v)).rgb * 0.5, 1.0);
        return;
    }
    
    vec3 sunCol = vec3(1.0, 0.9, 0.7);
    if(pc.sunPosEmissive.w > 0.5){
        outColor = vec4(sunCol, 1.0);
        return;
    }
    
    vec3 baseCol = vec3(0.5, 0.5, 0.5);
    vec3 shadowCol = vec3(0, 0, 0);
    
    vec3 normal = normalize(fragNormal);
    vec3 toSun = normalize(pc.sunPosEmissive.xyz - fragPos);
    float t = max(dot(normal, toSun), 0.0);
    outColor = vec4(shadowCol + baseCol * sunCol * t, 1.0);
}