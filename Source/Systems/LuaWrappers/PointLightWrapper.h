#ifndef POINTLIGHTWRAPPER_H
#define POINTLIGHTWRAPPER_H

#include "../LuaSystem.h"
#include "../../Engine.h"
#include "../../Components/PointLight.h"

static int l_SetPointLightColor (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    if(!lua_istable(L, 2)){
        PrintLog(Warning,"SetPointLightColor(Lua): Second argument must be a table with 'r', 'g' and 'b' numbers!\n");
        luaL_checktype(L, 2, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,2, "r");
    lua_getfield(L,2, "g");
    lua_getfield(L,2, "b");

    Vector3 color = {luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1)};

    SetPointLightColor(id, color);
    lua_pop(L, 3);
    return 0; //Return number of results
}

static int l_GetPointLightColor (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    Vector3 color = GetPointLightColor(id);

    lua_newtable(L);
    lua_pushliteral(L, "r");    //r index
    lua_pushnumber(L, color.x); //r value
    lua_rawset(L, -3);          //Store r in table

    lua_pushliteral(L, "g");    //g index
    lua_pushnumber(L, color.y); //g value
    lua_rawset(L, -3);          //Store g in table

    lua_pushliteral(L, "b");    //b index
    lua_pushnumber(L, color.z); //b value
    lua_rawset(L, -3);          //Store b in table

    return 1; //Return number of results
}

static int l_GetPointLightIntensity (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    float intensity = GetPointLightIntensity(id);
    lua_pushnumber(L, intensity); //Put the returned number on the stack
    return 1; //Return number of results
}

static int l_SetPointLightIntensity (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    float intensity = luaL_checknumber (L, 2);

    SetPointLightIntensity(id, intensity);
    return 0; //Return number of results
}

static int l_GetPointLightRange (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    float range = GetPointLightRange(id);
    lua_pushnumber(L, range); //Put the returned number on the stack
    return 1; //Return number of results
}

static int l_SetPointLightRange (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    float range = luaL_checknumber (L, 2);

    SetPointLightRange(id, range);
    return 0; //Return number of results
}

static int l_GetPointLightHueShift (lua_State *L) {
    lua_settop(L, 1);
    EntityID id = luaL_checkinteger (L, 1); //Get the argument
    float shift = GetPointLightHueShift(id);
    lua_pushnumber(L, shift); //Put the returned number on the stack
    return 1; //Return number of results
}

static int l_SetPointLightHueShift (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    float shift = luaL_checknumber (L, 2);

    SetPointLightHueShift(id, shift);
    return 0; //Return number of results
}


static void PointLightRegisterLuaFunctions(lua_State *L){
    lua_pushcfunction(L, l_SetPointLightColor);
    lua_setglobal(L, "SetPointLightColor");

    lua_pushcfunction(L, l_GetPointLightColor);
    lua_setglobal(L, "GetPointLightColor");

    lua_pushcfunction(L, l_SetPointLightIntensity);
    lua_setglobal(L, "SetPointLightIntensity");

    lua_pushcfunction(L, l_GetPointLightIntensity);
    lua_setglobal(L, "GetPointLightIntensity");

    lua_pushcfunction(L, l_SetPointLightRange);
    lua_setglobal(L, "SetPointLightRange");

    lua_pushcfunction(L, l_GetPointLightRange);
    lua_setglobal(L, "GetPointLightRange");

    lua_pushcfunction(L, l_SetPointLightHueShift);
    lua_setglobal(L, "SetPointLightHueShift");

    lua_pushcfunction(L, l_GetPointLightHueShift);
    lua_setglobal(L, "GetPointLightHueShift");
}

#endif
