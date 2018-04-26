#include "EngineLuaFunctions.h"

extern engineCore Core;

//-- Component functions --
static int l_GetComponentID(lua_State *L){
    const char* name = luaL_checkstring (L, 1);
    lua_pushinteger(L, GetComponentID((char *)name));
    return 1;
}

//-- Entity functions --
static int l_CreateEntity(lua_State *L){
    lua_pushinteger(L, CreateEntity());
    return 1;
}

static int l_DestroyEntity(lua_State *L){
    int id = luaL_checkinteger (L, 1);
    DestroyEntity(id);
    return 0;
}

static int l_IsValidEntity(lua_State *L){
    int id = luaL_checkinteger (L, 1);
    lua_pushboolean(L, IsValidEntity(id));
    return 1;
}

static int l_EntityIsPrefab(lua_State *L){
    int id = luaL_checkinteger (L, 1);
    lua_pushboolean(L, EntityIsPrefab(id));
    return 1;
}

static int l_GetPrefabPath(lua_State *L){
    EntityID id = luaL_checkinteger (L, 1);
    char* path = GetPrefabPath(id);
    lua_pushstring(L, path);
    return 1;
}

static int l_GetPrefabName(lua_State *L){
    EntityID id = luaL_checkinteger (L, 1);
    char* name = GetPrefabName(id);
    lua_pushstring(L, name);
    return 1;
}

static int l_AddComponentToEntity(lua_State *L){
    int component = luaL_checkinteger (L, 1);
    int entity = luaL_checkinteger (L, 2);
    AddComponentToEntity(component,entity);
    return 0;
}
static int l_RemoveComponentFromEntity(lua_State *L){
    int component = luaL_checkinteger (L, 1);
    int entity = luaL_checkinteger (L, 2);
    RemoveComponentFromEntity(component,entity);
    return 0;
}
/*
EntityID l_DuplicateEntity(EntityID entity){

}
int l_ExportEntityPrefab(EntityID entity, char path[], char name[]){

}
EntityID l_ImportEntityPrefab(char path[], char name[]){

}
int l_ExportScene(char path[], char name[]){

}
int l_LoadScene(char path[], char name[]){

}
int l_LoadSceneAdditive(char path[], char name[]){

}
ComponentMask l_GetEntityComponents(EntityID entity){

}
int l_IsEmptyComponentMask(ComponentMask mask){

}
int l_EntityContainsMask(EntityID entity, ComponentMask mask){

}
int l_EntityContainsComponent(EntityID entity, ComponentID component){

}
int l_MaskContainsComponent(ComponentMask mask, ComponentID component){

}
ComponentMask l_IntersectComponentMasks(ComponentMask mask1, ComponentMask mask2){

}


//-- Parenting functions --
int l_EntityIsParent(EntityID entity){

}
int l_EntityIsChild(EntityID entity){

}
void l_SetEntityParent(EntityID child, EntityID parent){

}
EntityID l_GetEntityParent(EntityID entity){

}
List* l_GetChildsList(EntityID parent){

}
int l_UnsetParent(EntityID child){

}

*/
//-- System functions --
static int l_GetSystemID(lua_State *L){
    const char* id = luaL_checkstring (L, 1);
    lua_pushinteger(L, GetSystemID((char *)id));
    return 1;
}

static int l_EnableSystem(lua_State *L){
    int id = luaL_checkinteger (L, 1);
    EnableSystem(id);
    return 0;
}

static int l_DisableSystem(lua_State *L){
    int id = luaL_checkinteger (L, 1);
    DisableSystem(id);
    return 0;
}

static int l_IsSystemEnabled(lua_State *L){
    int id = luaL_checkinteger (L, 1);
    lua_pushboolean(L, IsSystemEnabled(id));
    return 1;
}


//-------- Engine Functions -------------

static int l_ExitGame(lua_State *L){
    ExitGame();
    return 0;
}

static int l_GameExited(lua_State *L){
    lua_pushboolean(L, GameExited());
    return 1;
}

//-------- Rendering Functions -------------

static int l_PositionToGameScreenCoords(lua_State *L){
    //Get the position
    if(!lua_istable(L, 1)){
        printf("PositionToGameScreenCoords(Lua): First argument must be a table with 'x', 'y' and 'z' numbers!\n");
        luaL_checktype(L, 1, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,1, "x");
    lua_getfield(L,1, "y");
    lua_getfield(L,1, "z");

    Vector3 pos = {luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1)};
    pos = PositionToGameScreenCoords(pos);

    lua_newtable(L);
    lua_pushliteral(L, "x");  //x index
    lua_pushnumber(L, pos.x); //x value
    lua_rawset(L, -3);        //Store x in table

    lua_pushliteral(L, "y");  //y index
    lua_pushnumber(L, pos.y); //y value
    lua_rawset(L, -3);        //Store y in table

    return 1; //Return number of results
}

/*
void l_MoveCamera(float x, float y, float z){

}
*/
// ----------- Input functions ---------------

static const char *keyNames[] = {
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R","S", "T", "U", "V", "W", "X", "Y", "Z",
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
    "RETURN", "ESCAPE", "BACKSPACE", "TAB", "SPACE",
    "MINUS", "EQUALS", "LEFTBRACKET", "RIGHTBRACKET", "BACKSLASH", "NONUSHASH",
    "SEMICOLON", "APOSTROPHE", "GRAVE", "COMMA", "PERIOD", "SLASH", "CAPSLOCK",
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
    "PRINTSCREEN", "SCROLLLOCK", "PAUSE", "INSERT", "HOME", "PAGEUP", "DELETE", "END", "PAGEDOWN",
    "RIGHT", "LEFT", "DOWN", "UP",
    "KP_DIVIDE", "KP_MULTIPLY", "KP_MINUS", "KP_PLUS", "KP_ENTER",
    "KP_1", "KP_2", "KP_3", "KP_4", "KP_5", "KP_6", "KP_7", "KP_8", "KP_9", "KP_0", "KP_PERIOD",
    NULL};

static int l_GetKey(lua_State *L){
    int button = luaL_checkoption(L, 1, NULL, keyNames);
    lua_pushboolean(L, GetKey(button+SDL_SCANCODE_A) );
    return 1; //Return number of results
}

static int l_GetKeyDown(lua_State *L){
    int button = luaL_checkoption(L, 1, NULL, keyNames);
    lua_pushboolean(L, GetKeyDown(button+SDL_SCANCODE_A) );
    return 1; //Return number of results
} 

static int l_GetKeyUp(lua_State *L){
    int button = luaL_checkoption(L, 1, NULL, keyNames);
    lua_pushboolean(L, GetKeyUp(button+SDL_SCANCODE_A) );
    return 1; //Return number of results
} 

static const char *mouseButtonNames[] = {"MouseLeft", "MouseMiddle", "MouseRight", NULL};

static int l_GetMouseButton(lua_State *L){
    int button = luaL_checkoption(L, 1, "MouseLeft", mouseButtonNames);
    lua_pushboolean(L,GetMouseButton(button+1));
    return 1; //Return number of results
}

static int l_GetMouseButtonDown(lua_State *L){
    int button = luaL_checkoption(L, 1, "MouseLeft", mouseButtonNames);
    lua_pushboolean(L,GetMouseButtonDown(button+1));
    return 1; //Return number of results
}

static int l_GetMouseButtonUp(lua_State *L){
	int button = luaL_checkoption(L, 1, "MouseLeft", mouseButtonNames);
    lua_pushboolean(L,GetMouseButtonUp(button+1));
    return 1; //Return number of results
}

/*
void l_GetTextInput(char* outputTextPointer, int maxLength, int currentLength){

}

void l_StopTextInput(){
	
}*/


void EngineRegisterLuaFunctions(){

    lua_pushcfunction(Core.lua, l_GetComponentID);
    lua_setglobal(Core.lua, "GetComponentID");

    lua_pushcfunction(Core.lua, l_CreateEntity);
    lua_setglobal(Core.lua, "CreateEntity");
    lua_pushcfunction(Core.lua, l_DestroyEntity);
    lua_setglobal(Core.lua, "DestroyEntity");    
    lua_pushcfunction(Core.lua, l_IsValidEntity);
    lua_setglobal(Core.lua, "IsValidEntity");    

    lua_pushcfunction(Core.lua, l_EntityIsPrefab);
    lua_setglobal(Core.lua, "EntityIsPrefab");
    lua_pushcfunction(Core.lua, l_GetPrefabPath);
    lua_setglobal(Core.lua, "GetPrefabPath");
    lua_pushcfunction(Core.lua, l_GetPrefabName);
    lua_setglobal(Core.lua, "GetPrefabName");

    lua_pushcfunction(Core.lua, l_AddComponentToEntity);
    lua_setglobal(Core.lua, "AddComponentToEntity");
    lua_pushcfunction(Core.lua, l_RemoveComponentFromEntity);
    lua_setglobal(Core.lua, "RemoveComponentFromEntity");

    lua_pushcfunction(Core.lua, l_GetSystemID);
    lua_setglobal(Core.lua, "GetSystemID");
    lua_pushcfunction(Core.lua, l_EnableSystem);
    lua_setglobal(Core.lua, "EnableSystem");
    lua_pushcfunction(Core.lua, l_DisableSystem);
    lua_setglobal(Core.lua, "DisableSystem");
    lua_pushcfunction(Core.lua, l_IsSystemEnabled);
    lua_setglobal(Core.lua, "IsSystemEnabled");

    lua_pushcfunction(Core.lua, l_ExitGame);
    lua_setglobal(Core.lua, "ExitGame");
    lua_pushcfunction(Core.lua, l_GameExited);
    lua_setglobal(Core.lua, "GameExited");

    lua_pushcfunction(Core.lua, l_PositionToGameScreenCoords);
    lua_setglobal(Core.lua, "PositionToGameScreenCoords");

    lua_pushcfunction(Core.lua, l_GetKey);
    lua_setglobal(Core.lua, "GetKey");
    lua_pushcfunction(Core.lua, l_GetKeyDown);
    lua_setglobal(Core.lua, "GetKeyDown");
    lua_pushcfunction(Core.lua, l_GetKeyUp);
    lua_setglobal(Core.lua, "GetKeyUp");

    lua_pushcfunction(Core.lua, l_GetMouseButton);
    lua_setglobal(Core.lua, "GetMouseButton");
    lua_pushcfunction(Core.lua, l_GetMouseButtonDown);
    lua_setglobal(Core.lua, "GetMouseButtonDown");
    lua_pushcfunction(Core.lua, l_GetMouseButtonUp);
    lua_setglobal(Core.lua, "GetMouseButtonUp");
}