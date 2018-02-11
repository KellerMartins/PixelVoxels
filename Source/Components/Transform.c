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
    if(EntityContainsComponent(entity, GetComponentID("ParentChild"))){
        if(IsParent(entity)){
            Vector3 deltaPos = Subtract(transform->position,oldPos);
            
            ParentChild *parentComp = ECS.Components[GetComponentID("ParentChild")][entity].data;
            
            ListCellPointer current = GetFirstCell(parentComp->childs);
            while(current){
                EntityID child = *((EntityID*) GetElement(*current));

                if(EntityContainsComponent(child, ThisComponentID())){
                    SetPosition(child, Add(GetPosition(child),deltaPos));
                }
                current = GetNextCell(current);
            }
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