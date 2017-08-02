#ifndef VOXELRENDERER_H
#define VOXELRENDERER_H
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "utils.h"

typedef struct Pixel{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
	
}Pixel;

typedef struct AnchorPoint{
	char type;
  	char x;
	char y;
	char z;
}AnchorPoint;

typedef struct VoxelObject{
	int enabled;
	unsigned int timeOfActivation;

	char modificationStartX;
	char modificationEndX;

	char modificationStartY;
	char modificationEndY;

	char modificationStartZ;
	char modificationEndZ;

    unsigned char *model;
    unsigned char *lighting;
    unsigned short int **render;
	int numberOfPoints;
	AnchorPoint *points;

	unsigned int dimension[3];
	unsigned int maxDimension;

    unsigned int voxelCount;
	unsigned int voxelsRemaining;

	Vector3 position;
}VoxelObject;

typedef struct ObjectList{
	VoxelObject *list;
	unsigned int numberOfObjects;
}ObjectList;

typedef struct RendererArguments{
	Pixel *screen;
	VoxelObject **objs;
	unsigned int numObjs;
	VoxelObject **shadowCasters;
	unsigned int numCasters;
}RendererArguments;

typedef struct LightingArguments{
	VoxelObject *obj;
	VoxelObject *shadowCaster;
}LightingArguments;

typedef struct Ray{
	float origin[3];
	float direction[3];
	float inverseDirection[3];
}Ray;

void MoveCamera(float x, float y, float z);

void ClearScreen(Pixel* screen);
void FillBackground(Pixel* screen);
void PostProcess(Pixel* screen);

void *RenderThread(void *arguments);

void CalculateRendered(VoxelObject *obj);
void CalculateLighting(VoxelObject *obj);
void CalculateShadow(VoxelObject *obj,VoxelObject *shadowCaster);
void RenderObject(Pixel* screen,VoxelObject *obj);
#endif