#version 450

vec2 positions[3] = vec2[](
vec2(0.0, -0.5),
vec2(0.5, 0.5),
vec2(-0.5,0.5)
);

vec3 colors[3] = vec3[](
vec3(1.0, 0.0, 0.0),
vec3(0.0, 1.0, 0.0),
vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Consts{
    int res[2];
    float pos[6];
} pc;

void main(){
    float aspect_ratio = float(pc.res[0]) / float(pc.res[1]);

    gl_Position = vec4(pc.pos[gl_VertexIndex * 2] / aspect_ratio, pc.pos[gl_VertexIndex * 2 + 1] , 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}