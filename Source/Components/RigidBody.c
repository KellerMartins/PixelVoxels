#include "RigidBody.h"
#define STATIC_OBJECT_MASS 100000000000

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

void RigidBodyConstructor(EntityID entity){
    ECS.Components[ThisComponentID()][entity].data = calloc(1,sizeof(RigidBody));
}

void RigidBodyDestructor(EntityID entity){
    free(ECS.Components[ThisComponentID()][entity].data);
    ECS.Components[ThisComponentID()][entity].data = NULL;
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
    rb->mass = mass;
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