//Example component to illustrate the interface of the required functions of the components in this ECS architecture
//These functions are required to avoid coupling the components and the base engine, allowing to change all the components
//and add more components without modifying the engine, while supporting the manipulation of the components by the engine

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
//Receives an position of memory, where this functions should allocate the needed data
//It also needs to treat the case of the data argument being NULL
void ComponentExampleConstructor(void** data){
    if(!data) return;
    *data = calloc(1,sizeof(ExampleDataType));
}


//Runs on RemoveComponentFromEntity
//Receives an position of memory, where this functions free the current data and set it to NULL
//It also needs to treat the case of the data argument being NULL
void ComponentExampleDestructor(void** data){
    if(!data) return;
    free(*data);
    *data = NULL;
}


//Runs on DuplicateEntity
//Receives an position of memory, were the origin data to be copied is, and returns an new allocated space with the copy
//It also needs to treat the case of the data argument being NULL
void* ComponentExampleCopy(void* data){
    if(!data) return NULL;
    ExampleDataType *newExampleDataType = malloc(sizeof(ExampleDataType));
    memcpy(newExampleDataType,data,sizeof(ExampleDataType));
	return newExampleDataType;
}


//Runs on EncodeEntity
//Receives an position of memory containing the origin data and a cJSON* object to check for differences
//Returns an cJSON* object or NULL

//currentData is used to encode only the changes made to an prefab
//If currentData is NULL, it should only encode the data to a new cJSON* object
//If currentData is not NULL, it should check if the data contained in the currentData is different from the contained on the memory
//  In the case of difference between the two, it should encode the data in memory and return it in a cJSON* object
//  If they are equal, it should return NULL

//It also needs to treat the case of the data argument being NULL
cJSON* ComponentExampleEncode(void** data, cJSON* currentData){
    if(!data) return NULL;
    ExampleDataType *ex = *data; 

    int hasChanged = 0;
    if(currentData){
        //Check if any data has changed 
        if(ex->example != cJSON_GetObjectItem(currentData, "exampleChar")->valuestring[0]){
            hasChanged = 1;
        }
    }

    //Encode this component if its not from a prefab (who has currentData) or if it has changed
    if(!currentData || hasChanged){
        cJSON *obj = cJSON_CreateObject();
        char c[] = "c";
        c[0] = ex->example;

        cJSON_AddStringToObject(obj, "exampleChar", c);

        return obj;
    }
    return NULL;
}


//Runs on DecodeEntity
//Receives a cJSON* object containing the data
//Returns an void* with the component allocated
void* ComponentExampleDecode(cJSON **data){
    ExampleDataType *ex = malloc(sizeof(ExampleDataType));
    ex->example = cJSON_GetObjectItem(*data, "exampleChar")->valuestring[0];

    return ex;
}