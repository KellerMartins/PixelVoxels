#ifndef RIGIDBODY_H
#define RIGIDBODY_H
#include "../Engine.h"

void RigidBodyConstructor(EntityID entity);
void RigidBodyDestructor(EntityID entity);

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