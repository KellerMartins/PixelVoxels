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
in  vec3 ex_LocalPosition;

in float outlineHeight;
in vec2 v_uv;

out vec4 gl_FragColor;

uniform int spriteScale;
uniform vec3 camPos;
uniform vec3 objPos;
uniform vec3 centerPos;
uniform mat4 projection;
uniform mat3 rotation;

uniform mat4 shadowMatrix;

uniform vec3 sunDir;
uniform vec3 sunColor;
uniform sampler2D shadowDepth;
uniform sampler2D tex;
uniform sampler2D texPos;

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

    vec4 sprite = texture(tex, gl_PointCoord);
    vec4 spritePos = texture(texPos, gl_PointCoord) * vec4(-1,-1,1,1);
    vec3 normal = normalize(sprite.rgb * vec3(-1,-1,1));

    if(sprite.a < 0.5)
        discard;

    vec3 shadowPos = (((ex_LocalPosition) - centerPos) * rotation + objPos + spritePos.xyz);
    vec4 shadowCoords = shadowMatrix * vec4(shadowPos,1) + vec4(0,0,0.5,0);;
    float shadowCalc = ((shadowCoords.z)/2 + 0.5);
    vec2 coords = shadowCoords.st/2 + vec2(0.5);

    const float spread = 0.0035;
    float sub = shadowCalc*9;
    sub -= texture2D(shadowDepth, coords).x;
    sub -= texture2D(shadowDepth, coords + vec2( spread, -spread)).x;
    sub -= texture2D(shadowDepth, coords + vec2( spread,     0.0)).x;
    sub -= texture2D(shadowDepth, coords + vec2( spread,  spread)).x;
    sub -= texture2D(shadowDepth, coords + vec2(    0.0,  spread)).x;
    sub -= texture2D(shadowDepth, coords + vec2(    0.0, -spread)).x;
    sub -= texture2D(shadowDepth, coords + vec2(-spread,  spread)).x;
    sub -= texture2D(shadowDepth, coords + vec2(-spread,     0.0)).x;
    sub -= texture2D(shadowDepth, coords + vec2(-spread, -spread)).x;
    sub /= 9.0;

    float shadow = max(0.4,pow(1-smoothstep(0.015,0.1,sub),2.0));

    vec3 ambientAndSun = vec3(0.025) + max(0.0,dot(normalize(sunDir*-1),normal)) * sunColor * shadow;

    vec3 pointLighting = vec3(0);
    for(int i=0;i<MAX_POINT_LIGHTS;i++){
        if(lights[i].range<=0) continue;
        
        vec3 pointLightDir = normalize(lights[i].position.xyz - (ex_Position + spritePos.xyz));
        float pointLightDist = lights[i].range / pow( distance((ex_Position + spritePos.xyz),lights[i].position.xyz) ,2);
        
        float pointLightLighting = max(0,dot(normal,pointLightDir))*(pointLightDist);
        pointLighting += step(0.6,pointLightLighting) *lights[i].intensity* lights[i].color.rgb;
        pointLighting += (step(0.5,pointLightLighting)-step(0.6,pointLightLighting)) *lights[i].intensity* hueShift(lights[i].color.rgb,lights[i].shift/4.0)*0.5;
        pointLighting += (step(0.4,pointLightLighting)-step(0.5,pointLightLighting)) *lights[i].intensity* hueShift(lights[i].color.rgb,lights[i].shift) * 0.1;
    }
    float details = dot(normal, normalize(vec3(-1,-1,1)))>0.75? 1.2:1.0 * length(ex_Normal)>1? 1.1:1.0;

    //Based on this: http://www.codersnotes.com/notes/untonemapping/
    float k = 1.0;
    float expos = 1.1;
    gl_FragColor.rgb = tonemap( (log2(1.0-ex_Color)/-k) * (ambientAndSun + pointLighting) * details * expos);
    gl_FragColor.a = outlineHeight;
    gl_FragDepth = gl_FragCoord.z - (spritePos.x/4 + spritePos.y/4 + spritePos.z)/1000.0;
}