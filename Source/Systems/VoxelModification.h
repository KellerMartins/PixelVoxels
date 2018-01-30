#ifndef VOXELMODIFICATION_H
#define VOXELMODIFICATION_H
#include "../Engine.h"
#include "../Components/VoxelModel.h"

void VoxelModificationInit(System *systemObject);
void VoxelModificationUpdate();
void VoxelModificationFree();

#endif