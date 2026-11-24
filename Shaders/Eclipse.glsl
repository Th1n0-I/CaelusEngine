layout(set = 2, binding = 0) uniform occlusion{
    ivec4 info;
    vec4 sunPosInfo;
    vec4 occluders[28];
} occ;

const float pi = 3.14159265;

float getEclipseShadowAmount(vec3 pos){
    float minVisibility = 1;
    
    vec3 sunPos = occ.sunPosInfo.xyz;
    float sunRad = occ.sunPosInfo.w;
    
    float sunDist = distance(pos, sunPos);
    float sunAngSize = asin(sunRad / sunDist);
    
    for(int i = 0; i < occ.info.x; i++){
        vec3 occluderPos = occ.occluders[i].xyz;
        float occluderRad = occ.occluders[i].w;
        
        if(length(occluderPos - sunPos) > length(pos - sunPos)) continue;
        if(length(occluderPos - pos) > length(sunPos - pos)) continue;
        
        
        
        float occDist = distance(pos, occluderPos);
        if(length(normalize(occluderPos - pos) - normalize(sunPos - pos)) > (sunRad / sunDist + occluderRad / occDist) * 1.1) continue;
        if (occDist < occluderRad * 1.1) continue;
        
        float occAngSize = asin(occluderRad / occDist);
        
        vec3 a = normalize(sunPos - pos);
        vec3 b = normalize(occluderPos - pos);
        
        float angOfSep = atan(length(cross(a, b)), dot(a,b));
        
        if(occAngSize + sunAngSize <= angOfSep) continue; // No overlap
        if(sunAngSize + angOfSep <= occAngSize) return 0; // Fully blocked, no point checking more.
        if(occAngSize + angOfSep <= sunAngSize){
            float sunArea = sunAngSize * sunAngSize * pi;
            float occArea = occAngSize * occAngSize * pi;
            float visibility = (sunArea - occArea) / sunArea;
            minVisibility = min(visibility, minVisibility);
            continue;
        }
        
        float sunTheta = 2 * acos(clamp((sunAngSize * sunAngSize - occAngSize * occAngSize + angOfSep * angOfSep) / (2 * angOfSep * sunAngSize), -1, 1));
        float occTheta = 2 * acos(clamp((occAngSize * occAngSize - sunAngSize * sunAngSize + angOfSep * angOfSep) / (2 * angOfSep * occAngSize), -1, 1));
        
        float sunSegment = sunAngSize * sunAngSize * sunTheta / 2 - sunAngSize * sunAngSize * sin(sunTheta) / 2;
        float occSegment = occAngSize * occAngSize * occTheta / 2 - occAngSize * occAngSize * sin(occTheta) / 2;
        
        float totArea = sunSegment + occSegment;
        minVisibility = min(1 - totArea / (sunAngSize * sunAngSize * pi), minVisibility);
    }
    
    
    return minVisibility;
}