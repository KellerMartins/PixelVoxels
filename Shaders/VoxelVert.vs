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

uniform mat4 shadowMatrix;

out vec3 ex_Color;
out vec3 ex_Position;
out vec3 ex_Normal;
out vec3 ex_LocalPosition;

out float outlineHeight;

float round(float f){
    return fract(f)>=0.5? ceil(f):floor(f);
}

void main(void) {
    ex_LocalPosition = in_Position;
    vec3 localPos = (in_Position - centerPos) * rotation;
    float px = round(localPos.x + objPos.x)*spriteScale;
    float py = round(localPos.y + objPos.y)*spriteScale;
    float pz = round(localPos.z + objPos.z)*spriteScale;

    vec4 pixelPos = vec4( (px - py)*2 + round(-camPos.x) + 0.375,
                          (px + py)+ (pz + camPos.z)*2 + round(-camPos.y) + 0.375,
                          (localPos.z + objPos.z)-(localPos.y+localPos.x + objPos.y+objPos.x)/2 , 1);

    gl_Position = projection * pixelPos;

    vec3 globalPos = vec3(localPos.x + objPos.x,
                          localPos.y + objPos.y,
                          localPos.z + objPos.z);

    ex_Position = globalPos;
    ex_Color = in_Color;
    ex_Normal = in_Normal * rotation;

    outlineHeight = (pz + objPos.z)/256.0;

    //Discart vertex
    if(in_Position.x<0 || ex_Position.z<0 || ex_Position.z>256 || (length(ex_Normal) == 1 && dot(normalize(vec3(1,1,-1)), normalize(ex_Normal)) > 0.0)){
        gl_Position = vec4(-1000, -1000, -1000, 1);
    }
}