#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "../Engine.h"

void TransformConstructor(void** data);
void TransformDestructor(void** data);
void* TransformCopy(void* data);
cJSON* TransformEncode(void** data, cJSON* currentData);
void* TransformDecode(cJSON **data);

Vector3 GetPosition(EntityID entity);
Vector3 GetRotation(EntityID entity);
Matrix3x3 GetRotationMatrix(EntityID entity);

void GetGlobalTransform(EntityID entity, Vector3 *outPos, Vector3 *outEuler, Matrix3x3 *outRot);

void SetPosition(EntityID entity, Vector3 pos);
void SetRotation(EntityID entity, Vector3 rot);


#endif