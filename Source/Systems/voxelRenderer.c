#include "voxelRenderer.h"

static System *ThisSystem;

extern engineTime Time;
extern engineCore Core;
extern engineScreen Screen;
extern engineRendering Rendering;
extern engineECS ECS;

static ComponentID VoxelModelID = -1;

void VoxelRendererInit(){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("VoxelRenderer"));

    VoxelModelID = GetComponentID("VoxelModel");
}

void VoxelRendererFree(){
    //glDeleteTextures(0,&CubeID​);
}


void VoxelRendererUpdate(){

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
        Vector3 position = GetPosition(entity);
        Vector3 rotation = GetRotation(entity);
        //Configure OpenGL parameters to render point sprites

        //Render game objects only in the [0.01,1.0] range, as [0,0.01] is reserved for UI rendering
        glDepthRange(0.01, 1.0);

        glBindFramebuffer(GL_FRAMEBUFFER, Rendering.frameBuffer);
        glViewport(0,0,Screen.gameWidth,Screen.gameHeight);

        glEnable(GL_DEPTH_TEST);
        glAlphaFunc (GL_NOTEQUAL, 0.0f);
        glPointSize(4);

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
        glBufferData(GL_ARRAY_BUFFER, obj->numberOfVertices * 3 * sizeof(GLfloat), obj->vertices, GL_STREAM_DRAW);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, obj->numberOfVertices * 3 * sizeof(GLfloat), obj->vColors, GL_STREAM_DRAW);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo[2]);
        glBufferData(GL_ARRAY_BUFFER, obj->numberOfVertices * 3 * sizeof(GLfloat), obj->normal, GL_STREAM_DRAW);
        glEnableVertexAttribArray(2);

        glUseProgram(Rendering.Shaders[1]);

        glUniformMatrix4fv(glGetUniformLocation(Rendering.Shaders[1], "projection"), 1, GL_FALSE, &ProjectionMatrix[0]);
        glUniformMatrix3fv(glGetUniformLocation(Rendering.Shaders[1], "rotation"), 1, GL_FALSE, &RotationMatrix[0]);
        glUniform3f(glGetUniformLocation(Rendering.Shaders[1], "objPos"), position.x, position.y, position.z);
        glUniform3f(glGetUniformLocation(Rendering.Shaders[1], "centerPos"), obj->center.x, obj->center.y, obj->center.z);
        glUniform3f(glGetUniformLocation(Rendering.Shaders[1], "camPos"), Rendering.cameraPosition.x, Rendering.cameraPosition.y, Rendering.cameraPosition.z);
        glUniform1i(glGetUniformLocation(Rendering.Shaders[1], "tex"), 0);

        glDrawArrays(GL_POINTS, 0, obj->numberOfVertices);

        glUseProgram(0);

        glDisable(GL_DEPTH_TEST);

        //Return depth to default valuess
        glDepthRange(0, 1.0);
    }
}