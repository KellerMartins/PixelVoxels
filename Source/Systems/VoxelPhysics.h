#ifndef VOXELPHYSICS_H
#define VOXELPHYSICS_H

//Number of voxels to one meter
//A scale of 10 means that 10 voxels is equal to one meter (each voxel is 10 centimeters),
//and a scale of one would mean that each voxel is one meter
//Following that logic, a scale of 0.1 means each voxel is ten meters
#define WORLD_SCALE 10

#include "../Engine.h"
#include "../Components/VoxelModel.h"
#include "../Components/RigidBody.h"

double GetGravity();
void SetGravity(double g);

void VoxelPhysicsInit();
void VoxelPhysicsUpdate();
void VoxelPhysicsFree();

void VoxelPhysicsRegisterLuaFunctions();

#endif