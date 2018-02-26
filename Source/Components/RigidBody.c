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

extern engineECS ECS;

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

cJSON* RigidBodyEncode(void** data){
    RigidBody *rb = *data; 
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

Vector3 GetVelocity(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetVelocity: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->velocity;
}

Vector3 GetAcceleration(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetAcceleration: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->acceleration;
}

float GetMass(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetMass: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return 1;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->isStatic? STATIC_OBJECT_MASS : rb->mass;
}

float GetBounciness(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetBounciness: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return 1;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->bounciness;
}

void SetVelocity(EntityID entity, Vector3 vel){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetVelocity: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->velocity = vel;
}

void SetAcceleration(EntityID entity, Vector3 acc){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetAcceleration: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->acceleration = acc;
}

void SetMass(EntityID entity, float mass){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetMass: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->mass = clamp(mass,MASS_MIN,INFINITY);
}

void SetBounciness(EntityID entity, float bounciness){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetBounciness: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->bounciness = clamp(bounciness,0.2,1);
}

int UsesGravity(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("UsesGravity: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return 0;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->useGravity;
}

void SetUseGravity(EntityID entity,unsigned booleanVal){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetUseGravity: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->useGravity = booleanVal?1:0;
}

int IsStaticRigidbody(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("IsStaticRigidbody: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return 0;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->isStatic;
}

void SetStaticRigidbody(EntityID entity,unsigned booleanVal){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetStaticRigidbody: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->isStatic = booleanVal?1:0;
}