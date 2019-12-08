#ifndef RIGIDBODYWRAPPER_H
#define RIGIDBODYWRAPPER_H

#include "../LuaSystem.h"
#include "../../Engine.h"
#include "../../Components/RigidBody.h"

static int l_GetVelocity (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    Vector3 vel = GetVelocity(id);
    Vector3ToTable(L, vel); //Create return table and store the values
    return 1; //Return number of results
}

static int l_GetAcceleration (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    Vector3 acc = GetAcceleration(id);
    Vector3ToTable(L, acc); //Create return table and store the values
    return 1; //Return number of results
}

static int l_GetMass (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    float mass = GetMass(id);
    lua_pushnumber(L, mass); //Put the returned number on the stack
    return 1; //Return number of results
}

static int l_GetBounciness (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    float bounc = GetBounciness(id);
    lua_pushnumber(L, bounc); //Put the returned number on the stack
    return 1; //Return number of results
}

static int l_UsesGravity (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    int gr = UsesGravity(id);
    lua_pushboolean(L, gr); //Put the returned boolean on the stack
    return 1; //Return number of results
}

static int l_IsStaticRigidBody (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    int stRb = IsStaticRigidBody(id);
    lua_pushboolean(L, stRb); //Put the returned boolean on the stack
    return 1; //Return number of results
}


static int l_SetVelocity (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    if(!lua_istable(L, 2)){
        PrintLog(Warning,"SetVelocity(Lua): Second argument must be a table with 'x', 'y' and 'z' numbers!\n");
        luaL_checktype(L, 2, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,2, "x");
    lua_getfield(L,2, "y");
    lua_getfield(L,2, "z");

    Vector3 vel = {luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1)};

    SetVelocity(id, vel);
    lua_pop(L, 3);
    return 0; //Return number of results
}

static int l_SetAcceleration (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    if(!lua_istable(L, 2)){
        PrintLog(Warning,"SetAcceleration(Lua): Second argument must be a table with 'x', 'y' and 'z' numbers!\n");
        luaL_checktype(L, 2, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,2, "x");
    lua_getfield(L,2, "y");
    lua_getfield(L,2, "z");

    Vector3 acc = {luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1)};

    SetAcceleration(id, acc);
    lua_pop(L, 3);
    return 0; //Return number of results
}

static int l_SetMass (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    float mass = luaL_checknumber (L, 2);

    SetMass(id, mass);
    return 0; //Return number of results
}

static int l_SetBounciness (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    float bouc = luaL_checknumber (L, 2);

    SetBounciness(id, bouc);
    return 0; //Return number of results
}

static int l_SetUseGravity (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    luaL_checktype(L, 2, LUA_TBOOLEAN);

    int gr = lua_toboolean(L, -1);

    SetUseGravity(id, gr);
    return 0; //Return number of results
}

static int l_SetStaticRigidBody (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    luaL_checktype(L, 2, LUA_TBOOLEAN);

    int stRb = lua_toboolean(L, -1);

    SetStaticRigidBody(id, stRb);
    return 0; //Return number of results
}

static void RigidBodyRegisterLuaFunctions(lua_State *L){
    //Gets
    lua_pushcfunction(L, l_GetVelocity);
    lua_setglobal(L, "GetVelocity");

    lua_pushcfunction(L, l_GetAcceleration);
    lua_setglobal(L, "GetAcceleration");

    lua_pushcfunction(L, l_GetMass);
    lua_setglobal(L, "GetMass");

    lua_pushcfunction(L, l_GetBounciness);
    lua_setglobal(L, "GetBounciness");

    lua_pushcfunction(L, l_UsesGravity);
    lua_setglobal(L, "UsesGravity");

    lua_pushcfunction(L, l_IsStaticRigidBody);
    lua_setglobal(L, "IsStaticRigidBody");

    //Sets
    lua_pushcfunction(L, l_SetVelocity);
    lua_setglobal(L, "SetVelocity");

    lua_pushcfunction(L, l_SetAcceleration);
    lua_setglobal(L, "SetAcceleration");

    lua_pushcfunction(L, l_SetMass);
    lua_setglobal(L, "SetMass");

    lua_pushcfunction(L, l_SetBounciness);
    lua_setglobal(L, "SetBounciness");

    lua_pushcfunction(L, l_SetUseGravity);
    lua_setglobal(L, "SetUseGravity");

    lua_pushcfunction(L, l_SetStaticRigidBody);
    lua_setglobal(L, "SetStaticRigidBody");
}

#endif
