#version 150
// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1
in vec3 in_Position;
in vec3 in_Color;
uniform vec3 camPos;
uniform vec3 objPos;
uniform vec3 centerPos;
uniform mat4 projection;
uniform mat3 rotation;

// We output the ex_Color variable to the next shader in the chain
out vec3 ex_Color;
out vec3 ex_Position;
out float depth;
out vec3 pointLight;
out vec3 pointLightDir;

void main(void) {
    vec3 rotPos = (in_Position-centerPos) * rotation;
    rotPos += centerPos;
    float px = rotPos.x;
    float py = rotPos.y;
    float pz = rotPos.z;

    vec4 pixelPos = vec4( ((px + objPos.x) - (py + objPos.y))*2 + floor(-camPos.x) + 0.375,
                          ((px + objPos.x) + (py + objPos.y)) + (pz + objPos.z + camPos.z )*2 + floor(-camPos.y) + 0.375,
                          (pz-(py+px)/126.0 + objPos.z) , 1);

    gl_Position = projection * pixelPos;

    // GLSL allows shorthand use of vectors too, the following is also valid:
    // gl_Position = vec4(in_Position, 0.0, 1.0);
    // We're simply passing the color through unmodified
    vec3 globalPos = vec3(px + objPos.x,py + objPos.y,pz + objPos.z);
    ex_Color = in_Color;

    pointLight = 200.01/pow(distance(globalPos,vec3(-10,-10,30)),2) * vec3(1,1,1);
    pointLightDir = normalize(vec3(-10,-10,30)-globalPos);

    depth = in_Position.x<0? -1.0 : (pz + objPos.z)/256.0;
}