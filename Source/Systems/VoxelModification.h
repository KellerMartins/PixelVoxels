#ifndef VOXELMODIFICATION_H
#define VOXELMODIFICATION_H
#include "../Engine.h"
#include "../Components/VoxelModel.h"

void VoxelModificationInit();
void VoxelModificationUpdate(EntityID entity);
void VoxelModificationFree();

#endif