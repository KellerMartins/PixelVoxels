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

	if(index<0) //Support acessing from the end of the list
		index += list->length;

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

		list->length +=1;

	}else{
		//Index is list length or off bounds (consider as insertion in the end)
		InsertListEnd(list,e);
	}
}

void RemoveListCell(List *list,ListCellPointer cell){
	if(!cell->previous) return RemoveListStart(list);
	else if(!cell->next) return RemoveListEnd(list);

	cell->next->previous = cell->previous;
	cell->previous->next = cell->next;

	free(cell->element);
	free(cell);

	list->length -=1;
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

	if(index<0) //Support acessing from the end of the list
		index += list->length;

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
	if(!list.last) return NULL;
	return list.last->element;
}

void* GetFirstElement(List list){
	if(!list.first) return NULL;
	return list.first->element;
}

void* GetElementAt(List list,int index){
	int i;

	if(index<0) //Support acessing from the end of the list
		index += list.length;

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

double Distance(Vector3 a, Vector3 b){
	Vector3 AMinusB = Subtract(a,b);
	return sqrt(dot(AMinusB,AMinusB));
}

Vector3 VectorProjection(Vector3 a, Vector3 b){
	//https://en.wikipedia.org/wiki/Vector_projection
	Vector3 normalizedB = NormalizeVector(b);
	double a1 = dot(a, normalizedB);
	return ScalarMult(normalizedB,a1);
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

Vector3 RotatePoint(Vector3 p, Vector3 r, Vector3 pivot){
	return Add(RotateVector(Subtract(p,pivot), EulerAnglesToMatrix3x3(r)),pivot);
}

double DistanceFromPointToLine2D(Vector3 lP1,Vector3 lP2, Vector3 p){
	//https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
	return abs((lP2.y - lP1.y)*p.x - (lP2.x - lP1.x)*p.y + lP2.x*lP1.y - lP2.y*lP1.x)/Distance(lP1,lP2);
}

// ----------- Matrix3x3 type ---------------

inline Matrix3x3 Transpose(Matrix3x3 m){
	Matrix3x3 t;

	t.m[0][1] = m.m[1][0];
	t.m[1][0] = m.m[0][1];

	t.m[0][2] = m.m[2][0];
	t.m[2][0] = m.m[0][2];

	t.m[1][2] = m.m[2][1];
	t.m[2][1] = m.m[1][2];

	t.m[0][0] = m.m[0][0];
	t.m[1][1] = m.m[1][1];
	t.m[2][2] = m.m[2][2];

	return t;
}

Matrix3x3 Identity(){
	Matrix3x3 m;

	memset(m.m[0],0,3*sizeof(int));
	memset(m.m[1],0,3*sizeof(int));
	memset(m.m[2],0,3*sizeof(int));

	m.m[0][0] = 1;
	m.m[1][1] = 1;
	m.m[2][2] = 1;

	return m;
}


//Based on the article: Extracting Euler Angles from a Rotation Matrix - Mike Day, Insomniac Games
Vector3 Matrix3x3ToEulerAngles(Matrix3x3 m){
	Vector3 rotation = VECTOR3_ZERO;
	rotation.x = atan2(m.m[1][2],m.m[2][2]);

	float c2 = sqrt(m.m[0][0]*m.m[0][0] + m.m[0][1]*m.m[0][1] );
	rotation.y = atan2(-m.m[0][2],c2);

	float s1 = sin(rotation.x);
	float c1 = cos(rotation.x);
	rotation.z = atan2(s1*m.m[2][0] - c1*m.m[1][0], c1*m.m[1][1] - s1*m.m[2][1]);

	return rotation;
}

Matrix3x3 EulerAnglesToMatrix3x3(Vector3 rotation){

	float s1 = sin(rotation.x * PI/180.0);
	float c1 = cos(rotation.x * PI/180.0);
	float s2 = sin(rotation.y * PI/180.0);
	float c2 = cos(rotation.y * PI/180.0);
	float s3 = sin(rotation.z * PI/180.0);
	float c3 = cos(rotation.z * PI/180.0);

	Matrix3x3 m = {{{c2*c3            , c2*s3            , -s2  },
				    {s1*s2*c3 - c1*s3 , s1*s2*s3 + c1*c3 , s1*c2},
				    {c1*s2*c3 + s1*s3 , c1*s2*s3 - s1*c3 , c1*c2}}};

	return m;
}

//Vectors are interpreted as rows
inline Vector3 RotateVector(Vector3 v, Matrix3x3 m){
	return (Vector3){v.x*m.m[0][0] + v.y*m.m[1][0] + v.z*m.m[2][0], 
				     v.x*m.m[0][1] + v.y*m.m[1][1] + v.z*m.m[2][1],
					 v.x*m.m[0][2] + v.y*m.m[1][2] + v.z*m.m[2][2]};
}

Matrix3x3 MultiplyMatrix3x3(Matrix3x3 a, Matrix3x3 b){
	Matrix3x3 r;
	int i,j,k;

	for(i=0;i<3;i++){
		for(j=0;j<3;j++){
			r.m[i][j] = 0;
			for(k=0;k<3;k++){
				r.m[i][j] += a.m[i][k]*b.m[k][j];
			}
		}
	}

	return r;
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

// ----------- cJSON wrapper functions ---------------

cJSON *OpenJSON(char path[], char name[]){

	char fullPath[512+256];
    strncpy(fullPath,path,512);
    if(path[strlen(path)-1] != '/'){
        strcat(fullPath,"/");
    }
    strcat(fullPath,name);
    printf("Opening JSON: (%s)\n",fullPath);
    FILE* file = fopen(fullPath,"rb");

	if(file){
		fseek(file,0,SEEK_END);
		unsigned size = ftell(file);
		rewind(file);

		char *jsonString = malloc((size+1) * sizeof(char));
		fread(jsonString, sizeof(char), size, file);
		jsonString[size] = '\0';
		fclose(file);

		cJSON *json = cJSON_Parse(jsonString);
		if (!json)
		{
			//Error treatment
			const char *error_ptr = cJSON_GetErrorPtr();
			if (error_ptr != NULL)
			{
				fprintf(stderr, "OpenJSON: JSON error: %s\n", error_ptr);
			}
			free(jsonString);
			return NULL;

		}else{
			free(jsonString);
			return json;
		}
		
	}else{
		printf("OpenJSON: Failed to open json file!\n");
	}
	return NULL;
}

double JSON_GetObjectDouble(cJSON *object,char *string, double defaultValue){
	cJSON *obj = cJSON_GetObjectItem(object,string);
	if(obj) return obj->valuedouble;
	else return defaultValue;
}

Vector3 JSON_GetObjectVector3(cJSON *object,char *string, Vector3 defaultValue){

	cJSON *arr = cJSON_GetObjectItem(object,string);
	if(!arr) return defaultValue;

	Vector3 v = VECTOR3_ZERO;

	cJSON *item = cJSON_GetArrayItem(arr,0);
    if(item) v.x = item->valuedouble;

    item = cJSON_GetArrayItem(arr,1);
    if(item) v.y = item->valuedouble;

    item = cJSON_GetArrayItem(arr,2);
    if(item) v.z = item->valuedouble;

	return v;
}

// ----------- Lua stack manipulation functions ---------------

//Creates an table with the xyz entries and populate with the vector values
void Vector3ToTable(lua_State *L, Vector3 vector){

    lua_newtable(L);
    lua_pushliteral(L, "x");     //x index
    lua_pushnumber(L, vector.x); //x value
    lua_rawset(L, -3);           //Store x in table

    lua_pushliteral(L, "y");     //y index
    lua_pushnumber(L, vector.y); //y value
    lua_rawset(L, -3);           //Store y in table

    lua_pushliteral(L, "z");     //z index
    lua_pushnumber(L, vector.z); //z value
    lua_rawset(L, -3);           //Store z in table
}

// ----------- Misc. functions ---------------

//Compare if two zero terminated strings are exactly equal
int StringCompareEqual(char *stringA, char *stringB){
	int i, isEqual = 1;

	//Check characters until the string A ends
	for(i=0; stringA[i] != '\0'; i++){
		if(stringB[i] != stringA[i]){
			isEqual = 0; 
			break;
		}
	}

	//Check if B ends in the same point as A, if not B contains A, but is longer than A
	return isEqual? (stringB[i] == '\0'? 1:0) : 0;
}

//Same as above, but case insensitive
int StringCompareEqualCaseInsensitive(char *stringA, char *stringB){
	int i, isEqual = 1;

	//Check characters until the string A ends
	for(i=0; stringA[i] != '\0'; i++){
		if(tolower(stringB[i]) != tolower(stringA[i])){
			isEqual = 0; 
			break;
		}
	}

	//Check if B ends in the same point as A, if not B contains A, but is longer than A
	return isEqual? (stringB[i] == '\0'? 1:0) : 0;
}
