#include "ComponentExample.h"

//Function that returns the ID of this component
static ComponentID ThisComponentID(){
    static ComponentID CompID = -1;
    if(CompID<0)
        CompID = GetComponentID("ComponentExample");
    
    return CompID;
}

extern engineECS ECS;

typedef struct ExampleDataType{
    char example;
}ExampleDataType;

//Runs on AddComponentToEntity
void ComponentExampleConstructor(void** data){
    *data = calloc(1,sizeof(ExampleDataType));
}

//Runs on RemoveComponentFromEntity
void ComponentExampleDestructor(void** data){
    free(*data);
    *data = NULL;
}

void* ComponentExampleCopy(void* data){
    ExampleDataType *newExampleDataType = malloc(sizeof(ExampleDataType));
    memcpy(newExampleDataType,data,sizeof(ExampleDataType));
	return newExampleDataType;
}