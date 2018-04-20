#include "Engine.h"

engineCore Core;
engineScreen Screen;
engineTime Time;
engineECS ECS = {.maxEntities = 0,.maxUsedIndex = 0, .Entities = 0};
engineRendering Rendering;
engineInput Input;

unsigned char initializedEngine = 0;
unsigned char initializedECS = 0;
unsigned char initializedInput = 0;

int exitGame = 0;

//-------- ECS Functions -------------

//Initialize the Entity Component System lists and arrays
//Needs to be called before registering any component 
int InitECS(unsigned max_entities){
	if(initializedECS){
		printf("ECS already initialized!\n");
		return 0;
	}
	if(initializedEngine){
		printf("Engine already initialized! Initialize and configure ECS before initializing the engine!\n");
		return 0;
	}

	ECS.maxEntities = max_entities;
	ECS.Entities = calloc(max_entities,sizeof(Entity));
	ECS.AvaliableEntitiesIndexes = InitList(sizeof(int));

	//Populate avaliable entities with all indexes
	int i;
	for(i=0;i<max_entities;i++){
		InsertListEnd(&ECS.AvaliableEntitiesIndexes,(void*)&i);
	}	

	ECS.Components = NULL;
	ECS.ComponentTypes = InitList(sizeof(ComponentType));

	ECS.SystemList = InitList(sizeof(System));

	initializedECS = 1;
	return 1;
}

int RegisterNewComponent(char componentName[25],void (*constructorFunc)(void** data),void (*destructorFunc)(void** data),void*(*copyFunc)(void*),cJSON*(*encodeFunc)(void** data, cJSON* currentData),void* (*decodeFunc)(cJSON** data)){
	if(!initializedECS){
		printf("ECS not initialized! Initialize ECS before registering the components and systems\n");
		return -1;
	}

	if(!ECS.Components){
		ECS.Components = malloc(sizeof(Component*));
		ECS.Components[0] = calloc(ECS.maxEntities,sizeof(Component));
	}else{
		ECS.Components = realloc(ECS.Components,(GetLength(ECS.ComponentTypes) + 1) * sizeof(Component*));
		ECS.Components[GetLength(ECS.ComponentTypes)] = calloc(ECS.maxEntities,sizeof(Component));
	}

	ComponentType newType;
	strncpy(newType.name,componentName,25);
	newType.constructor = constructorFunc;
	newType.destructor = destructorFunc;
	newType.copy = copyFunc;
	newType.encode = encodeFunc;
	newType.decode = decodeFunc;

	InsertListEnd(&ECS.ComponentTypes,(void*)&newType);

	return GetLength(ECS.ComponentTypes)-1;
}

int RegisterNewSystem(char systemName[25], int priority, ComponentMask required, ComponentMask excluded, void (*initFunc)(), void (*updateFunc)(), void (*freeFunc)()){
	if(!initializedECS){
		printf("ECS not initialized! Initialize ECS before registering the components and systems\n");
		return -1;
	}
	System newSystem = (System){.priority = priority, .enabled = 1, .required = required, .excluded = excluded, .systemInit = initFunc, .systemUpdate = updateFunc, .systemFree = freeFunc};
	strncpy(newSystem.name,systemName,25);

	ListCellPointer current = GetLastCell(ECS.SystemList);
	unsigned index = GetLength(ECS.SystemList);
	while(current){
		System *curSystem = (System*)GetElement(*current);
		if(curSystem->priority >= priority) break;

		index--;
		current = GetPreviousCell(current);
	}
	
	InsertListIndex(&ECS.SystemList,(void*)&newSystem,index<0? 0: index);
	return GetLength(ECS.SystemList)-1;
}

//-- Component functions --
ComponentID GetComponentID(char componentName[25]){
	int index = 0;
	ListCellPointer current = GetFirstCell(ECS.ComponentTypes);
	while(current){
		ComponentType currType = *((ComponentType*)GetElement(*current));

		if(StringCompareEqual(currType.name, componentName)) return index;

		index++;
		current = GetNextCell(current);
	}
	printf("GetComponentID: Component named %s not found\n",componentName);
	return -1;
}

//Receives components name strings[25] and return a ComponentMask containing these components
ComponentMask CreateComponentMaskByName(int numComp, ...){
	ComponentMask newMask = {0};

	va_list args;
	va_start(args, numComp);
	int i;
	for(i=0;i<numComp;i++){
		int index = GetComponentID(va_arg(args,char *));
		if(index<0 || index>31){
			printf("CreateComponentMaskByName: Component index out of range! (%d)\n",index);
			continue;
		}

		newMask.mask |= 1<<index;
	}
	va_end(args);

	return newMask;
}

//Receives components index ints and return a ComponentMask containing these components
ComponentMask CreateComponentMaskByID(int numComp, ...){
	ComponentMask newMask = {0};

	va_list args;
	va_start(args, numComp);
	int i;
	for(i=0;i<numComp;i++){
		int index = va_arg(args,int);
		if(index<0 || index>31){
			printf("CreateComponentMaskByID: Component index out of range! (%d)\n",index);
			continue;
		}

		newMask.mask |= 1<<index;
	}
	va_end(args);

	return newMask;
}

//-- Entity functions --
EntityID CreateEntity(){
	int *avaliableIndex = (int *)GetFirstElement(ECS.AvaliableEntitiesIndexes);
	if(avaliableIndex){
		int index = *avaliableIndex;
		if(index>ECS.maxUsedIndex) ECS.maxUsedIndex = index;
		//Clear the entity before returning
		ECS.Entities[index].mask = CreateComponentMaskByID(0);
		ECS.Entities[index].isSpawned = 1;
		ECS.Entities[index].childs = InitList(sizeof(EntityID));

		RemoveListStart(&ECS.AvaliableEntitiesIndexes);
		return index;
	}
	printf("CreateEntity: No entity avaliable to spawn! (Max %d)\n",ECS.maxEntities);
	return -1;
}

void DestroyEntity(EntityID entity){
	if(!IsValidEntity(entity)){
		printf("DestroyEntity: Entity is not spawned or out of range!(%d)\n",entity);
		return;
	}

	if(EntityIsParent(entity)){
		ListCellPointer childCell;
		ListForEach(childCell,ECS.Entities[entity].childs){
			DestroyEntity(GetElementAsType(childCell, EntityID));
		}
	}

	int c = 0, mask = ECS.Entities[entity].mask.mask;
	ListCellPointer compCell;
	ListForEach(compCell,ECS.ComponentTypes){
		if(mask & 1){
			((ComponentType*)(GetElement(*compCell)))->destructor(&ECS.Components[c][entity].data);
		}
		mask >>=1;
		c++;
	}

	ECS.Entities[entity].mask.mask = 0;
	ECS.Entities[entity].isSpawned = 0;
	FreeList(&ECS.Entities[entity].childs);
	ECS.Entities[entity].isChild = 0;
	ECS.Entities[entity].isParent = 0;

	InsertListStart(&ECS.AvaliableEntitiesIndexes, (void*)&entity);
}

int IsValidEntity(EntityID entity){
	if(entity<0 || entity>=ECS.maxEntities) return 0;
	return ECS.Entities[entity].isSpawned;
}

int EntityIsPrefab(EntityID entity){
	if(entity<0 || entity>=ECS.maxEntities) return 0;
	return ECS.Entities[entity].isPrefab;
}

char *GetPrefabPath(EntityID entity){
	if(!EntityIsPrefab(entity)){
		printf("GetPrefabPath: Entity is not a prefab! (%d)", entity);
		return NULL;
	}
	return ECS.Entities[entity].prefabPath;
}
char *GetPrefabName(EntityID entity){
	if(!EntityIsPrefab(entity)){
		printf("GetPrefabName: Entity is not a prefab! (%d)", entity);
		return NULL;
	}
	return ECS.Entities[entity].prefabName;
}

void AddComponentToEntity(ComponentID component, EntityID entity){
	if(!IsValidEntity(entity)){
		printf("AddComponentToEntity: Entity is not spawned or out of range!(%d)\n",entity);
		return;
	}
	if(component<0 || component>=GetLength(ECS.ComponentTypes)){
		printf("AddComponentToEntity: Component index out of range!(%d)\n",component);
		return;
	}
	if(EntityContainsMask(entity, CreateComponentMaskByID(1,component))){
		printf("AddComponentToEntity: An entity can only have one of each component type! (E:%d C:%d)\n",entity, component);
		return;
	}
	

	ECS.Entities[entity].mask.mask |= 1 << component;
	
	//Get the component type
	ListCellPointer compType = GetCellAt(ECS.ComponentTypes,component);
	//Call the component constructor
	((ComponentType*)(GetElement(*compType)))->constructor(&ECS.Components[component][entity].data);
}

void RemoveComponentFromEntity(ComponentID component, EntityID entity){
	if(!IsValidEntity(entity)){
		printf("RemoveComponentToEntity: Entity is not spawned or out of range!(%d)\n",entity);
		return;
	}
	if(component<0 || component>=GetLength(ECS.ComponentTypes)){
		printf("RemoveComponentToEntity: Component index out of range!(%d)\n",component);
		return;
	}
	if(!EntityContainsMask(entity, CreateComponentMaskByID(1,component))){
		printf("AddComponentToEntity: This entity doesnt have this component! (E:%d C:%d)\n",entity, component);
		return;
	}
	ECS.Entities[entity].mask.mask &= ~(1 << component);

	//Get the component type
	ListCellPointer compType = GetCellAt(ECS.ComponentTypes,component);

	//Call the component destructor
	((ComponentType*)(GetElement(*compType)))->destructor(&ECS.Components[component][entity].data);
}

EntityID DuplicateEntity(EntityID entity){
	if(!IsValidEntity(entity)){
		printf("DuplicateEntity: Entity is not spawned or out of range!(%d)\n",entity);
		return -1;
	}

	EntityID newEntity = CreateEntity();
	
	int compIndex = 0;
	ListCellPointer compCell;
	ListForEach(compCell,ECS.ComponentTypes){
		if(EntityContainsComponent(entity,compIndex)){
			ECS.Components[compIndex][newEntity].data = GetElementAsType(compCell,ComponentType).copy(ECS.Components[compIndex][entity].data);
		}
		compIndex++;
	}
	ECS.Entities[newEntity].mask.mask = ECS.Entities[entity].mask.mask;

	if(EntityIsParent(entity)){
		ListCellPointer childCell;
		ListForEach(childCell,ECS.Entities[entity].childs){
			EntityID newChild = DuplicateEntity(GetElementAsType(childCell, EntityID));
			SetEntityParent(newChild, newEntity);
		}
	}

	if(EntityIsChild(entity)){
		SetEntityParent(newEntity, GetEntityParent(entity));
	}

	return newEntity;
}

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

//Warning: the encodingToPrefab flag is only valid for the first caller of this function,
//as if you are encoding to prefab, you will only encode the full data of the first, and the childs will still
//encode as normal prefabs (prefab path, name and changes) or normal objects

//Limitations:
//	* When not encoding to prefab (like when saving a scene with a prefab), only the local changes made to the first parent
//    are saved. Ex: Saving a scene with a character prefab positioned and with his child weapon translated, the translation of
//    the weapon will not be saved on the scene. But if you save the character prefab with his child weapon translated before saving
//    the scene, the translation will not be discarted

cJSON *EncodeEntity(EntityID entity,int encodingToPrefab){
	cJSON *entityObj = cJSON_CreateObject();
	cJSON *currentData = NULL;

	//When encoding to a scene or to a child of a prefab json, encode the path to this prefab and the changes made to the prefab
	if(EntityIsPrefab(entity) && !encodingToPrefab){
		cJSON_AddStringToObject(entityObj, "prefabPath", ECS.Entities[entity].prefabPath);
		cJSON_AddStringToObject(entityObj, "prefabName", ECS.Entities[entity].prefabName);

		currentData = OpenJSON(ECS.Entities[entity].prefabPath, ECS.Entities[entity].prefabName);
	}

	ListCellPointer compCell;
	ComponentID compID = 0;
	ListForEach(compCell,ECS.ComponentTypes){
		if(EntityContainsComponent(entity, compID)){
			ComponentType compType = GetElementAsType(compCell,ComponentType);

			//Get current json data if it exists
			cJSON *currentCompJSON = NULL;
			if(currentData)
				currentCompJSON = cJSON_GetObjectItemCaseSensitive(currentData, compType.name);

			cJSON *compEncoded = compType.encode(&ECS.Components[compID][entity].data,currentCompJSON);
			if(compEncoded){
				cJSON_AddItemToObject(entityObj, compType.name, compEncoded);
			}
		}
		compID++;
	}

	if(currentData)
		cJSON_Delete(currentData);

	if(EntityIsParent(entity) && ((EntityIsPrefab(entity) && encodingToPrefab) || !EntityIsPrefab(entity))){

		cJSON *childsArray = cJSON_AddArrayToObject(entityObj, "childs");

		ListCellPointer childCell;
		ListForEach(childCell, *GetChildsList(entity)){
			cJSON_AddItemToArray(childsArray, EncodeEntity(GetElementAsType(childCell,EntityID),0));
		}
	}

	return entityObj;
}

EntityID DecodeEntity(cJSON **entityObj){
	EntityID newEntity;

	cJSON *prefabPath = cJSON_GetObjectItem(*entityObj,"prefabPath");
	//Entity being decoded is a prefab
	if(prefabPath){
		cJSON *prefabName = cJSON_GetObjectItem(*entityObj,"prefabName");
		newEntity = ImportEntityPrefab(prefabPath->valuestring,prefabName->valuestring);
		if(newEntity<0)
			return -1;
	}else{
		//Entity being decoded is not a prefab
		newEntity = CreateEntity();
	}

	ListCellPointer compCell;
	ComponentID compID = 0;
	ListForEach(compCell,ECS.ComponentTypes){
		ComponentType compType = GetElementAsType(compCell,ComponentType);

		cJSON *comp = cJSON_GetObjectItemCaseSensitive(*entityObj, compType.name);
		if(comp){
			void *compData = compType.decode(&comp);

			//In case of a prefab, replace the prefab component with the override component
			if(ECS.Components[compID][newEntity].data){
				compType.destructor(&ECS.Components[compID][newEntity].data);
			}

			ECS.Components[compID][newEntity].data = compData;
			ECS.Entities[newEntity].mask.mask |= 1<<compID;
		}
		compID++;
	}

	cJSON *childsArray = cJSON_GetObjectItem(*entityObj, "childs");
	if(childsArray){
		cJSON *child;
		cJSON_ArrayForEach(child,childsArray){
			EntityID newChild = DecodeEntity(&child);

			if(newChild>=0)
				SetEntityParent(newChild,newEntity);
		}
	}
	return newEntity;
}

int ExportEntityPrefab(EntityID entity, char path[], char name[]){
	cJSON *entityObj = EncodeEntity(entity,1);

	char fullPath[512+256];
    strncpy(fullPath,path,512);
    if(path[strlen(path)-1] != '/'){
        strcat(fullPath,"/");
    }
    strcat(fullPath,name);
	strcat(fullPath,".prefab");
    printf("Saving prefab: (%s)\n",fullPath);
    FILE* file = fopen(fullPath,"w");

	if(file){
		char *jsonString = cJSON_Print(entityObj);
		fprintf(file,"%s",jsonString);
		free(jsonString);
		fclose(file);

		ECS.Entities[entity].isPrefab = 1;
		strncpy(ECS.Entities[entity].prefabPath,path,512);
		strncpy(ECS.Entities[entity].prefabName,name,256);
		strcat(ECS.Entities[entity].prefabName,".prefab");

		return 1;
	}else{
		printf("ExportEntityPrefab: Failed to create/open json file!\n");
		return 0;
	}
	
	cJSON_Delete(entityObj);
}

EntityID ImportEntityPrefab(char path[], char name[]){
	cJSON *entityObj = OpenJSON(path, name);
	if(entityObj){
		EntityID newEntity = DecodeEntity(&entityObj);

		if(newEntity<0)
			return -1;

		ECS.Entities[newEntity].isPrefab = 1;
		strcpy(ECS.Entities[newEntity].prefabPath,path);
		strcpy(ECS.Entities[newEntity].prefabName,name);

		cJSON_Delete(entityObj);
		return newEntity;
	}
	return -1;
}

int ExportScene(char path[], char name[]){

	cJSON *sceneObj = cJSON_CreateObject();
	cJSON *entitiesArray = cJSON_AddArrayToObject(sceneObj, "entities");

	int i;
	for(i=0;i<=ECS.maxUsedIndex;i++){
		if(IsValidEntity(i) && !EntityIsChild(i)){
			cJSON_AddItemToArray(entitiesArray, EncodeEntity(i,0));
		}
	}

	char fullPath[512+256];
    strncpy(fullPath,path,512);
    if(path[strlen(path)-1] != '/'){
        strcat(fullPath,"/");
    }
    strcat(fullPath,name);
	strcat(fullPath,".scene");
    printf("Saving scene: (%s)\n",fullPath);
    FILE* file = fopen(fullPath,"w");

	if(file){
		char *jsonString = cJSON_Print(sceneObj);
		fprintf(file,"%s",jsonString);
		free(jsonString);
		fclose(file);
		return 1;
	}else{
		printf("ExportScene: Failed to create/open json file!\n");
		return 0;
	}
	
	cJSON_Delete(sceneObj);
}

int LoadScene(char path[], char name[]){
	int i;
	for(i=0;i<=ECS.maxUsedIndex;i++){
		if(IsValidEntity(i)){
			DestroyEntity(i);
		}
	}
	return LoadSceneAdditive(path,name);
}

int LoadSceneAdditive(char path[], char name[]){
	cJSON *sceneObj = OpenJSON(path, name);
	if(sceneObj){
		cJSON *entityArray = cJSON_GetObjectItemCaseSensitive(sceneObj, "entities");
		cJSON *entityObj = NULL;
		cJSON_ArrayForEach(entityObj, entityArray){	
			//Entity construction
			DecodeEntity(&entityObj);
		}

		cJSON_Delete(sceneObj);
		return 1;
	}
	return 0;
}

ComponentMask GetEntityComponents(EntityID entity){
	if(!IsValidEntity(entity)){
		printf("GetEntityComponents: Entity is not spawned or out of range!(%d)\n",entity);
		return (ComponentMask){0};
	}
	return ECS.Entities[entity].mask;
}

int IsEmptyComponentMask(ComponentMask mask){
	return mask.mask? 0:1;
}

int EntityContainsMask(EntityID entity, ComponentMask mask){
	if(!mask.mask) return 0;
	return (ECS.Entities[entity].mask.mask & mask.mask) == mask.mask;
}

int EntityContainsComponent(EntityID entity, ComponentID component){
	if(!IsValidEntity(entity)){
		printf("EntityContainsComponent: Entity is not spawned or out of range!(%d)\n",entity);
		return 0;
	}
	if(component<0 || component>GetLength(ECS.ComponentTypes)){
		printf("EntityContainsComponent: Component index out of range!(%d)\n",component);
		return 0;
	}
	unsigned long compMask = 1<<component;
	return (ECS.Entities[entity].mask.mask & compMask) == compMask;
}

int MaskContainsComponent(ComponentMask mask, ComponentID component){
	if(component<0 || component>GetLength(ECS.ComponentTypes)){
		printf("MaskContainsComponent: Component index out of range!(%d)\n",component);
		return 0;
	}

	return mask.mask>>component & 1;
}

ComponentMask IntersectComponentMasks(ComponentMask mask1, ComponentMask mask2){
	return (ComponentMask){mask1.mask & mask2.mask};
}


//-- Parenting functions --
int EntityIsParent(EntityID entity){
    if(!IsValidEntity(entity)){
        printf("EntityIsParent: Entity is not spawned or out of range!(%d)\n",entity);
        return 0;
    }
    return ECS.Entities[entity].isParent;
}

int EntityIsChild(EntityID entity){
    if(!IsValidEntity(entity)){
        printf("EntityIsChild: Entity is not spawned or out of range!(%d)\n",entity);
        return 0;
    }
    return ECS.Entities[entity].isChild;
}

void SetEntityParent(EntityID child, EntityID parent){
    if(child == parent){
        printf("SetEntityParent: Child and parent can't be the same! (%d) (%d)\n",child,parent);
        return;
    }
	if(!IsValidEntity(child)){
        printf("EntityIsParent: Child is not spawned or out of range!(%d)\n",child);
        return;
    }
	if(!IsValidEntity(parent)){
        printf("EntityIsParent: Parent is not spawned or out of range!(%d)\n",parent);
        return;
    }

    //If is already a child of another parent, remove it first
    if(EntityIsChild(child)){
        UnsetParent(child);
    }

    InsertListEnd(&ECS.Entities[parent].childs, &child);
    ECS.Entities[child].parent = parent;

    ECS.Entities[child].isChild = 1;
    ECS.Entities[parent].isParent = 1;
}

EntityID GetEntityParent(EntityID entity){
	if(!IsValidEntity(entity)){
        printf("GetEntityParent: Entity is not spawned or out of range!(%d)\n",entity);
        return 0;
    }
    return ECS.Entities[entity].isChild? ECS.Entities[entity].parent:-1;
}

List* GetChildsList(EntityID parent){
	if(!IsValidEntity(parent)){
        printf("GetChildsList: Parent is not spawned or out of range!(%d)\n",parent);
        return NULL;
    }
    return ECS.Entities[parent].isParent? &ECS.Entities[parent].childs : NULL;
}

int UnsetParent(EntityID child){
	if(!IsValidEntity(child)){
        printf("UnsetParent: Child is not spawned or out of range!(%d)\n",child);
        return 0;
    }
    if(!EntityIsChild(child)){
        printf("UnsetParent: Entity is not anyone's child. (%d)\n",child);
        return 0;
    }

    EntityID parentID = GetEntityParent(child);

    //Find the index of the child in the list of childs
    int index = 0;
    ListCellPointer current = GetFirstCell(ECS.Entities[parentID].childs);
    while(current){
        EntityID cID = *((EntityID*) GetElement(*current));

        //If found
        if(cID == child){
            RemoveListIndex(&ECS.Entities[parentID].childs,index);

            ECS.Entities[child].isChild = 0;

            //If his old parent has an empty list of childs, set isParent to zero
            if(IsListEmpty(ECS.Entities[parentID].childs)){
                ECS.Entities[parentID].isParent = 0;
            }

            return 1;
        }
        current = GetNextCell(current);
        index++;
    }

    //Return zero if can't find the child's index in the list (Indicative of implementation error)
    printf("RemoveChild: Child is not an parent's child (Shouldn't happen). (P:%d  C:%d)\n",parentID,child);
    return 0;
}


//-- System functions --
SystemID GetSystemID(char systemName[25]){
	int index = 0;
	ListCellPointer current;
	ListForEach(current,ECS.SystemList){
		System currSys = GetElementAsType(current,System);
		if(StringCompareEqual(currSys.name, systemName)) return index;	
		index++;
	}
	return -1;
}

void EnableSystem(SystemID system){
	ListCellPointer sys = GetCellAt(ECS.SystemList,system);
	GetElementAsType(sys,System).enabled = 1;
}

void DisableSystem(SystemID system){
	ListCellPointer sys = GetCellAt(ECS.SystemList,system);
	GetElementAsType(sys,System).enabled = 0;
}

int IsSystemEnabled(SystemID system){
	ListCellPointer sys = GetCellAt(ECS.SystemList,system);
	return GetElementAsType(sys,System).enabled;
}

//-------- Engine Functions -------------

//Engine initialization function
//Return 1 if suceeded, 0 if failed
int InitEngine(){
	if(!initializedECS){
		printf("ECS not initialized! Initialize and configure ECS before initializing the engine!\n");
		return 0;
	}

	if(!initializedInput) InitializeInput();
	exitGame = 0;

	printf("Initializing Engine...\n");

    Screen.windowWidth = 1280;
    Screen.windowHeight = 720;
    Screen.gameScale = 2;
    Screen.maxFPS = 60;

	Time.frameTicks = 0;
	Time.msTime = 0;
    Time.deltaTime = 0;

    Time.nowCounter = SDL_GetPerformanceCounter();
	Time.lastCounter = 0;
	
	//Core initializations

	srand( (unsigned)time(NULL) );

	if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG){
		printf("SDL Image could not initialize! \n");
		EndEngine(1);
        return 0;
	}

	if(TTF_Init()==-1) {
    	printf("TTF_Init could not initialize! %s\n", TTF_GetError());
		EndEngine(1);
        return 0;
	}

    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
        printf("SDL could not initialize! SLD_Error: %s\n", SDL_GetError());
		EndEngine(1);
        return 0;
    }
	Core.window = SDL_CreateWindow( "Vopix Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,  Screen.windowWidth,  Screen.windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if(Core.window == NULL){
		printf("Window could not be created! SDL_Error %s\n", SDL_GetError() );
		EndEngine(1);
        return 0;
	}

	//Define internal game resolution
	Screen.gameWidth = Screen.windowWidth/Screen.gameScale;
	Screen.gameHeight = Screen.windowHeight/Screen.gameScale;

	//Initialize renderer
	Core.renderer = SDL_CreateRenderer(Core.window, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderSetLogicalSize(Core.renderer, Screen.gameWidth, Screen.gameHeight);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	//
	//OpenGl initializations
	//

	//Setting OpenGL version
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );

	//Creating OpenGL context
	Core.glContext = SDL_GL_CreateContext(Core.window);
    if (Core.glContext == NULL)
    {
        printf("Cannot create OpenGL context with error: %s\n",SDL_GetError());
		EndEngine(1);
        return 0;
    }

	//Initialize GLEW
	glewExperimental = GL_TRUE; 
	GLenum glewError = glewInit();
	if( glewError != GLEW_OK )
	{
		printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
		EndEngine(1);
        return 0;
	}

	//Unset Vsync
	if( SDL_GL_SetSwapInterval( 0 ) < 0 )
	{
		printf( "Warning: Unable to unset VSync! SDL Error: %s\n", SDL_GetError() );
	}

	//Initialize OpenGL features
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);

	Rendering.cameraPosition = (Vector3){0,0,0};
	Rendering.clearScreenColor = (SDL_Color){0,0,0,0};

    //Framebuffer
    glGenFramebuffers(1, &Rendering.frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, Rendering.frameBuffer);

    //Screen Render Texture
    glGenTextures(1, &Rendering.screenTexture);
    glBindTexture(GL_TEXTURE_2D, Rendering.screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, Screen.gameWidth, Screen.gameHeight, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenRenderbuffers(1, &Rendering.depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, Rendering.depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Screen.gameWidth, Screen.gameHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Rendering.depthRenderBuffer);

    // Set "screenTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, Rendering.screenTexture, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		printf("InitEngine: Error generating framebuffer! (%x)\n",glCheckFramebufferStatus(GL_FRAMEBUFFER));
		EndEngine(1);
        return 0;
	}

    glBindFramebuffer(GL_FRAMEBUFFER,0);

    // VAO Generation and binding
    glGenVertexArrays(1, &Rendering.vao);
    glBindVertexArray(Rendering.vao);

    // VBO generation and binding
	// VBOs for 3D rendering
    glGenBuffers(3, Rendering.vbo3D);

    glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo3D[0]); //Vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo3D[1]); //Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo3D[2]); //Normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//VBOS for 2D rendering
	glGenBuffers(2, Rendering.vbo2D);

    glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo3D[0]); //Vertex
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo3D[1]); //UV
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
    //Compile shaders
    if(!CompileAndLinkShader("Shaders/ScreenVert.vs","Shaders/ScreenFrag.fs",0)) printf(">Failed to compile/link Screen shader! Description above\n\n");
    else printf(">Compiled/linked Screen shader sucessfully!\n");

    if(!CompileAndLinkShader("Shaders/VoxelVert.vs","Shaders/VoxelFrag.fs",1)) printf(">Failed to compile/link VoxelVert shader! Description above\n\n");
    else printf(">Compiled/linked Voxel shader sucessfully!\n");

	if(!CompileAndLinkShader("Shaders/VoxelSmallVert.vs","Shaders/VoxelSmallFrag.fs",2)) printf(">Failed to compile/link VoxelSmall shader! Description above\n\n");
    else printf(">Compiled/linked VoxelSmall shader sucessfully!\n");

	if(!CompileAndLinkShader("Shaders/UIVert.vs","Shaders/UIFrag.fs",3)) printf(">Failed to compile/link UI shader! Description above\n\n");
    else printf(">Compiled/linked UI shader sucessfully!\n");

	//Load voxel palette
	LoadVoxelPalette("Assets/Game/Textures/magicaPalette.png");

	Core.lua = luaL_newstate();
	luaL_openlibs(Core.lua);

	//Call initialization function of all systems
	ListCellPointer current = GetFirstCell(ECS.SystemList);
	while(current){
		System *curSystem = ((System *)GetElement(*current));
		curSystem->systemInit();
		current = GetNextCell(current);
	}

	//Disable text input
	SDL_StopTextInput();

	initializedEngine = 1;
    printf("Engine sucessfully initialized!\n\n");
    return 1;
}

void EngineUpdate(){
	//Start elapsed ms time and delta time calculation
    Time.frameTicks = SDL_GetTicks();
	Time.lastCounter = Time.nowCounter;
	Time.nowCounter = SDL_GetPerformanceCounter();
	Time.deltaTime = (double)((Time.nowCounter - Time.lastCounter)*1000 / SDL_GetPerformanceFrequency() )*0.001;

	ClearRender(Rendering.clearScreenColor);
	InputUpdate();

	//Run systems updates

	//Remove missing child connections from parents
	int e;
	for(e=0;e<=ECS.maxUsedIndex;e++){
		if(IsValidEntity(e) && EntityIsParent(e)){
			int i = 0;
			ListCellPointer child = GetFirstCell(*GetChildsList(e));
			while(child){
				if(!IsValidEntity(GetElementAsType(child, EntityID))){
					child = GetNextCell(child);
					RemoveListIndex(GetChildsList(e),i);
				}else{
					i++;
					child = GetNextCell(child);
				}
				
			}
		}
	}

	//Iterate through the systems list
	ListCellPointer currentSystem = GetFirstCell(ECS.SystemList);
	ListForEach(currentSystem,ECS.SystemList){
		System sys = GetElementAsType(currentSystem,System);
		if(!sys.enabled) continue;
		sys.systemUpdate();
	}

}
void EngineUpdateEnd(){
    SDL_GL_SwapWindow(Core.window);
    Time.msTime = SDL_GetTicks()-Time.frameTicks;
    while( SDL_GetTicks()-Time.frameTicks <  (1000/Screen.maxFPS) ){ }
}

void EndEngine(int errorOcurred){

	//Call deallocation functions of systems
	ListCellPointer current = GetFirstCell(ECS.SystemList);
	while(current){
		System curSystem = *((System *)GetElement(*current));
		curSystem.systemFree();

		current = GetNextCell(current);
	}
	FreeList(&ECS.SystemList);

	//Free all ECS structures and lists
	int i,j;
	for(i=0;i<GetLength(ECS.ComponentTypes);i++){
		for(j=0;j<ECS.maxEntities;j++){
			if(ECS.Components[i][j].data)
			((ComponentType*)(GetElementAt(ECS.ComponentTypes,i)))->destructor(&ECS.Components[i][j].data);
		}
		free(ECS.Components[i]);
	}
	free(ECS.Components);
	FreeList(&ECS.ComponentTypes);

	FreeList(&ECS.AvaliableEntitiesIndexes);
	free(ECS.Entities);

	initializedECS = 0;

	FreeInput();

	//Finish core systems		
	if(Core.renderer)
		SDL_DestroyRenderer(Core.renderer);

    if(Core.window)
		SDL_DestroyWindow(Core.window);

	IMG_Quit();

	if(TTF_WasInit())
		TTF_Quit();

	if(SDL_WasInit(SDL_INIT_EVERYTHING)!=0)
    	SDL_Quit();

	if(Core.lua)
		lua_close(Core.lua);

    if(errorOcurred){
		printf("Engine finished with errors!\n");
		system("pause");
	}else{
		printf("Engine finished sucessfully\n");
	}
}

void ExitGame(){
	exitGame = 1;
}

int GameExited(){
	return exitGame;
}

//-------- Rendering Functions -------------

Vector3 PositionToGameScreenCoords(Vector3 position){
	Vector3 screenPos;
	position = (Vector3){roundf(position.x),roundf(position.y),roundf(position.z)};

	screenPos.x = (int)(((position.x) - (position.y))*2 + roundf(-Rendering.cameraPosition.x)) + 0.375;
    screenPos.y = (int)(((position.x) + (position.y)) + (position.z + Rendering.cameraPosition.z )*2 + roundf(-Rendering.cameraPosition.y)) + 0.375;
	screenPos.z = (position.z)/256.0;

	return screenPos;
}

void ClearRender(SDL_Color col){
    glBindFramebuffer(GL_FRAMEBUFFER, Rendering.frameBuffer);

    glClearColor(col.r/255.0, col.g/255.0, col.b/255.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderToScreen(){

    //Define the projection matrix
	GLfloat ProjectionMatrix[4][4] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}};
    float right = Screen.windowWidth;
    float left = 0;
    float top = Screen.windowHeight;
    float bottom = 0;
    float near = -0.1;
    float far = 0.1;
    
    ProjectionMatrix[0][0] = 2.0f/(right-left);
    ProjectionMatrix[1][1] = 2.0f/(top-bottom);
    ProjectionMatrix[2][2] = -2.0f/(far-near);
    ProjectionMatrix[3][3] = 1;
    ProjectionMatrix[3][0] = -(right + left)/(right - left);
    ProjectionMatrix[3][1] = -(top + bottom)/(top - bottom);
    ProjectionMatrix[3][2] = -(far + near)/(far - near);

    glViewport(0,0,Screen.windowWidth,Screen.windowHeight);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Rendering.screenTexture);

    glUseProgram(Rendering.Shaders[0]);
	glUniform1i(glGetUniformLocation(Rendering.Shaders[0], "fbo_texture"), 0);
	glUniformMatrix4fv(glGetUniformLocation(Rendering.Shaders[0], "projection"), 1, GL_FALSE, (const GLfloat*)&ProjectionMatrix[0]);

    glUniform1f(glGetUniformLocation(Rendering.Shaders[0], "pWidth"), 1.0/(float)Screen.gameWidth);
    glUniform1f(glGetUniformLocation(Rendering.Shaders[0], "pHeight"), 1.0/(float)Screen.gameHeight);

    glUniform1f(glGetUniformLocation(Rendering.Shaders[0], "vignettePower"), 0.25);
    glUniform1f(glGetUniformLocation(Rendering.Shaders[0], "redShiftPower"), 2);    
    glUniform1f(glGetUniformLocation(Rendering.Shaders[0], "redShiftSpread"), 0);
    
	GLfloat quadVertex[8] = {0, Screen.windowHeight, 0, 0, Screen.windowWidth, Screen.windowHeight, Screen.windowWidth, 0};
    GLfloat quadUV[8] = {0,1, 0,0, 1,1, 1,0};

	//Passing rectangle to the vertex VBO
	glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo2D[0]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), quadVertex, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//Passing rectangle uvs the uv VBO
	glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo2D[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), quadUV, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisable( GL_TEXTURE_2D );
    glUseProgram(0);
}


//Debug text renderer, use UIRenderer for the game
void RenderTextDebug(char *text, SDL_Color color, int x, int y, TTF_Font* font) 
{	
	if(!font) return;
	if(!text) return;
	if(text[0] == '\0') return;

    SDL_Surface * originalFont = TTF_RenderText_Solid(font, text, color);
	SDL_Surface * sFont = SDL_ConvertSurfaceFormat(originalFont,SDL_PIXELFORMAT_ARGB8888,0);

	SDL_FreeSurface(originalFont);
    if(!sFont){printf("Failed to render text!\n"); return;}

    //Define the projection matrix
	GLfloat ProjectionMatrix[4][4] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}};
    float right = Screen.windowWidth;
    float left = 0;
    float top = Screen.windowHeight;
    float bottom = 0;
    float near = -0.1;
    float far = 0.1;
    
    ProjectionMatrix[0][0] = 2.0f/(right-left);
    ProjectionMatrix[1][1] = 2.0f/(top-bottom);
    ProjectionMatrix[2][2] = -2.0f/(far-near);
    ProjectionMatrix[3][3] = 1;
    ProjectionMatrix[3][0] = -(right + left)/(right - left);
    ProjectionMatrix[3][1] = -(top + bottom)/(top - bottom);
    ProjectionMatrix[3][2] = -(far + near)/(far - near);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sFont->w, sFont->h, 0, GL_BGRA, 
                    GL_UNSIGNED_BYTE, sFont->pixels);

	GLfloat quadVertex[8] = {x, y + sFont->h + 0.375, x, y, x + sFont->w + 0.375, y + sFont->h + 0.375, x + sFont->w + 0.375, y};
    GLfloat quadUV[8] = {0,0, 0,1, 1,0, 1,1};

	//Passing rectangle to the vertex VBO
	glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo2D[0]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), quadVertex, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//Passing rectangle uvs the uv VBO
	glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo2D[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), quadUV, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glUseProgram(Rendering.Shaders[3]);

	//Passing uniforms to shader
	glUniform1i(glGetUniformLocation(Rendering.Shaders[3], "texture"), 0);
	glUniformMatrix4fv(glGetUniformLocation(Rendering.Shaders[3], "projection"), 1, GL_FALSE, (const GLfloat*)&ProjectionMatrix[0]);
	glUniform3f(glGetUniformLocation(Rendering.Shaders[3], "color"), 1.0f, 1.0f, 1.0f);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glUseProgram(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(sFont);
}

const GLchar *LoadShaderSource(char *filename) {
    if(!filename) return NULL;

    FILE *file = fopen(filename, "r");             // open 
    fseek(file, 0L, SEEK_END);                     // find the end
    size_t size = ftell(file);                     // get the size in bytes
    GLchar *shaderSource = calloc(sizeof(GLchar), size + 1);        // allocate enough bytes
    rewind(file);                                  // go back to file beginning
    fread(shaderSource, size, sizeof(char), file); // read each char into ourblock
    fclose(file);                                  // close the stream

    return shaderSource;
}

int CompileAndLinkShader(char *vertPath, char *fragPath, unsigned shaderIndex){
    //Create an empty vertex shader handle
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vShaderSource = LoadShaderSource(vertPath);

    //Send the vertex shader source code to GL
    glShaderSource(vertexShader, 1, &vShaderSource, 0);

    //Compile the vertex shader
    glCompileShader(vertexShader);

    GLint isCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        GLchar *infoLog = (GLchar *) malloc(maxLength * sizeof(GLchar));
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
        
        glDeleteShader(vertexShader);
        free((void*)vShaderSource);

        printf("Vertex Shader Info Log:\n%s\n",infoLog);
        
        free(infoLog);
        
        return 0;
    }

    //Create an empty fragment shader handle
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fShaderSource = LoadShaderSource(fragPath);

    //Send the fragment shader source code to GL
    glShaderSource(fragmentShader, 1, &fShaderSource, 0);

    //Compile the fragment shader
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        GLchar *infoLog = (GLchar *) malloc(maxLength * sizeof(GLchar));
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
        
        //We don't need the shader anymore.
        glDeleteShader(fragmentShader);
        free((void*)fShaderSource);

        glDeleteShader(vertexShader);
        free((void*)vShaderSource);

        printf("Fragment Shader Info Log:\n%s\n",infoLog);
        
        free(infoLog);

        return 0;
    }

    //Vertex and fragment shaders are successfully compiled.
    //Now time to link them together into a program.
    //Get a program object.
    Rendering.Shaders[shaderIndex] = glCreateProgram();

    //Attach our shaders to our program
    glAttachShader(Rendering.Shaders[shaderIndex], vertexShader);
    glAttachShader(Rendering.Shaders[shaderIndex], fragmentShader);

    //Link our program
    glLinkProgram(Rendering.Shaders[shaderIndex]);

    //Note the different functions here: glGetProgram* instead of glGetShader*.
    GLint isLinked = 0;
    glGetProgramiv(Rendering.Shaders[shaderIndex], GL_LINK_STATUS, (int *)&isLinked);
    if(isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(Rendering.Shaders[shaderIndex], GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        GLchar *infoLog = (GLchar *) malloc(maxLength * sizeof(GLchar));
        glGetProgramInfoLog(Rendering.Shaders[shaderIndex], maxLength, &maxLength, &infoLog[0]);
        
        //We don't need the program anymore.
        glDeleteProgram(Rendering.Shaders[shaderIndex]);
        //Don't leak shaders either.
        glDeleteShader(vertexShader);
        free((void*)vShaderSource);
        glDeleteShader(fragmentShader);
        free((void*)fShaderSource);

        printf("Shader linkage Info Log:\n%s\n",infoLog);
        
        free(infoLog);
        return 0;
    }

    //Always detach shaders after a successful link.
    glDetachShader(Rendering.Shaders[shaderIndex], vertexShader);
    glDetachShader(Rendering.Shaders[shaderIndex], fragmentShader);
    free((void*)vShaderSource);
    free((void*)fShaderSource);

    return 1;
}

void ReloadShaders(){
    int i;
    for(i=0;i<sizeof(Rendering.Shaders) / sizeof(Rendering.Shaders[0]); i++){
        glDeleteProgram(Rendering.Shaders[i]);
    }

    if(!CompileAndLinkShader("Shaders/ScreenVert.vs","Shaders/ScreenFrag.fs",0)) 
        printf(">Failed to compile/link Screen shader! Description above\n\n");
    else 
        printf(">Compiled/linked Screen shader sucessfully!\n\n");

    if(!CompileAndLinkShader("Shaders/VoxelVert.vs","Shaders/VoxelFrag.fs",1)) 
        printf(">Failed to compile/link Voxel shader! Description above\n\n");
    else 
        printf(">Compiled/linked Voxel shader sucessfully!\n\n");

	if(!CompileAndLinkShader("Shaders/VoxelSmallVert.vs","Shaders/VoxelSmallFrag.fs",2)) 
        printf(">Failed to compile/link VoxelSmall shader! Description above\n\n");
    else 
        printf(">Compiled/linked VoxelSmall shader sucessfully!\n\n");

	if(!CompileAndLinkShader("Shaders/UIVert.vs","Shaders/UIFrag.fs",3)) 
		printf(">Failed to compile/link UI shader! Description above\n\n");
    else 
		printf(">Compiled/linked UI shader sucessfully!\n");
}

void LoadVoxelPalette(char path[]){
    SDL_Surface * palSurf = IMG_Load(path);
    if(!palSurf){ printf(">Error loading palette!\n"); return; }

    int i;
    Uint8 r,g,b,a;

    for(i=0;i<256;i++){
        Uint32 *sPix = (Uint32 *)(palSurf->pixels + i* palSurf->format->BytesPerPixel);

        SDL_GetRGBA(*sPix,palSurf->format,&r,&g,&b,&a);
        Pixel color = {clamp(b,0,255),clamp(g,0,255),clamp(r,0,255),a};
        Rendering.voxelColors[i+1] = color;
    }
    SDL_FreeSurface(palSurf);
    printf(">Loaded palette sucessfully!\n");
}

void MoveCamera(float x, float y, float z){
    Rendering.cameraPosition.x +=x*Time.deltaTime;
    Rendering.cameraPosition.y +=y*Time.deltaTime;
    Rendering.cameraPosition.z +=z*Time.deltaTime;
    //printf("CamPos: |%2.1f|%2.1f|%2.1f|\n",cameraPosition.x,cameraPosition.y,cameraPosition.z);
}

// ----------- Input functions ---------------

int InitializeInput(){
	if(initializedInput){
		printf("Input already initialized!\n");
		return 0;
	}
	
	Input.keyboardLast = (Uint8 *)calloc(SDL_NUM_SCANCODES,sizeof(Uint8));
	Input.keyboardCurrent = SDL_GetKeyboardState(NULL);

	Input.mouseX = 0;
	Input.mouseY = 0;
	Input.deltaMouseX = 0;
	Input.deltaMouseY = 0;

	Input.mouseWheelX = 0;
	Input.mouseWheelY = 0;

	Input.textInput = NULL;
	Input.textInputMax = 0;
	Input.textInputLength = 0;
	Input.textInputCursorPos = 0;

	initializedInput = 1;
	return 1;
}

int FreeInput(){
	if(!initializedInput){
		printf("FreeInput: Input not initialized!\n");
		return 0;
	}
	
	free(Input.keyboardLast);
	initializedInput = 0;

	return 1;
}

void InputUpdate(){
    memcpy(Input.keyboardLast,Input.keyboardCurrent,SDL_NUM_SCANCODES*sizeof(Uint8));

	int oldMouseX = Input.mouseX;
	int oldMouseY = Input.mouseY;

	SDL_GetMouseState(&Input.mouseX,&Input.mouseY);

	Input.deltaMouseX = Input.mouseX - oldMouseX;
	Input.deltaMouseY = Input.mouseY - oldMouseY;

	Input.mouseButtonLast[0] = Input.mouseButtonCurrent[0];
	Input.mouseButtonLast[1] = Input.mouseButtonCurrent[1];
	Input.mouseButtonLast[2] = Input.mouseButtonCurrent[2];

	Input.mouseWheelX = 0;
	Input.mouseWheelY = 0;

	int maxl = Input.textInputMax-Input.textInputLength;

    while (SDL_PollEvent(&Input.event)) {

        switch (Input.event.type)
        {
			case SDL_MOUSEWHEEL:			
				Input.mouseWheelX = Input.event.wheel.x;
				Input.mouseWheelY = Input.event.wheel.y;
			break;
            case SDL_QUIT:
                exitGame = 1;
                break;

			case SDL_MOUSEBUTTONDOWN:
				switch (Input.event.button.button)
				{
					case SDL_BUTTON_LEFT:
						Input.mouseButtonCurrent[0] = 1;
						break;
					case SDL_BUTTON_RIGHT:
						Input.mouseButtonCurrent[2] = 1;
						break;
					case SDL_BUTTON_MIDDLE:
						Input.mouseButtonCurrent[1] = 1;
						break;
				}
			break;

			case SDL_MOUSEBUTTONUP:
				switch (Input.event.button.button)
				{
					case SDL_BUTTON_LEFT:
						Input.mouseButtonCurrent[0] = 0;
						break;
					case SDL_BUTTON_RIGHT:
						Input.mouseButtonCurrent[2] = 0;
						break;
					case SDL_BUTTON_MIDDLE:
						Input.mouseButtonCurrent[1] = 0;
						break;
				}
			break;

			case SDL_TEXTINPUT:
				if(maxl>0){
					char buff[100];
					strncpy(buff, Input.textInput+Input.textInputCursorPos,Input.textInputLength-Input.textInputCursorPos);
					strncpy(Input.textInput+Input.textInputCursorPos, Input.event.text.text,1);
					strncpy(Input.textInput+Input.textInputCursorPos+1, buff,Input.textInputLength-Input.textInputCursorPos);
					Input.textInputLength += 1;
					Input.textInputCursorPos +=1;
				}
			break;
        }
    }

	
	if(SDL_IsTextInputActive()){

		//Repeat action if the key is hold
		static double keyboardHold = 0;
		keyboardHold+= Time.deltaTime;

		//Backspace delete
		if(GetKeyDown(SDL_SCANCODE_BACKSPACE) || (GetKey(SDL_SCANCODE_BACKSPACE) && keyboardHold >= 0.05)){
			if(Input.textInputCursorPos>0){
				memmove(Input.textInput + Input.textInputCursorPos-1,Input.textInput + Input.textInputCursorPos,Input.textInputLength - Input.textInputCursorPos);
				memset(Input.textInput + Input.textInputLength-1, '\0',1);
				Input.textInputLength -= 1;
				Input.textInputCursorPos -=1;
			}
			keyboardHold = 0;
		}
		
		//Del delete
		if(GetKeyDown(SDL_SCANCODE_DELETE) || (GetKey(SDL_SCANCODE_DELETE) && keyboardHold >= 0.05)){//string
			if(Input.textInputLength-Input.textInputCursorPos>0){
				//Deletes the character in the cursor position by moving the sucessing characters to the left and setting the last character as a '\0'
				memmove(Input.textInput + Input.textInputCursorPos,Input.textInput + Input.textInputCursorPos+1,Input.textInputLength - (Input.textInputCursorPos + 1));
				memset(Input.textInput + Input.textInputLength-1, '\0',1);
				Input.textInputLength -= 1;
			}
			keyboardHold = 0;
		}
		
		//Cursor movement
		if(GetKeyDown(SDL_SCANCODE_LEFT) || (GetKey(SDL_SCANCODE_LEFT) && keyboardHold >= 0.05)){
			Input.textInputCursorPos = Input.textInputCursorPos<1? 0:Input.textInputCursorPos-1;
			keyboardHold = 0;
		}
		if(GetKeyDown(SDL_SCANCODE_RIGHT) || (GetKey(SDL_SCANCODE_RIGHT) && keyboardHold >= 0.05)){
			Input.textInputCursorPos = Input.textInputCursorPos<Input.textInputLength? Input.textInputCursorPos+1:Input.textInputLength;
			keyboardHold = 0;
		}

		//Home and End shortcuts
		if(GetKeyDown(SDL_SCANCODE_HOME)){
			Input.textInputCursorPos = 0;
		}
		if(GetKeyDown(SDL_SCANCODE_END)){
			Input.textInputCursorPos = Input.textInputLength;
		}

		//Increase delay between the key press and key repetition
		if(GetKeyDown(SDL_SCANCODE_BACKSPACE) || GetKeyDown(SDL_SCANCODE_DELETE) || GetKeyDown(SDL_SCANCODE_LEFT) || GetKeyDown(SDL_SCANCODE_RIGHT))
			keyboardHold = -0.5;
	}
}

int GetKey(SDL_Scancode key){
	if(!initializedInput){
		printf("GetKey: Input not initialized!\n");
		return 0;
	}
	if(key >= SDL_NUM_SCANCODES){
		printf("GetKey: Key out of bounds! (%d)\n",key);
		return 0;
	}
	return Input.keyboardCurrent[key];
}
int GetKeyDown(SDL_Scancode key){
	if(!initializedInput){
		printf("GetKeyDown: Input not initialized!\n");
		return 0;
	}
	if(key >= SDL_NUM_SCANCODES){
		printf("GetKeyDown: Key out of bounds! (%d)\n",key);
		return 0;
	}
	return (Input.keyboardCurrent[key] && !Input.keyboardLast[key]);
} 
int GetKeyUp(SDL_Scancode key){
	if(!initializedInput){
		printf("GetKeyUp: Input not initialized!\n");
		return 0;
	}
	if(key >= SDL_NUM_SCANCODES){
		printf("GetKeyUp: Key out of bounds! (%d)\n",key);
		return 0;
	}
	return (!Input.keyboardCurrent[key] && Input.keyboardLast[key]);
} 


int GetMouseButton(int button){
	if(!initializedInput){
		printf("GetMouseButton: Input not initialized!\n");
		return 0;
	}
	if(button < SDL_BUTTON_LEFT || button > SDL_BUTTON_RIGHT){
		printf("GetMouseButton: Button out of bounds! (%d)\n",button);
		return 0;
	}
	return (Input.mouseButtonCurrent[button-SDL_BUTTON_LEFT]);
}
int GetMouseButtonDown(int button){
	if(!initializedInput){
		printf("GetMouseButtonDown: Input not initialized!\n");
		return 0;
	}
	if(button < SDL_BUTTON_LEFT || button > SDL_BUTTON_RIGHT){
		printf("GetMouseButtonDown: Button out of bounds! (%d)\n",button);
		return 0;
	}
	return (Input.mouseButtonCurrent[button-SDL_BUTTON_LEFT] && !Input.mouseButtonLast[button-SDL_BUTTON_LEFT]);
}
int GetMouseButtonUp(int button){
	if(!initializedInput){
		printf("GetMouseButtonUp: Input not initialized!\n");
		return 0;
	}
	if(button < SDL_BUTTON_LEFT || button > SDL_BUTTON_RIGHT){
		printf("GetMouseButtonUp: Button out of bounds! (%d)\n",button);
		return 0;
	}
	return (!Input.mouseButtonCurrent[button-SDL_BUTTON_LEFT] && Input.mouseButtonLast[button-SDL_BUTTON_LEFT]);
}

void GetTextInput(char* outputTextPointer, int maxLength, int currentLength){
	if(SDL_IsTextInputActive()){
		printf("GetTextInput: Text input active, stop with StopTextInput() before calling again.\n");
		return;
	}

    SDL_StartTextInput();
	Input.textInput = outputTextPointer;
	Input.textInputMax = maxLength;
	Input.textInputLength = currentLength;
	Input.textInputCursorPos = currentLength; 
}

void StopTextInput(){
	if(!SDL_IsTextInputActive()){
		printf("StopTextInput: Text input already disabled.\n");
		return;
	}
    SDL_StopTextInput();
	Input.textInput = NULL;
	Input.textInputMax = 0;
	Input.textInputLength = 0;
	Input.textInputCursorPos = 0;
}
// ----------- Misc. functions ---------------

void SaveTextureToPNG(SDL_Texture *tex, char* out){
    //Get texture dimensions
    int w, h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);

    SDL_Surface *sshot = SDL_CreateRGBSurface(0, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    SDL_Texture *target = SDL_CreateTexture(Core.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);

    //Copy texture to render target
    SDL_SetRenderTarget(Core.renderer, target);
    SDL_Rect rect = {0,0,w,h};
    SDL_RenderCopy(Core.renderer, tex, NULL,&rect);

    //Transfer render target pixels to surface
    SDL_RenderReadPixels(Core.renderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
    //Save surface to PNG
    IMG_SavePNG(sshot, out);

    //Return render target to default
    SDL_SetRenderTarget(Core.renderer, NULL);
    
    //Free allocated surface and texture
    SDL_FreeSurface(sshot);
    SDL_DestroyTexture(target);

}