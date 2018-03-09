#include "PointLighting.h"

static System *ThisSystem;
extern engineECS ECS;

ComponentID PointLightComp;

//The align floats are to make the Vector3 acessible as a Vector4 in the shader
typedef struct PLight{
    Vector3 position;
    float align1;
    Vector3 color;
    float align2;
    float intensity;
    float range;
    float align3;
    float align4;
}PLight;

PLight lights[MAX_POINT_LIGHTS];
GLuint pointLightsBuffer = 0;

GLuint GetPointLightsBuffer(){
    return pointLightsBuffer;
}

//Runs on engine start
void PointLightingInit(System *systemObject){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("PointLighting"));
    PointLightComp = GetComponentID("PointLight");

    glGenBuffers(1,&pointLightsBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, pointLightsBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(PLight)*MAX_POINT_LIGHTS, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER,0);
}



//Runs each GameLoop iteration
void PointLightingUpdate(){
    int currentLight = 0;

    //Run for all entities
    EntityID entity;
	for(entity = 0; entity <= ECS.maxUsedIndex; entity++){
        //Check if entity contains needed components
        //If no component is required , run for all
        if(!IsEmptyComponentMask(ThisSystem->required) && !EntityContainsMask(entity,ThisSystem->required) ) continue;
        //Check if entity doesn't contains the excluded components
        //If there is no restriction, run all with the required components
        if(!IsEmptyComponentMask(ThisSystem->excluded) && EntityContainsMask(entity,ThisSystem->excluded) ) continue;

        if(currentLight>=MAX_POINT_LIGHTS) break;

        GetGlobalTransform(entity, &lights[currentLight].position,NULL);
        lights[currentLight].color = ((PointLightData*) ECS.Components[PointLightComp][entity].data)->color;
        lights[currentLight].intensity = ((PointLightData*) ECS.Components[PointLightComp][entity].data)->intensity;
        lights[currentLight].range = ((PointLightData*) ECS.Components[PointLightComp][entity].data)->range;

        currentLight++;
    }

    if(currentLight<MAX_POINT_LIGHTS){
        for(;currentLight<MAX_POINT_LIGHTS; currentLight++){
            lights[currentLight].intensity = 0;
        }
    }

    glBindBuffer(GL_UNIFORM_BUFFER,pointLightsBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(PLight)*MAX_POINT_LIGHTS,lights);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

//Runs at engine finish
void PointLightingFree(){

}