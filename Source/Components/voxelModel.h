#ifndef VOXELMODEL_H
#define VOXELMODEL_H
#include "../utils.h"
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

typedef struct AnchorPoint{
	char type;
  	char x;
	char y;
	char z;
}AnchorPoint;

typedef struct VoxelObject{
	int enabled;
	unsigned int timeOfActivation;

	char modificationStartX;
	char modificationEndX;

	char modificationStartY;
	char modificationEndY;

	char modificationStartZ;
	char modificationEndZ;

    unsigned char *model;
    unsigned char *lighting;
    GLfloat *vertices;
	GLfloat *vColors;
	int numberOfVertices;
	int numberOfPoints;
	AnchorPoint *points;

	unsigned int dimension[3];

    unsigned int voxelCount;
	unsigned int voxelsRemaining;

	Vector3 position;
	Vector3 rotation;
	Vector3 center;
}VoxelObject;

//A simpler list implementation, where each element of the list is a pointer to
//an object. As this list is rarely changed during the game loop, I opted to keep
//it simple instead of using the other, more general list implementation on this
typedef struct VoxelObjectList{
	VoxelObject **list;
	unsigned int numberOfObjects;
}VoxelObjectList;

typedef struct MultiVoxelObject{
	int enabled;
	VoxelObjectList objects;

	Vector3 position;
	Vector3 rotation;
	Vector3 center;

}MultiVoxelObject;

void FreeObject(VoxelObject *obj);
void FreeMultiObject(MultiVoxelObject *obj);

VoxelObjectList InitializeObjectList();
void FreeObjectList(VoxelObjectList *list);
void AddObjectInList(VoxelObjectList *dest, VoxelObject *obj);
void RenderObjectList(VoxelObjectList objs, VoxelObjectList shadowCasters);
void CombineObjectLists(VoxelObjectList *dest,  int numberOfSources,...);

#endif