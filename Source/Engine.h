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
    void (*constructor)(ComponentID component, EntityID entity);
    void (*destructor)(ComponentID component, EntityID entity);
}ComponentType;

typedef struct System{
    unsigned priority;

    ComponentMask required;
    ComponentMask excluded;

    void (*systemInit)();
    void (*systemUpdate)(EntityID entity);
    void (*systemFree)();
}System;

typedef struct engineECS{
    unsigned maxEntities;

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

int RegisterNewComponent(char componentName[25],void (*constructorFunc)(ComponentID component, EntityID entity),void (*destructorFunc)(ComponentID component, EntityID entity));
int RegisterNewSystem(unsigned priority, ComponentMask required, ComponentMask excluded, void (*initFunc)(), void (*updateFunc)(EntityID entity), void (*freeFunc)());

ComponentID GetComponentID(char componentName[25]);

//Receives string
ComponentMask CreateComponentMask(int numComp, ...);
//Receives ComponentID
ComponentMask CreateComponentMaskByID(int numComp, ...);

EntityID CreateEntity();
void DestroyEntity();
void AddComponentToEntity(ComponentID component, EntityID entity);
void RemoveComponentFromEntity(ComponentID component, EntityID entity);
ComponentMask GetEntityComponents(EntityID entity);
int EntityContainsMask(Entity entity, ComponentMask mask);


//Engine functions
int InitEngine();
void EngineUpdate();
void EngineUpdateEnd();
void EndEngine(int errorOcurred);

#endif