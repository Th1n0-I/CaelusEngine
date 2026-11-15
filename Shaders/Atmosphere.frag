#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Consts{
    vec4 BodyPositionRadius;
    vec4 FOVRayleigh;
    float res[2];
    float camera[2];
} pc;

mat3 rotX(float a) { float c = cos(a), s = sin(a); return mat3(1,0,0,  0,c,s,  0,-s,c); }
mat3 rotY(float a) { float c = cos(a), s = sin(a); return mat3(c,0,-s, 0,1,0,  s,0,c); }

vec3 getRayDirection(){
    vec3 rayDir;
    vec2 ndc = (uv - 0.5) * 2.0;
    
    float f = tan(pc.FOVRayleigh.x * 0.5f);
    float aspect = pc.res[0] / pc.res[1];
    
    float deltaY = f * -ndc.y;
    float deltaX = f * aspect * ndc.x;
    
    vec3 newPos = vec3(deltaX, deltaY, -1.0);
    newPos = normalize(newPos);
    
    mat3 mat = rotX(-pc.camera[0]) * rotY(-pc.camera[1]);
    rayDir = mat * newPos;
    
    return rayDir;
}

void main(){
    outColor = vec4(getRayDirection() * 0.5 + 0.5, 1.0);
}