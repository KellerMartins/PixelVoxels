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

int RegisterNewComponent(char componentName[25],void (*constructorFunc)(void** data),void (*destructorFunc)(void** data),void*(*copyFunc)(void*)){
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
	int i,index = 0;
	ListCellPointer current = GetFirstCell(ECS.ComponentTypes);
	while(current){
		ComponentType currType = *((ComponentType*)GetElement(*current));

		int isEqual = 1;
		//Compare only if equal, breaking if any difference appears
		for(i=0;i<25 && currType.name[i] != '\0';i++){
			if(componentName[i]!=currType.name[i]){isEqual = 0; break;}
		}
		if(isEqual) return index;

		index++;
		current = GetNextCell(current);
	}
	printf("NOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO (%s)",componentName);
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

	int i, mask = ECS.Entities[entity].mask.mask;
	for(i=0;i<GetLength(ECS.ComponentTypes);i++){
		if(mask & 1){
			free(ECS.Components[i][entity].data);
			ECS.Components[i][entity].data = NULL;
		}
		mask >>=1;
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

	return newEntity;
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
	int i,index = 0;
	ListCellPointer current;
	ListForEach(current,ECS.SystemList){
		System currSys = GetElementAsType(current,System);

		int isEqual = 1;
		//Compare only if equal, breaking if any difference appears
		for(i=0;i<25 && currSys.name[i] != '\0';i++){
			if(systemName[i]!=currSys.name[i]){isEqual = 0; break;}
		}
		if(isEqual) return index;
		
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
	
	Core.soloud = Soloud_create();
	if(Soloud_initEx(Core.soloud,SOLOUD_CLIP_ROUNDOFF | SOLOUD_ENABLE_VISUALIZATION, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO)<0){
		printf("SoLoud could not initialize! \n");
		EndEngine(1);
        return 0;
	}

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
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

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
	glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

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

    // Color and vertex VBO generation and binding
    glGenBuffers(2, Rendering.vbo);

    glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
    //Compile shaders
    if(!CompileAndLinkShader("Shaders/ScreenVert.vs","Shaders/ScreenFrag.fs",0)) printf(">Failed to compile/link shader! Description above\n\n");
    else printf(">Compiled/linked shader sucessfully!\n");

     if(!CompileAndLinkShader("Shaders/VoxelVert.vs","Shaders/VoxelFrag.fs",1)) printf(">Failed to compile/link shader! Description above\n\n");
    else printf(">Compiled/linked shader sucessfully!\n");

	//Load voxel palette
	LoadVoxelPalette("Textures/magicaPalette.png");



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
	if(Core.soloud){
		Soloud_deinit(Core.soloud);
		Soloud_destroy(Core.soloud);
	}
			
	if(Core.renderer)
		SDL_DestroyRenderer(Core.renderer);

    if(Core.window)
		SDL_DestroyWindow(Core.window);

	IMG_Quit();

	if(TTF_WasInit())
		TTF_Quit();

	if(SDL_WasInit(SDL_INIT_EVERYTHING)!=0)
    	SDL_Quit();

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

    glEnable(GL_TEXTURE_2D);

    glViewport(0,0,Screen.windowWidth,Screen.windowHeight);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(Rendering.Shaders[0]);
    GLdouble loc = glGetUniformLocation(Rendering.Shaders[0], "pWidth");
    if (loc != -1) glUniform1f(loc, 1.0/(float)Screen.gameWidth);
    loc = glGetUniformLocation(Rendering.Shaders[0], "pHeight");
    if (loc != -1) glUniform1f(loc, 1.0/(float)Screen.gameHeight);

    loc = glGetUniformLocation(Rendering.Shaders[0], "vignettePower");
    if (loc != -1) glUniform1f(loc, 0.25);
    loc = glGetUniformLocation(Rendering.Shaders[0], "redShiftPower");
    if (loc != -1) glUniform1f(loc, 2);    
    loc = glGetUniformLocation(Rendering.Shaders[0], "redShiftSpread");
    if (loc != -1) glUniform1f(loc, 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Rendering.screenTexture);
    
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0,1); glVertex2f(-Screen.windowWidth/2,  Screen.windowHeight/2);
        glTexCoord2f(1,1); glVertex2f( Screen.windowWidth/2,  Screen.windowHeight/2);
        glTexCoord2f(1,0); glVertex2f( Screen.windowWidth/2, -Screen.windowHeight/2);
        glTexCoord2f(0,0); glVertex2f(-Screen.windowWidth/2, -Screen.windowHeight/2);
    }
    glEnd();

    glDisable( GL_TEXTURE_2D );
    glUseProgram(0);
}

void RenderText(char *text, SDL_Color color, int x, int y, TTF_Font* font) 
{	
	if(!text) return;
	if(text[0] == '\0') return;

    SDL_Surface * originalFont = TTF_RenderText_Solid(font, text, color);
	SDL_Surface * sFont = SDL_ConvertSurfaceFormat(originalFont,SDL_PIXELFORMAT_ARGB8888,0);

	SDL_FreeSurface(originalFont);
    if(!sFont){printf("Failed to render text!\n"); return;}

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0, Screen.windowWidth,0,Screen.windowHeight,-1,1); 
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glColor3f(1.0f, 1.0f, 1.0f);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sFont->w, sFont->h, 0, GL_BGRA, 
                    GL_UNSIGNED_BYTE, sFont->pixels);

    
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0,1); glVertex2f(x, y);
        glTexCoord2f(1,1); glVertex2f(x + sFont->w + 0.375, y);
        glTexCoord2f(1,0); glVertex2f(x + sFont->w + 0.375, y + sFont->h + 0.375);
        glTexCoord2f(0,0); glVertex2f(x, y + sFont->h + 0.375);
    }
    glEnd();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(sFont);
}

const GLchar *LoadShaderSource(char *filename) {
    if(!filename) return NULL;

    FILE *file = fopen(filename, "r");             // open 
    fseek(file, 0L, SEEK_END);                     // find the end
    size_t size = ftell(file);                     // get the size in bytes
    GLchar *shaderSource = calloc(1, size);        // allocate enough bytes
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
        printf(">Failed to compile/link shader! Description above\n\n");
    else 
        printf(">Compiled/linked shader sucessfully!\n\n");

    if(!CompileAndLinkShader("Shaders/VoxelVert.vs","Shaders/VoxelFrag.fs",1)) 
        printf(">Failed to compile/link shader! Description above\n\n");
    else 
        printf(">Compiled/linked shader sucessfully!\n\n");
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
		
		//Backspace delete
		if(GetKeyDown(SDL_SCANCODE_BACKSPACE)){
			if(Input.textInputCursorPos>0){
				memmove(Input.textInput + Input.textInputCursorPos-1,Input.textInput + Input.textInputCursorPos,Input.textInputLength - Input.textInputCursorPos);
				memset(Input.textInput + Input.textInputLength-1, '\0',1);
				Input.textInputLength -= 1;
				Input.textInputCursorPos -=1;
			}
		}
		
		//Del delete
		if(GetKeyDown(SDL_SCANCODE_DELETE)){//string
			if(Input.textInputLength-Input.textInputCursorPos>0){
				//Deletes the character in the cursor position by moving the sucessing characters to the left and setting the last character as a '\0'
				memmove(Input.textInput + Input.textInputCursorPos,Input.textInput + Input.textInputCursorPos+1,Input.textInputLength - (Input.textInputCursorPos + 1));
				memset(Input.textInput + Input.textInputLength-1, '\0',1);
				Input.textInputLength -= 1;
			}
		}
		
		//Cursor movement
		if(GetKeyDown(SDL_SCANCODE_LEFT)){
			Input.textInputCursorPos = Input.textInputCursorPos<1? 0:Input.textInputCursorPos-1;
		}
		if(GetKeyDown(SDL_SCANCODE_RIGHT)){
			Input.textInputCursorPos = Input.textInputCursorPos<Input.textInputLength? Input.textInputCursorPos+1:Input.textInputLength;
		}

		//Home and End shortcuts
		if(GetKeyDown(SDL_SCANCODE_HOME)){
			Input.textInputCursorPos = 0;
		}
		if(GetKeyDown(SDL_SCANCODE_END)){
			Input.textInputCursorPos = Input.textInputLength;
		}
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