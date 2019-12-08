#ifndef LUASYSTEM_H
#define LUASYSTEM_H
#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>

#include "../Engine.h"
#include "../Components/LuaScript.h"

void LuaSystemInit();
void LuaSystemUpdate();
void LuaSystemFree();

int LoadNewScript(char* scriptPath, char* scriptName, EntityID entity);
int ReloadAllScripts();

#endif