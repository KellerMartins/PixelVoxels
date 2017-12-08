#ifndef VOXELRENDERER_H
#define VOXELRENDERER_H
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
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
	unsigned int maxDimension;

    unsigned int voxelCount;
	unsigned int voxelsRemaining;

	Vector3 position;
	Vector3 rotation;
}VoxelObject;

typedef struct ObjectList{
	VoxelObject *list;
	unsigned int numberOfObjects;
}ObjectList;

typedef struct RendererArguments{
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

void ClearScreen();
void FillBackground();
void PostProcess();

void InitRenderer(Uint16 *dpth);
void UpdateScreenPointer(Pixel* scrn);
void FreeRenderer();
void *RenderThread(void *arguments);
//void PointLight(VoxelObject *obj,int x, int y, int z,int radius);
void CalculateRendered(VoxelObject *obj);
void CalculateLighting(VoxelObject *obj);
void CalculateShadow(VoxelObject *obj,VoxelObject *shadowCaster);
void RenderObject(VoxelObject *obj);

void RenderToScreen();
void ClearRender(SDL_Color col);

void RenderText(char *text, SDL_Color color, int x, int y, TTF_Font* font);

int CompileAndLinkShader();

SDL_Texture* RenderIcon(VoxelObject *obj);
void SaveTextureToPNG(SDL_Texture *tex, char* out);
#endif