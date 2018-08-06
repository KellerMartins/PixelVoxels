#include "VoxelRenderer.h"

static System *ThisSystem;

extern engineTime Time;
extern engineCore Core;
extern engineScreen Screen;
extern engineRendering Rendering;
extern engineECS ECS;
extern engineScene Scene;

extern GLuint shadowDepthTexture;

static ComponentID VoxelModelID = -1;

int cubeTexDimension = 1;
GLuint CubeTex[1] = {0};

void VoxelRendererInit(){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("VoxelRenderer"));

    VoxelModelID = GetComponentID("VoxelModel");

    //Load surface into a OpenGL texture
    glGenTextures(1, CubeTex);

    //Normal
    SDL_Surface *cubeimg = IMG_Load("Assets/Game/Textures/cube.png");
    if(!cubeimg){ PrintLog(Error,"VoxelRendererInit: Failed to load normal map texture!\n"); return; }
    glBindTexture(GL_TEXTURE_2D, CubeTex[0]);
    
    int Mode = GL_RGB;
    
    if(cubeimg->format->BytesPerPixel == 4) {
        Mode = GL_RGBA;
    }
    cubeTexDimension = min(cubeimg->w, cubeimg->h);
    glTexImage2D(GL_TEXTURE_2D, 0, Mode, cubeimg->w, cubeimg->h, 0, Mode, GL_UNSIGNED_BYTE, cubeimg->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    SDL_FreeSurface(cubeimg);
}

void VoxelRendererFree(){
    //glDeleteTextures(0,&CubeID​);
}


void VoxelRendererUpdate(){

    //Configure OpenGL parameters to render point sprites

    glViewport(0,0,Screen.gameWidth,Screen.gameHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, Rendering.frameBuffer);
    glEnable(GL_DEPTH_TEST);


    //Define the projection matrix
    float right = Screen.gameWidth/2;
    float left = -Screen.gameWidth/2;
    float top = Screen.gameHeight/2;
    float bottom = -Screen.gameHeight/2;
    float near = -500;
    float far = 500;
    
    GLfloat ProjectionMatrix[4][4]={{2.0f/(right-left), 0                 , 0                , -(right + left)/(right - left) },
                                    {0                , 2.0f/(top-bottom) , 0                , -(top + bottom)/(top - bottom) },
                                    {0                , 0                 , -2.0f/(far-near) , -(far + near)/(far - near)     },
                                    {0                , 0                 , 0                ,   1                            }};

    
    //Generate shadow view matrix
    GLfloat ViewMatrix[4][4]={{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    ShadowViewMatrix(ViewMatrix);

    Vector3 sunDir = GetTrieElementAs_Vector3(Scene.data, "sunDirection", (Vector3){0.75,0.2,-1.5});
    Vector3 sunColor = GetTrieElementAs_Vector3(Scene.data, "sunColor", (Vector3){1,1,1});

    //Get uniform locations
    static GLint projLocB = -1;
    static GLint projLocS = -1;
    if(projLocB < 0)
        projLocB = glGetUniformLocation(Rendering.Shaders[1], "projection");
    if(projLocS < 0)
        projLocS = glGetUniformLocation(Rendering.Shaders[2], "projection");

    static GLint shadowViewLocB = -1;
    static GLint shadowViewLocS = -1;
    if(shadowViewLocB < 0)
        shadowViewLocB = glGetUniformLocation(Rendering.Shaders[1], "shadowView");
    if(shadowViewLocS < 0)
        shadowViewLocS = glGetUniformLocation(Rendering.Shaders[2], "shadowView");

    static GLint shadowDepthLocB = -1;
    static GLint shadowDepthLocS = -1;
    if(shadowDepthLocB < 0)
        shadowDepthLocB = glGetUniformLocation(Rendering.Shaders[1], "shadowDepth");
    if(shadowDepthLocS < 0)
        shadowDepthLocS = glGetUniformLocation(Rendering.Shaders[2], "shadowDepth");

    static GLint rotLocB = -1;
    static GLint rotLocS = -1;
    if(rotLocB < 0)
        rotLocB = glGetUniformLocation(Rendering.Shaders[1], "rotation");
    if(rotLocS < 0)
        rotLocS = glGetUniformLocation(Rendering.Shaders[2], "rotation");

    static GLint objPosLocB = -1;
    static GLint objPosLocS = -1;
    if(objPosLocB < 0)
        objPosLocB = glGetUniformLocation(Rendering.Shaders[1], "objPos");
    if(objPosLocS < 0)
        objPosLocS = glGetUniformLocation(Rendering.Shaders[2], "objPos");

    static GLint centerPosLocB = -1;
    static GLint centerPosLocS = -1;
    if(centerPosLocB < 0)
        centerPosLocB = glGetUniformLocation(Rendering.Shaders[1], "centerPos");
    if(centerPosLocS < 0)
        centerPosLocS = glGetUniformLocation(Rendering.Shaders[2], "centerPos");

    static GLint camPosLocB = -1;
    static GLint camPosLocS = -1;
    if(camPosLocB < 0)
        camPosLocB = glGetUniformLocation(Rendering.Shaders[1], "camPos");
    if(camPosLocS < 0)
        camPosLocS = glGetUniformLocation(Rendering.Shaders[2], "camPos");

    static GLint sunDirLocB = -1;
    static GLint sunDirLocS = -1;
    if(sunDirLocB < 0)
        sunDirLocB = glGetUniformLocation(Rendering.Shaders[1], "sunDir");
    if(sunDirLocS < 0)
        sunDirLocS = glGetUniformLocation(Rendering.Shaders[2], "sunDir");

    static GLint sunColorLocB = -1;
    static GLint sunColorLocS = -1;
    if(sunColorLocB < 0)
        sunColorLocB = glGetUniformLocation(Rendering.Shaders[1], "sunColor");
    if(sunColorLocS < 0)
        sunColorLocS = glGetUniformLocation(Rendering.Shaders[2], "sunColor");

    static GLint lightBlockB = -1;
    static GLint lightBlockS = -1;
    if(lightBlockB < 0)
        lightBlockB = glGetUniformBlockIndex(Rendering.Shaders[1], "PointLight");
    if(lightBlockS < 0)
        lightBlockS = glGetUniformBlockIndex(Rendering.Shaders[2], "PointLight");
        

    static GLint spriteScaleLoc = -1;
    if(spriteScaleLoc < 0)
        spriteScaleLoc = glGetUniformLocation(Rendering.Shaders[1], "spriteScale");

    static GLint texLoc = -1;
    if(texLoc < 0)
        texLoc = glGetUniformLocation(Rendering.Shaders[1], "tex");
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetShadowDepthTexture());

    EntityID entity;
	for(entity = 0; entity <= ECS.maxUsedIndex; entity++){

        //Check if entity contains needed components
        //If no component is required , run for all
        if(!IsEmptyComponentMask(ThisSystem->required) && !EntityContainsMask(entity,ThisSystem->required) ) continue;
        //Check if entity doesn't contains the excluded components
        //If there is no restriction, run all with the required components
        if(!IsEmptyComponentMask(ThisSystem->excluded) && EntityContainsMask(entity,ThisSystem->excluded) ) continue;

        VoxelModel *obj = GetVoxelModelPointer(entity);
        if(!obj->model || !obj->enabled){
            continue;
        }
        Vector3 position;
        Matrix3x3 rotation;
        GetGlobalTransform(entity, &position, NULL,&rotation);

        if(obj->smallScale){    
            glPointSize(2);
        }else{
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, CubeTex[0]);

            glPointSize(cubeTexDimension);
        }

        //Setup buffers
        glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[0]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[1]);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[2]);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);


        GLint lightBlock, projLoc, rotLoc, objPosLoc, centerPosLoc, camPosLoc, shadowViewLoc, shadowDepthLoc, sunDirLoc, sunColorLoc;
        if(obj->smallScale){
            glUseProgram(Rendering.Shaders[2]);

            lightBlock = lightBlockS;
            projLoc = projLocS;
            rotLoc = rotLocS;
            objPosLoc = objPosLocS;
            centerPosLoc = centerPosLocS;
            camPosLoc = camPosLocS;
            shadowViewLoc = shadowViewLocS;
            shadowDepthLoc = shadowDepthLocS;
            sunDirLoc = sunDirLocS;
            sunColorLoc = sunColorLocS;
            
            glUniformBlockBinding(Rendering.Shaders[2], lightBlock, 0);
        }else{
            glUseProgram(Rendering.Shaders[1]);
            
            lightBlock = lightBlockB;
            projLoc = projLocB;
            rotLoc = rotLocB;
            objPosLoc = objPosLocB;
            centerPosLoc = centerPosLocB;
            camPosLoc = camPosLocB;
            shadowViewLoc = shadowViewLocB;
            shadowDepthLoc = shadowDepthLocB;
            sunDirLoc = sunDirLocB;
            sunColorLoc = sunColorLocB;

            glUniformBlockBinding(Rendering.Shaders[1], lightBlock, 0);
            glUniform1i(texLoc, 1);
            glUniform1i(spriteScaleLoc, cubeTexDimension/5.0f);
        }
 
        glBindBufferBase(GL_UNIFORM_BUFFER, lightBlock, GetPointLightsBuffer());

        glUniformMatrix4fv(projLoc, 1, GL_FALSE, (const GLfloat*)&ProjectionMatrix[0]);
        glUniformMatrix3fv(rotLoc, 1, GL_FALSE, (const GLfloat*)&Transpose(rotation).m[0]);
        glUniform3f(objPosLoc, position.x, position.y, position.z);
        glUniform3f(centerPosLoc, obj->center.x, obj->center.y, obj->center.z);
        glUniform3f(camPosLoc, Rendering.cameraPosition.x, Rendering.cameraPosition.y, Rendering.cameraPosition.z);
        glUniform3f(sunDirLoc, sunDir.x, sunDir.y, sunDir.z);
        glUniform3f(sunColorLoc, sunColor.x, sunColor.y, sunColor.z);
        
        glUniform1i(shadowDepthLoc, 0);
        glUniformMatrix4fv(shadowViewLoc, 1, GL_FALSE, (const GLfloat*)&ViewMatrix[0]);

        glDrawArrays(GL_POINTS, 0, obj->numberOfVertices);

        glUseProgram(0);
    }

    glDisable(GL_DEPTH_TEST);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}