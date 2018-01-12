#version 150
// It was expressed that some drivers required this next line to function properly
precision highp float;

in  vec3 ex_Color;
in float depth;
in vec3 pointLight;
in vec3 pointLightDir;
in vec2 v_uv;

out vec4 gl_FragColor;

uniform sampler2D tex;

void main(void) {
    if(depth<0)
        discard;

    vec3 sunDir = normalize(vec3(0.75,0.2,1.5));
    vec3 normal = normalize(texture2D(tex, gl_PointCoord).rgb);

    float alpha = texture2D(tex, gl_PointCoord).a;
    if(alpha < 0.5)
        discard;

    vec3 lighting = vec3(0.05,0,0.1) + max(0,dot(sunDir,normal)) * vec3(1,1,1) + max(0,dot(pointLightDir,normal))*pointLight;
    
    gl_FragColor = vec4(lighting * ex_Color,1.0);


    gl_FragColor.a = depth;
}