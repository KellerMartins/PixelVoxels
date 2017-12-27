#ifndef UTILS_H
#define UTILS_H
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#define PI_OVER_180 0.01745329251
#define PI 3.14159265359
#define ONE_OVER_256 0.00390625
#define VECTOR3_ZERO (Vector3){0.0f,0.0f,0.0f}
#define VECTOR3_FORWARD (Vector3){1.0f,0.0f,0.0f}
#define VECTOR3_UP (Vector3){0.0f,0.0f,1.0f}
#define VECTOR3_DOWN (Vector3){0.0f,0.0f,-1.0f}
#define VECTOR3_LEFT (Vector3){0.0f,1.0f,0.0f}

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define sign(x) (x > 0 ? 1 : (x < 0 ? -1 : 0))
#define clamp(x,m,M) (x < m? m : (x > M ? M : x))
#define FRAC0(x) (x - floorf(x))
#define FRAC1(x) (1 - x + floorf(x))

#define cross(u,v)   (Vector3){ (u).y * (v).z - (u).z * (v).y , (u).z * (v).x - (u).x * (v).z, (u).x * (v).y - (u).y * (v).x}
#define dot(u,v)   ( (u).x * (v).x + (u).y * (v).y + (u).z * (v).z )
#define norm(v)     sqrt(Dot(v,v))     // norm = length of  vector
#define distance(u,v)      Norm(Subtract(u,v))          // distance = norm of difference

typedef struct Vector3{
	float x;
	float y;
	float z;
}Vector3;

//Generic list implementation
//In this implementation, every new element added is copied to the list, not just referenced
//If there is the need to use this list to reference an variable, initialize the list as a pointers list
//Untested for now

typedef struct ListCell* ListCellPointer;
typedef struct ListCell{
  void *element;
  ListCellPointer next;
  ListCellPointer previous;
}ListCell;

typedef struct List{
  unsigned elementSize;
  ListCellPointer first;
  ListCellPointer last;
  unsigned length;
}List;

List InitList(unsigned size);
void FreeList(List *list);

int IsListEmpty(List list);

void InsertListEnd(List *list, void* e);
void InsertListStart(List *list, void* e);
void InsertListIndex(List *list, void* e, unsigned index);

void RemoveListEnd(List *list);
void RemoveListStart(List *list);
void RemoveListIndex(List *list,unsigned index);

void* GetElement(ListCell c);
void* GetLast(List list);
void* GetFirst(List list);
void* GetAt(List list,unsigned index);

ListCellPointer GetNextCell(ListCellPointer c);
unsigned GetElementSize(List list);
unsigned GetLength(List list);



void InitFPS();
void ProcessFPS();
float GetFPS();

Vector3 NormalizeVector(Vector3 v);
Vector3 Add(Vector3 a, Vector3 b);
Vector3 Subtract(Vector3 a, Vector3 b);
Vector3 Reflection(Vector3 *v1,Vector3 *v2);
Vector3 RotatePoint(Vector3 p, float rx, float ry, float rz, float pivotX, float pivotY, float pivotZ);
int Step(float edge, float x );
float Smoothstep(float edge0, float edge1, float x);
int Modulus(int a, int b);
float fModulus(float a, float b);
#endif