#version 450

vec4 positions[3] = vec4[](
vec4(0.0, -0.5, 0.0, 1.0),
vec4(0.5, 0.5, 0.0, 1.0),
vec4(-0.5,0.5, 0.0, 1.0)
);

vec3 colors[3] = vec3[](
vec3(1.0, 0.0, 0.0),
vec3(0.0, 1.0, 0.0),
vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Consts{
    int res[2];
    float pad_[2];
    mat4 matrix;
} pc;

void main(){
    vec4 pos = pc.matrix * positions[gl_VertexIndex];
    gl_Position = pos;
    fragColor = colors[gl_VertexIndex];
}