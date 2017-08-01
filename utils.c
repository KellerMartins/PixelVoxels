#include "utils.h"

void NormalizeVector(Vector3* v){
	float length = 1/sqrt((v->x*v->x)+(v->y*v->y)+(v->z*v->z));
	v->x *=length;
	v->y *=length;
	v->z *=length;
	//printf("\n|Lenght %f|| %f %f %f |",length,v->x,v->y,v->z);
}
void NormalizeAVector(float* v){
	float length = 1/sqrt((v[0]*v[0])+(v[1]*v[1])+(v[2]*v[2]));
	v[0] *=length;
	v[1] *=length;
	v[2] *=length;
	//printf("\n|Lenght %f|| %f %f %f |",length,v->x,v->y,v->z);
}

Vector3 Reflection(Vector3 *v1,Vector3 *v2)
{
	float dotpr = dot(*v2,*v1);
    Vector3 result;
			result.x = v2->x*2*dotpr;
			result.y = v2->y*2*dotpr;
			result.z = v2->z*2*dotpr;

    result.x =v1->x-result.x;
	result.y =v1->y-result.y;
	result.z =v1->z-result.z;

    return result;
}

int Step(float edge, float x ) 
{
   return x<edge? 0:1;
} 

float Smoothstep(float edge0, float edge1, float x)
{
    // Scale, bias and saturate x to 0..1 range
    x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    // Evaluate polynomial
    return x*x*(3 - 2 * x);
}