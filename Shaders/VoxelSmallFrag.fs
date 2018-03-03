#version 150
// It was expressed that some drivers required this next line to function properly
precision highp float;

in  vec3 ex_Color;
in  vec3 ex_Normal;
in float depth;
in vec3 pointLightCol;
in float pointLightDist;
in vec3 pointLightDir;
in vec2 v_uv;

out vec4 gl_FragColor;

void main(void) {
    if(depth<0)
        discard;

    vec3 sunDir = normalize(vec3(-0.75,-0.2,1.5));
    vec3 normal = ex_Normal;

    vec3 ambientAndSun = vec3(0.14,0.1,0.2) + max(0,dot(sunDir,normal))* vec3(1,1,1);
    vec3 pointLighting = max(0,step(0.325,max(0,dot(normal,pointLightDir))*(pointLightDist)))*0.75*pointLightCol;

    gl_FragColor = vec4((ambientAndSun + pointLighting) * ex_Color,1.0);
    gl_FragColor.a = depth;
}