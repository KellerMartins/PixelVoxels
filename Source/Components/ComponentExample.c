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
    if(!data) return;
    *data = calloc(1,sizeof(ExampleDataType));
}

//Runs on RemoveComponentFromEntity
void ComponentExampleDestructor(void** data){
    if(!data) return;
    free(*data);
    *data = NULL;
}

void* ComponentExampleCopy(void* data){
    if(!data) return NULL;
    ExampleDataType *newExampleDataType = malloc(sizeof(ExampleDataType));
    memcpy(newExampleDataType,data,sizeof(ExampleDataType));
	return newExampleDataType;
}

cJSON* ComponentExampleEncode(void** data){
    ExampleDataType *ex = *data; 
    cJSON *obj = cJSON_CreateObject();
    char c[] = "c";
    c[0] = ex->example;

    cJSON_AddStringToObject(obj, "exampleChar", c);

    return obj;
}

void* ComponentExampleDecode(cJSON **data){
    ExampleDataType *ex = malloc(sizeof(ExampleDataType));
    ex->example = cJSON_GetObjectItem(*data, "exampleChar")->valuestring[0];

    return ex;
}