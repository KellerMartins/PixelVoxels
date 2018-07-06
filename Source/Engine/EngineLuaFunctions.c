#include "EngineLuaFunctions.h"

extern engineECS ECS;
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

static int l_DuplicateEntity(lua_State *L){
    int id = luaL_checkinteger (L, 1);
    lua_pushinteger(L, DuplicateEntity(id));
    return 1;
}

static int l_ExportEntityPrefab(lua_State *L){
    int id = luaL_checkinteger (L, 1);
    const char* path = luaL_checkstring (L, 2);
    const char* name = luaL_checkstring (L, 3);
    lua_pushinteger(L, ExportEntityPrefab(id, (char*)path, (char*)name));
    return 1;
}

static int l_ImportEntityPrefab(lua_State *L){
    const char* path = luaL_checkstring (L, 1);
    const char* name = luaL_checkstring (L, 2);
    lua_pushinteger(L, ImportEntityPrefab((char*)path, (char*)name));
    return 1;
}
static int l_ExportScene(lua_State *L){
    const char* path = luaL_checkstring (L, 1);
    const char* name = luaL_checkstring (L, 2);
    lua_pushinteger(L, ExportScene((char*)path, (char*)name));
    return 1;
}

static int l_LoadScene(lua_State *L){
    const char* path = luaL_checkstring (L, 1);
    const char* name = luaL_checkstring (L, 2);
    lua_pushinteger(L, LoadScene((char*)path, (char*)name));
    return 1;
}

static int l_LoadSceneAdditive(lua_State *L){
    const char* path = luaL_checkstring (L, 1);
    const char* name = luaL_checkstring (L, 2);
    lua_pushinteger(L, LoadSceneAdditive((char*)path, (char*)name));
    return 1;
}

static int l_GetEntityComponents(lua_State *L){
    int id = luaL_checkinteger (L, 1);

    if(!IsValidEntity(id)){
		printf("GetEntityComponents(Lua): Entity is not spawned or out of range!(%d)\n",id);
		return 0;
	}

    //Create a new table to insert the components of this entity
    lua_newtable(L);

    int c;
	for(c=0; c<ECS.numberOfComponents; c++){
		if(EntityContainsComponent(id,c)){
            //Put the component in the components table
			lua_pushinteger(L, c+1);
            lua_pushstring(L, ECS.ComponentTypes[c].name);
            lua_rawset(L, -3);
		}
	}

    return 1;
}

static int l_EntityContainsComponents(lua_State *L){
    int id = luaL_checkinteger (L, 1);
    if(!lua_istable(L, 2)){
        printf("EntityContainsComponents(Lua): Second argument must be a array table with components names or indexes!\n");
        luaL_checktype(L, 2, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    size_t componentsCount = lua_rawlen(L,2);

    size_t i;
    for(i=1; i<=componentsCount; i++){

        lua_rawgeti(L,2,i);

        //If passed as string, get its index first, else, get the id directly
        if(lua_type(L,-1) == LUA_TSTRING){
            const char* comp = luaL_checkstring(L, -1);
            int compID = GetComponentID((char*)comp);

            //If the component name doesn't exits, return a False bool
            if(compID <0){
                lua_pushboolean(L,0);
                return 1;
            }

            //If this entity doesn't contain this component, return a False bool
            if(!EntityContainsComponent(id, compID)){
                lua_pushboolean(L,0);
                return 1;
            }
        }else if(lua_isinteger(L, -1)){
            int comp = luaL_checkinteger(L, -1);
            //If this entity doesn't contain this component, return a False bool
            if(!EntityContainsComponent(id, comp)){
                lua_pushboolean(L,0);
                return 1;
            }
        }else{
            //If another type of object was passed, return False
            lua_pushboolean(L,0);
            return 1;
        }

    }

    lua_pushboolean(L,1);
    return 1;
}


//-- Parenting functions --
static int l_EntityIsParent(lua_State *L){
    int id = luaL_checkinteger (L, 1);
    lua_pushboolean(L, EntityIsParent(id));
    return 1;
}

static int l_EntityIsChild(lua_State *L){
    int id = luaL_checkinteger (L, 1);
    lua_pushboolean(L, EntityIsChild(id));
    return 1;
}

static int l_SetEntityParent(lua_State *L){
    int childId = luaL_checkinteger (L, 1);
    int parentId = luaL_checkinteger (L, 2);
    SetEntityParent(childId, parentId);
    return 0;
}
static int l_GetEntityParent(lua_State *L){
    int id = luaL_checkinteger (L, 1);
    lua_pushinteger(L, GetEntityParent(id));
    return 1;
}
/*
List* l_GetChildsList(EntityID parent){

}
*/
static int l_UnsetParent(lua_State *L){
    int child = luaL_checkinteger (L, 1);
    lua_pushboolean(L, UnsetParent(child));
    return 1;
}


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

    return 1;
}


static int l_MoveCamera(lua_State *L){
    if(!lua_istable(L, 1)){
        printf("MoveCamera(Lua): First argument must be a table with 'x', 'y' and 'z' numbers!\n");
        luaL_checktype(L, 1, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,1, "x");
    lua_getfield(L,1, "y");
    lua_getfield(L,1, "z");

    MoveCamera(luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1));
    lua_pop(L, 3);
    return 0;
}

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
    lua_pushcfunction(Core.lua, l_DuplicateEntity);
    lua_setglobal(Core.lua, "DuplicateEntity");

    lua_pushcfunction(Core.lua, l_ExportEntityPrefab);
    lua_setglobal(Core.lua, "ExportEntityPrefab");
    lua_pushcfunction(Core.lua, l_ImportEntityPrefab);
    lua_setglobal(Core.lua, "ImportEntityPrefab");
    lua_pushcfunction(Core.lua, l_ExportScene);
    lua_setglobal(Core.lua, "ExportScene");
    lua_pushcfunction(Core.lua, l_LoadScene);
    lua_setglobal(Core.lua, "LoadScene");
    lua_pushcfunction(Core.lua, l_LoadSceneAdditive);
    lua_setglobal(Core.lua, "LoadSceneAdditive");

    lua_pushcfunction(Core.lua, l_GetEntityComponents);
    lua_setglobal(Core.lua, "GetEntityComponents");
    lua_pushcfunction(Core.lua, l_EntityContainsComponents);
    lua_setglobal(Core.lua, "EntityContainsComponents");

    lua_pushcfunction(Core.lua, l_EntityIsParent);
    lua_setglobal(Core.lua, "EntityIsParent");
    lua_pushcfunction(Core.lua, l_EntityIsChild);
    lua_setglobal(Core.lua, "EntityIsChild");
    lua_pushcfunction(Core.lua, l_SetEntityParent);
    lua_setglobal(Core.lua, "SetEntityParent");
    lua_pushcfunction(Core.lua, l_GetEntityParent);
    lua_setglobal(Core.lua, "GetEntityParent");
    lua_pushcfunction(Core.lua, l_UnsetParent);
    lua_setglobal(Core.lua, "UnsetParent");

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
    lua_pushcfunction(Core.lua, l_MoveCamera);
    lua_setglobal(Core.lua, "MoveCamera");

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