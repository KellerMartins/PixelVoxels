#ifndef ENGINE_H
#define ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "soloud_c.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#include "utils.h"

//Global engine data
typedef struct engineCore{
    SDL_Renderer * renderer;
    SDL_Window* window;	
    SDL_GLContext glContext;
    Soloud *soloud;
}engineCore;

typedef struct engineInput{
    //Keyboard key state arrays
    const Uint8 *keyboardCurrent;
    Uint8 *keyboardLast;

    int mouseX;
    int mouseY;
    int deltaMouseX;
    int deltaMouseY;

    int mouseButtonCurrent[3];
    int mouseButtonLast[3];

    int mouseWheelX;
    int mouseWheelY;

    SDL_Event event;
}engineInput;

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

typedef struct engineRendering{
    Vector3 cameraPosition;

    GLuint frameBuffer;
    GLuint screenTexture;
    GLuint depthRenderBuffer;
    GLuint vao , vbo[2];

    GLuint Shaders[2];

    Pixel voxelColors[256];
    SDL_Color clearScreenColor;
}engineRendering;

typedef struct engineTime{
    double deltaTime;
    unsigned frameTicks;
    unsigned msTime;
    Uint64 nowCounter;
    Uint64 lastCounter;
}engineTime;


//ECS Types
typedef struct ComponentMask{
    unsigned long mask;
}ComponentMask;

typedef struct Entity{
    ComponentMask mask;
}Entity;

typedef int EntityID;

typedef struct Component{
    void *data;
}Component;

typedef int ComponentID;

typedef struct ComponentType{
    char name[25];
    unsigned nameSize;
    void (*constructor)(EntityID entity);
    void (*destructor)(EntityID entity);
}ComponentType;

typedef struct System System;
typedef struct System{
    unsigned priority;

    ComponentMask required;
    ComponentMask excluded;

    void (*systemInit)(System *systemObject);
    void (*systemUpdate)();
    void (*systemFree)();
}System;

typedef struct engineECS{
    unsigned maxEntities;
    long int maxUsedIndex;

    //Dinamically allocated array of Entity structs
    Entity *Entities;
    List AvaliableEntitiesIndexes;

    //Array containing dinamicaly allocated arrays with components
    //Each index of the main array is set to be unique to that component type
    Component **Components;
    //List mapping string component names to indexes
    List ComponentTypes;

    //List containing all systems
    List SystemList;
}engineECS;

//ECS functions
int InitECS(unsigned max_entities);

int RegisterNewComponent(char componentName[25],void (*constructorFunc)(EntityID entity),void (*destructorFunc)(EntityID entity));
int RegisterNewSystem(unsigned priority, ComponentMask required, ComponentMask excluded, void (*initFunc)(System *systemObject), void (*updateFunc)(), void (*freeFunc)());

ComponentID GetComponentID(char componentName[25]);

//Receives string
ComponentMask CreateComponentMaskByName(int numComp, ...);
//Receives ComponentID
ComponentMask CreateComponentMaskByID(int numComp, ...);

EntityID CreateEntity();
void DestroyEntity();
void AddComponentToEntity(ComponentID component, EntityID entity);
void RemoveComponentFromEntity(ComponentID component, EntityID entity);

ComponentMask GetEntityComponents(EntityID entity);
int IsEmptyComponentMask(ComponentMask mask);
int EntityContainsMask(EntityID entity, ComponentMask mask);
int EntityContainsComponent(EntityID entity, ComponentID component);
int MaskContainsComponent(ComponentMask mask, ComponentID component);
ComponentMask IntersectComponentMasks(ComponentMask mask1, ComponentMask mask2);

//Engine functions
int InitEngine();
int FreeInput();
void EngineUpdate();
void EngineUpdateEnd();
void EndEngine(int errorOcurred);
void ExitGame();
int GameExited();

//Rendering functions
Vector3 PositionToGameScreenCoords(Vector3 position);
void ClearRender(SDL_Color col);
void RenderToScreen();
void RenderText(char *text, SDL_Color color, int x, int y, TTF_Font* font);
int CompileAndLinkShader();
void ReloadShaders();
void LoadVoxelPalette(char path[]);
void MoveCamera(float x, float y, float z);

//Input functions
int InitializeInput();
void InputUpdate();
int GetKey(SDL_Scancode key);
int GetKeyDown(SDL_Scancode key);
int GetKeyUp(SDL_Scancode key);

int GetMouseButton(int button);
int GetMouseButtonDown(int button);
int GetMouseButtonUp(int button);

//Misc.
void SaveTextureToPNG(SDL_Texture *tex, char* out);

#endif