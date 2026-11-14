#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragPos;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Consts{
    int res[2];
    float spin[2];
    mat4 matrix;
    vec4 bodyPosRadius;
    vec4 sunPosEmissive;
} pc;

layout(set = 0, binding = 0) uniform sampler2D starMap;

vec2 uvFromDirection(vec3 d) {
    return vec2(0.5 + atan(d.z, d.x) / (2.0 * 3.1415926535), 0.5 - asin(d.y) / 3.1415926535);
}

mat3 rotX(float a) { float c = cos(a), s = sin(a); return mat3(1,0,0,  0,c,s,  0,-s,c); }
mat3 rotY(float a) { float c = cos(a), s = sin(a); return mat3(c,0,-s, 0,1,0,  s,0,c); }

void main(){
    if(pc.sunPosEmissive.w > 1.5) {
        outColor = vec4(texture(starMap, uvFromDirection(normalize(fragPos))).rgb * 0.5, 1.0);
        return;
    }

    vec3 normal = normalize(fragNormal);
    mat3 planetRot = rotX(pc.spin[0]) * rotY(pc.spin[1]);
    vec3 surfaceDir = transpose(planetRot) * normal;
    vec2 uv = uvFromDirection(surfaceDir);
    
    vec3 sunCol = vec3(1.0, 0.9, 0.7);
    if(pc.sunPosEmissive.w > 0.5){
        
        outColor = vec4(texture(starMap, vec2(1.0 - uv.x, uv.y)).rgb, 1.0);;
        return;
    }
    
    
    vec4 texCol = vec4(texture(starMap, vec2(1.0 - uv.x, uv.y)).rgb, 1.0);
    
    vec3 baseCol = vec3(0.5, 0.5, 0.5) * texCol.rgb;
    vec3 shadowCol = vec3(0, 0, 0);
    
    
    vec3 toSun = normalize(pc.sunPosEmissive.xyz - fragPos);
    float t = max(dot(normal, toSun), 0.0);
    outColor = vec4(shadowCol + baseCol * sunCol * t, 1.0);
}