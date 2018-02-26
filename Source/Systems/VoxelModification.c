#include "VoxelModification.h"

static System *ThisSystem;
static ComponentID VoxelModelID = -1;

extern engineECS ECS;

void VoxelModificationInit(){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("VoxelModification"));

    VoxelModelID = GetComponentID("VoxelModel");
}

void VoxelModificationUpdate(){
    EntityID entity;
	for(entity = 0; entity <= ECS.maxUsedIndex; entity++){

        //Check if entity contains needed components
        //If no component is required , run for all
        if(!IsEmptyComponentMask(ThisSystem->required) && !EntityContainsMask(entity,ThisSystem->required) ) continue;
        //Check if entity doesn't contains the excluded components
        //If there is no restriction, run all with the required components
        if(!IsEmptyComponentMask(ThisSystem->excluded) && EntityContainsMask(entity,ThisSystem->excluded) ) continue;
        VoxelModel *obj = GetVoxelModelPointer(entity);

        if(obj->model && obj->modificationStartZ >=0){
            
            CalculateRendered(entity);
            CalculateLighting(entity);

            obj->modificationStartX = -1;
            obj->modificationEndX = -1;

            obj->modificationStartY = -1;
            obj->modificationEndY = -1;

            obj->modificationStartZ = -1;
            obj->modificationEndZ = -1;
        }
    }
}

void VoxelModificationFree(){
    
}