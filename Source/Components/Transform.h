#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "../Engine.h"
#include "ParentChild.h"

void TransformConstructor(EntityID entity);
void TransformDestructor(EntityID entity);

Vector3 GetPosition(EntityID entity);
Vector3 GetRotation(EntityID entity);

void SetPosition(EntityID entity, Vector3 pos);
void SetRotation(EntityID entity, Vector3 rot);

#endif