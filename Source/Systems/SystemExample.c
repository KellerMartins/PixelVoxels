#include "SystemExample.h"

static System *ThisSystem;

extern engineECS ECS;

//Runs on engine start
void SystemExampleInit(System *systemObject){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("SystemExample"));
}

//Runs each GameLoop iteration
void SystemExampleUpdate(){

    //Run for all entities
    EntityID entity;
	for(entity = 0; entity <= ECS.maxUsedIndex; entity++){
        //Check if entity contains needed components
        //If no component is required , run for all
        if(!IsEmptyComponentMask(ThisSystem->required) && !EntityContainsMask(entity,ThisSystem->required) ) continue;
        //Check if entity doesn't contains the excluded components
        //If there is no restriction, run all with the required components
        if(!IsEmptyComponentMask(ThisSystem->excluded) && EntityContainsMask(entity,ThisSystem->excluded) ) continue;
    }
}

//Runs at engine finish
void SystemExampleFree(){
    
}