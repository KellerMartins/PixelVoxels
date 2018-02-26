#ifndef RIGIDBODY_H
#define RIGIDBODY_H
#include "../Engine.h"

#define MASS_MIN 0.000000001
#define STATIC_OBJECT_MASS 100000000000

void RigidBodyConstructor(void** data);
void RigidBodyDestructor(void** data);
void* RigidBodyCopy(void* data);
cJSON* RigidBodyEncode(void** data);
void* RigidBodyDecode(cJSON **data);

Vector3 GetVelocity(EntityID entity);
Vector3 GetAcceleration(EntityID entity);
float GetMass(EntityID entity);
float GetBounciness(EntityID entity);

void SetVelocity(EntityID entity, Vector3 vel);
void SetAcceleration(EntityID entity, Vector3 acc);
void SetMass(EntityID entity, float mass);
void SetBounciness(EntityID entity, float bounciness);

int UsesGravity(EntityID entity);
void SetUseGravity(EntityID entity,unsigned booleanVal);

int IsStaticRigidbody(EntityID entity);
void SetStaticRigidbody(EntityID entity,unsigned booleanVal);

#endif