#include "Transform.h"

//Can't exist without being an component from an entity
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
    Vector3 oldPos = transform->position;
    transform->position = pos;

    //Modify childs position too
    if(EntityIsParent(entity)){
        Vector3 deltaPos = Subtract(transform->position,oldPos);
        
        ListCellPointer current = GetFirstCell(ECS.Entities[entity].childs);
        while(current){
            EntityID child = *((EntityID*) GetElement(*current));

            if(EntityContainsComponent(child, ThisComponentID())){
                SetPosition(child, Add(GetPosition(child),deltaPos));
            }
            current = GetNextCell(current);
        }
    }
}

void SetRotation(EntityID entity, Vector3 rot){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetRotation: Entity doesn't have a Transform component. (%d)\n",entity);
        return;
    }
    Transform *transform = (Transform *)ECS.Components[ThisComponentID()][entity].data;
    transform->rotation = rot;
}