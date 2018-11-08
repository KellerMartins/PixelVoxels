#version 330
uniform sampler2D texture;

in vec2 uv;

void main(void) {
    gl_FragColor = vec4(texture2D(texture, uv).rgb,1);
}