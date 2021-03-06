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
GLuint CubeTex[2];

void VoxelRendererInit(){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("VoxelRenderer"));

    VoxelModelID = GetComponentID("VoxelModel");

    //Load surface into a OpenGL texture
    glGenTextures(2, CubeTex);

    //Normal
    SDL_Surface *cubeimg = IMG_Load("Assets/Game/Textures/cubeBig.png");
    if(!cubeimg){ PrintLog(Error,"VoxelRendererInit: Failed to load normal map texture!\n"); return; }
    glBindTexture(GL_TEXTURE_2D, CubeTex[0]);
    
    int Mode = GL_RGB;
    
    if(cubeimg->format->BytesPerPixel == 4) {
        Mode = GL_RGBA;
    }
    
    cubeTexDimension = min(cubeimg->w, cubeimg->h);
    SetSpriteScale(cubeTexDimension/5.0);

    glTexImage2D(GL_TEXTURE_2D, 0, Mode, cubeimg->w, cubeimg->h, 0, Mode, GL_UNSIGNED_BYTE, cubeimg->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    SDL_FreeSurface(cubeimg);

    //Position
    cubeimg = IMG_Load("Assets/Game/Textures/cubeBigPos.png");
    if(!cubeimg){ PrintLog(Error,"VoxelRendererInit: Failed to load position map texture!\n"); return; }
    glBindTexture(GL_TEXTURE_2D, CubeTex[1]);

    if(cubeimg->format->BytesPerPixel == 4) {
        Mode = GL_RGBA;
    }
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
    
    Matrix4x4 ProjectionMatrix = GetProjectionMatrix(Screen.gameWidth/2, -Screen.gameWidth/2, 
                                                     Screen.gameHeight/2,-Screen.gameHeight/2,
                                                     -500, 500);


    Vector3 sunDir = GetTrieElementAs_Vector3(Scene.data, "sunDirection", (Vector3){0.75,0.2,-1.5});
    Vector3 sunColor = GetTrieElementAs_Vector3(Scene.data, "sunColor", (Vector3){1,1,1});

    //Big voxels specific locations
    GLint texLoc = glGetUniformLocation(Rendering.Shaders[1], "tex");
    GLint texPosLoc = glGetUniformLocation(Rendering.Shaders[1], "texPos");
    
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

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, CubeTex[1]);

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

        int usedProgram = 0;
        if(obj->smallScale){
            usedProgram = 2;
            glUseProgram(Rendering.Shaders[2]);

            
        }else{
            usedProgram = 1;
            glUseProgram(Rendering.Shaders[1]);

            glUniform1i(texLoc, 1);
            glUniform1i(texPosLoc, 2);
        }

        GLint lightBlock =  glGetUniformBlockIndex(Rendering.Shaders[usedProgram], "PointLight");
        GLint projLoc = glGetUniformLocation(Rendering.Shaders[usedProgram], "projection");
        GLint rotLoc = glGetUniformLocation(Rendering.Shaders[usedProgram], "rotation");
        GLint spriteScaleLoc = glGetUniformLocation(Rendering.Shaders[usedProgram], "spriteScale");
        GLint objPosLoc = glGetUniformLocation(Rendering.Shaders[usedProgram], "objPos");
        GLint centerPosLoc = glGetUniformLocation(Rendering.Shaders[usedProgram], "centerPos");
        GLint camPosLoc = glGetUniformLocation(Rendering.Shaders[usedProgram], "camPos");
        GLint shadowMatrixLoc = glGetUniformLocation(Rendering.Shaders[usedProgram], "shadowMatrix");
        GLint shadowDepthLoc = glGetUniformLocation(Rendering.Shaders[usedProgram], "shadowDepth");;
        GLint sunDirLoc = glGetUniformLocation(Rendering.Shaders[usedProgram], "sunDir");
        GLint sunColorLoc = glGetUniformLocation(Rendering.Shaders[usedProgram], "sunColor");
 
        glBindBufferBase(GL_UNIFORM_BUFFER, lightBlock, GetPointLightsBuffer());
        glUniformBlockBinding(Rendering.Shaders[usedProgram], lightBlock, 0);

        glUniformMatrix4fv(projLoc, 1, GL_FALSE, (const GLfloat*)&ProjectionMatrix.m);
        glUniformMatrix3fv(rotLoc, 1, GL_FALSE, (const GLfloat*)&Transpose(rotation).m[0]);
        glUniform1i(spriteScaleLoc, Rendering.spriteScale);
        glUniform3f(objPosLoc, position.x, position.y, position.z);
        glUniform3f(centerPosLoc, obj->center.x, obj->center.y, obj->center.z);
        glUniform3f(camPosLoc, Rendering.cameraPosition.x, Rendering.cameraPosition.y, Rendering.cameraPosition.z);
        glUniform3f(sunDirLoc, sunDir.x, sunDir.y, sunDir.z);
        glUniform3f(sunColorLoc, sunColor.x, sunColor.y, sunColor.z);
        
        glUniform1i(shadowDepthLoc, 0);
        glUniformMatrix4fv(shadowMatrixLoc, 1, GL_FALSE, GetShadowMatrix());

        glDrawArrays(GL_POINTS, 0, obj->numberOfVertices);

        glUseProgram(0);
    }

    glDisable(GL_DEPTH_TEST);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}