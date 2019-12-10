#ifndef UTILS_H
#define UTILS_H
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <SDL2/SDL.h>
#include "Libs/cJSON.h"
#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>

#define PI 3.14159265358979323846
#define VECTOR3_ZERO (Vector3){0.0f,0.0f,0.0f}
#define VECTOR3_FORWARD (Vector3){1.0f,0.0f,0.0f}
#define VECTOR3_UP (Vector3){0.0f,0.0f,1.0f}
#define VECTOR3_DOWN (Vector3){0.0f,0.0f,-1.0f}
#define VECTOR3_LEFT (Vector3){0.0f,1.0f,0.0f}

#define INT_INFINITY  0x3f3f3f3f

#ifndef PATH_MAX
#define PATH_MAX 260
#endif
#ifndef FILENAME_MAX
#define FILENAME_MAX 256
#endif

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

//Trie structures and interface

//Add all supported types here, as 'Trie_type'
typedef enum TrieType {Trie_None, Trie_Pointer, Trie_String, Trie_Vector3, Trie_double, Trie_float, Trie_char, Trie_int }TrieType;

//Structure used to retrieve all the data from the trie
//Add all supported types inside the union, as 'type* typeValue'
typedef struct TrieElement{
    char *key;
    TrieType type;
    unsigned size;
    union{
        void* pointerValue;
        char* stringValue;
        Vector3* vector3Value;
        double* doubleValue;
        float* floatValue;
        char* charValue;
        int* intValue;
    };
}TrieElement;

#define TRIE_ALPHABET_SIZE 256
typedef struct TrieCell{
    TrieType elementType;
    unsigned maxKeySize;
    union{
        unsigned numberOfElements;
        unsigned elementSize;
    };
    //In a leaf and branch, the '\0' points to the data stored, while in a trunk it points to NULL
    //In both branch and trunk, all the other characters points to other Tries
    void* branch[TRIE_ALPHABET_SIZE];
}Trie;

Trie InitTrie();
void FreeTrie(Trie *trie);

void InsertTrie(Trie *trie, const char* key, const void *value, int size, TrieType valueType);
void InsertTrieString(Trie *trie, const char* key, const char* value);

int TrieContainsKey(Trie trie, const char* key);
void* GetTrieElement(Trie trie, const char* key);
void* GetTrieElementWithProperties(Trie trie, const char* key, int *sizeOut, TrieType *typeOut);
void* GetTrieElementAsPointer(Trie trie, const char* key, void* defaultValue);
char* GetTrieElementAsString(Trie trie, const char* key, char* defaultValue);

//Macro to generate headers for the insertion and retrieval functions
//Remember to call the function template macro on utils.c and to modify the TrieType enum when adding more types
#define TRIE_TYPE_FUNCTION_HEADER_MACRO(type) \
void InsertTrie_ ## type (Trie *trie, const char* key,  type value);\
type GetTrieElementAs_ ## type (Trie trie, const char* key,  type defaultValue);

TRIE_TYPE_FUNCTION_HEADER_MACRO(Vector3)
TRIE_TYPE_FUNCTION_HEADER_MACRO(double)
TRIE_TYPE_FUNCTION_HEADER_MACRO(float)
TRIE_TYPE_FUNCTION_HEADER_MACRO(char)
TRIE_TYPE_FUNCTION_HEADER_MACRO(int)

//Functions to obtain all data inside a trie
//The data returned should be used before any new replace modifications
//are made in the trie, as the data pointed can be freed when replaced
TrieElement *GetTrieElementsArray(Trie trie, int* outElementsCount);
void FreeTrieElementsArray(TrieElement* elementsArray, int elementsCount);



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

void RemoveListCell(List *list,ListCellPointer cell);
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

double DistanceFromPointToLine2D(Vector3 lP1,Vector3 lP2, Vector3 p);


typedef struct Matrix3x3{
	float m[3][3];
}Matrix3x3;

Matrix3x3 Transpose(Matrix3x3 m);
Matrix3x3 Identity();
Vector3 Matrix3x3ToEulerAngles(Matrix3x3 m);
Matrix3x3 EulerAnglesToMatrix3x3(Vector3 rotation);
Vector3 RotateVector(Vector3 v, Matrix3x3 m);
Vector3 RotatePoint(Vector3 p, Vector3 r, Vector3 pivot);
Matrix3x3 MultiplyMatrix3x3(Matrix3x3 a, Matrix3x3 b);

typedef struct Matrix4x4{
	float m[4][4];
}Matrix4x4;

Matrix4x4 Identity4x4();
Matrix4x4 GetProjectionMatrix(float right, float left, float top, float bottom, float near, float far);

float Lerp(double t, float a, float b);
int Step(float edge, float x );
float Smoothstep(float edge0, float edge1, float x);
int Modulus(int a, int b);
float fModulus(float a, float b);


cJSON *OpenJSON(char path[], char name[]);
double JSON_GetObjectDouble(cJSON *object,char *string, double defaultValue);
Vector3 JSON_GetObjectVector3(cJSON *object,char *string, Vector3 defaultValue);
cJSON *JSON_CreateVector3(Vector3 value);

void Vector3ToTable(lua_State *L, Vector3 vector);


typedef enum LogLevel {NoLogging, Error, Warning, Info, Debug}LogLevel;
void SetLogLevel(LogLevel lvl);
void PrintLog(LogLevel lvl, const char *string, ...);
void OpenLogFile(const char *path);
void CloseLogFile();

int StringCompareEqual(char *stringA, char *stringB);
int StringCompareEqualCaseInsensitive(char *stringA, char *stringB);

#endif