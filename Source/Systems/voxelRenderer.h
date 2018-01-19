#ifndef VOXELRENDERER_H
#define VOXELRENDERER_H
#include "../Engine.h"
#include "../Components/voxelModel.h"

typedef struct Pixel{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
	
}Pixel;

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

void CalculateRendered(VoxelObject *obj);
void CalculateLighting(VoxelObject *obj);
void CalculateShadow(VoxelObject *obj,VoxelObject *shadowCaster);
//void PointLight(VoxelObject *obj,int x, int y, int z,int radius);

void RenderText(char *text, SDL_Color color, int x, int y, TTF_Font* font);
SDL_Texture* RenderIcon(VoxelObject *obj);

void SaveTextureToPNG(SDL_Texture *tex, char* out);

int CompileAndLinkShader();
void ReloadShaders();
void LoadPalette(char path[]);
#endif