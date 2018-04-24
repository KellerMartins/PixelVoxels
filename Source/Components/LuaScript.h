#ifndef LUASCRIPT_H
#define LUASCRIPT_H
#include "../Engine.h"

void LuaScriptConstructor(void** data);
void LuaScriptDestructor(void** data);
void* LuaScriptCopy(void* data);
cJSON* LuaScriptEncode(void** data, cJSON* currentData);
void* LuaScriptDecode(cJSON **data);

void SetLuaScript(EntityID entity, char* scriptPath, char* scriptName);
void SetLuaScriptIndex(EntityID entity, int index);

int GetLuaScriptIndex(EntityID entity);
char *GetLuaScriptName(EntityID entity);
char *GetLuaScriptPath(EntityID entity);

void LuaScriptRegisterLuaFunctions();

#endif