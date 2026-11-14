#version 450
layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform lineConsts{
    mat4 viewProj;
    vec4 color;
} pc;

void main() {
    gl_Position = pc.viewProj * vec4(inPosition, 1.0);
}