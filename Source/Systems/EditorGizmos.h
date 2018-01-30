#ifndef EDITORSYSTEM_H
#define EDITORSYSTEM_H
#include "../Engine.h"
#include "../Components/VoxelModel.h"

void EditorGizmosInit(System *systemObject);
void EditorGizmosUpdate();
void EditorGizmosFree();

#endif