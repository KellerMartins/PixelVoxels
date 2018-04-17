#version 120
in vec3 in_Position;

uniform sampler2D texture;
uniform vec3 color;
uniform mat4 projection;
//varying vec2 texcoord;

void main(void) {
   gl_Position = projection * vec4(in_Position, 1.0);
   //texcoord = (v_coord + 1.0)/2.0;
}