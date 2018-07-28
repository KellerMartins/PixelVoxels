#include "Shadows.h"

static System *ThisSystem;

extern engineECS ECS;
extern engineRendering Rendering;
extern engineScreen Screen;

GLuint shadowFramebuffer;
GLuint shadowDepthTexture;

unsigned int shadowTextureWidth;
unsigned int shadowTextureHeight;

//Runs on engine start
void ShadowsInit(){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("Shadows"));

    shadowTextureWidth = 256;
    shadowTextureHeight = 256;

    glGenFramebuffers(1, &shadowFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);

    glGenTextures(1, &shadowDepthTexture);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowTextureWidth, shadowTextureHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

    // Set FBO attachements
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexture, 0);
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		PrintLog(Error,"ShadowsInit: Error generating framebuffer! (%x)\n",glCheckFramebufferStatus(GL_FRAMEBUFFER));
        ThisSystem->enabled = 0;
	}

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//Runs each GameLoop iteration
void ShadowsUpdate(){

    glViewport(0,0,shadowTextureWidth,shadowTextureWidth);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(Rendering.Shaders[4]);


    Vector3 sunDirection = (Vector3){-1,-0.1,1};
    GLfloat ViewMatrix[4][4]={{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    ShadowViewMatrix(ViewMatrix, sunDirection);


    //Get uniform locations
    static GLint projLoc = -1;
    if(projLoc < 0)
        projLoc = glGetUniformLocation(Rendering.Shaders[4], "projection");

    static GLint viewLoc = -1;
    if(viewLoc < 0)
        viewLoc = glGetUniformLocation(Rendering.Shaders[4], "view");

    static GLint rotLoc = -1;
    if(rotLoc < 0)
        rotLoc = glGetUniformLocation(Rendering.Shaders[4], "rotation");

    static GLint objPosLoc = -1;
    if(objPosLoc < 0)
        objPosLoc = glGetUniformLocation(Rendering.Shaders[4], "objPos");

    static GLint centerPosLoc = -1;
    if(centerPosLoc < 0)
        centerPosLoc = glGetUniformLocation(Rendering.Shaders[4], "centerPos");

    static GLint camPosLoc = -1;
    if(camPosLoc < 0)
        camPosLoc = glGetUniformLocation(Rendering.Shaders[4], "camPos");

    static GLint scaleLoc = -1;
    if(scaleLoc < 0)
        scaleLoc = glGetUniformLocation(Rendering.Shaders[4], "scale");


    //glUniformMatrix4fv(glGetUniformLocation(Rendering.Shaders[4], "projection"), 1, GL_FALSE, (const GLfloat*)&ProjectionMatrix[0]);
    glUniformMatrix4fv(glGetUniformLocation(Rendering.Shaders[4], "view"), 1, GL_FALSE, (const GLfloat*)&ViewMatrix[0]);

    //Run for all entities
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
            glPointSize(1);
            glUniform1i(scaleLoc, 2);
        }else{
            glPointSize(2);
            glUniform1i(scaleLoc, 1);
        }

        //Setup buffers
        glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[0]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        
        glUniformMatrix3fv(rotLoc, 1, GL_FALSE, (const GLfloat*)&Transpose(rotation).m[0]);
        glUniform3f(objPosLoc, position.x, position.y, position.z);
        glUniform3f(centerPosLoc, obj->center.x, obj->center.y, obj->center.z);
        glUniform3f(camPosLoc, Rendering.cameraPosition.x, Rendering.cameraPosition.y, Rendering.cameraPosition.z);
        

        glDrawArrays(GL_POINTS, 0, obj->numberOfVertices);
    }

    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if(GetKey(SDL_SCANCODE_V))
        RenderTextureToScreen(shadowDepthTexture);
}

//Runs at engine finish
void ShadowsFree(){
    glDeleteBuffers(1, &shadowFramebuffer);
}