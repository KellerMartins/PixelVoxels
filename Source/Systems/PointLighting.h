#ifndef POINTLIGHTING_H
#define POINTLIGHTING_H
#include "../Engine.h"
#include "../Components/Transform.h"
#include "../Components/PointLight.h"

#define MAX_POINT_LIGHTS 10

GLuint GetPointLightsBuffer();

void PointLightingInit();
void PointLightingUpdate();
void PointLightingFree();

#endif