#include "voxelModel.h"

void FreeObject(VoxelObject *obj){
    if(!obj->model) return;

    free(obj->model);
    free(obj->lighting);
    free(obj->vertices);
    free(obj->vColors);

    obj->model = NULL;
}

void FreeMultiObject(MultiVoxelObject *obj){
    FreeObjectList(&obj->objects);
}

VoxelObjectList InitializeObjectList(){
    VoxelObjectList list;
    list.list = NULL;
    list.numberOfObjects = 0;
    return list;
}
void FreeObjectList(VoxelObjectList *list){
    if(!list){printf("Cant free list: list is empty!\n"); return;}

    int i;
    for(i=0;i<list->numberOfObjects;i++){
       FreeObject(list->list[i]);
    }
    free(list->list);
}

void AddObjectInList(VoxelObjectList *dest, VoxelObject *obj){
    if(!dest->list){
        dest->list = malloc(sizeof(VoxelObject *));
        dest->list[0] = obj;
        dest->numberOfObjects = 1;
    }else{
        VoxelObject **newList = malloc( (dest->numberOfObjects+1) * sizeof(VoxelObject *));
        memcpy(newList,dest->list,dest->numberOfObjects*sizeof(VoxelObject *));

        free(dest->list);
        dest->list = newList;
        dest->list[dest->numberOfObjects++] = obj;
    }
}

//Combine multiple ObjectLists into one
//If dest is NULL, the resulting list contains all sources
//If it isnt, the resulting list contains the dest and all sources, and the original list of the dest is freed
void CombineObjectLists(VoxelObjectList *dest, int numberOfSources,...){

    if(!dest){printf("Error: destination list is NULL!\n"); return;}

    va_list args;
	
    //Counts the number of objects in the final list
    int i,j,objectsCount = 0;
	va_start(args,numberOfSources);

        for(i=0; i<numberOfSources; i++){
            VoxelObjectList current = va_arg(args,VoxelObjectList);
            objectsCount += current.numberOfObjects;
        }

    va_end(args);

    if(dest->list){
        objectsCount += dest->numberOfObjects;
    }
    
    //Allocate the final list
	VoxelObject **final = calloc(objectsCount,sizeof(VoxelObject *));

    //Iterate in every source list, getting the objects in their list and putting in the final
	va_start(args,numberOfSources);
	int pos = 0;

    if(dest->list){
        for(j=0; j<dest->numberOfObjects; j++){
			final[pos++] = dest->list[j];
		}
    }

	for(i=0; i<numberOfSources; i++){

		VoxelObjectList current = va_arg(args,VoxelObjectList);

		for(j=0; j<current.numberOfObjects; j++){
			final[pos++] = current.list[j];
		}
	}
    va_end(args);

    dest->list = final;
    dest->numberOfObjects = objectsCount;
}