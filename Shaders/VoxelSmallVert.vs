#version 150

in vec3 in_Position;
in vec3 in_Color;
in vec3 in_Normal;

struct PLight{
    vec4 position;
    vec4 color;
    float shift;
    float intensity;
    float range;
    float align4;
};
const int MAX_POINT_LIGHTS = 10;
layout (std140) uniform PointLight {
    PLight lights[MAX_POINT_LIGHTS];
};

uniform vec3 camPos;
uniform vec3 objPos;
uniform vec3 centerPos;
uniform mat4 projection;
uniform mat3 rotation;

uniform mat4 shadowView;

out vec3 ex_Color;
out vec3 ex_Position;
out vec3 ex_Normal;

out vec4 shadowCoords;
out float depth;
out vec3 pointLighting;

float round(float f){
    return fract(f)>=0.5? ceil(f):floor(f);
}

void main(void) {
    vec3 rotPos = (in_Position - centerPos)/2 * rotation;
    float px = rotPos.x;
    float py = rotPos.y;
    float pz = rotPos.z;

    vec4 pixelPos = vec4( ((px + objPos.x) - (py + objPos.y))*2 + round(-camPos.x) + 0.375,
                          ((px + objPos.x) + (py + objPos.y)) + (pz + objPos.z + camPos.z )*2 + round(-camPos.y)+ 0.375,
                          (pz + objPos.z)-(py+px + objPos.y+objPos.x)/2, 1);

    gl_Position = projection * pixelPos;
    vec3 globalPos = vec3(px + objPos.x,py + objPos.y,pz + objPos.z);

    ex_Position = globalPos;
    ex_Color = in_Color;
    ex_Normal = normalize(in_Normal) * rotation;

    depth = (pz + objPos.z)/256.0;

    //Calculate shadow projected coords
    vec3 shadowPos = ((in_Position - centerPos)/2 * rotation + objPos);
    shadowCoords = shadowView * vec4(shadowPos,1)+vec4(0,0,0.2,0);
    

    //Calculate point lighting
    pointLighting = vec3(0.0);
    for(int i=0;i<MAX_POINT_LIGHTS;i++){
        if(lights[i].range<=0) continue;
        
        vec3 pointLightDir = normalize(lights[i].position.xyz - globalPos);
        float pointLightDist = 0.5*lights[i].range / pow( distance(globalPos,lights[i].position.xyz) ,2);

        pointLighting += max(0,step(0.325,max(0,dot(ex_Normal,pointLightDir))*(pointLightDist))) *lights[i].intensity* lights[i].color.rgb;
    }

    //Discard vertexes
    if(in_Position.x<0 || ex_Position.z<0 || ex_Position.z>256 || (length(in_Normal) == 1 && dot(normalize(vec3(1,1,-1)), ex_Normal) > 0.5)){
        gl_Position = vec4(-100, -100, -100, 1);
    }
    
}