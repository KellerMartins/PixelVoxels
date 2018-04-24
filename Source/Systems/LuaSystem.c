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

        //If the entity hasnt loaded his script yet, load it first  
        if(scriptIndex == -1){
            scriptIndex = LoadNewScript(GetLuaScriptPath(entity), GetLuaScriptName(entity), entity);
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
            ls->hasError = 1;
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

int LoadNewScript(char* scriptPath, char* scriptName, EntityID entity){
    lua_State *L = Core.lua;
    
    //Concatenate the path and name to a full path and add an '/' to the path in case it is missing
    char fullPath[512+256];
    strncpy(fullPath,scriptPath,512);
    if(scriptPath[strlen(scriptPath)-1] != '/'){
        strcat(fullPath,"/");
    }
    strcat(fullPath,scriptName);

    //Check if script isnt already in the list
    int i = 0;
    ListCellPointer cell;
    ListForEach(cell, luaScriptList){
        LuaScript *script = GetElement(*cell);
        if(StringCompareEqual(script->scriptName, scriptName)){
            if(StringCompareEqual(script->scriptPath, scriptPath)){
                //Script is already loaded
                //If it has errors, just return its index and dont run in the new environment
                if(script->hasError){
                    return i;
                }

                //If the script is error free, just reload and prime run it in the new environment
                if(luaL_loadfile(L, fullPath)) {
                    printf("LoadNewScript: Couldn't reload file (%s): %s\n", scriptName, lua_tostring(L, -1));
                    script->hasError = 1;
                    return i;
                }

                //Prime run script to get the functions and to define variables
                if (lua_pcall(L, 0, 1, 0)) {
                    printf("LoadNewScript: Failed to re-run script (%s): %s\n", scriptName, lua_tostring(L, -1));
                    script->hasError = 1;
                }
                return i;
            }
        }
        i++;
    }

    //Load the new script chunk into Lua
    if(luaL_loadfile(L, fullPath)) {
        printf("LoadNewScript: Couldn't load file (%s): %s\n", scriptName, lua_tostring(L, -1));
        
        //Put the script on the list, but mark as with error
        LuaScript newScript;
        newScript.hasError = 1;
        newScript.loopFunction = NULL;
        strcpy(newScript.scriptName, scriptName);
        strcpy(newScript.scriptPath, scriptPath);

        InsertListEnd(&luaScriptList, &newScript);

        return GetLength(luaScriptList)-1;
    } 

    //Prime run script to get the functions and to define variables
    if (lua_pcall(L, 0, 1, 0)) {
        printf("LoadNewScript: Failed to run script (%s): %s\n", scriptName, lua_tostring(L, -1));

        //Put the script on the list, but mark as with error
        LuaScript newScript;
        newScript.hasError = 1;
        newScript.loopFunction = NULL;
        strcpy(newScript.scriptName, scriptName);
        strcpy(newScript.scriptPath, scriptPath);

        InsertListEnd(&luaScriptList, &newScript);

        return GetLength(luaScriptList)-1;
    }

    //If the value returned is not a string with the name of the loop function, mark as with error
    if(lua_type(L, -1) != LUA_TSTRING){
        printf("LoadNewScript: Returned value is not a string (%s): %s\n", scriptName, lua_tostring(L, -1));

        //Put the script on the list, but mark as with error
        LuaScript newScript;
        newScript.hasError = 1;
        newScript.loopFunction = NULL;
        strcpy(newScript.scriptName, scriptName);
        strcpy(newScript.scriptPath, scriptPath);

        InsertListEnd(&luaScriptList, &newScript);

        return GetLength(luaScriptList)-1;
    }

    //If no errors, just put on the list

    //Get returned value as string
    const char* functionName = lua_tostring(L, -1);
    int functionNameLength = strlen(functionName)+1;

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

int ReloadAllScripts(){
    int noErrors = 1;
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
            noErrors = 0;
            continue;
        }

        //Prime run script to get the functions and to define variables
        if (lua_pcall(L, 0, 1, 0)) {
            printf("ReloadAllScripts: Failed to run script (%s): %s\n", ls->scriptName, lua_tostring(L, -1));
            ls->hasError = 1;
            noErrors = 0;
            continue;
        }

        //If the value returned is not a string with the name of the loop function, ignore this script
        if(lua_type(L, -1) != LUA_TSTRING){
            printf("ReloadAllScripts: Returned value is not a string (%s): %s\n", ls->scriptName, lua_tostring(L, -1));
            ls->hasError = 1;
            noErrors = 0;
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

    return noErrors;
}