#ifndef UTILS_H
#define UTILS_H
#include <math.h>
#include <stdarg.h>
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
#define norm(v)     sqrt(dot(v,v))     // norm = length of  vector
#define d(u,v)      norm(u-v)          // distance = norm of difference

typedef struct Vector3{
	float x;
	float y;
	float z;
}Vector3;

void NormalizeVector(Vector3* v);
void NormalizeAVector(float* v);
Vector3 Reflection(Vector3 *v1,Vector3 *v2);
int Step(float edge, float x );
float Smoothstep(float edge0, float edge1, float x);
#endif