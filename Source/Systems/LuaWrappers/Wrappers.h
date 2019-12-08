#ifndef WRAPPERS_H
#define WRAPPERS_H

#include "EngineWrapper.h"
#include "LuaScriptWrapper.h"
#include "PointLightWrapper.h"
#include "RigidBodyWrapper.h"
#include "TransformWrapper.h"
#include "UIRendererWrapper.h"
#include "VoxelModelWrapper.h"
#include "VoxelPhysicsWrapper.h"

static void RegisterWrappers(lua_State *L){
    EngineRegisterLuaFunctions(L);
    LuaScriptRegisterLuaFunctions(L);
    PointLightRegisterLuaFunctions(L);
    RigidBodyRegisterLuaFunctions(L);
    TransformRegisterLuaFunctions(L);
    UIRendererRegisterLuaFunctions(L);
    VoxelModelRegisterLuaFunctions(L);
    VoxelPhysicsRegisterLuaFunctions(L);
}

#endif