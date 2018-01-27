#include "VoxelModification.h"

static ComponentID VoxelModelID = -1;

extern engineECS ECS;

void VoxelModificationInit(){
    VoxelModelID = GetComponentID("VoxelModel");
}

void VoxelModificationUpdate(EntityID entity){
    
    VoxelModel *obj = GetVoxelModelPointer(entity);

    if(obj->modificationStartZ >=0){
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

void VoxelModificationFree(){
    
}