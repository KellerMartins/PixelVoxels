#ifndef EDITOR_H
#define EDITOR_H
#include "../Engine.h"
#include "../Components/Transform.h"
#include "../Libs/tinydir.h"
#include "UIRenderer.h"
#include <ctype.h>

#include "Editor/FileBrowser.h"
#include "Editor/DialogWindow.h"
#include "Editor/MenuWindow.h"
#include "Editor/ComponentsPanel.h"
#include "Editor/EntitiesPanel.h"
#include "Editor/PlayModePanel.h"
#include "Editor/EditorGizmos.h"
#include "Editor/EditorUI.h"

void EditorInit();
void EditorUpdate();
void EditorFree();

#endif