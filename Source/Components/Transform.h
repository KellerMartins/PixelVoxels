#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "../Engine.h"
#include "ParentChild.h"

void TransformConstructor(void** data);
void TransformDestructor(void** data);
void* TransformCopy(void* data);

Vector3 GetPosition(EntityID entity);
Vector3 GetRotation(EntityID entity);

void SetPosition(EntityID entity, Vector3 pos);
void SetRotation(EntityID entity, Vector3 rot);

#endif