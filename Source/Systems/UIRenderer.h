#ifndef UIRENDERER_H
#define UIRENDERER_H
#include "../Engine.h"

void UIRendererInit();
void UIRendererUpdate();
void UIRendererFree();

void MorphRectangle(Vector3 vertices[4], Vector3 min, Vector3 max);

#endif