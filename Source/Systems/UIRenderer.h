#ifndef UIRENDERER_H
#define UIRENDERER_H
#include "../Engine.h"

void UIRendererInit();
void UIRendererUpdate();
void UIRendererFree();

void DrawTextColored(char *text, Vector3 color, int x, int y, TTF_Font* font);
void DrawRectangle(Vector3 min, Vector3 max, float r, float g, float b);
void DrawRectangleTextured(Vector3 min, Vector3 max, GLuint texture, float r, float g, float b);
void DrawPoint(Vector3 pos, float size, GLuint texture, float r, float g, float b);
void DrawLine(Vector3 min, Vector3 max, float thickness, float r, float g, float b);

#endif