#ifndef POINTLIGHT_H
#define POINTLIGHT_H
#include "../Engine.h"

typedef struct PointLightData{
    Vector3 color;
    float intensity;
    float range;
}PointLightData;

void PointLightConstructor(void** data);
void PointLightDestructor(void** data);
void* PointLightCopy(void* data);
cJSON* PointLightEncode(void** data);
void* PointLightDecode(cJSON **data);

#endif