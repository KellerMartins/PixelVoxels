#ifndef ENTITIESPANEL_H
#define ENTITIESPANEL_H

#include "../../Engine.h"
#include "../UIRenderer.h"

#include "FileBrowser.h"
#include "EditorUI.h"

void DrawEntitiesPanel();
void DrawEntityElement(EntityID entity, int *entityHeight, int depth);

#endif