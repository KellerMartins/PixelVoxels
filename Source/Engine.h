#ifndef ENGINE_H
#define ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>

#include "Libs/cJSON.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#include "utils.h"

#include "Engine/EngineCore.h"
#include "Engine/EngineECS.h"
#include "Engine/EngineRendering.h"
#include "Engine/EngineInput.h"
#include "Engine/EngineScene.h"

//Engine functions called from main
int InitEngine();
void EngineUpdate();
void EngineUpdateEnd();
void EndEngine(int errorOcurred);



#endif