#ifndef TRANSFORMWRAPPER_H
#define TRANSFORMWRAPPER_H

#include "../LuaSystem.h"
#include "../../Engine.h"
#include "../../Components/Transform.h"

static int l_SetPosition (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    if(!lua_istable(L, 2)){
        PrintLog(Warning,"SetPosition(Lua): Second argument must be a table with 'x', 'y' and 'z' numbers!\n");
        luaL_checktype(L, 2, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,2, "x");
    lua_getfield(L,2, "y");
    lua_getfield(L,2, "z");

    Vector3 pos = {luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1)};

    SetPosition(id, pos);
    lua_pop(L, 3);
    return 0; //Return number of results
}

static int l_SetRotation (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    if(!lua_istable(L, 2)){
        PrintLog(Warning,"SetRotation(Lua): Second argument must be a table with 'x', 'y' and 'z' numbers!\n");
        luaL_checktype(L, 2, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,2, "x");
    lua_getfield(L,2, "y");
    lua_getfield(L,2, "z");

    Vector3 rot = {luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1)};

    SetRotation(id, rot);
    lua_pop(L, 3);
    return 0; //Return number of results
}

static int l_GetPosition (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    Vector3 pos = GetPosition(id);
    Vector3ToTable(L, pos); //Create return table and store the values
    return 1; //Return number of results
}

static int l_GetRotation (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    Vector3 pos = GetRotation(id);
    Vector3ToTable(L, pos); //Create return table and store the values
    return 1; //Return number of results
}

static int l_GetGlobalTransform (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    Vector3 pos, rot;
    GetGlobalTransform(id, &pos, &rot,NULL);

    Vector3ToTable(L, pos); //Create return tables and store the values
    Vector3ToTable(L, rot);
    return 2; //Return number of results
}


static void TransformRegisterLuaFunctions(lua_State *L){
    lua_pushcfunction(L, l_SetPosition);
    lua_setglobal(L, "SetPosition");

    lua_pushcfunction(L, l_SetRotation);
    lua_setglobal(L, "SetRotation");

    lua_pushcfunction(L, l_GetPosition);
    lua_setglobal(L, "GetPosition");

    lua_pushcfunction(L, l_GetRotation);
    lua_setglobal(L, "GetRotation");

    lua_pushcfunction(L, l_GetGlobalTransform);
    lua_setglobal(L, "GetGlobalTransform");
}

#endif
