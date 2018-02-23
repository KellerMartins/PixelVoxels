#version 150
// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1
in vec3 in_Position;
in vec3 in_Color;
uniform int spriteScale;
uniform vec3 camPos;
uniform vec3 objPos;
uniform vec3 centerPos;
uniform mat4 projection;
uniform mat3 rotation;

// We output the ex_Color variable to the next shader in the chain
out vec3 ex_Color;
out vec3 ex_Position;
out float depth;
out vec3 pointLightCol;
out float pointLightDist;
out vec3 pointLightDir;

float round(float f){
    return fract(f)>=0.5? ceil(f):floor(f);
}

void main(void) {
    vec3 rotPos = (in_Position+centerPos) * rotation;
    float px = rotPos.x;
    float py = rotPos.y;
    float pz = rotPos.z;

    vec4 pixelPos = vec4( floor(((px + objPos.x) - (py + objPos.y))*spriteScale*2 + round(-camPos.x) + 0.375),
                          floor(((px + objPos.x) + (py + objPos.y))*spriteScale + (pz + objPos.z + camPos.z )*spriteScale*2 + round(-camPos.y) + 0.375),
                          (pz + objPos.z)-(py+px + objPos.y+objPos.x)/2 , 1);

    gl_Position = projection * pixelPos;
    
    vec3 globalPos = vec3(px + objPos.x,py + objPos.y,pz + objPos.z);
    ex_Color = in_Color;

    pointLightCol =  vec3(1,1,1);
    pointLightDist = 200.01/pow(distance(globalPos,vec3(-10,-10,30)),2);
    pointLightDir = normalize(vec3(-10,-10,30)-globalPos);

    if(in_Position.x<0 || globalPos.z>256)
        depth = -1.0;
    else
        depth = (pz + objPos.z)/256.0;
}