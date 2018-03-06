#version 150
// It was expressed that some drivers required this next line to function properly
precision highp float;

struct PLight{
    vec4 position;
    vec4 color;
    float intensity;
    float range;
    float align3;
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
        if(lights[i].intensity<=0) continue;
        
        vec3 pointLightDir = normalize(lights[i].position.xyz - ex_Position);
        float pointLightDist = lights[i].range / pow( distance(ex_Position,lights[i].position.xyz) ,2);
        
        float pointLightLighting = max(0,dot(normal,pointLightDir))*(pointLightDist);
        pointLighting += step(0.6,pointLightLighting) *lights[i].intensity* lights[i].color.rgb * 0.333;
        pointLighting += step(0.5,pointLightLighting) *lights[i].intensity* lights[i].color.rgb * 0.333;
        pointLighting += step(0.4,pointLightLighting) *lights[i].intensity* lights[i].color.rgb * 0.333;
    }

    gl_FragColor = vec4((ambientAndSun + pointLighting) * ex_Color,1.0);
    gl_FragColor.a = depth;
}