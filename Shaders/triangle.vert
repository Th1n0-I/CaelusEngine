#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Consts{
    int res[2];
    float pad_[2];
    mat4 matrix;
} pc;

void main(){
    gl_Position = pc.matrix * vec4(inPosition, 1.0);
    fragColor = inPosition;
}