#ifndef VOXELRENDERER_H
#define VOXELRENDERER_H
#include "../Engine.h"
#include "../Components/VoxelModel.h"
#include "PointLighting.h"
#include "Shadows.h"

void VoxelRendererInit();
void VoxelRendererUpdate();
void VoxelRendererFree();

#endif