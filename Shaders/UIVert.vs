#version 330
in vec3 in_Position;
in vec2 in_Coordinates;

uniform sampler2D texture;
uniform vec3 color;
uniform mat4 projection;
out vec2 uv;

void main(void) {
   gl_Position = projection * vec4(in_Position, 1.0);
   uv = (in_Coordinates);
}