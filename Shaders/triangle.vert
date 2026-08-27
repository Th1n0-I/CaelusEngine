#version 450

vec4 positions[3] = vec4[](
vec4(0.0, -0.5, 0.0, 1.0),
vec4(0.5, 0.5, 0.0, 1.0),
vec4(-0.5,0.5, 0.0, 1.0)
);

vec3 corners[8] = vec3[](
vec3(-0.5,-0.5,-0.5), vec3( 0.5,-0.5,-0.5), vec3( 0.5, 0.5,-0.5), vec3(-0.5, 0.5,-0.5),
vec3(-0.5,-0.5, 0.5), vec3( 0.5,-0.5, 0.5), vec3( 0.5, 0.5, 0.5), vec3(-0.5, 0.5, 0.5)
);

int indices[36] = int[](
0,1,2, 0,2,3,   // back
4,5,6, 4,6,7,   // front
0,3,7, 0,7,4,   // left
1,5,6, 1,6,2,   // right
0,4,5, 0,5,1,   // bottom
3,2,6, 3,6,7    // top
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
    vec3 p = corners[indices[gl_VertexIndex]];
    gl_Position = pc.matrix * vec4(p, 1.0);
    fragColor = p + 0.5;
}