#version 330
in vec3 in_Position;
in vec3 in_Normal;

uniform int scale;
uniform vec3 objPos;
uniform vec3 centerPos;
uniform mat3 rotation;

uniform vec3 sunDir;
uniform mat4 shadowMatrix;

float round(float f){
    return fract(f)>=0.5? ceil(f):floor(f);
}

void main(void) {
    vec3 pos = ((in_Position - centerPos)/scale * rotation + objPos);
    gl_Position = shadowMatrix * vec4(pos,1) + vec4(0,0,0.5,0);

    //Discart backface vertices
    if(dot(normalize(sunDir), normalize(in_Normal)*rotation) > 0.33){
        gl_Position = vec4(-1000, -1000, -1000, 1);
    }

}