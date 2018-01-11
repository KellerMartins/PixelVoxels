#version 150
// It was expressed that some drivers required this next line to function properly
precision highp float;

in  vec3 ex_Color;
in float depth;
in vec2 v_uv;
out vec4 gl_FragColor;

uniform sampler2D tex;

void main(void) {
    // Pass through our original color with full opacity.
    gl_FragColor = texture2D(tex, gl_PointCoord) * vec4(ex_Color,1.0);
    if(gl_FragColor.a < 0.5)
        discard;

    gl_FragColor.a = depth;
}