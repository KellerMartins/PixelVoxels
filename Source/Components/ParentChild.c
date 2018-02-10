#include "ParentChild.h"

//Function that returns the ID of this component
static ComponentID ThisComponentID(){
    static ComponentID CompID = -1;
    if(CompID<0)
        CompID = GetComponentID("ParentChild");
    
    return CompID;
}

extern engineECS ECS;

void ParentChildConstructor(void** data){
    *data = calloc(1,sizeof(ParentChild));
    ParentChild *parent = *data;

    parent->childs = InitList(sizeof(EntityID));
    parent->isParent = 0;
    parent->isChild = 0;
}

void ParentChildDestructor(void** data){
    ParentChild *parent = *data;
    FreeList(&parent->childs);

    free(*data);
    *data = NULL;
}

void* ParentChildCopy(void* data){
    ParentChild *newParentChild = malloc(sizeof(ParentChild));
    memcpy(newParentChild,data,sizeof(ParentChild));

    newParentChild->childs = InitList(sizeof(EntityID));

    ListCellPointer cell;
    ListForEach(cell,((ParentChild*)data)->childs){
        InsertListEnd(&newParentChild->childs,GetElement(*cell));
    }

	return newParentChild;
}

int IsParent(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("IsParent: Entity doesn't have a ParentChild component. (%d)\n",entity);
        return 0;
    }
    ParentChild *parent = (ParentChild *)ECS.Components[ThisComponentID()][entity].data;
    return parent->isParent;
}

int IsChild(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("IsChild: Entity doesn't have a ParentChild component. (%d)\n",entity);
        return 0;
    }
    ParentChild *child = (ParentChild *)ECS.Components[ThisComponentID()][entity].data;
    return child->isChild;
}

void SetParent(EntityID child, EntityID parent){

    //If the future child doesn't have this component, add to it
    if(!EntityContainsComponent(child, ThisComponentID())){
        AddComponentToEntity(ThisComponentID(),child);
    }

    ParentChild *childComp = (ParentChild *)ECS.Components[ThisComponentID()][child].data;

    //If is already a child of another parent, remove it first
    if(IsChild(child)){
        UnsetParent(child);
    }

    //If the future parent doesn't have this component already, add to it
    if(!EntityContainsComponent(parent, ThisComponentID())){
        AddComponentToEntity(ThisComponentID(), parent);
    }

    ParentChild *parentComp = (ParentChild *)ECS.Components[ThisComponentID()][parent].data;

    InsertListEnd(&parentComp->childs, &child);
    childComp->parent = parent;

    childComp->isChild = 1;
    parentComp->isParent = 1;
}

int UnsetParent(EntityID child){
    if(!EntityContainsComponent(child, ThisComponentID())){
        printf("UnsetParent: Child doesn't have a ParentChild component. (%d)\n",child);
        return 0;
    }
    if(!IsChild(child)){
        printf("UnsetParent: Entity is not anyone's child. (%d)\n",child);
        return 0;
    }

    ParentChild *childComp = ECS.Components[ThisComponentID()][child].data;
    ParentChild *parentComp = ECS.Components[ThisComponentID()][childComp->parent].data;

    //Find the index of the child in the list of childs
    int index = 0;
    ListCellPointer current = GetFirstCell(parentComp->childs);
    while(current){
        EntityID cID = *((EntityID*) GetElement(*current));

        //If found
        if(cID == child){
            RemoveListIndex(&parentComp->childs,index);

            childComp->isChild = 0;

            //If his old parent has an empty list of childs, set isParent to zero
            if(IsListEmpty(parentComp->childs)){
                parentComp->isParent = 0;
            }

            return 1;
        }
        current = GetNextCell(current);
        index++;
    }

    //Return zero if can't find the child's index in the list (Indicative of implementation error)
    printf("RemoveChild: Child is not an parent's child (Shouldn't happen). (P:%d  C:%d)\n",childComp->parent,child);
    return 0;
}