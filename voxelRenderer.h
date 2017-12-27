#ifndef VOXELRENDERER_H
#define VOXELRENDERER_H
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
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

    unsigned int voxelCount;
	unsigned int voxelsRemaining;

	Vector3 position;
	Vector3 rotation;
	Vector3 center;
}VoxelObject;

//A simpler list implementation, where each element of the list is a pointer to
//an object. As this list is rarely changed during the game loop, I opted to keep
//it simple instead of using the other, more general list implementation on this
typedef struct VoxelObjectList{
	VoxelObject **list;
	unsigned int numberOfObjects;
}VoxelObjectList;

typedef struct MultiVoxelObject{
	int enabled;
	VoxelObjectList objects;

	Vector3 position;
	Vector3 rotation;
	Vector3 center;

}MultiVoxelObject;

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

void ClearScreen();
void FillBackground();
void PostProcess();

void InitRenderer();
void FreeRenderer();

void RenderToScreen();
void ClearRender(SDL_Color col);
void RenderObject(VoxelObject *obj);

void FreeObject(VoxelObject *obj);

VoxelObjectList InitializeObjectList();
void FreeObjectList(VoxelObjectList *list);
void AddObjectInList(VoxelObjectList *dest, VoxelObject *obj);
void RenderObjectList(VoxelObjectList objs, VoxelObjectList shadowCasters);
void CombineObjectLists(VoxelObjectList *dest,  int numberOfSources,...);


void CalculateRendered(VoxelObject *obj);
void CalculateLighting(VoxelObject *obj);
void CalculateShadow(VoxelObject *obj,VoxelObject *shadowCaster);
//void PointLight(VoxelObject *obj,int x, int y, int z,int radius);

void RenderText(char *text, SDL_Color color, int x, int y, TTF_Font* font);
SDL_Texture* RenderIcon(VoxelObject *obj);

void SaveTextureToPNG(SDL_Texture *tex, char* out);

int CompileAndLinkShader();
void LoadPalette(char path[]);
#endif