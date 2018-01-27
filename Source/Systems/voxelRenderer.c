#include "voxelRenderer.h"

extern engineTime Time;
extern engineCore Core;
extern engineScreen Screen;
extern engineRendering Rendering;
extern engineECS ECS;

static ComponentID VoxelModelID = -1;

int cubeTexDimension = 1;
GLuint CubeTex[1] = {0};


void VoxelRendererInit(){

    VoxelModelID = GetComponentID("VoxelModel");

    //Load surface into a OpenGL texture
    glGenTextures(1, CubeTex);

    //Normal
    SDL_Surface *cubeimg = IMG_Load("Textures/cube.png");
    if(!cubeimg){ printf("Failed to load!\n"); return; }
    glBindTexture(GL_TEXTURE_2D, CubeTex[0]);
    
    int Mode = GL_RGB;
    
    if(cubeimg->format->BytesPerPixel == 4) {
        Mode = GL_RGBA;
    }
    cubeTexDimension = max(cubeimg->w, cubeimg->h);
    glTexImage2D(GL_TEXTURE_2D, 0, Mode, cubeimg->w, cubeimg->h, 0, Mode, GL_UNSIGNED_BYTE, cubeimg->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    SDL_FreeSurface(cubeimg);
}

void VoxelRendererFree(){
    //glDeleteTextures(0,&CubeID​);
}


void VoxelRendererUpdate(EntityID entity){
    
    VoxelModel *obj = GetVoxelModelPointer(entity);
    Vector3 position = GetPosition(entity);
    Vector3 rotation = GetRotation(entity);
    //Configure OpenGL parameters to render point sprites

    glBindFramebuffer(GL_FRAMEBUFFER, Rendering.frameBuffer);
    glViewport(0,0,Screen.gameWidth,Screen.gameHeight);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, CubeTex[0]);

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_DEPTH_TEST);
    glAlphaFunc (GL_NOTEQUAL, 0.0f);
    glPointSize(cubeTexDimension);
    glEnable(GL_POINT_SPRITE);

    //Define matrices
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

    float sinx = sin(rotation.x * PI_OVER_180);
    float cosx = cos(rotation.x * PI_OVER_180);
    float siny = sin(rotation.y * PI_OVER_180);
    float cosy = cos(rotation.y * PI_OVER_180);
    float sinz = sin(rotation.z * PI_OVER_180);
    float cosz = cos(rotation.z * PI_OVER_180);

    GLfloat RotationMatrix[3][3]={{cosy*cosz , cosz*sinx*siny - cosx*sinz , cosx*cosz*siny + sinx*sinz},
                                  {cosy*sinz , cosx*cosz + sinx*siny*sinz , cosx*siny*sinz - cosz*sinx},
                                  {-siny     , cosy*sinx                  , cosx*cosy                 }};


    glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, obj->numberOfVertices * 3 * sizeof(GLfloat), obj->vertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, obj->numberOfVertices * 3 * sizeof(GLfloat), obj->vColors, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);

    glUseProgram(Rendering.Shaders[1]);

    glUniformMatrix4fv(glGetUniformLocation(Rendering.Shaders[1], "projection"), 1, GL_FALSE, &ProjectionMatrix[0]);
    glUniformMatrix3fv(glGetUniformLocation(Rendering.Shaders[1], "rotation"), 1, GL_FALSE, &RotationMatrix[0]);
    glUniform3f(glGetUniformLocation(Rendering.Shaders[1], "objPos"), position.x, position.y, position.z);
    glUniform3f(glGetUniformLocation(Rendering.Shaders[1], "centerPos"), obj->center.x, obj->center.y, obj->center.z);
    glUniform3f(glGetUniformLocation(Rendering.Shaders[1], "camPos"), Rendering.cameraPosition.x, Rendering.cameraPosition.y, Rendering.cameraPosition.z);
    glUniform1i(glGetUniformLocation(Rendering.Shaders[1], "spriteScale"), cubeTexDimension/5);
    glUniform1i(glGetUniformLocation(Rendering.Shaders[1], "tex"), 0);

    glDrawArrays(GL_POINTS, 0, obj->numberOfVertices);

    glUseProgram(0);
    glDisable(GL_POINT_SPRITE);
    glDisable(GL_DEPTH_TEST);
    
}