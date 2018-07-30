#version 150
// It was expressed that some drivers required this next line to function properly
precision highp float;

struct PLight{
    vec4 position;
    vec4 color;
    float shift;
    float intensity;
    float range;
    float align4;
};
const int MAX_POINT_LIGHTS = 10;
layout (std140) uniform PointLight {
    PLight lights[MAX_POINT_LIGHTS];
};

in  vec3 ex_Position;
in  vec3 ex_Color;
in  vec3 ex_Normal;

in vec4 shadowCoords;
in float depth;
in  vec3 pointLighting;
in  vec2 v_uv;

out vec4 gl_FragColor;

uniform sampler2D shadowDepth;

void main(void) {
    
    vec3 sunDir = normalize(vec3(-0.75,-0.2,1.5));

    float shadowCalc = ((shadowCoords.z)/2 + 0.5);
    float distanceFromLight = texture2D(shadowDepth,(shadowCoords.st)/2 + vec2(0.5) ).x;
    float shadow = max(0.5,1-smoothstep(0.08,0.1,shadowCalc-distanceFromLight));

    vec3 ambientAndSun = vec3(0,0,0) + max(0,dot(sunDir,ex_Normal))* vec3(1,1,1) * shadow;

    gl_FragColor.rgb = 1.0 - pow(1.0-ex_Color,(ambientAndSun + pointLighting));
    gl_FragColor.a = depth;
}
