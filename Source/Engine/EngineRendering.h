#ifndef ENGINERENDERING_H
#define ENGINERENDERING_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include "../utils.h"

#include "EngineCore.h"

typedef struct engineRendering{
    Vector3 cameraPosition;

    GLuint frameBuffer;
    GLuint screenTexture;
    GLuint depthRenderBuffer;
    GLuint vao2D, vbo2D[3];

    GLuint Shaders[4];

    Pixel voxelColors[256];
    SDL_Color clearScreenColor;
}engineRendering;

int InitRenderer();

//Rendering functions
Vector3 PositionToGameScreenCoords(Vector3 position);
Vector3 PositionToCameraCoords(Vector3 position);
void ClearRender(SDL_Color col);
void RenderToScreen();
void RenderTextDebug(char *text, SDL_Color color, int x, int y, TTF_Font* font);
int CompileAndLinkShader();
void ReloadShaders();
void LoadVoxelPalette(char path[]);
void MoveCamera(float x, float y, float z);
void TranslateCamera(float x, float y, float z);

#endif