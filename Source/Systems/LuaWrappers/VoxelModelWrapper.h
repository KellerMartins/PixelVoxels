#ifndef VOXELMODELWRAPPER_H
#define VOXELMODELWRAPPER_H

#include "../LuaSystem.h"
#include "../../Engine.h"
#include "../../Components/VoxelModel.h"

//Gets

static int l_GetVoxelModelCenter(lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    Vector3 center = GetVoxelModelCenter(id);
    Vector3ToTable(L, center); //Create return table and store the values
    return 1; //Return number of results
}

static int l_IsVoxelModelEnabled (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    int enabled = IsVoxelModelEnabled(id);
    lua_pushboolean(L, enabled); //Put the returned boolean on the stack
    return 1; //Return number of results
}

static int l_IsVoxelModelSmallScale (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    int enabled = IsVoxelModelSmallScale(id);
    lua_pushboolean(L, enabled); //Put the returned boolean on the stack
    return 1; //Return number of results
}

static int l_IsMultiVoxelModelFile (lua_State *L) {
    lua_settop(L, 2);
    //Get the arguments
    const char* path = luaL_checkstring (L, 1);
    const char* name = luaL_checkstring (L, 2);

    int result = IsMultiVoxelModelFile((char*)path, (char*)name);
    lua_pushboolean(L, result); //Put the returned boolean on the stack
    return 1; //Return number of results
}

//Sets

static int l_SetVoxelModelCenter (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    if(!lua_istable(L, 2)){
        PrintLog(Warning,"SetVoxelModelCenter(Lua): Second argument must be a table with 'x', 'y' and 'z' numbers!\n");
        luaL_checktype(L, 2, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,2, "x");
    lua_getfield(L,2, "y");
    lua_getfield(L,2, "z");

    Vector3 center = {luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1)};

    SetVoxelModelCenter(id, center);
    lua_pop(L, 3);
    return 0; //Return number of results
}

static int l_SetVoxelModelEnabled (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    luaL_checktype(L, 2, LUA_TBOOLEAN);

    int enable = lua_toboolean(L, -1);

    SetVoxelModelEnabled(id, enable);
    return 0; //Return number of results
}

static int l_SetVoxelModelSmallScale (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    luaL_checktype(L, 2, LUA_TBOOLEAN);

    int boolVal = lua_toboolean(L, -1);

    SetVoxelModelSmallScale(id, boolVal);
    return 0; //Return number of results
}

static int l_LoadVoxelModel (lua_State *L) {
    lua_settop(L, 3);
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    const char* path = luaL_checkstring (L, 2);
    const char* name = luaL_checkstring (L, 3);

    LoadVoxelModel(id, (char*)path, (char*)name);
    return 0; //Return number of results
}

static int l_LoadMultiVoxelModel (lua_State *L) {
    lua_settop(L, 3);
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    const char* path = luaL_checkstring (L, 2);
    const char* name = luaL_checkstring (L, 3);

    LoadMultiVoxelModel(id, (char*)path, (char*)name);
    return 0; //Return number of results
}

static void VoxelModelRegisterLuaFunctions(lua_State *L){
    lua_pushcfunction(L, l_SetVoxelModelCenter);
    lua_setglobal(L, "SetVoxelModelCenter");

    lua_pushcfunction(L, l_GetVoxelModelCenter);
    lua_setglobal(L, "GetVoxelModelCenter");

    lua_pushcfunction(L, l_IsVoxelModelEnabled);
    lua_setglobal(L, "IsVoxelModelEnabled");

    lua_pushcfunction(L, l_SetVoxelModelEnabled);
    lua_setglobal(L, "SetVoxelModelEnabled");

    lua_pushcfunction(L, l_IsVoxelModelSmallScale);
    lua_setglobal(L, "IsVoxelModelSmallScale");

    lua_pushcfunction(L, l_SetVoxelModelSmallScale);
    lua_setglobal(L, "SetVoxelModelSmallScale");

    lua_pushcfunction(L, l_IsMultiVoxelModelFile);
    lua_setglobal(L, "IsMultiVoxelModelFile");

    lua_pushcfunction(L, l_LoadVoxelModel);
    lua_setglobal(L, "LoadVoxelModel");

    lua_pushcfunction(L, l_LoadMultiVoxelModel);
    lua_setglobal(L, "LoadMultiVoxelModel");
}

#endif
