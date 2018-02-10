#ifndef VOXELMODEL_H
#define VOXELMODEL_H
#include "../Engine.h"
#include "Transform.h"
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

typedef struct VoxelModel{
	int enabled;

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

	unsigned int dimension[3];

    unsigned int voxelCount;
	unsigned int voxelsRemaining;

	Vector3 center;
}VoxelModel;

void VoxelModelConstructor(void** data);
void VoxelModelDestructor(void** data);
void* VoxelModelCopy(void* data);

VoxelModel* GetVoxelModelPointer(EntityID entity);

void LoadVoxelModel(EntityID entity, char modelPath[]);
void CalculateRendered(EntityID entity);
void CalculateLighting(EntityID entity);
//void LoadMultiVoxelModel(EntityID entity, char modelPath[]);


#endif