#include "utils.h"

//Número de frames armazenados
#define FRAME_VALUES 10
Uint32 frameTimes[FRAME_VALUES];
Uint32 frameTicksLast;
Uint32 frameCount;
float framesPerSecond;

float GetFPS(){
	return framesPerSecond;
}

void InitFPS() {
	//Inicializa FPS em 0
	memset(frameTimes, 0, sizeof(frameTimes));
	frameCount = 0;
	framesPerSecond = 0;
	frameTicksLast = SDL_GetTicks();
}

void ProcessFPS() {
	Uint32 frameTimesIndex;
	Uint32 currentTicks;
	Uint32 count;
	Uint32 i;

	frameTimesIndex = frameCount % FRAME_VALUES;

	currentTicks = SDL_GetTicks();
	// save the frame time value
	frameTimes[frameTimesIndex] = currentTicks - frameTicksLast;

	// save the last frame time for the next fpsthink
	frameTicksLast = currentTicks;

	// increment the frame count
	frameCount++;

	// Work out the current framerate
	// I've included a test to see if the whole array has been written to or not. This will stop
	// strange values on the first few (FRAME_VALUES) frames.
	if (frameCount < FRAME_VALUES) {
		count = frameCount;
	} else {
		count = FRAME_VALUES;
	}

	// add up all the values and divide to get the average frame time.
	framesPerSecond = 0;
	for (i = 0; i < count; i++) {
		framesPerSecond += frameTimes[i];
	}

	framesPerSecond /= count;

	// now to make it an actual frames per second value...
	framesPerSecond = 1000.f / framesPerSecond;
}

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