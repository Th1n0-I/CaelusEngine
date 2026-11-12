#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main(){
    vec3 baseCol = vec3(0.5, 0.5, 0.5);
    vec3 sunCol = vec3(1.0, 1.0, 1.0);
    vec3 shadowCol = vec3(0.002, 0.002, 0.008);
    vec3 sunDir = normalize(vec3(-1.0, -1.0, -1.0));
    vec3 normal = fragColor;
    float t = dot(normal, -sunDir);
    outColor = vec4(mix(shadowCol, sunCol, clamp(t, 0, 1)) * baseCol, 1.0);
}