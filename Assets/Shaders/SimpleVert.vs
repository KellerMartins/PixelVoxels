#version 330
in vec2 in_Position;
in vec2 in_Coordinates;

uniform mat4 projection;
uniform sampler2D texture;

out vec2 uv;

void main(void) {
   gl_Position = projection * vec4(in_Position,0.0, 1.0);
   uv = in_Coordinates;
}