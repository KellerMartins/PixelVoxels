#version 150

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
in float outlineHeight;
in  vec3 pointLighting;
in  vec2 v_uv;

out vec4 gl_FragColor;

uniform vec3 sunDir;
uniform vec3 sunColor;
uniform sampler2D shadowDepth;

vec3 tonemap(vec3 x)
{
    //Reinhard
    return x / (x + 1.0);

    //ACESFilm
    //return clamp((x*(2.51*x+0.03))/(x*(2.43*x+0.59)+0.14), 0.0, 1.0);

    //Uncharted 2
    //return ((x*(0.15*x+0.10*0.50)+0.20*0.02)/(x*(0.15*x+0.50)+0.20*0.30))-0.02/0.30;
}

void main(void) {

    float shadowCalc = ((shadowCoords.z)/2 + 0.5);
    
    vec2 coords = shadowCoords.st/2 + vec2(0.5);
    const float spread = 0.0035;
    float sub = shadowCalc*5;
    sub -= texture2D(shadowDepth, coords).x;
    sub -= texture2D(shadowDepth, coords + vec2( spread,     0.0)).x;
    sub -= texture2D(shadowDepth, coords + vec2(    0.0,  spread)).x;
    sub -= texture2D(shadowDepth, coords + vec2(    0.0, -spread)).x;
    sub -= texture2D(shadowDepth, coords + vec2(-spread,     0.0)).x;
    sub /= 5.0;

    float shadow = max(0.4,1-smoothstep(0.015,0.1,sub));

    vec3 ambientAndSun = vec3(0.025) + max(0,dot(normalize(sunDir*-1),ex_Normal))* sunColor * shadow;

    float k = 1.0;
    float expos = 1.2;
    gl_FragColor.rgb = tonemap( (log2(1.0-ex_Color)/-k) * (ambientAndSun + pointLighting) * expos);
    gl_FragColor.a = outlineHeight;
}
