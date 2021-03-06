#include "LuaSystem.h"
#include "LuaWrappers/Wrappers.h"

static System *ThisSystem;
static lua_State *lua;

extern engineECS ECS;
extern engineCore Core;

typedef struct LuaScript{
    //Status 0 = error, 1 = no problems, 2 = reloaded but not prime ran
    char status;
    char* loopFunction;
    char scriptPath[512];
	char scriptName[256];
}LuaScript;

List luaScriptList;

//Internal functions
int ReloadScript(LuaScript* ls);

//Runs on engine start
void LuaSystemInit(){
    lua = luaL_newstate();
	luaL_openlibs(lua);

    RegisterWrappers(lua);

    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("LuaSystem"));
    luaScriptList = InitList(sizeof(LuaScript));
}

//Runs each GameLoop iteration
void LuaSystemUpdate(){
    lua_State *L = lua;
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

        //Skip scripts with errors
        if(ls->status == 0)
            continue;

        //If hasn't prime run, reload and run
        if(ls->status == 2)
            ReloadScript(ls);

        //Run loop function from Lua script
        lua_getglobal(L, ls->loopFunction );
        lua_pushinteger(L, entity);
        status = lua_pcall(L, 1, 0, 0);
        if (status) {
            PrintLog(Error,"LuaSystem: Failed to run script: %s\n", lua_tostring(L, -1));
            ls->status = 0;
        }
    }
}

//Runs at engine finish
void LuaSystemFree(){
    if(lua)
		lua_close(lua);
    
    ListCellPointer cell = GetFirstCell(luaScriptList);
    while(cell){
        ListCellPointer aux = cell;
        cell = GetNextCell(cell);
        
        free(((LuaScript*)GetElement(*aux))->loopFunction);
    }
}

int LoadNewScript(char* scriptPath, char* scriptName, EntityID entity){
    lua_State *L = lua;
    
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
                //If it has errors, just return its index
                if(script->status == 0){
                    return i;
                }

                //If the script is error free, just reload and prime run it
                if(luaL_loadfile(L, fullPath)) {
                    PrintLog(Error,"LoadNewScript: Couldn't reload file (%s): %s\n", scriptName, lua_tostring(L, -1));
                    script->status = 0;
                    return i;
                }

                //Prime run script to get the functions and to define variables
                if (lua_pcall(L, 0, 1, 0)) {
                    PrintLog(Error,"LoadNewScript: Failed to re-run script (%s): %s\n", scriptName, lua_tostring(L, -1));
                    script->status = 0;
                }
                return i;
            }
        }
        i++;
    }

    //Load the new script chunk into Lua
    if(luaL_loadfile(L, fullPath)) {
        PrintLog(Error,"LoadNewScript: Couldn't load file (%s): %s\n", scriptName, lua_tostring(L, -1));
        
        //Put the script on the list, but mark as with error
        LuaScript newScript;
        newScript.status = 0;
        newScript.loopFunction = NULL;
        strcpy(newScript.scriptName, scriptName);
        strcpy(newScript.scriptPath, scriptPath);

        InsertListEnd(&luaScriptList, &newScript);

        return GetLength(luaScriptList)-1;
    } 

    //Struct of the new script
    LuaScript newScript;

    //If this system is enabled, 
    //Prime run script to get the functions and to define variables
    if(ThisSystem->enabled){
        if (lua_pcall(L, 0, 1, 0)) {
            PrintLog(Error,"LoadNewScript: Failed to run script (%s): %s\n", scriptName, lua_tostring(L, -1));

            //Put the script on the list, but mark as with error
            LuaScript newScript;
            newScript.status = 0;
            newScript.loopFunction = NULL;
            strcpy(newScript.scriptName, scriptName);
            strcpy(newScript.scriptPath, scriptPath);

            InsertListEnd(&luaScriptList, &newScript);

            return GetLength(luaScriptList)-1;
        }

        //If the value returned is not a string with the name of the loop function, mark as with error
        if(lua_type(L, -1) != LUA_TSTRING){
            PrintLog(Error,"LoadNewScript: Returned value is not a string (%s): %s\n", scriptName, lua_tostring(L, -1));

            //Put the script on the list, but mark as with error
            LuaScript newScript;
            newScript.status = 0;
            newScript.loopFunction = NULL;
            strcpy(newScript.scriptName, scriptName);
            strcpy(newScript.scriptPath, scriptPath);

            InsertListEnd(&luaScriptList, &newScript);

            return GetLength(luaScriptList)-1;
        }



        //Get returned value as string
        const char* functionName = lua_tostring(L, -1);
        int functionNameLength = strlen(functionName)+1;

        //Copy the string to the struct
        newScript.loopFunction = malloc(functionNameLength * sizeof(char));
        strncpy(newScript.loopFunction, functionName, functionNameLength);
    }

    //If no errors, just put on the list

    newScript.status = ThisSystem->enabled? 1:2;
    strcpy(newScript.scriptName, scriptName);
    strcpy(newScript.scriptPath, scriptPath);

    InsertListEnd(&luaScriptList, &newScript);
    
    //Removed return value from stack
    lua_pop(L, 1);
    return GetLength(luaScriptList)-1;
}

//Returns 1 if reloaded sucessfully, and 0 if an error ocurred
int ReloadScript(LuaScript* ls){
    char fullPath[512+256];
    //Concatenate path and file name to fullPath
    strncpy(fullPath,ls->scriptPath,512);
    if(ls->scriptPath[strlen(ls->scriptPath)-1] != '/'){
        strcat(fullPath,"/");
    }
    strcat(fullPath,ls->scriptName);

    lua_State *L = lua;

    //Reload the script chunk into Lua
    if(luaL_loadfile(L, fullPath)) {
        PrintLog(Error,"ReloadScript: Couldn't load file (%s): %s\n", ls->scriptName, lua_tostring(L, -1));
        ls->status = 0;
        return 0;
    }

    //If the lua system is enabled,
    //Prime run script to get the functions and to define variables
    if(ThisSystem->enabled){
        if (lua_pcall(L, 0, 1, 0)) {
            PrintLog(Error,"ReloadScript: Failed to run script (%s): %s\n", ls->scriptName, lua_tostring(L, -1));
            ls->status = 0;
            return 0;
        }

        //If the value returned is not a string with the name of the loop function, ignore this script
        if(lua_type(L, -1) != LUA_TSTRING){
            PrintLog(Error,"ReloadScript: Returned value is not a string (%s): %s\n", ls->scriptName, lua_tostring(L, -1));
            ls->status = 0;
            return 0;
        }

        //Get returned value as string
        const char* functionName = lua_tostring(L, -1);
        int functionNameLength = strlen(functionName)+1;

        //Copy the string to the struct, and add it to the list
        free(ls->loopFunction);
        ls->loopFunction = malloc(functionNameLength * sizeof(char));
        strncpy(ls->loopFunction, functionName, functionNameLength);
        
        //Removed return value from stack
        lua_pop(L, 1);
    }

    ls->status = ThisSystem->enabled? 1:2;

    return 1;
}

int ReloadAllScripts(){
    int noErrors = 1;
    

    ListCellPointer cell;
    ListForEach(cell, luaScriptList){
        LuaScript *ls = GetElement(*cell);
        noErrors *= ReloadScript(ls);
    }

    return noErrors;
}