#version 120
uniform sampler2D texture;
uniform vec3 color;
//varying vec2 texcoord;

void main(void) {
    //vec3 tex = texture2D(texture, texcoord).rgb;
    gl_FragColor = vec4(color,1);
}