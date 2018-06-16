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

in  vec3 ex_Color;
in  vec3 ex_Position;
in float depth;
in vec3 pointLightCol;
in float pointLightDist;
in vec3 pointLightDir;
in vec2 v_uv;

out vec4 gl_FragColor;

uniform sampler2D tex;

vec3 hueShift( vec3 color, float hueAdjust ){

    const vec3  kRGBToYPrime = vec3 (0.299, 0.587, 0.114);
    const vec3  kRGBToI      = vec3 (0.596, -0.275, -0.321);
    const vec3  kRGBToQ      = vec3 (0.212, -0.523, 0.311);

    const vec3  kYIQToR     = vec3 (1.0, 0.956, 0.621);
    const vec3  kYIQToG     = vec3 (1.0, -0.272, -0.647);
    const vec3  kYIQToB     = vec3 (1.0, -1.107, 1.704);

    float   YPrime  = dot (color, kRGBToYPrime);
    float   I       = dot (color, kRGBToI);
    float   Q       = dot (color, kRGBToQ);
    float   hue     = atan (Q, I);
    float   chroma  = sqrt (I * I + Q * Q);

    hue += hueAdjust;

    Q = chroma * sin (hue);
    I = chroma * cos (hue);

    vec3    yIQ   = vec3 (YPrime, I, Q);

    return vec3( dot (yIQ, kYIQToR), dot (yIQ, kYIQToG), dot (yIQ, kYIQToB) );

}

void main(void) {
    if(depth<0)
        discard;

    vec3 sunDir = normalize(vec3(-0.75,-0.2,1.5));
    vec3 normal = normalize(texture2D(tex, gl_PointCoord).rgb * vec3(-1,-1,1));

    float alpha = texture2D(tex, gl_PointCoord).a;
    if(alpha < 0.5)
        discard;

    vec3 ambientAndSun = vec3(0.04,0,0.1) + max(0,dot(sunDir,normal)) * vec3(1,1,1);

    vec3 pointLighting = vec3(0);
    for(int i=0;i<MAX_POINT_LIGHTS;i++){
        if(lights[i].range<=0) continue;
        
        vec3 pointLightDir = normalize(lights[i].position.xyz - ex_Position);
        float pointLightDist = lights[i].range / pow( distance(ex_Position,lights[i].position.xyz) ,2);
        
        float pointLightLighting = max(0,dot(normal,pointLightDir))*(pointLightDist);
        pointLighting += step(0.6,pointLightLighting) *lights[i].intensity* lights[i].color.rgb;
        pointLighting += (step(0.5,pointLightLighting)-step(0.6,pointLightLighting)) *lights[i].intensity* hueShift(lights[i].color.rgb,lights[i].shift/4.0)*0.5;
        pointLighting += (step(0.4,pointLightLighting)-step(0.5,pointLightLighting)) *lights[i].intensity* hueShift(lights[i].color.rgb,lights[i].shift) * 0.1;
    }

    gl_FragColor = vec4((ambientAndSun + pointLighting) * ex_Color,1.0);
    gl_FragColor.a = depth;
}