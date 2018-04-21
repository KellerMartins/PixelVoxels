#include "LuaSystem.h"

static System *ThisSystem;

extern engineECS ECS;
extern engineCore Core;

typedef struct LuaScript{
    char hasError;
    char* loopFunction;
    char scriptPath[512];
	char scriptName[256];
}LuaScript;

List luaScriptList;

//Runs on engine start
void LuaSystemInit(){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("LuaSystem"));
    luaScriptList = InitList(sizeof(LuaScript));
}

//Runs each GameLoop iteration
void LuaSystemUpdate(){
    lua_State *L = Core.lua;
    int status;

    //Run for all entities
    EntityID entity;
	for(entity = 0; entity <= ECS.maxUsedIndex; entity++){

        //Check if entity contains needed components
        if(!IsEmptyComponentMask(ThisSystem->required) && !EntityContainsMask(entity,ThisSystem->required) ) continue;
        //Check if entity doesn't contains the excluded components
        if(!IsEmptyComponentMask(ThisSystem->excluded) && EntityContainsMask(entity,ThisSystem->excluded) ) continue;

        int scriptIndex = GetLuaScriptIndex(entity);

        //If the script had problems, skip it
        if(scriptIndex == -2)
            continue;

        //If the entity hasnt loaded his script yet, load it first  
        if(scriptIndex == -1){
            scriptIndex = LoadNewScript(GetLuaScriptPath(entity), GetLuaScriptName(entity));
            if(scriptIndex<0){
                printf("LuaSystem: Failed to activate script!\n");
                SetLuaScriptIndex(entity,-2);
                continue;
            }
            SetLuaScriptIndex(entity,scriptIndex);
        }
        LuaScript *ls = (LuaScript*)GetElementAt(luaScriptList, scriptIndex);

        //Skip reloaded scripts with errors
        if(ls->hasError)
            continue;

        //Run loop function from Lua script
        lua_getglobal(L, ls->loopFunction );
        lua_pushinteger(L, entity);
        status = lua_pcall(L, 1, 0, 0);
        if (status) {
            fprintf(stderr, "LuaSystem: Failed to run script: %s\n", lua_tostring(L, -1));
            SetLuaScriptIndex(entity,-2);
        }
    }
}

//Runs at engine finish
void LuaSystemFree(){
    ListCellPointer cell = GetFirstCell(luaScriptList);
    while(cell){
        ListCellPointer aux = cell;
        cell = GetNextCell(cell);
        
        free(((LuaScript*)GetElement(*aux))->loopFunction);
    }
}

int LoadNewScript(char* scriptPath, char* scriptName){
    //Check if script isnt already in the list
    int i = 0;
    ListCellPointer cell;
    ListForEach(cell, luaScriptList){
        LuaScript script = GetElementAsType(cell, LuaScript);
        if(StringCompareEqual(script.scriptName, scriptName)){
            if(StringCompareEqual(script.scriptPath, scriptPath))
                return i;
        }
        i++;
    }

    //Concatenate the path and name to a full path and add an '/' to the path in case it is missing
    char fullPath[512+256];
    strncpy(fullPath,scriptPath,512);
    if(scriptPath[strlen(scriptPath)-1] != '/'){
        strcat(fullPath,"/");
    }
    strcat(fullPath,scriptName);

    lua_State *L = Core.lua;

    //Load the new script chunk into Lua
    if(luaL_loadfile(L, fullPath)) {
        printf("LoadNewScript: Couldn't load file: %s\n", lua_tostring(L, -1));
        return -2;
    }

    //Prime run script to get the functions and to define variables
    if (lua_pcall(L, 0, 1, 0)) {
        printf("LoadNewScript: Failed to run script: %s\n", lua_tostring(L, -1));
        return -2;
    }

    //If the value returned is not a string with the name of the loop function, ignore this script
    if(lua_type(L, -1) != LUA_TSTRING) return -1;

    //Get returned value as string
    const char* functionName = lua_tostring(L, -1);
    int functionNameLength = strlen(functionName);

    //Copy the string to the struct, and add it to the list
    LuaScript newScript;
    newScript.hasError = 0;
    newScript.loopFunction = malloc(functionNameLength * sizeof(char));
    strncpy(newScript.loopFunction, functionName, functionNameLength);
    strcpy(newScript.scriptName, scriptName);
    strcpy(newScript.scriptPath, scriptPath);

    InsertListEnd(&luaScriptList, &newScript);
    
    //Removed return value from stack
    lua_pop(L, 1);
    return GetLength(luaScriptList)-1;
}

void ReloadAllScripts(){
    char fullPath[512+256];

    ListCellPointer cell;
    ListForEach(cell, luaScriptList){
        LuaScript *ls = GetElement(*cell);
        
        //Concatenate path and file name to fullPath
        strncpy(fullPath,ls->scriptPath,512);
        if(ls->scriptPath[strlen(ls->scriptPath)-1] != '/'){
            strcat(fullPath,"/");
        }
        strcat(fullPath,ls->scriptName);

        lua_State *L = Core.lua;

        //Reload the script chunk into Lua
        if(luaL_loadfile(L, fullPath)) {
            printf("ReloadAllScripts: Couldn't load file (%s): %s\n", ls->scriptName, lua_tostring(L, -1));
            ls->hasError = 1;
            continue;
        }

        //Prime run script to get the functions and to define variables
        if (lua_pcall(L, 0, 1, 0)) {
            printf("ReloadAllScripts: Failed to run script (%s): %s\n", ls->scriptName, lua_tostring(L, -1));
            ls->hasError = 1;
            continue;
        }

        //If the value returned is not a string with the name of the loop function, ignore this script
        if(lua_type(L, -1) != LUA_TSTRING){
            printf("ReloadAllScripts: Returned value is not a string (%s): %s\n", ls->scriptName, lua_tostring(L, -1));
            ls->hasError = 1;
            continue;
        }

        //Get returned value as string
        const char* functionName = lua_tostring(L, -1);
        int functionNameLength = strlen(functionName);

        //Copy the string to the struct, and add it to the list
        free(ls->loopFunction);
        ls->loopFunction = malloc(functionNameLength * sizeof(char));
        strncpy(ls->loopFunction, functionName, functionNameLength);
        
        //Removed return value from stack
        lua_pop(L, 1);

        ls->hasError = 0;
    }
}