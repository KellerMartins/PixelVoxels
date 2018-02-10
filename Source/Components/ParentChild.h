#ifndef PARENT_H
#define PARENT_H
#include "../Engine.h"

//Note that an parent can be an child and vice versa
//But, by creation, the entity containing this component is not an parent nor an child
typedef struct ParentChild{
    int isParent;
    int isChild;
    //If parent: List of his child EntityIDs
    List childs;
    //If child: ID of his parent
    EntityID parent;

}ParentChild;

void ParentChildConstructor(void** data);
void ParentChildDestructor(void** data);
void* ParentChildCopy(void* data);

int IsParent(EntityID entity);
int IsChild(EntityID entity);

void SetParent(EntityID child, EntityID parent);
int UnsetParent(EntityID child);

#endif