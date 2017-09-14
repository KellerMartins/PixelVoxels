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
    int numberOfInstances;
    int avaliableInstances;
    ObjectType type;
    VoxelObject baseObj;
    VoxelObject **objs;
    
}PoolObject;
void InputStart();
void InputUpdate();
void FreeInput();

void GameStart();
void GameUpdate();

void InitializePool();
void FreePool();
void PoolUpdate();

void Spawn(unsigned int index,int x, int y, int z);

void MoveObject(VoxelObject *obj,float x, float y, float z,	VoxelObject **col,const int numCol,int damageColRadius,int damageObjRadius);
void ExplodeAtPoint(VoxelObject *obj,int x, int y, int z,int radius);
VoxelObject **VoxelPointerArrayUnion(int numberOfPointers,int totalPointerSize,...);

#endif