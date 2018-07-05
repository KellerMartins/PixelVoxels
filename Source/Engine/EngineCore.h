#ifndef ENGINECORE_H
#define ENGINECORE_H
#include <stdlib.h>
#include <time.h>

#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

typedef struct engineCore{
    SDL_Renderer * renderer;
    SDL_Window* window;	
    SDL_GLContext glContext;
    lua_State *lua;
}engineCore;

typedef struct engineScreen{
    //Internal resolution, used in rendering
    int gameWidth;
    int gameHeight;

    //Window resolution
    int windowWidth;
    int windowHeight;

    //Scale of division, as gameRes = windowRes/gameScale
    double gameScale;

    unsigned maxFPS;
}engineScreen;

typedef struct engineTime{
    double deltaTime;
    unsigned frameTicks;
    unsigned msTime;
    Uint64 nowCounter;
    Uint64 lastCounter;
}engineTime;

void ExitGame();
int GameExited();

int InitCore();
void InitTime();
void InitScreen(int windowWidth, int windowHeight, int scale, int maxFPS);

void UpdateTime();
void WaitUntilNextFrame();

#endif