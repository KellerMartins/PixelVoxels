#version 330
uniform sampler2D fbo_texture;
uniform float pWidth;
uniform float pHeight;

uniform float vignettePower;
uniform float redShiftPower;
uniform float redShiftSpread;

in vec2 uv;

vec4 when_gt(vec4 x, vec4 y) {
   return max(sign(x - y), 0.0);
}

void main(void) {
    float curDepth = texture2D(fbo_texture, uv).a;
    vec3 pixel = texture2D(fbo_texture, uv).rgb;

    //Outline

    const float ObjOutline = 0.0;
    const float UIOutline = 0.1;
    
    vec4 neighbor = texture2D(fbo_texture, vec2(uv.x + pWidth,uv.y));
    float borderMult = neighbor.a<0.9? ObjOutline : UIOutline;
    pixel = (neighbor.a - curDepth) > 0.01? neighbor.rgb*borderMult:pixel;

    neighbor = texture2D(fbo_texture, vec2(uv.x - pWidth,uv.y));
    borderMult = neighbor.a<0.9? ObjOutline : UIOutline;
    pixel = (neighbor.a - curDepth) > 0.01? neighbor.rgb*borderMult:pixel;

    neighbor = texture2D(fbo_texture, vec2(uv.x,uv.y + pHeight));
    borderMult = neighbor.a<0.9? ObjOutline : UIOutline;
    pixel = (neighbor.a - curDepth) > 0.01? neighbor.rgb*borderMult:pixel;

    neighbor = texture2D(fbo_texture, vec2(uv.x,uv.y - pHeight));
    borderMult = neighbor.a<0.9? ObjOutline : UIOutline;
    pixel = (neighbor.a - curDepth) > 0.01? neighbor.rgb*borderMult:pixel;

    //Vignette
    vec3 dist = vec3((uv.x - 0.5) * 1.25,(uv.y - 0.5f) * 1.25,0);
    float vignette = clamp(1 - dot(dist, dist)*vignettePower,0,1);

    //Red Shift
    float redShift;
    if(redShiftSpread>0){
        float aberrationMask = clamp(dot(dist, dist)*redShiftPower,0,1)*redShiftSpread;
        redShift = texture2D(fbo_texture, vec2(uv.x - aberrationMask*pWidth,uv.y)).r;
    }else{
        redShift = pixel.r;
    }

    gl_FragColor = vec4(vec3(redShift,pixel.gb)*vignette,1);
}