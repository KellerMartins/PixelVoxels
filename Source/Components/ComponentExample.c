#include "ComponentExample.h"

//Function that returns the ID of this component
static ComponentID ThisComponentID(){
    static ComponentID CompID = -1;
    if(CompID<0)
        CompID = GetComponentID("ComponentExample");
    
    return CompID;
}

extern engineECS ECS;

//Runs on AddComponentToEntity
void ComponentExampleConstructor(EntityID entity){

}

//Runs on RemoveComponentFromEntity
void ComponentExampleDestructor(EntityID entity){
    
}