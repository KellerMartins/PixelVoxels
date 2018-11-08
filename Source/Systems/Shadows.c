#include "Shadows.h"

static System *ThisSystem;

extern engineECS ECS;
extern engineRendering Rendering;
extern engineScreen Screen;
extern engineScene Scene;

GLuint shadowFramebuffer;
GLuint shadowDepthTexture;

unsigned int shadowTextureWidth;
unsigned int shadowTextureHeight;

Vector3 sunDirection = (Vector3){0.75,0.2,-1.5};
GLfloat shadowMatrix[4][4];

GLuint GetShadowDepthTexture(){
    return shadowDepthTexture;
}

const GLfloat* GetShadowMatrix(){
    return (const GLfloat*)&shadowMatrix[0];
}

void GetModelBounds(EntityID entity, Vector3* min, Vector3* max, Vector3* screenMin, Vector3* screenMax);
void MultiplyByOrthographic(GLfloat m[4][4], float right , float left, float top, float bottom, float near, float far);
void IdentityMatrix4x4(GLfloat m[4][4]);
void ShadowViewMatrix(GLfloat viewMatrix[4][4], Vector3 position);


//Runs on engine start
void ShadowsInit(){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("Shadows"));

    shadowTextureWidth = 1024;
    shadowTextureHeight = 1024;

    glGenFramebuffers(1, &shadowFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);

    glGenTextures(1, &shadowDepthTexture);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowTextureWidth, shadowTextureHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

    //Get shadow bounds
    int shadowedObjects = 0;
    Vector3 lastObjPos = VECTOR3_ZERO;
    Vector3 min = (Vector3){INFINITY,INFINITY,INFINITY};
    Vector3 max = (Vector3){-INFINITY,-INFINITY,-INFINITY};
    EntityID entity;
	for(entity = 0; entity <= ECS.maxUsedIndex; entity++){

        //Check for required and excluded components
        if(!IsEmptyComponentMask(ThisSystem->required) && !EntityContainsMask(entity,ThisSystem->required) ) continue;
        if(!IsEmptyComponentMask(ThisSystem->excluded) && EntityContainsMask(entity,ThisSystem->excluded) ) continue;

        Vector3 minObj, maxObj;
        Vector3 screenMin, screenMax;
        GetModelBounds(entity, &minObj, &maxObj, &screenMin, &screenMax);
        
        //Ignore objects that are outside of the screen
        if(screenMax.x < 0 || screenMax.y < 0 || screenMin.x > Screen.windowWidth || screenMin.y > Screen.windowHeight) continue;
        shadowedObjects++;
        lastObjPos = ScalarMult(Add(minObj, maxObj), 0.5);

        //Expand bounds to encapsulate the AABB of this model
        if(minObj.x < min.x) min.x = minObj.x;
        if(minObj.y < min.y) min.y = minObj.y;
        if(minObj.z < min.z) min.z = minObj.z;
        if(maxObj.x > max.x) max.x = maxObj.x;
        if(maxObj.y > max.y) max.y = maxObj.y;
        if(maxObj.z > max.z) max.z = maxObj.z;
    }
    if(shadowedObjects == 1){
        min = lastObjPos;
        max = lastObjPos;
    }

    min.z = clamp(min.z, 0, 255);
    max.z = clamp(max.z, 0, 255);

    IdentityMatrix4x4(shadowMatrix);
    ShadowViewMatrix(shadowMatrix, (Vector3){(max.x+min.x)/2, (max.y+min.y)/2, (max.z+min.z)/2});
    MultiplyByOrthographic(shadowMatrix, 210, -210, 210, -210, 210, -210);

    glViewport(0,0,shadowTextureWidth,shadowTextureWidth);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(Rendering.Shaders[4]);
    
    sunDirection = GetTrieElementAs_Vector3(Scene.data, "sunDirection", (Vector3){0.75,0.2,-1.5});


    //Get uniform locations
    static GLint projLoc = -1;
    if(projLoc < 0)
        projLoc = glGetUniformLocation(Rendering.Shaders[4], "projection");

    static GLint viewLoc = -1;
    if(viewLoc < 0)
        viewLoc = glGetUniformLocation(Rendering.Shaders[4], "view");

    static GLint sunDirLoc = -1;
    if(sunDirLoc < 0)
        sunDirLoc = glGetUniformLocation(Rendering.Shaders[4], "sunDir");

    static GLint rotLoc = -1;
    if(rotLoc < 0)
        rotLoc = glGetUniformLocation(Rendering.Shaders[4], "rotation");

    static GLint objPosLoc = -1;
    if(objPosLoc < 0)
        objPosLoc = glGetUniformLocation(Rendering.Shaders[4], "objPos");

    static GLint centerPosLoc = -1;
    if(centerPosLoc < 0)
        centerPosLoc = glGetUniformLocation(Rendering.Shaders[4], "centerPos");

    static GLint scaleLoc = -1;
    if(scaleLoc < 0)
        scaleLoc = glGetUniformLocation(Rendering.Shaders[4], "scale");


    glUniformMatrix4fv(glGetUniformLocation(Rendering.Shaders[4], "shadowMatrix"), 1, GL_FALSE, (const GLfloat*)&shadowMatrix[0]);

    //Run for all entities
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
            glUniform1i(scaleLoc, 2);
        }else{
            glPointSize(4);
            glUniform1i(scaleLoc, 1);
        }

        //Setup buffers
        glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[0]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[2]);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
        
        glUniformMatrix3fv(rotLoc, 1, GL_FALSE, (const GLfloat*)&Transpose(rotation).m[0]);
        glUniform3f(objPosLoc, position.x, position.y, position.z);
        glUniform3f(centerPosLoc, obj->center.x, obj->center.y, obj->center.z);
        glUniform3f(sunDirLoc, sunDirection.x, sunDirection.y, sunDirection.z);
        

        glDrawArrays(GL_POINTS, 0, obj->numberOfVertices);
    }

    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if(GetKey(SDL_SCANCODE_V))
        RenderTextureToScreen(shadowDepthTexture);
}

//Runs at engine finish
void ShadowsFree(){
    glDeleteBuffers(1, &shadowFramebuffer);
}

void ShadowViewMatrix(GLfloat viewMatrix[4][4], Vector3 position){
    Vector3 screenZero = PositionToGameScreenCoords(VECTOR3_ZERO);
    Vector3 screenLeft = PositionToGameScreenCoords(VECTOR3_LEFT);
    Vector3 screenForward = PositionToGameScreenCoords(VECTOR3_FORWARD);

    Vector3 xdir = (Vector3){(screenForward.x - screenZero.x)/(Screen.windowWidth*Rendering.spriteScale),
                             (screenForward.y - screenZero.y)/(Screen.windowHeight*Rendering.spriteScale)};
    Vector3 ydir = (Vector3){(screenLeft.x - screenZero.x)/(Screen.windowWidth*Rendering.spriteScale),
                             (screenLeft.y - screenZero.y)/(Screen.windowHeight*Rendering.spriteScale)};

    position = Add(ScalarMult(xdir,position.x*1.5), 
                   ScalarMult(ydir,-position.y*1.5));
    Vector3 target = Subtract(position, sunDirection);

    Vector3 forwardVec = NormalizeVector(Subtract(position, target));
    Vector3 leftVec = NormalizeVector(cross(VECTOR3_UP,forwardVec));
    Vector3 upVec = cross(forwardVec,leftVec);

    viewMatrix[0][0] = -leftVec.x;
    viewMatrix[1][0] = -leftVec.y;
    viewMatrix[2][0] = -leftVec.z;
    viewMatrix[0][1] = upVec.x;
    viewMatrix[1][1] = upVec.y;
    viewMatrix[2][1] = upVec.z;
    viewMatrix[0][2] = forwardVec.x;
    viewMatrix[1][2] = forwardVec.y;
    viewMatrix[2][2]= forwardVec.z;
    viewMatrix[0][3] = 0;
    viewMatrix[1][3] = 0;
    viewMatrix[2][3] = 0;

    viewMatrix[3][0] = -leftVec.x * position.x - leftVec.y * position.y - leftVec.z * position.z;
    viewMatrix[3][1] = -upVec.x * position.x - upVec.y * position.y - upVec.z * position.z;
    viewMatrix[3][2] = -forwardVec.x * position.x - forwardVec.y * position.y - forwardVec.z * position.z;
    viewMatrix[3][3] = 1.0;
}

void MultiplyByOrthographic(GLfloat m[4][4], float right , float left, float top, float bottom, float near, float far){
    float orthX = -(right + left)/(right - left);
    float orthY = -(top + bottom)/(top - bottom);
    float orthZ = -(far + near)/(far - near);

    m[0][3] += m[0][0]*orthX + m[0][1]*orthY + m[0][2]*orthZ;
    m[1][3] += m[1][0]*orthX + m[1][1]*orthY + m[1][2]*orthZ;
    m[2][3] += m[2][0]*orthX + m[2][1]*orthY + m[2][2]*orthZ;

    m[0][0] *= 2.0f/(right-left);
    m[1][0] *= 2.0f/(top-bottom);
    m[2][0] *= -2.0f/(far-near);

    m[0][1] *= 2.0f/(right-left);
    m[1][1] *= 2.0f/(top-bottom);
    m[2][1] *= -2.0f/(far-near);

    m[0][2] *= 2.0f/(right-left);
    m[1][2] *= 2.0f/(top-bottom);
    m[2][2] *= -2.0f/(far-near);
}

void IdentityMatrix4x4(GLfloat m[4][4]){
    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            m[i][j] = i==j? 1:0;
        }
    }
}

void GetModelBounds(EntityID entity, Vector3* min, Vector3* max, Vector3* screenMin, Vector3* screenMax){

    if(min && max){
        *min = (Vector3){INFINITY,INFINITY,INFINITY};
        *max = (Vector3){-INFINITY,-INFINITY,-INFINITY};
    }

    if(screenMin && screenMax){
        *screenMin = (Vector3){INFINITY,INFINITY,0};
        *screenMax = (Vector3){-INFINITY,-INFINITY,0};
    }

    Vector3 pos;
    Matrix3x3 rot;
    GetGlobalTransform(entity, &pos, NULL, &rot);
    VoxelModel* obj = GetVoxelModelPointer(entity);

    Vector3 center = GetVoxelModelCenter(entity);
    Vector3 dim = (Vector3){obj->dimension[0], obj->dimension[1], obj->dimension[2]};

    Vector3 c[8];

    //Set local vertex coordinates
    c[0] = (Vector3){0    ,0    ,0    };
    c[1] = (Vector3){dim.x,0    ,0    };
    c[2] = (Vector3){0    ,dim.y,0    };
    c[3] = (Vector3){0    ,0    ,dim.z};
    c[4] = (Vector3){dim.x,dim.y,0    };
    c[5] = (Vector3){dim.x,0    ,dim.z};
    c[6] = (Vector3){0    ,dim.y,dim.z};
    c[7] = (Vector3){dim.x,dim.y,dim.z};

    //Apply rotation and transform to global
    for(int i=0; i<8; i++)
        c[i] = Add(RotateVector( ScalarMult(Subtract(c[i], center), IsVoxelModelSmallScale(entity)? 0.5:1), rot), pos);

    for(int i=0; i<8; i++){
        //Calculate AABB
        if(min && max){
            if(c[i].x < min->x) min->x = c[i].x;
            if(c[i].y < min->y) min->y = c[i].y;
            if(c[i].z < min->z) min->z = c[i].z;

            if(c[i].x > max->x) max->x = c[i].x;
            if(c[i].y > max->y) max->y = c[i].y;
            if(c[i].z > max->z) max->z = c[i].z;
        }

        //Calculate screen rectangle bounds
        if(screenMin && screenMax){
            Vector3 sc = PositionToGameScreenCoords(c[i]);

            if(sc.x < screenMin->x) screenMin->x = sc.x;
            if(sc.y < screenMin->y) screenMin->y = sc.y;

            if(sc.x > screenMax->x) screenMax->x = sc.x;
            if(sc.y > screenMax->y) screenMax->y = sc.y;
        }
    }
}

