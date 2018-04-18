#ifndef UIRENDERER_H
#define UIRENDERER_H
#include "../Engine.h"

void UIRendererInit();
void UIRendererUpdate();
void UIRendererFree();

void DrawText(char *text, SDL_Color color, int x, int y, TTF_Font* font);
void DrawRectangle(Vector3 min, Vector3 max, float r, float g, float b);
void MorphRectangle(Vector3 vertices[4], Vector3 min, Vector3 max);

#endif