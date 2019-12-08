#include "PointLight.h"

//Function that returns the ID of this component
static ComponentID ThisComponentID(){
    static ComponentID CompID = -1;
    if(CompID<0)
        CompID = GetComponentID("PointLight");
    
    return CompID;
}

extern engineCore Core;
extern engineECS ECS;

//Runs on AddComponentToEntity
void PointLightConstructor(void** data){
    if(!data) return;
    *data = calloc(1,sizeof(PointLightData));
    ((PointLightData*)*data)->color = (Vector3){1,1,1};
    ((PointLightData*)*data)->intensity = 0.75;
    ((PointLightData*)*data)->range = 100;
    ((PointLightData*)*data)->hueShift = 0;

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

cJSON* PointLightEncode(void** data, cJSON* currentData){
    if(!data) return NULL;
    PointLightData *pl = *data; 
   
    int hasChanged = 0;
    if(currentData){
        //Check if any data has changed
        cJSON *curColor = cJSON_GetObjectItem(currentData,"color");
        
        if(pl->color.x != (cJSON_GetArrayItem(curColor,0))->valuedouble ||
            pl->color.y != (cJSON_GetArrayItem(curColor,1))->valuedouble || 
            pl->color.z != (cJSON_GetArrayItem(curColor,2))->valuedouble || 
            pl->intensity != cJSON_GetObjectItem(currentData,"intensity")->valuedouble || 
            pl->range != cJSON_GetObjectItem(currentData,"range")->valuedouble||
            pl->hueShift != cJSON_GetObjectItem(currentData,"hueShift")->valuedouble
        ){
            hasChanged = 1;
        }
    }

    //Encode this component if its not from a prefab (who has currentData) or if it has changed
    if(!currentData || hasChanged){
        cJSON *obj = cJSON_CreateObject();
        
        cJSON *colorArr = cJSON_AddArrayToObject(obj,"color");
        cJSON_AddItemToArray(colorArr,cJSON_CreateNumber(pl->color.x));
        cJSON_AddItemToArray(colorArr,cJSON_CreateNumber(pl->color.y));
        cJSON_AddItemToArray(colorArr,cJSON_CreateNumber(pl->color.z));

        cJSON_AddNumberToObject(obj,"intensity",pl->intensity);
        cJSON_AddNumberToObject(obj,"range",pl->range);
        cJSON_AddNumberToObject(obj,"hueShift",pl->hueShift);

        return obj;
    }
    return NULL;
}

void* PointLightDecode(cJSON **data){
    PointLightData *pl = malloc(sizeof(PointLightData));

    pl->color = JSON_GetObjectVector3(*data,"color",VECTOR3_ZERO);
    pl->intensity = JSON_GetObjectDouble(*data,"intensity",0);
    pl->range = JSON_GetObjectDouble(*data,"range",0);
    pl->hueShift = JSON_GetObjectDouble(*data,"hueShift",0);

    return pl;
}

Vector3 GetPointLightColor(EntityID entity){
     if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"GetPointLightColor: Entity doesn't have a PointLight component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    return pl->color;
}

void SetPointLightColor(EntityID entity, Vector3 rgbColor){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"SetPointLightColor: Entity doesn't have a PointLight component. (%d)\n",entity);
        return;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    pl->color = rgbColor;

    pl->color.x = clamp(pl->color.x, 0, INFINITY);
    pl->color.y = clamp(pl->color.y, 0, INFINITY);
    pl->color.z = clamp(pl->color.z, 0, INFINITY);
}

float GetPointLightIntensity(EntityID entity){
     if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"GetPointLightIntensity: Entity doesn't have a PointLight component. (%d)\n",entity);
        return 0;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    return pl->intensity;
}

void SetPointLightIntensity(EntityID entity, float intensity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"SetPointLightIntensity: Entity doesn't have a PointLight component. (%d)\n",entity);
        return;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    pl->intensity = intensity;
}

float GetPointLightRange(EntityID entity){
     if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"GetPointLightRange: Entity doesn't have a PointLight component. (%d)\n",entity);
        return 0;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    return pl->range;
}

void SetPointLightRange(EntityID entity, float range){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"SetPointLightRange: Entity doesn't have a PointLight component. (%d)\n",entity);
        return;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    pl->range = clamp(range,0,INFINITY);
}

float GetPointLightHueShift(EntityID entity){
     if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"GetPointLightHueShift: Entity doesn't have a PointLight component. (%d)\n",entity);
        return 0;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    return pl->hueShift;
}

void SetPointLightHueShift(EntityID entity, float shift){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"SetPointLightHueShift: Entity doesn't have a PointLight component. (%d)\n",entity);
        return;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    pl->hueShift = shift;
}
