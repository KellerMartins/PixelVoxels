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

uniform int spriteScale;
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
out vec3 pointLightCol;
out float pointLightDist;
out vec3 pointLightDir[MAX_POINT_LIGHTS];

float round(float f){
    return fract(f)>=0.5? ceil(f):floor(f);
}

void main(void) {
    vec3 rotPos = (in_Position - centerPos) * rotation;
    float px = rotPos.x;
    float py = rotPos.y;
    float pz = rotPos.z;

    vec4 pixelPos = vec4( ((px + round(objPos.x)) - (py + round(objPos.y)))*spriteScale*2 + round(-camPos.x) + 0.375,
                          ((px + round(objPos.x)) + (py + round(objPos.y)))*spriteScale + (pz + round(objPos.z) + camPos.z )*spriteScale*2 + round(-camPos.y) + 0.375,
                          (pz + objPos.z)-(py+px + objPos.y+objPos.x)/2 , 1);

    gl_Position = projection * pixelPos;
    
    vec3 globalPos = vec3(px + objPos.x,py + objPos.y,pz + objPos.z);

    ex_Position = globalPos;
    ex_Color = in_Color;
    ex_Normal = in_Normal * rotation;

    vec3 shadowPos = ((in_Position - centerPos) * rotation + objPos)*0.005*vec3(1,1,0.5);
    shadowCoords = shadowView * vec4(shadowPos,1);

    for(int i=0;i<MAX_POINT_LIGHTS;i++){
        pointLightDir[i] = normalize(lights[i].position.xyz - globalPos);
    }

    if(in_Position.x<0 || globalPos.z>256)
        depth = -1.0;
    else
        depth = (pz + objPos.z)/256.0;
}