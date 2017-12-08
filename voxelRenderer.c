#include "voxelRenderer.h"

unsigned int voxColors[256] = {
    0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff, 0xff00ffff, 0xffffccff, 0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff, 0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff,
	0xff6699ff, 0xff3399ff, 0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff, 0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff, 0xff0033ff, 0xffff00ff,
	0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff, 0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc, 0xff66ffcc, 0xff33ffcc, 0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
	0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc, 0xff0099cc, 0xffff66cc, 0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc, 0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc,
	0xff6633cc, 0xff3333cc, 0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc, 0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99, 0xff00ff99, 0xffffcc99,
	0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99, 0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999, 0xff669999, 0xff339999, 0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
	0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399, 0xff003399, 0xffff0099, 0xffcc0099, 0xff990099, 0xff660099, 0xff330099, 0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66,
	0xff66ff66, 0xff33ff66, 0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66, 0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966, 0xff009966, 0xffff6666,
	0xffcc6666, 0xff996666, 0xff666666, 0xff336666, 0xff006666, 0xffff3366, 0xffcc3366, 0xff993366, 0xff663366, 0xff333366, 0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
	0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33, 0xff00ff33, 0xffffcc33, 0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33, 0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933,
	0xff669933, 0xff339933, 0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633, 0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333, 0xff003333, 0xffff0033,
	0xffcc0033, 0xff990033, 0xff660033, 0xff330033, 0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00, 0xff66ff00, 0xff33ff00, 0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
	0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900, 0xff009900, 0xffff6600, 0xffcc6600, 0xff996600, 0xff666600, 0xff336600, 0xff006600, 0xffff3300, 0xffcc3300, 0xff993300,
	0xff663300, 0xff333300, 0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000, 0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077, 0xff000055, 0xff000044,
	0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00, 0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700, 0xff005500, 0xff004400, 0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
	0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000, 0xffeeeeee, 0xffdddddd, 0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777, 0xff555555, 0xff444444, 0xff222222, 0xff111111
};

const GLchar *vShaderSource = 
"    attribute vec2 v_coord;"
"    uniform sampler2D fbo_texture;"
"    uniform float pWidth;"
"    uniform float pHeight;"
"    varying vec2 f_texcoord;"

"    void main(void) {"
"       gl_Position = vec4(v_coord, 0.0, 1.0);"
"       f_texcoord = (v_coord + 1.0)/2.0;"
"    }"
;

const GLchar *fShaderSource = 
"    uniform sampler2D fbo_texture;"
"    uniform float pWidth;"
"    uniform float pHeight;"
"    varying vec2 f_texcoord;"

"    vec4 when_gt(vec4 x, vec4 y) {"
"       return max(sign(x - y), 0.0);"
"    }"

"    void main(void) {"
"       float curDepth = texture2D(fbo_texture, f_texcoord).a;"
"       vec3 outlineColor = texture2D(fbo_texture, f_texcoord).rgb;"

"       vec4 neighbor = texture2D(fbo_texture, vec2(f_texcoord.x + pWidth,f_texcoord.y));"
"       outlineColor = (neighbor.a - curDepth) > 0.01? neighbor.rgb*0.54:outlineColor;"

"       neighbor = texture2D(fbo_texture, vec2(f_texcoord.x - pWidth,f_texcoord.y));"
"       outlineColor = (neighbor.a - curDepth) > 0.01? neighbor.rgb*0.54:outlineColor;"

"       neighbor = texture2D(fbo_texture, vec2(f_texcoord.x,f_texcoord.y + pHeight));"
"       outlineColor = (neighbor.a - curDepth) > 0.01? neighbor.rgb*0.54:outlineColor;"

"       neighbor = texture2D(fbo_texture, vec2(f_texcoord.x,f_texcoord.y - pHeight));"
"       outlineColor = (neighbor.a - curDepth) > 0.01? neighbor.rgb*0.54:outlineColor;"

"       gl_FragColor = vec4(outlineColor,1);"
"    }"
;

extern SDL_Renderer * renderer;
extern int GAME_SCREEN_WIDTH;
extern int GAME_SCREEN_HEIGHT;

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern double deltaTime;

Vector3 cameraPosition;

GLuint CubeID;
GLuint FramebufferName = 0;
GLuint renderedTexture = 0;
GLuint depthrenderbuffer = 0;

GLuint program = 0;

void MoveCamera(float x, float y, float z){
    cameraPosition.x +=x*deltaTime;
    cameraPosition.y +=y*deltaTime;
    cameraPosition.z +=z*deltaTime;
    //printf("CamPos: |%2.1f|%2.1f|%2.1f|\n",cameraPosition.x,cameraPosition.y,cameraPosition.z);
}

void InitRenderer(Uint16 *dpth){
    cameraPosition = (Vector3){-GAME_SCREEN_WIDTH/2,0,0};

    //Framebuffer
    glGenFramebuffers(1, &FramebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

    //Render Texture
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return;

    glBindFramebuffer(GL_FRAMEBUFFER,0);

    //Load surface into a OpenGL texture
    SDL_Surface *cubeimg = IMG_Load("Textures/cube.png");
    if(!cubeimg){ printf("Failed to load!\n"); return; }

    
    glGenTextures(1, &CubeID);
    glBindTexture(GL_TEXTURE_2D, CubeID);
    
    int Mode = GL_RGB;
    
    if(cubeimg->format->BytesPerPixel == 4) {
        Mode = GL_RGBA;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, Mode, cubeimg->w, cubeimg->h, 0, Mode, GL_UNSIGNED_BYTE, cubeimg->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    SDL_FreeSurface(cubeimg);

    //Compile shader
    if(!CompileAndLinkShader()) printf(">Failed to compile/link shader! Description above\n\n");
    else printf(">Compiled/linked shader sucessfully!\n\n");
}

void UpdateScreenPointer(Pixel* scrn){
    //screen = scrn;
}

void FreeRenderer(){
    //glDeleteTextures(0,&CubeID​);
 }

 void ClearRender(SDL_Color col){
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    //glViewport(0,0,GAME_SCREEN_WIDTH,GAME_SCREEN_HEIGHT);

    glClearColor(col.r/255.0, col.g/255.0, col.b/255.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
}

void RenderToScreen(){

    glEnable(GL_TEXTURE_2D);

    glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);
    GLdouble loc = glGetUniformLocation(program, "pWidth");
    if (loc != -1) glUniform1f(loc, 1.0/(float)GAME_SCREEN_WIDTH);
    //else printf("QWERQRQWEQWE");

    loc = glGetUniformLocation(program, "pHeight");
    if (loc != -1) glUniform1f(loc, 1.0/(float)GAME_SCREEN_HEIGHT);
    //else printf("ADASDASDASD");

    
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0,1); glVertex2f(-SCREEN_WIDTH/2,  SCREEN_HEIGHT/2);
        glTexCoord2f(1,1); glVertex2f( SCREEN_WIDTH/2,  SCREEN_HEIGHT/2);
        glTexCoord2f(1,0); glVertex2f( SCREEN_WIDTH/2, -SCREEN_HEIGHT/2);
        glTexCoord2f(0,0); glVertex2f(-SCREEN_WIDTH/2, -SCREEN_HEIGHT/2);
    }
    glEnd();

    glDisable( GL_TEXTURE_2D );
    glUseProgram(0);
}

void *RenderThread(void *arguments){
    RendererArguments *args = arguments;

	VoxelObject **objs = args->objs;
	unsigned int numObjs = args->numObjs;
	VoxelObject **shadowCasters = args->shadowCasters;
	unsigned int numCasters = args->numCasters;
    int i,endLoop = 0;
    pthread_t tID;

    for(i=0; i<numObjs; i++){
        if(objs[i]->enabled){
            if(objs[i]->maxDimension >= 100 && numObjs-(i+1)>0){
				RendererArguments renderArguments = {objs+(i+1),numObjs-(i+1),shadowCasters,numCasters};
				pthread_create(&tID, NULL, &RenderThread, (void *)&renderArguments);
                endLoop = 1;
            }
            if(objs[i]->modificationStartZ >=0){
                CalculateRendered(objs[i]);
                CalculateLighting(objs[i]);

                objs[i]->modificationStartX = -1;
                objs[i]->modificationEndX = -1;

                objs[i]->modificationStartY = -1;
                objs[i]->modificationEndY = -1;

                objs[i]->modificationStartZ = -1;
                objs[i]->modificationEndZ = -1;

            }
            RenderObject(objs[i]);
            if(shadowCasters!=NULL){
                for(int j=0;j<numCasters;j++){
                    CalculateShadow(objs[i],shadowCasters[j]);
                }
            }
            if(endLoop){
                pthread_join(tID, NULL);
                break;
            }
        }
    }
    return NULL;
}

void RenderObject(VoxelObject *obj){

    //unsigned int color = 0;

    int x,y,z,i,px,py,zp,startz,nv,colorIndex,edgeIndx,lightIndx,useRot = 0;
    int halfDimX = obj->dimension[0]/2.0, halfDimY = obj->dimension[1]/2.0,halfDimZ = obj->dimension[2]/2.0;
    float rx,ry,rz;
    float sinx = 1,cosx = 0;
    float siny = 1,cosy = 0;
    float sinz = 1,cosz = 0;
    //Termos que multiplicam as posicoes na rotacao
    float rxt1 = 1, rxt2 = 1, rxt3 = 1;
    float ryt1 = 1, ryt2 = 1, ryt3 = 1;
    float rzt1 = 1, rzt2 = 1;

    if(obj->rotation.x != 0.0f || obj->rotation.y != 0.0f || obj->rotation.z != 0.0f){
        useRot = 1;
        sinx = sin(obj->rotation.x * PI_OVER_180);
        cosx = cos(obj->rotation.x * PI_OVER_180);

        siny = sin(obj->rotation.y * PI_OVER_180);
        cosy = cos(obj->rotation.y * PI_OVER_180);
        
        sinz = sin(obj->rotation.z * PI_OVER_180);
        cosz = cos(obj->rotation.z * PI_OVER_180);

        //Pre calculo dos termos
        rxt1 = cosy*cosz; rxt2 = (cosz*sinx*siny - cosx*sinz); rxt3 = (cosx*cosz*siny + sinx*sinz);
        ryt1 = cosy*sinz; ryt2 = (cosx*siny*sinz - cosz*sinx); ryt3 = (cosx*cosz + sinx*siny*sinz);
        rzt1 = cosx*cosy; rzt2 = sinx*cosy;
    }
    const float edge = 0.80;
    const float base = 0.75;
    const float crease = 0.70;
    const float sunlight = 1.42;
    const float shadow = 0.79;

    double illuminFrac;
    float lightVal,edgeVal,heightVal;
    float r,g,b;
    startz = (obj->dimension[2]-1);

    //Checagem se fora da tela
    //> Desabilitada por enquanto, não utilizo elementos fora da tela ainda
    //if( /*Esquerda*/ ((obj->maxDimension+obj->position.x)-(obj->position.y))*2 + roundf(-cameraPosition.x) < 0 ||
    //    /*Direita*/  ((obj->position.x)-(obj->maxDimension+obj->position.y))*2 + roundf(-cameraPosition.x) > GAME_SCREEN_WIDTH ||
    //    /*Acima*/    ((obj->maxDimension+obj->position.x)+(obj->maxDimension+obj->position.y)) + roundf(-cameraPosition.y) < 0 ||
    //    /*Abaixo*/   ((obj->position.x)+(obj->position.y)) -(obj->maxDimension*2) + roundf(-cameraPosition.y) > GAME_SCREEN_HEIGHT
    //){
    //    return;
    //}
    
    //Configure OpenGL parameters to render point sprites

    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    glViewport(0,0,GAME_SCREEN_WIDTH,GAME_SCREEN_HEIGHT);

    glBindTexture(GL_TEXTURE_2D, CubeID);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_DEPTH_TEST);
    glAlphaFunc (GL_NOTEQUAL, 0.0f);
    glEnable(GL_ALPHA_TEST);
    glPointSize(5);
    glEnable(GL_POINT_SPRITE);
    glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

    //Define projection matrices
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,GAME_SCREEN_WIDTH, 0,GAME_SCREEN_HEIGHT, -500,500);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /*glBegin(GL_POINTS);
        for(x=obj->dimension[0]-1;x>0;x--){
            for(y=obj->dimension[1]-1;y>0;y--){
                for(z=0;z<obj->dimension[2];z++){
                    int index = (x + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                    if(obj->model[index]){
                        int py = ((x+obj->position.x)+(y+obj->position.y)) +((z+obj->position.z+roundf(cameraPosition.z))*2) + roundf(-cameraPosition.y);
                        int px = ((x+obj->position.x)-(y+obj->position.y))*2 + roundf(-cameraPosition.x);
                        glVertex3f(px+0.375, py+0.375, (z+obj->position.z)/10);
                    }
                }
            }
        }
    glEnd();*/

    glBegin(GL_POINTS);
    for(z=startz; z>=0; z--){
        heightVal = clamp(((1.0+(((z+obj->position.z+cameraPosition.z)*0.5))/128)),0,1.4);

        nv = obj->render[z][0];
        for(i = nv; i >0 ; i--){
            
            x = (obj->render[z][i] & 127);
            y = ((obj->render[z][i]>>7) & 127);

            colorIndex = (x) + ((z)) * obj->maxDimension + (y) * obj->maxDimension * obj->maxDimension;
            unsigned color = voxColors[obj->model[colorIndex]];

            //Aplica rotação na posição do voxel
            if(useRot){
                x -= halfDimX;
                y -= halfDimY;
                z -= halfDimZ;

                rx = x*rxt1 + y*rxt2 + z*rxt3;
                ry = x*ryt1 + z*ryt2 + y*ryt3;
                rz = z*rzt1 + y*rzt2 - x*siny;

                rx += halfDimX;
                ry += halfDimY;
                rz += halfDimZ;

                z += halfDimZ;
            }else{
                rx = x;
                ry = y;
                rz = z;
            }
            //rx = roundf(rx);
            //ry = roundf(ry);
            //rz = roundf(rz);

            //Clipping do objeto quando fora da faixa de 0 a 255
            zp = rz + roundf(obj->position.z+cameraPosition.z);
            if(zp<0 || zp>255) continue;
            
            //Obtém o nivel de iluminação do voxel e multiplica pela sombra dinâmica
            lightIndx = (obj->lighting[colorIndex] & 6)>>1;
            lightIndx *=obj->lighting[colorIndex] & 1;
            //Reseta a sombra dinâmica para 1 (sem sombra), para ser recalculada no prox frame 
            obj->lighting[colorIndex] |= 1;

            //Adiciona iluminação leve nas bordas
            edgeIndx = obj->lighting[colorIndex]>>3;
            lightVal = lightIndx == 1? 1:(lightIndx >= 2? sunlight:shadow);
            edgeVal = (edgeIndx<5? edge:edgeIndx == 5? base:crease);

            //Multiplica iluminações e já coloca a conversão da cor de (0,256) para (0,1)
            illuminFrac = lightVal * edgeVal * heightVal * ONE_OVER_256;

            //Transforma a cor de um Int16 para cada um dos componentes RGB
            r = clamp((color & 255)*illuminFrac,0,1);
            color = (color>>8);
            g = clamp((color & 255)*illuminFrac,0,1);
            color = (color>>8);
            b = clamp((color & 255)*illuminFrac,0,1);

            //Projeção das posições de 3 dimensões para duas na tela
            py = ((rx+obj->position.x)+(ry+obj->position.y)) +(zp*2) + roundf(-cameraPosition.y);
            px = ((rx+obj->position.x)-(ry+obj->position.y))*2 + roundf(-cameraPosition.x);

            glColor4f(r, g, b, (rz + obj->position.z)/256.0);
            glVertex3f(px+0.375, py+0.375, (rz-(ry+rx)/126 + obj->position.z));
        }
    }
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnd();

    glDisable(GL_POINT_SPRITE);

    glDisable( GL_TEXTURE_2D );
    
}

void CalculateRendered(VoxelObject *obj){
    if(obj->modificationStartZ <0 || obj->modificationEndZ <0 ){
        return;
    }
    int x,y,z,index,dir,occ,occPixel = 0,occUp,occLeft,occDown;
    for(z = obj->modificationStartZ; z<=obj->modificationEndZ ;z++){
        obj->render[z][0]=0;
        for(y = obj->dimension[1]-1; y>=0; y--){
            for(x = obj->dimension[0]-1; x>=0; x--){
                occ = 0;
                occUp   = 0;
                occLeft = 0;
                occDown = 0;

                index = (x + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                if(obj->model[index]!=0){
                    if(x!=0 && x<obj->maxDimension-1 && y!=0 && y<obj->maxDimension-1 && z!=0 && z<obj->maxDimension-1){
                        dir = (x + (z+1) * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);//0 0 1
                        if(obj->model[dir]!=0){
                            occ++; 
                            occUp=1;
                        }
                        dir = (x + (z-1) * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);//0 0 -1
                        if(obj->model[dir]!=0){
                            occ++;
                            occDown = 1;
                        }
                        dir = (x + z * obj->maxDimension + (y+1) * obj->maxDimension * obj->maxDimension);//0 1 0
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = (x + z * obj->maxDimension + (y-1) * obj->maxDimension * obj->maxDimension);//0 -1 0 
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ( (x+1) + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);//1 0 0
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ( (x-1) + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);//1 0 0 
                        if(obj->model[dir]!=0){
                            occ++;
                            occLeft = 1;
                        }
                    }
                    if(occ!=6){
                        if((occLeft && occDown && occUp) || y == obj->dimension[1]-1){
                            occPixel = 0;
                        }else{
                            if(!occLeft && !occDown && occUp){
                                occPixel = 1;
                            }
                            if(!occLeft && occDown && occUp){
                                occPixel = 1;
                            }
                            if(!occDown && !occUp && x>0){
                                occPixel = 2;
                            }
                            if(occLeft && !occDown){
                                occPixel = 2;
                            }
                        }
                        obj->render[z][0]++;
                        obj->render[z][(int)obj->render[z][0]] = (unsigned short int)((occPixel<<14) | ( y << 7) | x);
                    }
                }
            }
        }
    }
}


void CalculateLighting(VoxelObject *obj){

    int y,x,z,index,dir;
    int occlusion,lightAir,lightBlock;

    int zstart = obj->dimension[2]-1;
    int lightFinal;
    for(y=obj->modificationStartY; y<obj->modificationEndY; y++){
        for(x=obj->modificationStartX; x<obj->modificationEndX; x++){

            //Define a luz no topo do objeto, que é transportado para baixo a cada iteração em z
            lightAir = 1;
            lightBlock = 1;

            for(z=zstart; z!=0; z--){
                occlusion = 0;
                index = (x + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                
                if(obj->model[index]!=0){
                    if(z<obj->dimension[2]-1){ //Up
                        dir = (x + (z+1) * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                        occlusion += obj->model[dir]==0? 0:1;
                        //Ilumina o bloco caso o bloco acima seja vazio (com luz ou sombra), se não, mantém a cor
                        lightBlock = obj->model[dir]==0? lightAir*2:1;
                    }else{
                        lightBlock = 2;
                        occlusion = 3;
                    }
                    if(z>0){ //Down
                        dir = (x + (z-1) * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    if(x>0){ //Left
                        dir = ((x-1) + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                        occlusion += obj->model[dir]==0? 0:1;
                    }else{
                        occlusion = 2;
                    }
                    if(x<obj->dimension[0]-1){ //Right
                        dir = ((x+1) + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    if(y<obj->dimension[1]-1){ //Front
                        dir = (x + z * obj->maxDimension + (y+1) * obj->maxDimension * obj->maxDimension);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    if(y>0){ //Back
                        dir = (x + z * obj->maxDimension + (y-1) * obj->maxDimension * obj->maxDimension);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    lightFinal = lightBlock;
                }else{
                    if(z+1<obj->maxDimension){
                        dir = (x + (z+1) * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                        if(obj->model[dir]!=0){
                            lightAir = 0;
                        }
                    }
                    lightFinal = lightAir;
                }

                //lighting 8bits  [1-Empty] [3-Occlusion][2-Direct Light(2), Ambient(1) and self shadow(0)] [1-Shadow from caster]
                obj->lighting[index] = (unsigned char)( (((occlusion & 255)<<3)|((lightFinal & 3)<<1)) | (obj->lighting[index] & 1) );
            }
        }
    }
    /*for(y=obj->modificationStartY; y<obj->modificationEndY; y++){
        for(x=obj->modificationStartX; x<obj->modificationEndX; x++){
            for(z=zstart; z>=0; z--){
                index = (x + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                if(obj->model[index]==0){
                    PointLight(obj,x,y,z,3);
                }
            }
        }
    }*/
}

/*void PointLight(VoxelObject *obj,int x, int y, int z,int radius){
    int px,py,pz;

    px = x;
    py = y; 
    pz = z;

    int startx,endx,starty,endy,startz,endz;
    int ix,iy,iz,index,total = 0;

    startx = px-radius <0? 0:px-radius;
    starty = py-radius <0? 0:py-radius;
    startz = pz-radius <0? 0:pz-radius;

    endx = px+radius>obj->maxDimension? obj->maxDimension : px+radius;
    endy = py+radius>obj->maxDimension? obj->maxDimension : py+radius;
    endz = pz+radius>obj->maxDimension? obj->maxDimension : pz+radius;

    for(ix = startx;ix<endx;ix++){
        for(iy = starty;iy<endy;iy++){
            for(iz = startz;iz<endz;iz++){
                int randRadius = radius+0;//(rand() % 3);
                if( ((ix-px)*(ix-px))+((iy-py)*(iy-py))+((iz-pz)*(iz-pz)) <= (randRadius*randRadius)){
                    total++;
                    index = (ix) + (iz) * obj->maxDimension + (iy) * obj->maxDimension * obj->maxDimension;

                    //lighting => 8bits  [2-Empty] [3-Occlusion][2-Point Light(3) Direct Light(2), Ambient(1) and self shadow(0)] [1-Shadow from caster]
                    obj->lighting[index] = (unsigned char)(obj->lighting[index] | 0b00000110);
                }
            }
        }   
    }
}*/

void CalculateShadow(VoxelObject *obj,VoxelObject *shadowCaster){

    int y,x,z,o,index,dir,cx,cy,cz,useRot = 0;

    int halfDimX = shadowCaster->dimension[0]/2.0, halfDimY = shadowCaster->dimension[1]/2.0,halfDimZ = shadowCaster->dimension[2]/2.0;
    float rx,ry,rz;
    float sinx = 1,cosx = 0;
    float siny = 1,cosy = 0;
    float sinz = 1,cosz = 0;
    //Revisitar essa parte, projeção inverte em certos ângulos
    if(shadowCaster->rotation.x != 0.0f || shadowCaster->rotation.y != 0.0f || shadowCaster->rotation.z != 0.0f){
        useRot = 1;
        sinx = -sin(shadowCaster->rotation.x * PI_OVER_180);
        cosx = cos(shadowCaster->rotation.x * PI_OVER_180);

        siny = -sin(shadowCaster->rotation.y * PI_OVER_180);
        cosy = cos(shadowCaster->rotation.y * PI_OVER_180);
        
        sinz = -sin(shadowCaster->rotation.z * PI_OVER_180);
        cosz = cos(shadowCaster->rotation.z * PI_OVER_180);
    }

    
    int startx = shadowCaster->position.x-obj->position.x - shadowCaster->dimension[0]*0.2f;
    int starty = shadowCaster->position.y-obj->position.y - shadowCaster->dimension[1]*0.2f;
    int startz = (shadowCaster->position.z-obj->position.z)+shadowCaster->dimension[2];

    int endx = startx + shadowCaster->dimension[0]*1.5f;
    int endy = starty + shadowCaster->dimension[1]*1.5f;
    if(endx<0 || endy<0 ){
        return;
    }

    startx = startx <0? 0:startx;
    starty = starty <0? 0:starty;
    startz = startz <0? 0:startz;

    startz = startz >obj->maxDimension? obj->maxDimension:startz;
    endx = endx>obj->dimension[0]? obj->dimension[0] : endx;
    endy = endy>obj->dimension[1]? obj->dimension[1] : endy;
        
    int shadowVal,finalShadow = 1;
    for(y=starty; y<endy; y++){
        for(x=startx; x<endx; x++){

            shadowVal = 1;

            for(z=startz; z!=0; z--){
                index = (x + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                if(obj->model[index]==0){
                    if(shadowCaster->enabled == 0){
                        continue;
                    }
                    cx = x-shadowCaster->position.x+obj->position.x;
                    cy = y-shadowCaster->position.y+obj->position.y;
                    cz = z-shadowCaster->position.z+obj->position.z;

                    if(useRot==1){
                        cx -= halfDimX;
                        cy -= halfDimY;
                        cz -= halfDimZ;
        
                        rx = cx*cosy*cosz + cy*(cosz*sinx*siny - cosx*sinz) + cz*(cosx*cosz*siny + sinx*sinz);
                        ry = cx*cosy*sinz + cz*(cosx*siny*sinz - cosz*sinx) + cy*(cosx*cosz + sinx*siny*sinz);
                        rz = cz*cosx*cosy + cy*sinx*cosy - cx*siny;                      
                        
                        cx = rx + halfDimX;
                        cy = ry + halfDimY;
                        cz = rz + halfDimZ;
                    }
                    //cx = roundf(cx);
                    //cy = roundf(cy);
                    //cz = roundf(cz);

                    if(cx>-1 && cx<shadowCaster->maxDimension && cy>-1 && cy<shadowCaster->maxDimension && cz>-1 && cz<shadowCaster->maxDimension){
                        o = (cx + cz * shadowCaster->maxDimension + cy * shadowCaster->maxDimension * shadowCaster->maxDimension);
                        if(shadowCaster->model[o]!=0){
                            shadowVal = 0;
                        }
                    }
                    finalShadow = shadowVal;
                }else{
                    if(z<obj->dimension[2]-1){ //Up
                        dir = (x + (z+1) * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                        finalShadow = obj->model[dir]==0? shadowVal:1;
                    }
                }
                //lighting => 8bits  [1-Empty] [3-Occlusion][2-Direct Light(2), Ambient(1) and self shadow(0)] [1-Shadow from caster]
                obj->lighting[index] = (unsigned char)(obj->lighting[index] & (0b11111110  | (finalShadow&1)));
            }
        }
    }
}

SDL_Texture* RenderIcon(VoxelObject *obj){

    int iconWidth = obj->dimension[0];
    int iconHeight = obj->dimension[2];
    printf("\n%d %d %d\n",obj->dimension[0],obj->dimension[1],obj->dimension[2]);

    SDL_Texture *icon = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, iconWidth, iconHeight);
    
    Pixel *iconPixels;
	int pitch = iconWidth * sizeof(Pixel);
    Uint16 *iconDepth = (Uint16*) calloc(GAME_SCREEN_HEIGHT*GAME_SCREEN_WIDTH,sizeof(Uint16));

    SDL_LockTexture(icon, NULL, (void**)&iconPixels, &pitch);

    unsigned int color = 0;

    int x,y,z,i,px,py,startz,nv,cp = 0,colorIndex;
    Uint16 voxeld;
    int r,g,b;

    for(i=0;i<iconWidth*iconHeight;i++){
        iconDepth[i] = 0;
        iconPixels[i] = (Pixel){0,0,0,0};
    }

    startz = (obj->dimension[2]-1);
    
    for(z=startz; z>=0; z--){

        nv = obj->render[z][0];
        for(i = 1; i <= nv ; i++){
            
            x = (obj->render[z][i] & 127);
            y = ((obj->render[z][i]>>7) & 127);

            colorIndex = (x) + ((z)) * obj->maxDimension + (y) * obj->maxDimension * obj->maxDimension;
            color = voxColors[obj->model[colorIndex]];

            //Transforma a cor de um Int16 para cada um dos componentes RGB
            voxeld = (y*2 +obj->dimension[1])*2;
            r = clamp((color & 255),0,255);
            color = (color>>8);
            g = clamp((color & 255),0,255);
            color = (color>>8);
            b = clamp((color & 255),0,255);

            //Projeção das posições de 3 dimensões para duas na tela
            py = startz - z;
            px = x;

            cp = py*iconWidth + px;

            if(voxeld > iconDepth[cp]){                  
                iconPixels[cp].r = r;
                iconPixels[cp].g = g;
                iconPixels[cp].b = b;
                iconPixels[cp].a = 255;
                iconDepth[cp] = voxeld;
            }
        }
    }
    const float shadow = 0.75;
    const float edge = 1.2;
    const float occlusion = 0.75;

    int j;
    for(j=0;j<iconWidth;j++){
        int occluded = 0;
        int valOcludee = 0;
        for(i=1;i<iconHeight;i++){
            if(occluded){
                occluded = valOcludee <= iconDepth[i*iconWidth + j] ? 0:1;
                if(!occluded){
                    valOcludee = iconDepth[i*iconWidth + j];
                }else{
                    //Long shadow
                    iconPixels[i*iconWidth + j].r *= shadow;
                    iconPixels[i*iconWidth + j].g *= shadow;
                    iconPixels[i*iconWidth + j].b *= shadow;
                }
            }else{
                occluded = iconDepth[(i-1)*iconWidth + j] > iconDepth[i*iconWidth + j] ? 1:0;
                if(occluded){
                    //Long shadow
                    iconPixels[i*iconWidth + j].r *= shadow;
                    iconPixels[i*iconWidth + j].g *= shadow;
                    iconPixels[i*iconWidth + j].b *= shadow;
                    valOcludee = iconDepth[(i-1)*iconWidth + j];
                }else{
                    //Edge light
                    if(iconDepth[(i-1)*iconWidth + j] != iconDepth[i*iconWidth + j]){
                        iconPixels[i*iconWidth + j].r = clamp(iconPixels[i*iconWidth + j].r*edge,0,255);
                        iconPixels[i*iconWidth + j].g = clamp(iconPixels[i*iconWidth + j].g*edge,0,255);
                        iconPixels[i*iconWidth + j].b = clamp(iconPixels[i*iconWidth + j].b*edge,0,255);
                    }
                }
            }
            if(occluded){
                //Occlusion
                if(j!=0 && iconDepth[i*iconWidth + j-1] > iconDepth[i*iconWidth + j]){
                    iconPixels[i*iconWidth + j].r *= occlusion;
                    iconPixels[i*iconWidth + j].g *= occlusion;
                    iconPixels[i*iconWidth + j].b *= occlusion;
                }
                if(j!=iconWidth-1 && iconDepth[i*iconWidth + j+1] > iconDepth[i*iconWidth + j]){
                    iconPixels[i*iconWidth + j].r *= occlusion;
                    iconPixels[i*iconWidth + j].g *= occlusion;
                    iconPixels[i*iconWidth + j].b *= occlusion;
                }
                if(i!=iconHeight-1 && iconDepth[(i+1)*iconWidth + j] > iconDepth[i*iconWidth + j]){
                    iconPixels[i*iconWidth + j].r *= occlusion;
                    iconPixels[i*iconWidth + j].g *= occlusion;
                    iconPixels[i*iconWidth + j].b *= occlusion;
                }
            }
        }
    }

    SDL_UnlockTexture(icon);

    free(iconDepth);
    return icon;
}

void SaveTextureToPNG(SDL_Texture *tex, char* out){
    //Get texture dimensions
    int w, h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);

    SDL_Surface *sshot = SDL_CreateRGBSurface(0, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    SDL_Texture *target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);

    //Copy texture to render target
    SDL_SetRenderTarget(renderer, target);
    SDL_Rect rect = {0,0,w,h};
    SDL_RenderCopy(renderer, tex, NULL,&rect);

    //Transfer render target pixels to surface
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
    //Save surface to PNG
    IMG_SavePNG(sshot, out);

    //Return render target to default
    SDL_SetRenderTarget(renderer, NULL);
    
    //Free allocated surface and texture
    SDL_FreeSurface(sshot);
    SDL_DestroyTexture(target);

}

void RenderText(char *text, SDL_Color color, int x, int y, TTF_Font* font) 
{

    SDL_Surface * sFont = TTF_RenderText_Blended_Wrapped(font, text, color,500);
    if(!sFont){printf("Failed to render text!\n"); return;}

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0, SCREEN_WIDTH,0,SCREEN_HEIGHT,-1,1); 
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sFont->w, sFont->h, 0, GL_BGRA, 
                    GL_UNSIGNED_BYTE, sFont->pixels);

    
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0,1); glVertex2f(x, y);
        glTexCoord2f(1,1); glVertex2f(x + sFont->w, y);
        glTexCoord2f(1,0); glVertex2f(x + sFont->w, y + sFont->h);
        glTexCoord2f(0,0); glVertex2f(x, y + sFont->h);
    }
    glEnd();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(sFont);
}

int CompileAndLinkShader(){
    //Create an empty vertex shader handle
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    //Send the vertex shader source code to GL
    glShaderSource(vertexShader, 1, &vShaderSource, 0);

    //Compile the vertex shader
    glCompileShader(vertexShader);

    GLint isCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        GLchar *infoLog = (GLchar *) malloc(maxLength * sizeof(GLchar));
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
        
        //We don't need the shader anymore.
        glDeleteShader(vertexShader);

        printf("Vertex Shader Info Log:\n%s\n",infoLog);
        
        free(infoLog);
        
        //In this simple program, we'll just leave
        return 0;
    }

    //Create an empty fragment shader handle
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    //Send the fragment shader source code to GL
    glShaderSource(fragmentShader, 1, &fShaderSource, 0);

    //Compile the fragment shader
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        GLchar *infoLog = (GLchar *) malloc(maxLength * sizeof(GLchar));
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
        
        //We don't need the shader anymore.
        glDeleteShader(fragmentShader);
        //Either of them. Don't leak shaders.
        glDeleteShader(vertexShader);

        printf("Fragment Shader Info Log:\n%s\n",infoLog);
        
        free(infoLog);
        
        //In this simple program, we'll just leave
        return 0;
    }

    //Vertex and fragment shaders are successfully compiled.
    //Now time to link them together into a program.
    //Get a program object.
    program = glCreateProgram();

    //Attach our shaders to our program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    //Link our program
    glLinkProgram(program);

    //Note the different functions here: glGetProgram* instead of glGetShader*.
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
    if(isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        GLchar *infoLog = (GLchar *) malloc(maxLength * sizeof(GLchar));
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
        
        //We don't need the program anymore.
        glDeleteProgram(program);
        //Don't leak shaders either.
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        printf("Shader linkage Info Log:\n%s\n",infoLog);
        
        free(infoLog);
        //In this simple program, we'll just leave
        return 0;
    }

    //Always detach shaders after a successful link.
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);

    return 1;
}