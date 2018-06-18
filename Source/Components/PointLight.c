#include "PointLight.h"

//Function that returns the ID of this component
static ComponentID ThisComponentID(){
    static ComponentID CompID = -1;
    if(CompID<0)
        CompID = GetComponentID("PointLight");
    
    return CompID;
}

extern engineCore Core;
extern engineECS ECS;

//Runs on AddComponentToEntity
void PointLightConstructor(void** data){
    if(!data) return;
    *data = calloc(1,sizeof(PointLightData));
    ((PointLightData*)*data)->color = (Vector3){1,1,1};
    ((PointLightData*)*data)->intensity = 0.75;
    ((PointLightData*)*data)->range = 100;
    ((PointLightData*)*data)->hueShift = 0;

}

//Runs on RemoveComponentFromEntity
void PointLightDestructor(void** data){
    if(!data) return;
    free(*data);
    *data = NULL;
}

void* PointLightCopy(void* data){
    if(!data) return NULL;
    PointLightData *newPointLightData = malloc(sizeof(PointLightData));
    memcpy(newPointLightData,data,sizeof(PointLightData));
	return newPointLightData;
}

cJSON* PointLightEncode(void** data, cJSON* currentData){
    if(!data) return NULL;
    printf("Point\n");
    PointLightData *pl = *data; 
   
    int hasChanged = 0;
    if(currentData){
        //Check if any data has changed
        cJSON *curColor = cJSON_GetObjectItem(currentData,"color");
        
        if(pl->color.x != (cJSON_GetArrayItem(curColor,0))->valuedouble ||
            pl->color.y != (cJSON_GetArrayItem(curColor,1))->valuedouble || 
            pl->color.z != (cJSON_GetArrayItem(curColor,2))->valuedouble || 
            pl->intensity != cJSON_GetObjectItem(currentData,"intensity")->valuedouble || 
            pl->range != cJSON_GetObjectItem(currentData,"range")->valuedouble||
            pl->hueShift != cJSON_GetObjectItem(currentData,"hueShift")->valuedouble
        ){
            hasChanged = 1;
        }
    }

    //Encode this component if its not from a prefab (who has currentData) or if it has changed
    if(!currentData || hasChanged){
        cJSON *obj = cJSON_CreateObject();
        
        cJSON *colorArr = cJSON_AddArrayToObject(obj,"color");
        cJSON_AddItemToArray(colorArr,cJSON_CreateNumber(pl->color.x));
        cJSON_AddItemToArray(colorArr,cJSON_CreateNumber(pl->color.y));
        cJSON_AddItemToArray(colorArr,cJSON_CreateNumber(pl->color.z));

        cJSON_AddNumberToObject(obj,"intensity",pl->intensity);
        cJSON_AddNumberToObject(obj,"range",pl->range);
        cJSON_AddNumberToObject(obj,"hueShift",pl->hueShift);

        return obj;
    }
    return NULL;
}

void* PointLightDecode(cJSON **data){
    PointLightData *pl = malloc(sizeof(PointLightData));

    pl->color = JSON_GetObjectVector3(*data,"color",VECTOR3_ZERO);
    pl->intensity = JSON_GetObjectDouble(*data,"intensity",0);
    pl->range = JSON_GetObjectDouble(*data,"range",0);
    pl->hueShift = JSON_GetObjectDouble(*data,"hueShift",0);

    return pl;
}

Vector3 GetPointLightColor(EntityID entity){
     if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetPointLightColor: Entity doesn't have a PointLight component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    return pl->color;
}

void SetPointLightColor(EntityID entity, Vector3 rgbColor){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetPointLightColor: Entity doesn't have a PointLight component. (%d)\n",entity);
        return;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    pl->color = rgbColor;

    pl->color.x = clamp(pl->color.x, 0, INFINITY);
    pl->color.y = clamp(pl->color.y, 0, INFINITY);
    pl->color.z = clamp(pl->color.z, 0, INFINITY);
}

float GetPointLightIntensity(EntityID entity){
     if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetPointLightIntensity: Entity doesn't have a PointLight component. (%d)\n",entity);
        return 0;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    return pl->intensity;
}

void SetPointLightIntensity(EntityID entity, float intensity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetPointLightIntensity: Entity doesn't have a PointLight component. (%d)\n",entity);
        return;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    pl->intensity = intensity;
}

float GetPointLightRange(EntityID entity){
     if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetPointLightRange: Entity doesn't have a PointLight component. (%d)\n",entity);
        return 0;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    return pl->range;
}

void SetPointLightRange(EntityID entity, float range){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetPointLightRange: Entity doesn't have a PointLight component. (%d)\n",entity);
        return;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    pl->range = clamp(range,0,INFINITY);
}

float GetPointLightHueShift(EntityID entity){
     if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetPointLightHueShift: Entity doesn't have a PointLight component. (%d)\n",entity);
        return 0;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    return pl->hueShift;
}

void SetPointLightHueShift(EntityID entity, float shift){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetPointLightHueShift: Entity doesn't have a PointLight component. (%d)\n",entity);
        return;
    }
    PointLightData *pl = (PointLightData *)ECS.Components[ThisComponentID()][entity].data;
    pl->hueShift = shift;
}


//Lua interface functions

static int l_SetPointLightColor (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    if(!lua_istable(L, 2)){
        printf("SetPointLightColor(Lua): Second argument must be a table with 'r', 'g' and 'b' numbers!\n");
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


void PointLightRegisterLuaFunctions(){
    lua_pushcfunction(Core.lua, l_SetPointLightColor);
    lua_setglobal(Core.lua, "SetPointLightColor");

    lua_pushcfunction(Core.lua, l_GetPointLightColor);
    lua_setglobal(Core.lua, "GetPointLightColor");

    lua_pushcfunction(Core.lua, l_SetPointLightIntensity);
    lua_setglobal(Core.lua, "SetPointLightIntensity");

    lua_pushcfunction(Core.lua, l_GetPointLightIntensity);
    lua_setglobal(Core.lua, "GetPointLightIntensity");

    lua_pushcfunction(Core.lua, l_SetPointLightRange);
    lua_setglobal(Core.lua, "SetPointLightRange");

    lua_pushcfunction(Core.lua, l_GetPointLightRange);
    lua_setglobal(Core.lua, "GetPointLightRange");

    lua_pushcfunction(Core.lua, l_SetPointLightHueShift);
    lua_setglobal(Core.lua, "SetPointLightHueShift");

    lua_pushcfunction(Core.lua, l_GetPointLightHueShift);
    lua_setglobal(Core.lua, "GetPointLightHueShift");
}