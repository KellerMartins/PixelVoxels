#ifndef UTILS_H
#define UTILS_H
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
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

/* --- PRINTF_BYTE_TO_BINARY macro's --- */
#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)    \
    (((i) & 0x80ll) ? '1' : '0'), \
    (((i) & 0x40ll) ? '1' : '0'), \
    (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
    PRINTF_BINARY_PATTERN_INT8              PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
    PRINTF_BYTE_TO_BINARY_INT8((i) >> 8),   PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32 \
    PRINTF_BINARY_PATTERN_INT16             PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
    PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), PRINTF_BYTE_TO_BINARY_INT16(i)
#define PRINTF_BINARY_PATTERN_INT64    \
    PRINTF_BINARY_PATTERN_INT32             PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i) \
    PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), PRINTF_BYTE_TO_BINARY_INT32(i)
/* --- end macros --- */

#define sign(x) (x > 0 ? 1 : (x < 0 ? -1 : 0))
#define clamp(x,m,M) (x < m? m : (x > M ? M : x))
#define FRAC0(x) (x - floorf(x))
#define FRAC1(x) (1 - x + floorf(x))

#define cross(u,v)  (Vector3){ (u).y * (v).z - (u).z * (v).y , (u).z * (v).x - (u).x * (v).z, (u).x * (v).y - (u).y * (v).x}
#define dot(u,v)  ( (u).x * (v).x + (u).y * (v).y + (u).z * (v).z )
#define norm(v) sqrt(dot(v,v))                            // norm = length of  vector

typedef struct Vector3{
	float x;
	float y;
	float z;
}Vector3;

typedef struct Pixel{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
	
}Pixel;

//Generic list implementation
//In this implementation, every new element added is copied to the list, not just referenced
//If there is the need to use this list to reference an variable, initialize the list as a pointers list

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
void InsertListIndex(List *list, void* e, int index);

void RemoveListEnd(List *list);
void RemoveListStart(List *list);
void RemoveListIndex(List *list,int index);

void* GetElement(ListCell c);
void* GetLastElement(List list);
void* GetFirstElement(List list);
void* GetElementAt(List list,int index);

ListCellPointer GetNextCell(ListCellPointer c);
ListCellPointer GetPreviousCell(ListCellPointer c);
ListCellPointer GetLastCell(List list);
ListCellPointer GetFirstCell(List list);
ListCellPointer GetCellAt(List list,int index);

unsigned GetElementSize(List list);
unsigned GetLength(List list);

#define ListForEach(cellPointer,list) for(cellPointer = GetFirstCell(list); cellPointer != NULL; cellPointer = GetNextCell(cellPointer))
#define GetElementAsType(cellPointer, type)  (*((type*)GetElement(*cellPointer)))


void InitFPS();
void ProcessFPS();
float GetFPS();

Vector3 NormalizeVector(Vector3 v);
Vector3 Add(Vector3 a, Vector3 b);
Vector3 Subtract(Vector3 a, Vector3 b);
Vector3 ScalarMult(Vector3 v, float s);
double Distance(Vector3 a, Vector3 b);
Vector3 VectorProjection(Vector3 a, Vector3 b);

Vector3 Reflection(Vector3 *v1,Vector3 *v2);
Vector3 RotatePoint(Vector3 p, float rx, float ry, float rz, float pivotX, float pivotY, float pivotZ);

double DistanceFromPointToLine2D(Vector3 lP1,Vector3 lP2, Vector3 p);

float Lerp(double t, float a, float b);
int Step(float edge, float x );
float Smoothstep(float edge0, float edge1, float x);
int Modulus(int a, int b);
float fModulus(float a, float b);

int StringCompareEqual(char *stringA, char *stringB);
int StringCompareEqualCaseInsensitive(char *stringA, char *stringB);

#endif