#include "RigidBody.h"

//Can't exist without being an component from an entity
typedef struct RigidBody{
    float mass;
    float bounciness;
    Vector3 velocity;
    Vector3 acceleration;
    int useGravity;
    int isStatic;
}RigidBody;

//Function that returns the ID of this component
static ComponentID ThisComponentID(){
    static ComponentID CompID = -1;
    if(CompID<0)
        CompID = GetComponentID("RigidBody");
    
    return CompID;
}

extern engineCore Core;
extern engineECS ECS;

//Engine component manipulation functions

void RigidBodyConstructor(void** data){
    if(!data) return;
    *data = calloc(1,sizeof(RigidBody));
    ((RigidBody*) *data)->mass = 1;
    ((RigidBody*) *data)->useGravity = 1;
    ((RigidBody*) *data)->bounciness = 0.2;
}

void RigidBodyDestructor(void** data){
    if(!data) return;
    free(*data);
    *data = NULL;
}

void* RigidBodyCopy(void* data){
    if(!data) return NULL;
    RigidBody *newRigidBody = malloc(sizeof(RigidBody));
    memcpy(newRigidBody,data,sizeof(RigidBody));
	return newRigidBody;
}

cJSON* RigidBodyEncode(void** data, cJSON* currentData){
    if(!data) return NULL;
    printf("RB\n");
    RigidBody *rb = *data;
    
    
    int hasChanged = 0;
    if(currentData){
        //Check if any data has changed
        cJSON *curVelocity = cJSON_GetObjectItem(currentData,"velocity");
        if(rb->velocity.x != (cJSON_GetArrayItem(curVelocity,0))->valuedouble ||
           rb->velocity.y != (cJSON_GetArrayItem(curVelocity,1))->valuedouble || 
           rb->velocity.z != (cJSON_GetArrayItem(curVelocity,2))->valuedouble)
        {
            hasChanged = 1;
        }

        cJSON *curAcceleration = cJSON_GetObjectItem(currentData,"acceleration");
        if(rb->acceleration.x != (cJSON_GetArrayItem(curAcceleration,0))->valuedouble ||
           rb->acceleration.y != (cJSON_GetArrayItem(curAcceleration,1))->valuedouble || 
           rb->acceleration.z != (cJSON_GetArrayItem(curAcceleration,2))->valuedouble)
        {
            hasChanged = 1;
        }

        cJSON *curUseGravity= cJSON_GetObjectItem(currentData,"useGravity");
        if(rb->useGravity != cJSON_IsTrue(curUseGravity)){
            hasChanged = 1;
        }

        cJSON *curIsStatic= cJSON_GetObjectItem(currentData,"isStatic");
        if(rb->isStatic != cJSON_IsTrue(curIsStatic)){
            hasChanged = 1;
        }

        if(rb->mass != (cJSON_GetObjectItem(currentData,"mass"))->valuedouble){
            hasChanged = 1;
        }

        if(rb->bounciness != (cJSON_GetObjectItem(currentData,"bounciness"))->valuedouble){
            hasChanged = 1;
        }
    }

    //Encode this component if its not from a prefab (who has currentData) or if it has changed
    if(!currentData || hasChanged){           
        cJSON *obj = cJSON_CreateObject();

        cJSON_AddNumberToObject(obj,"mass",rb->mass);
        cJSON_AddNumberToObject(obj,"bounciness",rb->bounciness);

        cJSON *velocity = cJSON_AddArrayToObject(obj,"velocity");
        cJSON_AddItemToArray(velocity, cJSON_CreateNumber(rb->velocity.x));
        cJSON_AddItemToArray(velocity, cJSON_CreateNumber(rb->velocity.y));
        cJSON_AddItemToArray(velocity, cJSON_CreateNumber(rb->velocity.z));

        cJSON *acceleration = cJSON_AddArrayToObject(obj,"acceleration");
        cJSON_AddItemToArray(acceleration, cJSON_CreateNumber(rb->acceleration.x));
        cJSON_AddItemToArray(acceleration, cJSON_CreateNumber(rb->acceleration.y));
        cJSON_AddItemToArray(acceleration, cJSON_CreateNumber(rb->acceleration.z));

        cJSON_AddBoolToObject(obj,"useGravity",rb->useGravity);
        cJSON_AddBoolToObject(obj,"isStatic",rb->isStatic);

        return obj;
    }
    return NULL;
}

void* RigidBodyDecode(cJSON **data){
    RigidBody *rb = malloc(sizeof(RigidBody));

    rb->mass = (cJSON_GetObjectItem(*data,"mass"))->valuedouble;
    rb->bounciness = (cJSON_GetObjectItem(*data,"bounciness"))->valuedouble;

    cJSON *vel = cJSON_GetObjectItem(*data,"velocity");
    rb->velocity = (Vector3){(cJSON_GetArrayItem(vel,0))->valuedouble,
                             (cJSON_GetArrayItem(vel,1))->valuedouble,
                             (cJSON_GetArrayItem(vel,2))->valuedouble};

    cJSON *acc = cJSON_GetObjectItem(*data,"acceleration");
    rb->acceleration = (Vector3){(cJSON_GetArrayItem(acc,0))->valuedouble,
                                 (cJSON_GetArrayItem(acc,1))->valuedouble,
                                 (cJSON_GetArrayItem(acc,2))->valuedouble};

    rb->useGravity = cJSON_IsTrue(cJSON_GetObjectItem(*data,"useGravity"));
    rb->isStatic = cJSON_IsTrue(cJSON_GetObjectItem(*data,"isStatic"));

    return rb;
}


//Component interface functions

Vector3 GetVelocity(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetVelocity: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->velocity;
}

Vector3 GetAcceleration(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetAcceleration: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->acceleration;
}

float GetMass(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetMass: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return 1;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->isStatic? STATIC_OBJECT_MASS : rb->mass;
}

float GetBounciness(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetBounciness: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return 1;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->bounciness;
}

void SetVelocity(EntityID entity, Vector3 vel){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetVelocity: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->velocity = vel;
}

void SetAcceleration(EntityID entity, Vector3 acc){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetAcceleration: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->acceleration = acc;
}

void SetMass(EntityID entity, float mass){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetMass: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->mass = clamp(mass,MASS_MIN,INFINITY);
}

void SetBounciness(EntityID entity, float bounciness){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetBounciness: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->bounciness = clamp(bounciness,0.2,1);
}

int UsesGravity(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("UsesGravity: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return 0;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->useGravity;
}

void SetUseGravity(EntityID entity,unsigned booleanVal){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetUseGravity: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->useGravity = booleanVal?1:0;
}

int IsStaticRigidBody(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("IsStaticRigidBody: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return 0;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    return rb->isStatic;
}

void SetStaticRigidBody(EntityID entity,unsigned booleanVal){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetStaticRigidBody: Entity doesn't have a RigidBody component. (%d)\n",entity);
        return;
    }
    RigidBody *rb = (RigidBody *)ECS.Components[ThisComponentID()][entity].data;
    rb->isStatic = booleanVal?1:0;
}


//Lua interface functions

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
        printf("SetVelocity(Lua): Second argument must be a table with 'x', 'y' and 'z' numbers!\n");
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
        printf("SetAcceleration(Lua): Second argument must be a table with 'x', 'y' and 'z' numbers!\n");
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

void RigidBodyRegisterLuaFunctions(){
    //Gets
    lua_pushcfunction(Core.lua, l_GetVelocity);
    lua_setglobal(Core.lua, "GetVelocity");

    lua_pushcfunction(Core.lua, l_GetAcceleration);
    lua_setglobal(Core.lua, "GetAcceleration");

    lua_pushcfunction(Core.lua, l_GetMass);
    lua_setglobal(Core.lua, "GetMass");

    lua_pushcfunction(Core.lua, l_GetBounciness);
    lua_setglobal(Core.lua, "GetBounciness");

    lua_pushcfunction(Core.lua, l_UsesGravity);
    lua_setglobal(Core.lua, "UsesGravity");

    lua_pushcfunction(Core.lua, l_IsStaticRigidBody);
    lua_setglobal(Core.lua, "IsStaticRigidBody");

    //Sets
    lua_pushcfunction(Core.lua, l_SetVelocity);
    lua_setglobal(Core.lua, "SetVelocity");

    lua_pushcfunction(Core.lua, l_SetAcceleration);
    lua_setglobal(Core.lua, "SetAcceleration");

    lua_pushcfunction(Core.lua, l_SetMass);
    lua_setglobal(Core.lua, "SetMass");

    lua_pushcfunction(Core.lua, l_SetBounciness);
    lua_setglobal(Core.lua, "SetBounciness");

    lua_pushcfunction(Core.lua, l_SetUseGravity);
    lua_setglobal(Core.lua, "SetUseGravity");

    lua_pushcfunction(Core.lua, l_SetStaticRigidBody);
    lua_setglobal(Core.lua, "SetStaticRigidBody");
}