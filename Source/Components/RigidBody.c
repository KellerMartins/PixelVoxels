#include "RigidBody.h"

//Can't exist without being an component from an entity
typedef struct RigidBody{
    float mass;
    float bounciness;
    Vector3 velocity;
    Vector3 acceleration;
    int useGravity;
    int isStatic;
}RigidBody;

//Function that returns the ID of this component
static ComponentID ThisComponentID(){
    static ComponentID CompID = -1;
    if(CompID<0)
        CompID = GetComponentID("RigidBody");
    
    return CompID;
}

extern engineCore Core;
extern engineECS ECS;

//Engine component manipulation functions

void RigidBodyConstructor(void** data){
    if(!data) return;
    *data = calloc(1,sizeof(RigidBody));
    ((RigidBody*) *data)->mass = 1;
    ((RigidBody*) *data)->useGravity = 1;
    ((RigidBody*) *data)->bounciness = 0.2;
}

void RigidBodyDestructor(void** data){
    if(!data) return;
    free(*data);
    *data = NULL;
}

void* RigidBodyCopy(void* data){
    if(!data) return NULL;
    RigidBody *newRigidBody = malloc(sizeof(RigidBody));
    memcpy(newRigidBody,data,sizeof(RigidBody));
	return newRigidBody;
}

cJSON* RigidBodyEncode(void** data, cJSON* currentData){
    if(!data) return NULL;
    RigidBody *rb = *data;
    
    
    int hasChanged = 0;
    if(currentData){
        //Check if any data has changed
        cJSON *curVelocity = cJSON_GetObjectItem(currentData,"velocity");
        if(rb->velocity.x != (cJSON_GetArrayItem(curVelocity,0))->valuedouble ||
           rb->velocity.y != (cJSON_GetArrayItem(curVelocity,1))->valuedouble || 
           rb->velocity.z != (cJSON_GetArrayItem(curVelocity,2))->valuedouble)
        {
            hasChanged = 1;
        }

        cJSON *curAcceleration = cJSON_GetObjectItem(currentData,"acceleration");
        if(rb->acceleration.x != (cJSON_GetArrayItem(curAcceleration,0))->valuedouble ||
           rb->acceleration.y != (cJSON_GetArrayItem(curAcceleration,1))->valuedouble || 
           rb->acceleration.z != (cJSON_GetArrayItem(curAcceleration,2))->valuedouble)
        {
            hasChanged = 1;
        }

        cJSON *curUseGravity= cJSON_GetObjectItem(currentData,"useGravity");
        if(rb->useGravity != cJSON_IsTrue(curUseGravity)){
            hasChanged = 1;
        }

        cJSON *curIsStatic= cJSON_GetObjectItem(currentData,"isStatic");
        if(rb->isStatic != cJSON_IsTrue(curIsStatic)){
            hasChanged = 1;
        }

        if(rb->mass != (cJSON_GetObjectItem(currentData,"mass"))->valuedouble){
            hasChanged = 1;
        }

        if(rb->bounciness != (cJSON_GetObjectItem(currentData,"bounciness"))->valuedouble){
            hasChanged = 1;
        }
    }

    //Encode this component if its not from a prefab (who has currentData) or if it has changed
    if(!currentData || hasChanged){           
        cJSON *obj = cJSON_CreateObject();

        cJSON_AddNumberToObject(obj,"mass",rb->mass);
        cJSON_AddNumberToObject(obj,"bounciness",rb->bounciness);

        cJSON *velocity = cJSON_AddArrayToObject(obj,"velocity");
        cJSON_AddItemToArray(velocity, cJSON_CreateNumber(rb->velocity.x));
        cJSON_AddItemToArray(velocity, cJSON_CreateNumber(rb->velocity.y));
        cJSON_AddItemToArray(velocity, cJSON_CreateNumber(rb->velocity.z));

        cJSON *acceleration = cJSON_AddArrayToObject(obj,"acceleration");
        cJSON_AddItemToArray(acceleration, cJSON_CreateNumber(rb->acceleration.x));
        cJSON_AddItemToArray(acceleration, cJSON_CreateNumber(rb->acceleration.y));
        cJSON_AddItemToArray(acceleration, cJSON_CreateNumber(rb->acceleration.z));

        cJSON_AddBoolToObject(obj,"useGravity",rb->useGravity);
        cJSON_AddBoolToObject(obj,"isStatic",rb->isStatic);

        return obj;
    }
    return NULL;
}

void* RigidBodyDecode(cJSON **data){
    RigidBody *rb = malloc(sizeof(RigidBody));

    rb->mass = (cJSON_GetObjectItem(*data,"mass"))->valuedouble;
    rb->bounciness = (cJSON_GetObjectItem(*data,"bounciness"))->valuedouble;

    cJSON *vel = cJSON_GetObjectItem(*data,"velocity");
    rb->velocity = (Vector3){(cJSON_GetArrayItem(vel,0))->valuedouble,
                             (cJSON_GetArrayItem(vel,1))->valuedouble,
                             (cJSON_GetArrayItem(vel,2))->valuedouble};

    cJSON *acc = cJSON_GetObjectItem(*data,"acceleration");
    rb->acceleration = (Vector3){(cJSON_GetArrayItem(acc,0))->valuedouble,
                                 (cJSON_GetArrayItem(acc,1))->valuedouble,
                                 (cJSON_GetArrayItem(acc,2))->valuedouble};

    rb->useGravity = cJSON_IsTrue(cJSON_GetObjectItem(*data,"useGravity"));
    rb->isStatic = cJSON_IsTrue(cJSON_GetObjectItem(*data,"isStatic"));

    return rb;
}


//Component interface functions

Vector3 GetVelocity(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"GetVelocity: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->velocity;
}

Vector3 GetAcceleration(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"GetAcceleration: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->acceleration;
}

float GetMass(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"GetMass: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return 1;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->isStatic? STATIC_OBJECT_MASS : rb->mass;
}

float GetBounciness(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"GetBounciness: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return 1;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->bounciness;
}

void SetVelocity(EntityID entity, Vector3 vel){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"SetVelocity: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->velocity = vel;
}

void SetAcceleration(EntityID entity, Vector3 acc){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"SetAcceleration: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->acceleration = acc;
}

void SetMass(EntityID entity, float mass){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"SetMass: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->mass = clamp(mass,MASS_MIN,INFINITY);
}

void SetBounciness(EntityID entity, float bounciness){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"SetBounciness: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->bounciness = clamp(bounciness,0.2,1);
}

int UsesGravity(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"UsesGravity: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return 0;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->useGravity;
}

void SetUseGravity(EntityID entity,unsigned booleanVal){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"SetUseGravity: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->useGravity = booleanVal?1:0;
}

int IsStaticRigidBody(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"IsStaticRigidBody: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return 0;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->isStatic;
}

void SetStaticRigidBody(EntityID entity,unsigned booleanVal){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"SetStaticRigidBody: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->isStatic = booleanVal?1:0;
}
