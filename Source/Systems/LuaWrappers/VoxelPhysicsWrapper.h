#ifndef VOXELPHYSICSRAPPER_H
#define VOXELPHYSICSRAPPER_H

#include "../LuaSystem.h"
#include "../../Engine.h"
#include "../../Systems/VoxelPhysics.h"

static int l_GetGravity (lua_State *L) {
    lua_settop(L, 0);
    double gravity = GetGravity();
    lua_pushnumber(L, gravity); //Put the returned number on the stack
    return 1; //Return number of results
}

static int l_SetGravity (lua_State *L) {
    //Get the arguments
    double g = luaL_checknumber (L, 1);

    SetGravity(g);
    return 0; //Return number of results
}

static void VoxelPhysicsRegisterLuaFunctions(lua_State *L){
    lua_pushcfunction(L, l_GetGravity);
    lua_setglobal(L, "GetGravity");

    lua_pushcfunction(L, l_SetGravity);
    lua_setglobal(L, "SetGravity");
}

#endif
