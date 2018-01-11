#include "voxelRenderer.h"

Pixel voxColors[256];

extern SDL_Renderer * renderer;
extern int GAME_SCREEN_WIDTH;
extern int GAME_SCREEN_HEIGHT;

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern double deltaTime;

Vector3 cameraPosition;

GLuint CubeID;
GLuint frameBuffer = 0;
GLuint renderedTexture = 0;
GLuint depthRenderBuffer = 0;
GLuint vao = 0, vbo[2] = {0,0};

GLuint Shaders[2] = {0,0};

void MoveCamera(float x, float y, float z){
    cameraPosition.x +=x*deltaTime;
    cameraPosition.y +=y*deltaTime;
    cameraPosition.z +=z*deltaTime;
    //printf("CamPos: |%2.1f|%2.1f|%2.1f|\n",cameraPosition.x,cameraPosition.y,cameraPosition.z);
}

void InitRenderer(){
    cameraPosition = (Vector3){0,0,0};

    //Framebuffer
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    //Render Texture
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return;

    glBindFramebuffer(GL_FRAMEBUFFER,0);

    // VAO Generation and binding
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Color and vertex VBO generation and binding
    glGenBuffers(2, vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

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

    //Compile shaders
    if(!CompileAndLinkShader("Shaders/ScreenVert.vs","Shaders/ScreenFrag.fs",0)) printf(">Failed to compile/link shader! Description above\n\n");
    else printf(">Compiled/linked shader sucessfully!\n\n");

     if(!CompileAndLinkShader("Shaders/VoxelVert.vs","Shaders/VoxelFrag.fs",1)) printf(">Failed to compile/link shader! Description above\n\n");
    else printf(">Compiled/linked shader sucessfully!\n\n");
    

    LoadPalette("Textures/magicaPalette.png");
}

void ReloadShaders(){
    int i;
    for(i=0;i<sizeof(Shaders) / sizeof(Shaders[0]); i++){
        glDeleteProgram(Shaders[i]);
    }

    if(!CompileAndLinkShader("Shaders/ScreenVert.vs","Shaders/ScreenFrag.fs",0)) 
        printf(">Failed to compile/link shader! Description above\n\n");
    else 
        printf(">Compiled/linked shader sucessfully!\n\n");

    if(!CompileAndLinkShader("Shaders/VoxelVert.vs","Shaders/VoxelFrag.fs",1)) 
        printf(">Failed to compile/link shader! Description above\n\n");
    else 
        printf(">Compiled/linked shader sucessfully!\n\n");
}

void FreeRenderer(){
    //glDeleteTextures(0,&CubeID​);
 }

 void ClearRender(SDL_Color col){
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
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

    glUseProgram(Shaders[0]);
    GLdouble loc = glGetUniformLocation(Shaders[0], "pWidth");
    if (loc != -1) glUniform1f(loc, 1.0/(float)GAME_SCREEN_WIDTH);
    loc = glGetUniformLocation(Shaders[0], "pHeight");
    if (loc != -1) glUniform1f(loc, 1.0/(float)GAME_SCREEN_HEIGHT);

    loc = glGetUniformLocation(Shaders[0], "vignettePower");
    if (loc != -1) glUniform1f(loc, 0.25);
    loc = glGetUniformLocation(Shaders[0], "redShiftPower");
    if (loc != -1) glUniform1f(loc, 2);    
    loc = glGetUniformLocation(Shaders[0], "redShiftSpread");
    if (loc != -1) glUniform1f(loc, 0);
    
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

void RenderObjectList(VoxelObjectList objs, VoxelObjectList shadowCasters){

    int i;
    for(i=0; i<objs.numberOfObjects; i++){
        if(objs.list[i]->enabled){

            if(objs.list[i]->modificationStartZ >=0){
                CalculateRendered(objs.list[i]);
                CalculateLighting(objs.list[i]);

                objs.list[i]->modificationStartX = -1;
                objs.list[i]->modificationEndX = -1;

                objs.list[i]->modificationStartY = -1;
                objs.list[i]->modificationEndY = -1;

                objs.list[i]->modificationStartZ = -1;
                objs.list[i]->modificationEndZ = -1;

            }
            RenderObject(objs.list[i]);
            //if(shadowCasters.list!=NULL){
                //for(int j=0;j<shadowCasters.numberOfObjects;j++){
                    //CalculateShadow(objs.list[i],shadowCasters.list[j]);
                //}
            //}
        }
    }
}

void RenderObject(VoxelObject *obj){
    
    //Configure OpenGL parameters to render point sprites

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0,0,GAME_SCREEN_WIDTH,GAME_SCREEN_HEIGHT);

    glBindTexture(GL_TEXTURE_2D, CubeID);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_DEPTH_TEST);
    glAlphaFunc (GL_NOTEQUAL, 0.0f);
    glPointSize(5);
    glEnable(GL_POINT_SPRITE);

    //Define matrices
    float right = GAME_SCREEN_WIDTH/2;
    float left = -GAME_SCREEN_WIDTH/2;
    float top = GAME_SCREEN_HEIGHT/2;
    float bottom = -GAME_SCREEN_HEIGHT/2;
    float near = -500;
    float far = 500;
    
    GLfloat ProjectionMatrix[4][4]={{2.0f/(right-left), 0                 , 0                , -(right + left)/(right - left) },
                                    {0                , 2.0f/(top-bottom) , 0                , -(top + bottom)/(top - bottom) },
                                    {0                , 0                 , -2.0f/(far-near) , -(far + near)/(far - near)     },
                                    {0                , 0                 , 0                ,   1                            }};

    float sinx = sin(obj->rotation.x * PI_OVER_180);
    float cosx = cos(obj->rotation.x * PI_OVER_180);
    float siny = sin(obj->rotation.y * PI_OVER_180);
    float cosy = cos(obj->rotation.y * PI_OVER_180);
    float sinz = sin(obj->rotation.z * PI_OVER_180);
    float cosz = cos(obj->rotation.z * PI_OVER_180);

    GLfloat RotationMatrix[3][3]={{cosy*cosz , cosz*sinx*siny - cosx*sinz , cosx*cosz*siny + sinx*sinz},
                                  {cosy*sinz , cosx*cosz + sinx*siny*sinz , cosx*siny*sinz - cosz*sinx},
                                  {-siny     , cosy*sinx                  , cosx*cosy                 }};


    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, obj->numberOfVertices * 3 * sizeof(GLfloat), obj->vertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, obj->numberOfVertices * 3 * sizeof(GLfloat), obj->vColors, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);

    glUseProgram(Shaders[1]);

    glUniformMatrix4fv(glGetUniformLocation(Shaders[1], "projection"), 1, GL_FALSE, &ProjectionMatrix[0]);
    glUniformMatrix3fv(glGetUniformLocation(Shaders[1], "rotation"), 1, GL_FALSE, &RotationMatrix[0]);
    glUniform3f(glGetUniformLocation(Shaders[1], "objPos"), obj->position.x, obj->position.y, obj->position.z);
    glUniform3f(glGetUniformLocation(Shaders[1], "centerPos"), obj->center.x, obj->center.y, obj->center.z);
    glUniform3f(glGetUniformLocation(Shaders[1], "camPos"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniform1i(glGetUniformLocation(Shaders[1], "tex"), 0);

    glDrawArrays(GL_POINTS, 0, obj->numberOfVertices);

    glUseProgram(0);
    glDisable(GL_POINT_SPRITE);
    glDisable(GL_DEPTH_TEST);
    
}

void CalculateRendered(VoxelObject *obj){
    if(obj->modificationStartZ <0 || obj->modificationEndZ <0 ){
        return;
    }

    List visibleVoxels = InitList(sizeof(Vector3));

    int x,y,z,i,index,dir,occ;
    int xLimitS = clamp(obj->modificationStartX-1 ,0,obj->dimension[0]-1);
    int xLimitE = clamp(obj->modificationEndX+1   ,0,obj->dimension[0]-1);
    int yLimitS = clamp(obj->modificationStartY-1 ,0,obj->dimension[1]-1);
    int yLimitE = clamp(obj->modificationEndY+1   ,0,obj->dimension[1]-1);
    int zLimitS = clamp(obj->modificationStartZ-1 ,0,obj->dimension[2]-1);
    int zLimitE = clamp(obj->modificationEndZ+1   ,0,obj->dimension[2]-1);

    for(i=0;i<obj->numberOfVertices*3;i+=3){
        Vector3 voxelPos = {roundf(obj->vertices[i]),roundf(obj->vertices[i+1]),roundf(obj->vertices[i+2])};

        if( (voxelPos.x<xLimitS || voxelPos.x>xLimitE || voxelPos.y<yLimitS || voxelPos.y>yLimitE || voxelPos.z<zLimitS || voxelPos.z>zLimitE))
            InsertListEnd(&visibleVoxels,(void*)&voxelPos);           
    }

    for(z = zLimitE; z>=zLimitS ;z--){
        for(y = yLimitE; y>=yLimitS; y--){
            for(x = xLimitS; x<=xLimitE; x++){
                occ = 0;

                index = (x + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                
                if(obj->model[index]!=0){
                    
                    if(x!=0 && x<obj->dimension[0]-1 && y!=0 && y<obj->dimension[1]-1 && z!=0 && z<obj->dimension[2]-1){
                        dir = (x + (z+1) * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);//0 0 1
                        if(obj->model[dir]!=0){
                            occ++; 
                        }
                        dir = (x + (z-1) * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);//0 0 -1
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = (x + z * obj->dimension[0] + (y+1) * obj->dimension[0] * obj->dimension[2]);//0 1 0
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = (x + z * obj->dimension[0] + (y-1) * obj->dimension[0] * obj->dimension[2]);//0 -1 0 
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ( (x+1) + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);//1 0 0
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ( (x-1) + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);//1 0 0 
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                    }
                    if(occ!=6){
                        Vector3 vPos = {x,y,z};
                        InsertListStart(&visibleVoxels,(void*)&vPos);
                    }
                }
            }
        }
    }

    free(obj->vertices);
    obj->vertices = malloc(GetLength(visibleVoxels) * 3 * sizeof(GLfloat));
    obj->numberOfVertices = GetLength(visibleVoxels);

    ListCellPointer current = visibleVoxels.first;
    i = 0;
    while(current){
        memcpy(&obj->vertices[i],GetElement(*current),sizeof(Vector3));
        i+=3;
        current = GetNextCell(current);
    }
    FreeList(&visibleVoxels);
}


void CalculateLighting(VoxelObject *obj){

    int y,x,z,index,dir;
    int occlusion,lightAir,lightBlock;

    int zstart = obj->dimension[2]-1;
    int lightFinal;
    for(y=obj->modificationStartY; y<=obj->modificationEndY; y++){
        for(x=obj->modificationStartX; x<=obj->modificationEndX; x++){

            //Define a luz no topo do objeto, que é transportado para baixo a cada iteração em z
            lightAir = 1;
            lightBlock = 1;

            for(z=zstart; z>=0; z--){
                occlusion = 0;
                index = (x + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                
                if(obj->model[index]!=0){
                    if(z<obj->dimension[2]-1){ //Up
                        dir = (x + (z+1) * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                        occlusion += obj->model[dir]==0? 0:1;
                        //Ilumina o bloco caso o bloco acima seja vazio (com luz ou sombra), se não, mantém a cor
                        lightBlock = obj->model[dir]==0? lightAir*2:1;
                    }else{
                        lightBlock = 2;
                        occlusion = 3;
                    }
                    if(z>0){ //Down
                        dir = (x + (z-1) * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    if(x>0){ //Left
                        dir = ((x-1) + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                        occlusion += obj->model[dir]==0? 0:1;
                    }else{
                        occlusion = 2;
                    }
                    if(x<obj->dimension[0]-1){ //Right
                        dir = ((x+1) + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    if(y<obj->dimension[1]-1){ //Front
                        dir = (x + z * obj->dimension[0] + (y+1) * obj->dimension[0] * obj->dimension[2]);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    if(y>0){ //Back
                        dir = (x + z * obj->dimension[0] + (y-1) * obj->dimension[0] * obj->dimension[2]);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    lightFinal = lightBlock;
                }else{
                    if(z+1<obj->dimension[2]){
                        dir = (x + (z+1) * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
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

    const float edge = 0.80;
    const float base = 0.75;
    const float crease = 0.70;
    const float sunlight = 1.42;
    const float shadow = 0.79;

    free(obj->vColors);
    obj->vColors = malloc(obj->numberOfVertices * 3 * sizeof(GLfloat));

    int i;
    for(i = 0; i <obj->numberOfVertices*3; i+=3){
        x = roundf(obj->vertices[i]);
        y = roundf(obj->vertices[i+1]);
        z = roundf(obj->vertices[i+2]);

        float heightVal = clamp(((1.0+(((z+obj->position.z+cameraPosition.z)*0.5))/128)),0,1.4);

        unsigned colorIndex = (x) + (z) * obj->dimension[0] + (y) * obj->dimension[0] * obj->dimension[2];
        Pixel color = voxColors[obj->model[colorIndex]];
        
        //Obtém o nivel de iluminação do voxel e multiplica pela sombra dinâmica
        int lightIndx = (obj->lighting[colorIndex] & 6)>>1;
        lightIndx *= obj->lighting[colorIndex] & 1;

        //Adiciona iluminação leve nas bordas
        int edgeIndx = obj->lighting[colorIndex]>>3;
        float lightVal = lightIndx == 1? 1:(lightIndx >= 2? sunlight:shadow);
        float edgeVal = (edgeIndx<5? edge:edgeIndx == 5? base:crease);

        //Multiplica iluminações e já coloca a conversão da cor de (0,256) para (0,1)
        double illuminFrac = lightVal * edgeVal * heightVal * ONE_OVER_256;

        //Transforma a cor de um Int16 para cada um dos componentes RGB
        obj->vColors[i] = clamp(color.r * illuminFrac,0,1);
        obj->vColors[i+1] = clamp(color.g * illuminFrac,0,1);
        obj->vColors[i+2] = clamp(color.b * illuminFrac,0,1);
    }

}


void CalculateShadow(VoxelObject *obj,VoxelObject *shadowCaster){

    int y,x,z,o,index,dir,cx,cy,cz,useRot = 0;

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

    startz = startz >obj->dimension[2]? obj->dimension[2]:startz;
    endx = endx>obj->dimension[0]? obj->dimension[0] : endx;
    endy = endy>obj->dimension[1]? obj->dimension[1] : endy;
        
    int shadowVal,finalShadow = 1;
    for(y=starty; y<endy; y++){
        for(x=startx; x<endx; x++){

            shadowVal = 1;

            for(z=startz; z!=0; z--){
                index = (x + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                if(obj->model[index]==0){
                    if(shadowCaster->enabled == 0){
                        continue;
                    }
                    cx = x-shadowCaster->position.x+obj->position.x;
                    cy = y-shadowCaster->position.y+obj->position.y;
                    cz = z-shadowCaster->position.z+obj->position.z;

                    if(useRot==1){
                        cx -= shadowCaster->center.x;
                        cy -= shadowCaster->center.y;
                        cz -= shadowCaster->center.z;
        
                        rx = cx*cosy*cosz + cy*(cosz*sinx*siny - cosx*sinz) + cz*(cosx*cosz*siny + sinx*sinz);
                        ry = cx*cosy*sinz + cz*(cosx*siny*sinz - cosz*sinx) + cy*(cosx*cosz + sinx*siny*sinz);
                        rz = cz*cosx*cosy + cy*sinx*cosy - cx*siny;                      
                        
                        cx = rx + shadowCaster->center.x;
                        cy = ry + shadowCaster->center.y;
                        cz = rz + shadowCaster->center.z;
                    }
                    cx = roundf(cx);
                    cy = roundf(cy);
                    cz = roundf(cz);

                    if(cx>-1 && cx<shadowCaster->dimension[0] && cy>-1 && cy<shadowCaster->dimension[1] && cz>-1 && cz<shadowCaster->dimension[2]){
                        o = (cx + cz * shadowCaster->dimension[0] + cy * shadowCaster->dimension[0] * shadowCaster->dimension[2]);
                        if(shadowCaster->model[o]!=0){
                            shadowVal = 0;
                        }
                    }
                    finalShadow = shadowVal;
                }else{
                    if(z<obj->dimension[2]-1){ //Up
                        dir = (x + (z+1) * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
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

    Pixel color;

    int x,y,z,i,px,py,cp = 0,colorIndex;
    Uint16 voxeld;
    int r,g,b;

    for(i=0;i<iconWidth*iconHeight;i++){
        iconDepth[i] = 0;
        iconPixels[i] = (Pixel){0,0,0,0};
    }

    for(i = 0; i <obj->numberOfVertices*3; i+=3){
        
        x = roundf(obj->vertices[i]);
        y = roundf(obj->vertices[i+1]);
        z = roundf(obj->vertices[i+2]);

        colorIndex = (x) + ((z)) * obj->dimension[0] + (y) * obj->dimension[0] * obj->dimension[2];
        color = voxColors[obj->model[colorIndex]];

        //Transforma a cor de um Int16 para cada um dos componentes RGB
        voxeld = (y*2 +obj->dimension[1])*2;
        r = clamp(color.r,0,255);
        g = clamp(color.g,0,255);
        b = clamp(color.b,0,255);

        //Projeção das posições de 3 dimensões para duas na tela
        py = (obj->dimension[2]-1) - z;
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

void LoadPalette(char path[]){
    SDL_Surface * palSurf = IMG_Load(path);
    if(!palSurf){ printf(">Error loading palette!\n"); return; }

    int i;
    Uint8 r,g,b,a;

    for(i=0;i<256;i++){
        Uint32 *sPix = (Uint32 *)(palSurf->pixels + i* palSurf->format->BytesPerPixel);

        SDL_GetRGBA(*sPix,palSurf->format,&r,&g,&b,&a);
        Pixel color = {clamp(b,0,255),clamp(g,0,255),clamp(r,0,255),a};
        voxColors[i+1] = color;
    }
    SDL_FreeSurface(palSurf);
    printf(">Loaded palette sucessfully!\n\n");
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

const GLchar *LoadShaderSource(char *filename) {
    if(!filename) return NULL;

    FILE *file = fopen(filename, "r");             // open 
    fseek(file, 0L, SEEK_END);                     // find the end
    size_t size = ftell(file);                     // get the size in bytes
    GLchar *shaderSource = calloc(1, size);        // allocate enough bytes
    rewind(file);                                  // go back to file beginning
    fread(shaderSource, size, sizeof(char), file); // read each char into ourblock
    fclose(file);                                  // close the stream

    return shaderSource;
}

int CompileAndLinkShader(char *vertPath, char *fragPath, unsigned shaderIndex){
    //Create an empty vertex shader handle
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vShaderSource = LoadShaderSource(vertPath);

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
        free((void*)vShaderSource);

        printf("Vertex Shader Info Log:\n%s\n",infoLog);
        
        free(infoLog);
        
        //In this simple program, we'll just leave
        return 0;
    }

    //Create an empty fragment shader handle
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fShaderSource = LoadShaderSource(fragPath);

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
        free((void*)fShaderSource);
        //Either of them. Don't leak shaders.
        glDeleteShader(vertexShader);
        free((void*)vShaderSource);

        printf("Fragment Shader Info Log:\n%s\n",infoLog);
        
        free(infoLog);
        
        //In this simple program, we'll just leave
        return 0;
    }

    //Vertex and fragment shaders are successfully compiled.
    //Now time to link them together into a program.
    //Get a program object.
    Shaders[shaderIndex] = glCreateProgram();

    //Attach our shaders to our program
    glAttachShader(Shaders[shaderIndex], vertexShader);
    glAttachShader(Shaders[shaderIndex], fragmentShader);

    //Link our program
    glLinkProgram(Shaders[shaderIndex]);

    //Note the different functions here: glGetProgram* instead of glGetShader*.
    GLint isLinked = 0;
    glGetProgramiv(Shaders[shaderIndex], GL_LINK_STATUS, (int *)&isLinked);
    if(isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(Shaders[shaderIndex], GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        GLchar *infoLog = (GLchar *) malloc(maxLength * sizeof(GLchar));
        glGetProgramInfoLog(Shaders[shaderIndex], maxLength, &maxLength, &infoLog[0]);
        
        //We don't need the program anymore.
        glDeleteProgram(Shaders[shaderIndex]);
        //Don't leak shaders either.
        glDeleteShader(vertexShader);
        free((void*)vShaderSource);
        glDeleteShader(fragmentShader);
        free((void*)fShaderSource);

        printf("Shader linkage Info Log:\n%s\n",infoLog);
        
        free(infoLog);
        //In this simple program, we'll just leave
        return 0;
    }

    //Always detach shaders after a successful link.
    glDetachShader(Shaders[shaderIndex], vertexShader);
    glDetachShader(Shaders[shaderIndex], fragmentShader);
    free((void*)vShaderSource);
    free((void*)fShaderSource);

    return 1;
}

void FreeObject(VoxelObject *obj){
    if(!obj->model) return;

    free(obj->model);
    free(obj->lighting);
    free(obj->vertices);
    free(obj->vColors);

    obj->model = NULL;
}

void FreeMultiObject(MultiVoxelObject *obj){
    FreeObjectList(&obj->objects);
}

VoxelObjectList InitializeObjectList(){
    VoxelObjectList list;
    list.list = NULL;
    list.numberOfObjects = 0;
    return list;
}
void FreeObjectList(VoxelObjectList *list){
    if(!list){printf("Cant free list: list is empty!\n"); return;}

    int i;
    for(i=0;i<list->numberOfObjects;i++){
       FreeObject(list->list[i]);
    }
    free(list->list);
}

void AddObjectInList(VoxelObjectList *dest, VoxelObject *obj){
    if(!dest->list){
        dest->list = malloc(sizeof(VoxelObject *));
        dest->list[0] = obj;
        dest->numberOfObjects = 1;
    }else{
        VoxelObject **newList = malloc( (dest->numberOfObjects+1) * sizeof(VoxelObject *));
        memcpy(newList,dest->list,dest->numberOfObjects*sizeof(VoxelObject *));

        free(dest->list);
        dest->list = newList;
        dest->list[dest->numberOfObjects++] = obj;
    }
}

//Combine multiple ObjectLists into one
//If dest is NULL, the resulting list contains all sources
//If it isnt, the resulting list contains the dest and all sources, and the original list of the dest is freed
void CombineObjectLists(VoxelObjectList *dest, int numberOfSources,...){

    if(!dest){printf("Error: destination list is NULL!\n"); return;}

    va_list args;
	
    //Counts the number of objects in the final list
    int i,j,objectsCount = 0;
	va_start(args,numberOfSources);

        for(i=0; i<numberOfSources; i++){
            VoxelObjectList current = va_arg(args,VoxelObjectList);
            objectsCount += current.numberOfObjects;
        }

    va_end(args);

    if(dest->list){
        objectsCount += dest->numberOfObjects;
    }
    
    //Allocate the final list
	VoxelObject **final = calloc(objectsCount,sizeof(VoxelObject *));

    //Iterate in every source list, getting the objects in their list and putting in the final
	va_start(args,numberOfSources);
	int pos = 0;

    if(dest->list){
        for(j=0; j<dest->numberOfObjects; j++){
			final[pos++] = dest->list[j];
		}
    }

	for(i=0; i<numberOfSources; i++){

		VoxelObjectList current = va_arg(args,VoxelObjectList);

		for(j=0; j<current.numberOfObjects; j++){
			final[pos++] = current.list[j];
		}
	}
    va_end(args);

    dest->list = final;
    dest->numberOfObjects = objectsCount;
}