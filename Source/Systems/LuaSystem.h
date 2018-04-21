#ifndef LUASYSTEM_H
#define LUASYSTEM_H
#include "../Engine.h"
#include "../Components/LuaScript.h"
void LuaSystemInit();
void LuaSystemUpdate();
void LuaSystemFree();

int LoadNewScript(char* scriptPath, char* scriptName);
void ReloadAllScripts();

#endif