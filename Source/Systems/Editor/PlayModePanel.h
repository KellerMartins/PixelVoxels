#ifndef PLAYMODEPANEL_H
#define PLAYMODEPANEL_H

#include "../../Engine.h"
#include "../UIRenderer.h"

#include "EditorUI.h"

void AllocatePlayModeData();
void FreePlayModeData();

void DrawPlayModeWidget();

void EnterPlayMode();
void ExitPlayMode();
void EnablePlayModeSystems();
void DisablePlayModeSystems();

#endif