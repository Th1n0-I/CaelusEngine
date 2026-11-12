#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragPos;

layout(push_constant) uniform Consts{
    int res[2];
    float pad_[2];
    mat4 matrix;
    vec4 bodyPosRadius;
    vec4 sunPosEmissive;
} pc;

void main(){
    gl_Position = pc.matrix * vec4(inPosition, 1.0);
    fragNormal = inPosition;
    fragPos = pc.bodyPosRadius.xyz + pc.bodyPosRadius.w * inPosition;
}