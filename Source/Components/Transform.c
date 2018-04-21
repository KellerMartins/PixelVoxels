#include "Transform.h"

//Can't exist without being an component from an entity
//Defines the local transform of an entity
typedef struct Transform{
    Vector3 position;
    Vector3 rotation;
}Transform;

//Function that returns the ID of this component
static ComponentID ThisComponentID(){
    static ComponentID CompID = -1;
    if(CompID<0)
        CompID = GetComponentID("Transform");
    
    return CompID;
}

extern engineECS ECS;
extern engineCore Core;

void TransformConstructor(void** data){
    if(!data) return;
    *data = calloc(1,sizeof(Transform));
}

void TransformDestructor(void** data){
    if(!data) return;
    free(*data);
    *data = NULL;
}

void* TransformCopy(void* data){
    if(!data) return NULL;
    Transform *newTransform = malloc(sizeof(Transform));
    memcpy(newTransform,data,sizeof(Transform));
	return newTransform;
}

cJSON* TransformEncode(void** data, cJSON* currentData){
    if(!data) return NULL;
    printf("Transform\n");
    Transform *tr = *data; 

    int hasChanged = 0;
    if(currentData){
        //Check if any data has changed
        cJSON *curPos = cJSON_GetObjectItem(currentData,"position");
        cJSON *curRot = cJSON_GetObjectItem(currentData,"rotation");
        if(tr->position.x != (cJSON_GetArrayItem(curPos,0))->valuedouble ||
           tr->position.y != (cJSON_GetArrayItem(curPos,1))->valuedouble || 
           tr->position.z != (cJSON_GetArrayItem(curPos,2))->valuedouble || 
           tr->rotation.x != (cJSON_GetArrayItem(curRot,0))->valuedouble || 
           tr->rotation.y != (cJSON_GetArrayItem(curRot,1))->valuedouble || 
           tr->rotation.z != (cJSON_GetArrayItem(curRot,2))->valuedouble
        ){
            hasChanged = 1;
        }
    }

    //Encode this component if its not from a prefab (who has currentData) or if it has changed
    if(!currentData || hasChanged){
        cJSON *obj = cJSON_CreateObject();

        cJSON *position = cJSON_AddArrayToObject(obj,"position");
        cJSON_AddItemToArray(position, cJSON_CreateNumber(tr->position.x));
        cJSON_AddItemToArray(position, cJSON_CreateNumber(tr->position.y));
        cJSON_AddItemToArray(position, cJSON_CreateNumber(tr->position.z));

        cJSON *rotation = cJSON_AddArrayToObject(obj,"rotation");
        cJSON_AddItemToArray(rotation, cJSON_CreateNumber(tr->rotation.x));
        cJSON_AddItemToArray(rotation, cJSON_CreateNumber(tr->rotation.y));
        cJSON_AddItemToArray(rotation, cJSON_CreateNumber(tr->rotation.z));

        return obj;
    }
    return NULL;
}

void* TransformDecode(cJSON **data){
    Transform *tr = malloc(sizeof(Transform));
    cJSON *pos = cJSON_GetObjectItem(*data,"position");
    tr->position = (Vector3){(cJSON_GetArrayItem(pos,0))->valuedouble,
                             (cJSON_GetArrayItem(pos,1))->valuedouble,
                             (cJSON_GetArrayItem(pos,2))->valuedouble};

    cJSON *rot = cJSON_GetObjectItem(*data,"rotation");
    tr->rotation = (Vector3){(cJSON_GetArrayItem(rot,0))->valuedouble,
                             (cJSON_GetArrayItem(rot,1))->valuedouble,
                             (cJSON_GetArrayItem(rot,2))->valuedouble};

    return tr;
}

Vector3 GetPosition(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetPosition: Entity doesn't have a Transform component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    Transform *transform = (Transform *)ECS.Components[ThisComponentID()][entity].data;
    return transform->position;
}

Vector3 GetRotation(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetRotation: Entity doesn't have a Transform component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    Transform *transform = (Transform *)ECS.Components[ThisComponentID()][entity].data;
    return transform->rotation;
}

void SetPosition(EntityID entity, Vector3 pos){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetPosition: Entity doesn't have a Transform component. (%d)\n",entity);
        return;
    }
    Transform *transform = (Transform *)ECS.Components[ThisComponentID()][entity].data;
    transform->position = pos;
}

void SetRotation(EntityID entity, Vector3 rot){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetRotation: Entity doesn't have a Transform component. (%d)\n",entity);
        return;
    }
    Transform *transform = (Transform *)ECS.Components[ThisComponentID()][entity].data;
    transform->rotation = rot;
}

void GetGlobalTransform(EntityID entity, Vector3 *outPos, Vector3 *outRot){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetGlobalTransform: Entity doesn't have a Transform component. (%d)\n",entity);
        return;
    }
    Transform *transform = (Transform *)ECS.Components[ThisComponentID()][entity].data;

    if(EntityIsChild(entity)){
        Vector3 parentGlobalPos, parentGlobalRot;
        GetGlobalTransform(GetEntityParent(entity),&parentGlobalPos, &parentGlobalRot);
        Vector3 rot = (Vector3){parentGlobalRot.x,parentGlobalRot.y,parentGlobalRot.z};

        float sinx = sin(rot.x * PI_OVER_180);
        float cosx = cos(rot.x * PI_OVER_180);
        float siny = sin(rot.y * PI_OVER_180);
        float cosy = cos(rot.y * PI_OVER_180);
        float sinz = sin(rot.z * PI_OVER_180);
        float cosz = cos(rot.z * PI_OVER_180);

        float rxt1 = cosy*cosz, rxt2 = (cosz*sinx*siny - cosx*sinz), rxt3 = (cosx*cosz*siny + sinx*sinz);
        float ryt1 = cosy*sinz, ryt2 = (cosx*siny*sinz - cosz*sinx), ryt3 = (cosx*cosz + sinx*siny*sinz);
        float rzt1 = cosx*cosy, rzt2 = sinx*cosy,                    rzt3 = siny;
        
        Vector3 rotatedPos;
        //Apply rotation matrix
        Vector3 p = (Vector3){transform->position.x, transform->position.y, transform->position.z};
        rotatedPos.x = p.x*rxt1 + p.y*rxt2 + p.z*rxt3;
        rotatedPos.y = p.x*ryt1 + p.z*ryt2 + p.y*ryt3;
        rotatedPos.z = p.z*rzt1 + p.y*rzt2 - p.x*rzt3;

        rotatedPos = (Vector3){ rotatedPos.x + parentGlobalPos.x, rotatedPos.y + parentGlobalPos.y, rotatedPos.z + parentGlobalPos.z};

        if(outPos) *outPos = rotatedPos;
        //Currently incorrect, need to fix
        if(outRot) *outRot = Add(transform->rotation, parentGlobalRot);
        
    }else{
        if(outPos) *outPos = transform->position;
        if(outRot) *outRot = transform->rotation;
    }
}

static int l_SetPosition (lua_State *L) {
    //Get the arguments
    EntityID id = luaL_checkinteger (L, 1);
    if(!lua_istable(L, 2)){
        printf("SetPosition(Lua): Second argument must be a table with 'x', 'y' and 'z' numbers!\n");
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
        printf("SetRotation(Lua): Second argument must be a table with 'x', 'y' and 'z' numbers!\n");
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


void TransformRegisterLuaFunctions(){
    lua_pushcfunction(Core.lua, l_SetPosition);
    lua_setglobal(Core.lua, "SetPosition");

    lua_pushcfunction(Core.lua, l_SetRotation);
    lua_setglobal(Core.lua, "SetRotation");

    lua_pushcfunction(Core.lua, l_GetPosition);
    lua_setglobal(Core.lua, "GetPosition");

    lua_pushcfunction(Core.lua, l_GetRotation);
    lua_setglobal(Core.lua, "GetRotation");
}