#include "utils.h"

//-------- FPS counter functions -------------

#define STORED_FRAMES 10
Uint32 frameTimes[STORED_FRAMES];
Uint32 frameTicksLast;
Uint32 frameCount;
float framesPerSecond;

float GetFPS(){
	return framesPerSecond;
}

void InitFPS() {
	//Initialize FPS at 0
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

	frameTimesIndex = frameCount % STORED_FRAMES;

	currentTicks = SDL_GetTicks();
	// save the frame time value
	frameTimes[frameTimesIndex] = currentTicks - frameTicksLast;

	// save the last frame time for the next fpsthink
	frameTicksLast = currentTicks;

	// increment the frame count
	frameCount++;

	// Work out the current framerate
	// I've included a test to see if the whole array has been written to or not. This will stop
	// strange values on the first few (STORED_FRAMES) frames.
	if (frameCount < STORED_FRAMES) {
		count = frameCount;
	} else {
		count = STORED_FRAMES;
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

//-------- Generic List Functions -------------

List InitList(unsigned size){
	List l;
	l.first = NULL;
	l.last = NULL;
	l.elementSize = size;
	l.length = 0;

	return l;
}

void FreeList(List *list){
	while(list->first){
		RemoveListStart(list);
	}
}

int IsListEmpty(List list){
	return list.first == NULL? 1:0;
}

void InsertListEnd(List *list, void* e){
	ListCellPointer newCell = malloc(sizeof(ListCell));
	newCell->previous = list->last;
	newCell->next = NULL;

	newCell->element = malloc(list->elementSize);
	memcpy(newCell->element,e,list->elementSize);

	if(list->last){
		list->last->next = newCell;
		list->last = newCell;
	}else{
		list->first = newCell;
		list->last = newCell;
	}

	list->length +=1;
}

void InsertListStart(List *list, void* e){
	ListCellPointer newCell = malloc(sizeof(ListCell));
	newCell->previous = NULL;
	newCell->next = list->first;

	newCell->element = malloc(list->elementSize);
	memcpy(newCell->element,e,list->elementSize);

	if(list->first){
		list->first->previous = newCell;
		list->first = newCell;
	}else{
		list->first = newCell;
		list->last = newCell;
	}

	list->length +=1;
}

void InsertListIndex(List *list, void* e, int index){
	int i;
	//Get the element that will go after the element to be inserted
	ListCellPointer current = list->first;
	for(i=0;i<index;i++){
		current = GetNextCell(current);
	}

	//If the index is already ocupied
	if(current != NULL){
		ListCellPointer newCell = malloc(sizeof(ListCell));
		newCell->element = malloc(list->elementSize);
		memcpy(newCell->element,e,list->elementSize);
		newCell->next = current;

		//Connect the cells to their new parents
		newCell->previous = current->previous;	
		current->previous = newCell;

		//If the index is 0 (first), set newCell as first
		if(list->first == current){
			list->first = newCell;
		}
			
		//If the index is list length (last), set newCell as last
		if(list->last == current){
			list->last = newCell;
		}

		//If the previous is not null, point his next to newCell
		if(newCell->previous){
			newCell->previous->next = newCell;
		}

	}else{
		//Index is list length or off bounds (consider as insertion in the end)
		InsertListEnd(list,e);
	}

	list->length +=1;
}

void RemoveListEnd(List *list){
	if(list->last->previous){
		ListCellPointer aux = list->last->previous;
		free(list->last->element);
		free(list->last);

		aux->next = NULL;
		list->last = aux;
	}else{
		free(list->last->element);
		free(list->last);

		list->last = NULL;
		list->first = NULL;
	}

	list->length -=1;
}

void RemoveListStart(List *list){
	if(IsListEmpty(*list)) return;

	if(list->first->next){
		ListCellPointer aux = list->first->next;
		free(list->first->element);
		free(list->first);

		aux->previous = NULL;
		list->first = aux;
	}else{
		free(list->first->element);
		free(list->first);

		list->first = NULL;
		list->last = NULL;
	}

	list->length -=1;
}

void RemoveListIndex(List *list,int index){
	int i;
	if(index == 0) return RemoveListStart(list);
	else if(index == list->length-1) return RemoveListEnd(list);

	ListCellPointer current = list->first;
	for(i=0;i<index;i++){
		current = GetNextCell(current);
	}

	current->next->previous = current->previous;
	current->previous->next = current->next;

	free(current->element);
	free(current);

	list->length -=1;
}

void* GetElement(ListCell c){
	return c.element;
}

void* GetLastElement(List list){
	return list.last->element;
}

void* GetFirstElement(List list){
	return list.first->element;
}

void* GetElementAt(List list,int index){
	int i;

	ListCellPointer current = list.first;
	for(i=0;i<index;i++){
		current = GetNextCell(current);
	}

	return current->element;
}

ListCellPointer GetNextCell(ListCellPointer c){
	if(!c) return NULL;
	return c->next;
}

ListCellPointer GetPreviousCell(ListCellPointer c){
	if(!c) return NULL;
	return c->previous;
}

ListCellPointer GetFirstCell(List list){
	return list.first;
}

ListCellPointer GetLastCell(List list){
	return list.last;
}

ListCellPointer GetCellAt(List list,int index){
	int i;

	ListCellPointer current = list.first;
	for(i=0;i<index;i++){
		current = GetNextCell(current);
	}

	return current;
}


unsigned GetElementSize(List list){
	return list.elementSize;
}

unsigned GetLength(List list){
	return list.length;
}

//-------- Vector Functions ---------

Vector3 NormalizeVector(Vector3 v){
	float length = 1/sqrt((v.x*v.x)+(v.y*v.y)+(v.z*v.z));
	v.x *=length;
	v.y *=length;
	v.z *=length;
	return v;
}

Vector3 Add(Vector3 a, Vector3 b){
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

Vector3 Subtract(Vector3 a, Vector3 b){
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

Vector3 ScalarMult(Vector3 v, float s){
	return (Vector3){ v.x * s, v.y * s, v.z * s };
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

Vector3 RotatePoint(Vector3 p, float rx, float ry, float rz, float pivotX, float pivotY, float pivotZ){

	float rotx,roty,rotz,x,y,z;

	float sinx = sin(rx* PI_OVER_180);
	float cosx = cos(rx* PI_OVER_180);

	float siny = sin(ry * PI_OVER_180);
	float cosy = cos(ry * PI_OVER_180);
		
	float sinz = sin(rz * PI_OVER_180);
	float cosz = cos(rz * PI_OVER_180);

	x = p.x - pivotX;
	y = p.y - pivotY;
	z = p.z - pivotZ;

	rotx = x*cosy*cosz + y*(cosz*sinx*siny - cosx*sinz) + z*(cosx*cosz*siny + sinx*sinz);
	roty = x*cosy*sinz + z*(cosx*siny*sinz - cosz*sinx) + y*(cosx*cosz + sinx*siny*sinz);
	rotz = z*cosx*cosy + y*sinx*cosy - x*siny;

	x = rotx + pivotX;
	y = roty + pivotY;
	z = rotz + pivotZ;

	p.x = x;
	p.y = y;
	p.z = z;
	return p;

}

// ----------- Numeric functions ---------------

float Lerp(double t, float a, float b){
    return (1-t)*a + t*b;
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

//Modulus function, returning only positive values
int Modulus(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

float fModulus(float a, float b)
{
    float r = fmod(a,b);
    return r < 0 ? r + b : r;
}

// ----------- Misc. functions ---------------

