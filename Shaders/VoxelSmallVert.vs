#version 150
// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1
in vec3 in_Position;
in vec3 in_Color;
in vec3 in_Normal;

struct PLight{
    vec4 position;
    vec4 color;
    float intensity;
    float range;
    float align3;
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

// We output the ex_Color variable to the next shader in the chain
out vec3 ex_Color;
out vec3 ex_Position;
out vec3 ex_Normal;
out float depth;
out vec3 pointLightDir[MAX_POINT_LIGHTS];

float round(float f){
    return fract(f)>=0.5? ceil(f):floor(f);
}

void main(void) {
    vec3 rotPos = (in_Position - centerPos)/2 * rotation;
    float px = rotPos.x;
    float py = rotPos.y;
    float pz = rotPos.z;

    vec4 pixelPos = vec4( ((px + round(objPos.x)) - (py + round(objPos.y)))*2 + round(-camPos.x) + 0.375,
                          ((px + round(objPos.x)) + (py + round(objPos.y))) + (pz + round(objPos.z) + camPos.z )*2 + round(-camPos.y)+ 0.375,
                          (pz + objPos.z)-(py+px + objPos.y+objPos.x)/2, 1);

    gl_Position = projection * pixelPos;
    
    vec3 globalPos = vec3(px + objPos.x,py + objPos.y,pz + objPos.z);

    ex_Position = globalPos;
    ex_Color = in_Color;
    ex_Normal = normalize(in_Normal) * rotation;

    for(int i=0;i<MAX_POINT_LIGHTS;i++){
        pointLightDir[i] = normalize(lights[i].position.xyz - globalPos);
    }

    if(in_Position.x<0 || globalPos.z>256)
        depth = -1.0;
    else
        depth = (pz + objPos.z)/256.0;
}