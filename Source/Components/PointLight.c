#include "PointLight.h"

//Function that returns the ID of this component
static ComponentID ThisComponentID(){
    static ComponentID CompID = -1;
    if(CompID<0)
        CompID = GetComponentID("PointLight");
    
    return CompID;
}

extern engineECS ECS;

//Runs on AddComponentToEntity
void PointLightConstructor(void** data){
    if(!data) return;
    *data = calloc(1,sizeof(PointLightData));
    ((PointLightData*)*data)->color = (Vector3){1,1,1};
    ((PointLightData*)*data)->intensity = 0.75;
    ((PointLightData*)*data)->range = 100;

}

//Runs on RemoveComponentFromEntity
void PointLightDestructor(void** data){
    if(!data) return;
    free(*data);
    *data = NULL;
}

void* PointLightCopy(void* data){
    if(!data) return NULL;
    PointLightData *newPointLightData = malloc(sizeof(PointLightData));
    memcpy(newPointLightData,data,sizeof(PointLightData));
	return newPointLightData;
}

cJSON* PointLightEncode(void** data){
    PointLightData *pl = *data; 
    cJSON *obj = cJSON_CreateObject();

    cJSON *colorArr = cJSON_AddArrayToObject(obj,"color");
    cJSON_AddItemToArray(colorArr,cJSON_CreateNumber(pl->color.x));
    cJSON_AddItemToArray(colorArr,cJSON_CreateNumber(pl->color.y));
    cJSON_AddItemToArray(colorArr,cJSON_CreateNumber(pl->color.z));

    cJSON_AddNumberToObject(obj,"intensity",pl->intensity);
    cJSON_AddNumberToObject(obj,"range",pl->range);

    return obj;
}

void* PointLightDecode(cJSON **data){
    PointLightData *pl = malloc(sizeof(PointLightData));

    cJSON *colorArr = cJSON_GetObjectItem(*data,"color");
    pl->color.x = cJSON_GetArrayItem(colorArr,0)->valuedouble;
    pl->color.y = cJSON_GetArrayItem(colorArr,1)->valuedouble;
    pl->color.z = cJSON_GetArrayItem(colorArr,2)->valuedouble;

    pl->color.x = clamp(pl->color.x,0,1);
    pl->color.y = clamp(pl->color.y,0,1);
    pl->color.z = clamp(pl->color.z,0,1);

    pl->intensity = cJSON_GetObjectItem(*data,"intensity")->valuedouble;
    pl->range = cJSON_GetObjectItem(*data,"range")->valuedouble;

    return pl;
}

Vector3 GetPointLightColor(EntityID entity){
     if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetPointLightColor: Entity doesn't have a PointLight component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    return pl->color;
}

void SetPointLightColor(EntityID entity, Vector3 rgbColor){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetPointLightColor: Entity doesn't have a PointLight component. (%d)\n",entity);
        return;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    pl->color = rgbColor;
}

float GetPointLightIntensity(EntityID entity){
     if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetPointLightIntensity: Entity doesn't have a PointLight component. (%d)\n",entity);
        return 0;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    return pl->intensity;
}

void SetPointLightIntensity(EntityID entity, float intensity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetPointLightIntensity: Entity doesn't have a PointLight component. (%d)\n",entity);
        return;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    pl->intensity = clamp(intensity,0,INFINITY);
}

float GetPointLightRange(EntityID entity){
     if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetPointLightRange: Entity doesn't have a PointLight component. (%d)\n",entity);
        return 0;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    return pl->range;
}

void SetPointLightRange(EntityID entity, float range){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetPointLightRange: Entity doesn't have a PointLight component. (%d)\n",entity);
        return;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    pl->range = clamp(range,0,INFINITY);
}