#ifndef VOXELRENDERER_H
#define VOXELRENDERER_H
#include "../Engine.h"
#include "../Components/VoxelModel.h"

void VoxelRendererInit(System *systemObject);
void VoxelRendererUpdate();
void VoxelRendererFree();

#endif