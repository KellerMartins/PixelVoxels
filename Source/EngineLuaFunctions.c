#include "EngineLuaFunctions.h"

extern engineCore Core;
/*
//-- Component functions --
ComponentID l_GetComponentID(char componentName[25]){

}
ComponentMask l_CreateComponentMaskByName(int numComp, ...){

}
ComponentMask l_CreateComponentMaskByID(int numComp, ...){

}

//-- Entity functions --
EntityID l_CreateEntity(){

}
void l_DestroyEntity(EntityID entity){

}
int l_IsValidEntity(EntityID entity){

}
int l_EntityIsPrefab(EntityID entity){

}
char *GetPrefabPath(EntityID entity){

}
char *l_GetPrefabName(EntityID entity){

}
void l_AddComponentToEntity(ComponentID component, EntityID entity){

}
void l_RemoveComponentFromEntity(ComponentID component, EntityID entity){

}
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


//-- System functions --
SystemID l_GetSystemID(char systemName[25]){

}
void l_EnableSystem(SystemID system){

}
void l_DisableSystem(SystemID system){

}
int l_IsSystemEnabled(SystemID system){

}

//-------- Engine Functions -------------

void l_ExitGame(){

}
int l_GameExited(){

}

//-------- Rendering Functions -------------
*/
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