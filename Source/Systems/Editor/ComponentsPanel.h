#ifndef COMPONENTSPANEL_H
#define COMPONENTSPANEL_H

#include "../../Engine.h"
#include "../UIRenderer.h"
#include "../../Components/VoxelModel.h"
#include "../../Components/RigidBody.h"
#include "../../Components/PointLight.h"
#include "../../Components/LuaScript.h"

#include "FileBrowser.h"

void DrawComponentsPanel();
int DrawComponentHeader(ComponentID component, int* curHeight);

#endif