#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Consts{
    float exposure;
} pc;

layout(set = 0, binding = 0) uniform sampler2D hdrTex;

void main() {
    vec4 col = textureLod(hdrTex, uv, 0);
    col.r = col.r / (1 + col.r);
    col.g = col.g / (1 + col.g);
    col.b = col.b / (1 + col.b);
    outColor = vec4(col.rgb, 1.0);
}
