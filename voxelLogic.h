#ifndef VOXELLOGIC_H
#define VOXELLOGIC_H
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "voxelRenderer.h"
#include "voxelLoader.h"
#include "utils.h"

#define POOLSIZE 2

#define GetKey(n) keyboard_current[n]
#define GetKeyDown(n) (keyboard_current[n] && !keyboard_last[n])
#define GetKeyUp(n) (!keyboard_current[n] && keyboard_last[n])

typedef enum ObjectType{PLAYER,BULLET,ENEMY}ObjectType;

typedef struct PoolObject{
    int avaliableInstances;
    ObjectType type;
    VoxelObject baseObj;
    VoxelObjectList objs;
    
}PoolObject;
void InputStart();
void InputUpdate();
void FreeInput();

void GameStart();
void GameUpdate();

void InitializePool();
void FreePool();
void PoolUpdate();
void Spawn(unsigned int index,float x, float y, float z, float rx, float ry, float rz);

void MoveObject(VoxelObject *obj, Vector3 movement, Vector3 rotation, VoxelObject **col,const int numCol,int damageColRadius,int damageObjRadius);
void MoveObjectTo(VoxelObject *obj, Vector3 movement, Vector3 rotation,	VoxelObject **col,const int numCol,int damageColRadius,int damageObjRadius);

void ExplodeAtPoint(VoxelObject *obj,int x, int y, int z,int radius);
VoxelObject **VoxelPointerArrayUnion(int numberOfPointers,int totalPointerSize,...);

#endif