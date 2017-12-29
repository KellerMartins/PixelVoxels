uniform sampler2D fbo_texture;
uniform float pWidth;
uniform float pHeight;
varying vec2 f_texcoord;

uniform float vignettePower;
uniform float redShiftPower;
uniform float redShiftSpread;

vec4 when_gt(vec4 x, vec4 y) {
   return max(sign(x - y), 0.0);
}

void main(void) {
    float curDepth = texture2D(fbo_texture, f_texcoord).a;
    vec3 pixel = texture2D(fbo_texture, f_texcoord).rgb;

    //Outline
    vec4 neighbor = texture2D(fbo_texture, vec2(f_texcoord.x + pWidth,f_texcoord.y));
    pixel = (neighbor.a - curDepth) > 0.01? neighbor.rgb*0.54:pixel;
    neighbor = texture2D(fbo_texture, vec2(f_texcoord.x - pWidth,f_texcoord.y));
    pixel = (neighbor.a - curDepth) > 0.01? neighbor.rgb*0.54:pixel;
    neighbor = texture2D(fbo_texture, vec2(f_texcoord.x,f_texcoord.y + pHeight));
    pixel = (neighbor.a - curDepth) > 0.01? neighbor.rgb*0.54:pixel;
    neighbor = texture2D(fbo_texture, vec2(f_texcoord.x,f_texcoord.y - pHeight));
    pixel = (neighbor.a - curDepth) > 0.01? neighbor.rgb*0.54:pixel;

    //Vignette
    vec3 dist = vec3((f_texcoord.x - 0.5f) * 1.25f,(f_texcoord.y - 0.5f) * 1.25f,0);
    float vignette = clamp(1 - dot(dist, dist)*vignettePower,0,1);

    //Red Shift
    float redShift;
    if(redShiftSpread>0){
        float aberrationMask = clamp(dot(dist, dist)*redShiftPower,0,1)*redShiftSpread;
        redShift = texture2D(fbo_texture, vec2(f_texcoord.x - aberrationMask*pWidth,f_texcoord.y));
    }else{
        redShift = pixel.r;
    }

    gl_FragColor = vec4(vec3(redShift,pixel.gb)*vignette,1);
}