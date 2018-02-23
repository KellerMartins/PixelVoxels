#ifndef EDITOR_H
#define EDITOR_H
#include "../Engine.h"
#include "../Components/VoxelModel.h"
#include "../Components/RigidBody.h"
#include "../Libs/tinydir.h"
#include <ctype.h>

void EditorInit();
void EditorUpdate();
void EditorFree();

#endif