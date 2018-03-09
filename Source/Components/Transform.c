#include "Transform.h"

//Can't exist without being an component from an entity
//Defines the local transform of an entity
typedef struct Transform{
    Vector3 position;
    Vector3 rotation;
}Transform;

//Function that returns the ID of this component
static ComponentID ThisComponentID(){
    static ComponentID CompID = -1;
    if(CompID<0)
        CompID = GetComponentID("Transform");
    
    return CompID;
}

extern engineECS ECS;

void TransformConstructor(void** data){
    if(!data) return;
    *data = calloc(1,sizeof(Transform));
}

void TransformDestructor(void** data){
    if(!data) return;
    free(*data);
    *data = NULL;
}

void* TransformCopy(void* data){
    if(!data) return NULL;
    Transform *newTransform = malloc(sizeof(Transform));
    memcpy(newTransform,data,sizeof(Transform));
	return newTransform;
}

cJSON* TransformEncode(void** data){
    Transform *tr = *data; 
    cJSON *obj = cJSON_CreateObject();

    cJSON *position = cJSON_AddArrayToObject(obj,"position");
    cJSON_AddItemToArray(position, cJSON_CreateNumber(tr->position.x));
    cJSON_AddItemToArray(position, cJSON_CreateNumber(tr->position.y));
    cJSON_AddItemToArray(position, cJSON_CreateNumber(tr->position.z));

    cJSON *rotation = cJSON_AddArrayToObject(obj,"rotation");
    cJSON_AddItemToArray(rotation, cJSON_CreateNumber(tr->rotation.x));
    cJSON_AddItemToArray(rotation, cJSON_CreateNumber(tr->rotation.y));
    cJSON_AddItemToArray(rotation, cJSON_CreateNumber(tr->rotation.z));

    return obj;
}

void* TransformDecode(cJSON **data){
    Transform *tr = malloc(sizeof(Transform));
    cJSON *pos = cJSON_GetObjectItem(*data,"position");
    tr->position = (Vector3){(cJSON_GetArrayItem(pos,0))->valuedouble,
                             (cJSON_GetArrayItem(pos,1))->valuedouble,
                             (cJSON_GetArrayItem(pos,2))->valuedouble};

    cJSON *rot = cJSON_GetObjectItem(*data,"rotation");
    tr->rotation = (Vector3){(cJSON_GetArrayItem(rot,0))->valuedouble,
                             (cJSON_GetArrayItem(rot,1))->valuedouble,
                             (cJSON_GetArrayItem(rot,2))->valuedouble};

    return tr;
}

Vector3 GetPosition(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetPosition: Entity doesn't have a Transform component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    Transform *transform = (Transform *)ECS.Components[ThisComponentID()][entity].data;
    return transform->position;
}

Vector3 GetRotation(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetRotation: Entity doesn't have a Transform component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    Transform *transform = (Transform *)ECS.Components[ThisComponentID()][entity].data;
    return transform->rotation;
}

void SetPosition(EntityID entity, Vector3 pos){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetPosition: Entity doesn't have a Transform component. (%d)\n",entity);
        return;
    }
    Transform *transform = (Transform *)ECS.Components[ThisComponentID()][entity].data;
    transform->position = pos;
}

void SetRotation(EntityID entity, Vector3 rot){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetRotation: Entity doesn't have a Transform component. (%d)\n",entity);
        return;
    }
    Transform *transform = (Transform *)ECS.Components[ThisComponentID()][entity].data;
    transform->rotation = rot;
}

void GetGlobalTransform(EntityID entity, Vector3 *outPos, Vector3 *outRot){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetGlobalTransform: Entity doesn't have a Transform component. (%d)\n",entity);
        return;
    }
    Transform *transform = (Transform *)ECS.Components[ThisComponentID()][entity].data;

    if(EntityIsChild(entity)){
        Vector3 parentGlobalPos, parentGlobalRot;
        GetGlobalTransform(GetEntityParent(entity),&parentGlobalPos, &parentGlobalRot);

        Vector3 rot = (Vector3){parentGlobalRot.x,parentGlobalRot.y,parentGlobalRot.z};

        float sinx = sin(rot.x * PI_OVER_180);
        float cosx = cos(rot.x * PI_OVER_180);
        float siny = sin(rot.y * PI_OVER_180);
        float cosy = cos(rot.y * PI_OVER_180);
        float sinz = sin(rot.z * PI_OVER_180);
        float cosz = cos(rot.z * PI_OVER_180);

        float rxt1 = cosy*cosz, rxt2 = (cosz*sinx*siny - cosx*sinz), rxt3 = (cosx*cosz*siny + sinx*sinz);
        float ryt1 = cosy*sinz, ryt2 = (cosx*siny*sinz - cosz*sinx), ryt3 = (cosx*cosz + sinx*siny*sinz);
        float rzt1 = cosx*cosy, rzt2 = sinx*cosy,                    rzt3 = siny;
        
        Vector3 pos = transform->position;
        Vector3 rotatedPos = {pos.x, pos.y, pos.z};

        //Apply rotation matrix
        Vector3 p = (Vector3){rotatedPos.x, rotatedPos.y, rotatedPos.z};
        rotatedPos.x = p.x*rxt1 + p.z*rxt2 + p.y*rxt3;
        rotatedPos.y = p.z*ryt1 + p.x*ryt2 + p.y*ryt3;
        rotatedPos.z = p.z*rzt1 + p.x*rzt2 - p.y*rzt3;

        rotatedPos = (Vector3){ rotatedPos.x + parentGlobalPos.x, rotatedPos.y + parentGlobalPos.y, rotatedPos.z + parentGlobalPos.z};

        if(outPos) *outPos = rotatedPos;
        if(outRot) *outRot = Add(transform->rotation, parentGlobalRot);
        
    }else{
        if(outPos) *outPos = transform->position;
        if(outRot) *outRot = transform->rotation;
    }
}