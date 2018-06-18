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

//Global engine data
typedef struct engineCore{
    SDL_Renderer * renderer;
    SDL_Window* window;	
    SDL_GLContext glContext;
    lua_State *lua;
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

    char *textInput;
    unsigned textInputMax;
    unsigned textInputLength;
    unsigned textInputCursorPos;

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
    GLuint vao , vbo3D[3], vbo2D[3];

    GLuint Shaders[4];

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

typedef int EntityID;

typedef struct Entity{
    ComponentMask mask;
    int isSpawned;
    int isParent;
    int isChild;
    int isPrefab;
    char prefabPath[4096];
    char prefabName[256];

    //If parent: List of his child EntityIDs
    List childs;
    //If child: ID of his parent
    EntityID parent;
}Entity;

typedef struct Component{
    void *data;
}Component;

typedef int ComponentID;

typedef struct ComponentType{
    char name[25];
    void (*constructor)(void** data);
    void (*destructor)(void** data);
    void*(*copy)(void*);

    cJSON*(*encode)(void** data, cJSON* currentData);
    void* (*decode)(cJSON** data);
}ComponentType;

typedef int SystemID;

typedef struct System System;
typedef struct System{
    char name[25];
    int priority;
    int enabled;

    ComponentMask required;
    ComponentMask excluded;

    void (*systemInit)();
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

int RegisterNewComponent(char componentName[25],void (*constructorFunc)(void** data),void (*destructorFunc)(void** data),void*(*copyFunc)(void*),cJSON*(*encodeFunc)(void** data, cJSON* currentData),void* (*decodeFunc)(cJSON** data));
int RegisterNewSystem(char systemName[25], int priority, ComponentMask required, ComponentMask excluded, void (*initFunc)(), void (*updateFunc)(), void (*freeFunc)());

ComponentID GetComponentID(char componentName[25]);

//Receives string
ComponentMask CreateComponentMaskByName(int numComp, ...);
//Receives ComponentID
ComponentMask CreateComponentMaskByID(int numComp, ...);

EntityID CreateEntity();
void DestroyEntity();
int IsValidEntity(EntityID entity);
int EntityIsPrefab(EntityID entity);
char *GetPrefabPath(EntityID entity);
char *GetPrefabName(EntityID entity);
void AddComponentToEntity(ComponentID component, EntityID entity);
void RemoveComponentFromEntity(ComponentID component, EntityID entity);
EntityID DuplicateEntity(EntityID entity);

ComponentMask GetEntityComponents(EntityID entity);
int IsEmptyComponentMask(ComponentMask mask);
int EntityContainsMask(EntityID entity, ComponentMask mask);
int EntityContainsComponent(EntityID entity, ComponentID component);
int MaskContainsComponent(ComponentMask mask, ComponentID component);
ComponentMask IntersectComponentMasks(ComponentMask mask1, ComponentMask mask2);

int EntityIsParent(EntityID entity);
int EntityIsChild(EntityID entity);
void SetEntityParent(EntityID child, EntityID parent);
EntityID GetEntityParent(EntityID entity);
List* GetChildsList(EntityID parent);
int UnsetParent(EntityID child);

SystemID GetSystemID(char systemName[25]);
void EnableSystem(SystemID system);
void DisableSystem(SystemID system);
int IsSystemEnabled(SystemID system);

int ExportEntityPrefab(EntityID entity, char path[], char name[]);
int ExportScene(char path[], char name[]);

EntityID ImportEntityPrefab(char path[], char name[]);
int LoadScene(char path[], char name[]);
int LoadSceneAdditive(char path[], char name[]);

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
Vector3 PositionToCameraCoords(Vector3 position);
void ClearRender(SDL_Color col);
void RenderToScreen();
void RenderTextDebug(char *text, SDL_Color color, int x, int y, TTF_Font* font);
int CompileAndLinkShader();
void ReloadShaders();
void LoadVoxelPalette(char path[]);
void MoveCamera(float x, float y, float z);
void TranslateCamera(float x, float y, float z);

//Input functions
int InitializeInput();
void InputUpdate();
int GetKey(SDL_Scancode key);
int GetKeyDown(SDL_Scancode key);
int GetKeyUp(SDL_Scancode key);

int GetMouseButton(int button);
int GetMouseButtonDown(int button);
int GetMouseButtonUp(int button);

void GetTextInput(char* outputTextPointer, int maxLength, int currentLength);
void StopTextInput();

//Lua stack manipulation functions
void Vector3ToTable(lua_State *L, Vector3 vector);

//cJSON wrappers
double JSON_GetObjectDouble(cJSON *object,char *string, double defaultValue);
Vector3 JSON_GetObjectVector3(cJSON *object,char *string, Vector3 defaultValue);

//Misc.
void SaveTextureToPNG(SDL_Texture *tex, char* out);

#endif