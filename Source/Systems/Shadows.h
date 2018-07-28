#ifndef SHADOWS_H
#define SHADOWS_H
#include "../Engine.h"
#include "UIRenderer.h"
#include "../Components/VoxelModel.h"

void ShadowsInit();
void ShadowsUpdate();
void ShadowsFree();

GLuint GetShadowDepthTexture();
void ShadowViewMatrix(GLfloat viewMatrix[4][4]);

#endif