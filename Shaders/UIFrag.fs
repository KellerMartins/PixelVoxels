#version 330
uniform sampler2D texture;
uniform vec3 color;
in vec2 uv;

void main(void) {
    vec4 tex = texture2D(texture, uv).rgba;
    gl_FragColor = vec4(color,tex.a);
}