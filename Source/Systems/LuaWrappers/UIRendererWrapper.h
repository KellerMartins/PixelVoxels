#ifndef UIRENDERERWRAPPER_H
#define UIRENDERERWRAPPER_H

#include <stdlib.h>
#include "../LuaSystem.h"
#include "../../Engine.h"
#include "../../Systems/UIRenderer.h"

//From Systems/UIRenderer.c
extern List luaFonts;
extern char **luaFontNames;

static int l_LoadFontTTF(lua_State *L){
    if(!IsSystemEnabled(GetSystemID("LuaSystem"))) return 0;

    const char* fontPath = luaL_checkstring (L, 1);
    const char* fontName = luaL_checkstring (L, 2);
    int fontSize = luaL_checkinteger (L, 3);

    char fullPath[512+256];
    char *fontID = (char*) malloc((256+3) * sizeof(char));

    snprintf(fontID,256+3, "%.251s%d",fontName,fontSize);

    //Check if this font with this size is already loaded
    char *curName = luaFontNames[0];
    int i=0;
    while(curName){
        i++;
        if(StringCompareEqual(curName, fontID)){
            //This font with this size is already loaded
            free(fontID);
            return 0;
        }
        curName = luaFontNames[i];
    }

    //Concatenate path, name and extension, and complete '/' if needed
    if(fontPath[strlen(fontPath)-1] == '/')
        sprintf(fullPath, "%.512s%.251s.ttf",fontPath,fontName);
    else
        sprintf(fullPath, "%.511s/%.251s.ttf",fontPath,fontName);
        

    TTF_Font *newFont = TTF_OpenFont(fullPath,fontSize);
    if(!newFont){
        PrintLog(Error,"LoadFontTTF(Lua): failed to load font! (%s)\n", fullPath);
        return 0;
    }

    char **newFontNames = (char**) realloc(luaFontNames,(GetLength(luaFonts)+2) * sizeof(char*)); //+1 for the new string, and another for the NULL
    if(!newFontNames || !newFont){
        PrintLog(Error,"LoadFontTTF(Lua): failed to realloc the font name vector!\n");
        return 0;
    }

    InsertListEnd(&luaFonts, &newFont);

    luaFontNames = newFontNames;
    luaFontNames[GetLength(luaFonts)] = NULL;
    luaFontNames[GetLength(luaFonts)-1] = fontID;

    return 0;
}

//(WIP)
static int l_DrawTextColored (lua_State *L) {
    //Get the arguments
    //Text to be displayed
    const char* text = luaL_checkstring (L, 1);

    //Text color
    if(!lua_istable(L, 2)){
        PrintLog(Warning,"DrawTextColored(Lua): Second argument must be a table with 'r', 'g' and 'b' numbers!\n");
        luaL_checktype(L, 2, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,2, "r");
    lua_getfield(L,2, "g");
    lua_getfield(L,2, "b");

    Vector3 uiColor = {luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1)};

    //Text position
    if(!lua_istable(L, 3)){
        PrintLog(Warning,"DrawTextColored(Lua): Third argument must be a table with 'x' and 'y' numbers!\n");
        luaL_checktype(L, 3, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,3, "x");
    lua_getfield(L,3, "y");

    int px = luaL_checknumber(L,-2);
    int py = luaL_checknumber(L,-1);

    //Font index
    int fontIndex = luaL_checkoption(L, 4, NULL, (const char *const*) luaFontNames);

    DrawTextColored((char*)text, uiColor, px, py, GetElementAsType(GetCellAt(luaFonts, fontIndex),TTF_Font*) );
    return 0; //Return number of results
}

static int l_DrawRectangle (lua_State *L) {
    //Get the arguments

    //Minimum rectangle coordinates
    if(!lua_istable(L, 1)){
        PrintLog(Warning,"DrawRectangle(Lua): First argument must be a table with 'x' and 'y' numbers!\n");
        luaL_checktype(L, 1, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,1, "x");
    lua_getfield(L,1, "y");
    Vector3 min = {luaL_checknumber(L,-2), luaL_checknumber(L,-1),0};

    //Maximum rectangle coordinates
    if(!lua_istable(L, 2)){
        PrintLog(Warning,"DrawRectangle(Lua): Second argument must be a table with 'x' and 'y' numbers!\n");
        luaL_checktype(L, 2, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,2, "x");
    lua_getfield(L,2, "y");
    Vector3 max = {luaL_checknumber(L,-2), luaL_checknumber(L,-1),0};

    //Rectangle colors
    if(!lua_istable(L, 3)){
        PrintLog(Warning,"DrawRectangle(Lua): Third argument must be a table with 'r', 'g' and 'b' numbers!\n");
        luaL_checktype(L, 3, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,3, "r");
    lua_getfield(L,3, "g");
    lua_getfield(L,3, "b");
    Vector3 uiColor = {luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1)};

    DrawRectangle(min, max, uiColor.x, uiColor.y, uiColor.z);
    return 0; //Return number of results
}

static int l_DrawLine (lua_State *L) {
    //Get the arguments

    //Line start coordinate
    if(!lua_istable(L, 1)){
        PrintLog(Warning,"DrawLine(Lua): First argument must be a table with 'x' and 'y' numbers!\n");
        luaL_checktype(L, 1, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,1, "x");
    lua_getfield(L,1, "y");
    Vector3 start = {luaL_checknumber(L,-2), luaL_checknumber(L,-1),0};


    //Line end coordinate
    if(!lua_istable(L, 2)){
        PrintLog(Warning,"DrawLine(Lua): Second argument must be a table with 'x' and 'y' numbers!\n");
        luaL_checktype(L, 2, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,2, "x");
    lua_getfield(L,2, "y");
    Vector3 end = {luaL_checknumber(L,-2), luaL_checknumber(L,-1),0};

    float thickness = luaL_checknumber (L, 3);

    //Rectangle colors
    if(!lua_istable(L, 4)){
        PrintLog(Warning,"DrawRectangle(Lua): Third argument must be a table with 'r', 'g' and 'b' numbers!\n");
        luaL_checktype(L, 4, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,4, "r");
    lua_getfield(L,4, "g");
    lua_getfield(L,4, "b");
    Vector3 uiColor = {luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1)};

    DrawLine(start, end, thickness, uiColor.x, uiColor.y, uiColor.z);
    return 0; //Return number of results
}

static void UIRendererRegisterLuaFunctions(lua_State *L){
    lua_pushcfunction(L, l_DrawTextColored);
    lua_setglobal(L, "DrawTextColored");

    lua_pushcfunction(L, l_DrawRectangle);
    lua_setglobal(L, "DrawRectangle");

    lua_pushcfunction(L, l_DrawLine);
    lua_setglobal(L, "DrawLine");

    lua_pushcfunction(L, l_LoadFontTTF);
    lua_setglobal(L, "LoadFontTTF");
}

#endif
