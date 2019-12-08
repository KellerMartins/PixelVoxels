#ifndef LUASCRIPTWRAPPER_H
#define LUASCRIPTWRAPPER_H

#include "../LuaSystem.h"
#include "../../Engine.h"
#include "../../Components/LuaScript.h"

//Get

static int l_GetLuaScriptName (lua_State *L) {
    lua_settop(L, 1);
    //Get the argument
    EntityID id = luaL_checkinteger (L, 1);

    char* name = GetLuaScriptName(id);
    lua_pushstring(L, name); //Put the returned string on the stack
    return 1; //Return number of results
}

static int l_GetLuaScriptPath (lua_State *L) {
    lua_settop(L, 1);
    //Get the argument
    EntityID id = luaL_checkinteger (L, 1);

    char* path = GetLuaScriptPath(id);
    lua_pushstring(L, path); //Put the returned string on the stack
    return 1; //Return number of results
}

//Set

static int l_SetLuaScript (lua_State *L) {
    lua_settop(L, 3);
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    const char* path = luaL_checkstring (L, 2);
    const char* name = luaL_checkstring (L, 3);

    SetLuaScript(id, (char*)path, (char*)name);
    return 0; //Return number of results
}

static void LuaScriptRegisterLuaFunctions(lua_State *L){
    lua_pushcfunction(L, l_GetLuaScriptName);
    lua_setglobal(L, "GetLuaScriptName");

    lua_pushcfunction(L, l_GetLuaScriptPath);
    lua_setglobal(L, "GetLuaScriptPath");

    lua_pushcfunction(L, l_SetLuaScript);
    lua_setglobal(L, "SetLuaScript");
}

#endif
