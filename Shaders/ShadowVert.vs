#version 330
in vec3 in_Position;

uniform int scale;
uniform vec3 camPos;
uniform vec3 objPos;
uniform vec3 centerPos;
uniform mat3 rotation;

uniform mat4 projection;
uniform mat4 view;

float round(float f){
    return fract(f)>=0.5? ceil(f):floor(f);
}

void main(void) {
    vec3 pos = ((in_Position - centerPos)/scale * rotation + objPos - vec3(0.0,64.0,0))*0.01;
    gl_Position = view * vec4(pos,1);
}