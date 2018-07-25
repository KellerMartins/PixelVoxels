#ifndef VOXELMODEL_H
#define VOXELMODEL_H
#include "../Engine.h"
#include "Transform.h"
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

typedef struct VoxelModel{
	int enabled;
	int smallScale;

	char modificationStartX;
	char modificationEndX;

	char modificationStartY;
	char modificationEndY;

	char modificationStartZ;
	char modificationEndZ;

    unsigned char *model;
    unsigned char *lighting;
	GLuint vbo[3];
	int numberOfVertices;

	unsigned int dimension[3];

    unsigned int voxelCount;
	unsigned int voxelsRemaining;

	Vector3 center;

	char modelPath[512];
	char modelName[256];
	char objectName[65];
}VoxelModel;

void VoxelModelConstructor(void** data);
void VoxelModelDestructor(void** data);
void* VoxelModelCopy(void* data);
cJSON* VoxelModelEncode(void** data, cJSON* currentData);
void* VoxelModelDecode(cJSON **data);

VoxelModel* GetVoxelModelPointer(EntityID entity);

Vector3 GetVoxelModelCenter(EntityID entity);
void SetVoxelModelCenter(EntityID entity, Vector3 center);
int IsVoxelModelEnabled(EntityID entity);
void SetVoxelModelEnabled(EntityID entity, int booleanVal);
int IsVoxelModelSmallScale(EntityID entity);
void SetVoxelModelSmallScale(EntityID entity, int booleanVal);

void LoadVoxelModel(EntityID entity, char modelPath[], char modelName[]);
void LoadMultiVoxelModel(EntityID entity, char modelPath[], char modelName[]);
int IsMultiVoxelModelFile(char modelPath[], char modelName[]);

void CalculateRendered(EntityID entity);
void CalculateLighting(EntityID entity);

void VoxelModelRegisterLuaFunctions();

#endif