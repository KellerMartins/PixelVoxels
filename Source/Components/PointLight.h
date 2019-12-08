#ifndef POINTLIGHT_H
#define POINTLIGHT_H
#include "../Engine.h"

typedef struct PointLightData{
    Vector3 color;
    float intensity;
    float range;
    float hueShift;
}PointLightData;

void PointLightConstructor(void** data);
void PointLightDestructor(void** data);
void* PointLightCopy(void* data);
cJSON* PointLightEncode(void** data, cJSON* currentData);
void* PointLightDecode(cJSON **data);

Vector3 GetPointLightColor(EntityID entity);
void SetPointLightColor(EntityID entity, Vector3 rgbColor);

float GetPointLightIntensity(EntityID entity);
void SetPointLightIntensity(EntityID entity, float intensity);

float GetPointLightRange(EntityID entity);
void SetPointLightRange(EntityID entity, float range);

float GetPointLightHueShift(EntityID entity);
void SetPointLightHueShift(EntityID entity, float shift);

#endif