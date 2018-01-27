#ifndef VOXELRENDERER_H
#define VOXELRENDERER_H
#include "../Engine.h"
#include "../Components/VoxelModel.h"

void VoxelRendererInit();
void VoxelRendererUpdate(EntityID entity);
void VoxelRendererFree();

#endif