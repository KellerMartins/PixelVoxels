#include "Engine.h"

engineCore Core;
engineScreen Screen;
engineTime Time;
engineECS ECS = {0,NULL};

unsigned char initializedEngine = 0;
unsigned char initializedECS = 0;

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

int RegisterNewComponent(char componentName[25],void (*constructorFunc)(ComponentID component, EntityID entity),void (*destructorFunc)(ComponentID component, EntityID entity)){
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
	newType.nameSize = strlen(componentName);
	newType.constructor = constructorFunc;
	newType.destructor = destructorFunc;

	InsertListEnd(&ECS.ComponentTypes,(void*)&newType);

	return GetLength(ECS.ComponentTypes)-1;
}

int RegisterNewSystem(unsigned priority, ComponentMask required, ComponentMask excluded, void (*initFunc)(), void (*updateFunc)(EntityID entity), void (*freeFunc)()){
	if(!initializedECS){
		printf("ECS not initialized! Initialize ECS before registering the components and systems\n");
		return -1;
	}
	System newSystem = (System){priority, required, excluded, initFunc, updateFunc, freeFunc};

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

ComponentID GetComponentID(char componentName[25]){
	int i,index = 0;
	ListCellPointer current = GetFirstCell(ECS.ComponentTypes);
	while(current){
		ComponentType currType = *((ComponentType*)GetElement(*current));

		//Compare only if equal, breaking if any difference appears
		if(currType.nameSize == strlen(componentName)){
			for(i=0;i<currType.nameSize;i++){
				if(componentName[i]!=currType.name[i]) break;
			}
			//If no difference is found, return the current type index
			if(i==currType.nameSize){
				return index;
			}
		}
		index++;
		current = GetNextCell(current);
	}
	return -1;
}

//Receives components name strings[25] and return a ComponentMask containing these components
ComponentMask CreateComponentMask(int numComp, ...){
	ComponentMask newMask = {0};

	va_list args;
	va_start(args, numComp);
	int i;
	for(i=0;i<numComp;i++){
		int index = GetComponentID(va_arg(args,char *));
		if(index<0 || index>31){
			printf("Component index out of range! (%d)\n",index);
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
			printf("Component index out of range!\n");
			continue;
		}

		newMask.mask |= 1<<index;
	}
	va_end(args);

	return newMask;
}

EntityID CreateEntity(){
	int *avaliableIndex = (int *)GetFirstElement(ECS.AvaliableEntitiesIndexes);
	if(avaliableIndex){
		int index = *avaliableIndex;
		//Clear the entity before returning
		ECS.Entities[index].mask = CreateComponentMaskByID(0);

		RemoveListStart(&ECS.AvaliableEntitiesIndexes);
		return index;
	}
	printf("No entity avaliable to spawn!\n");
	return -1;
}

void DestroyEntity(EntityID entity){
	if(entity<0 || entity>=ECS.maxEntities){
		printf("DestroyEntity: Entity index out of range!(%d)\n",entity);
		return;
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
	InsertListStart(&ECS.AvaliableEntitiesIndexes, (void*)&entity);
}

void AddComponentToEntity(ComponentID component, EntityID entity){
	if(entity<0 || entity>=ECS.maxEntities){
		printf("AddComponentToEntity: Entity index out of range!(%d)\n",entity);
		return;
	}
	if(component<0 || component>=GetLength(ECS.ComponentTypes)){
		printf("AddComponentToEntity: Component index out of range!(%d)\n",component);
		return;
	}

	ECS.Entities[entity].mask.mask |= 1 << component;

	//Get the component type
	int i;
	ListCellPointer current = GetFirstCell(ECS.ComponentTypes);
	for(i=0;i<component;i++){
		current = GetNextCell(current);
	}

	//Call the component constructor
	((ComponentType*)(GetElement(*current)))->constructor(component, entity);
}

void RemoveComponentFromEntity(ComponentID component, EntityID entity){
	if(entity<0 || entity>=ECS.maxEntities){
		printf("RemoveComponentToEntity: Entity index out of range!(%d)\n",entity);
		return;
	}
	if(component<0 || component>=GetLength(ECS.ComponentTypes)){
		printf("RemoveComponentToEntity: Component index out of range!(%d)\n",component);
		return;
	}

	ECS.Entities[entity].mask.mask &= ~(1 << component);

	//Get the component type
	int i;
	ListCellPointer current = GetFirstCell(ECS.ComponentTypes);
	for(i=0;i<component;i++){
		current = GetNextCell(current);
	}

	//Call the component destructor
	((ComponentType*)(GetElement(*current)))->destructor(component, entity);
}

ComponentMask GetEntityComponents(EntityID entity){
	if(entity<0 || entity>=ECS.maxEntities){
		printf("GetEntityComponents: Entity index out of range!(%d)\n",entity);
		return (ComponentMask){0};
	}
	return ECS.Entities[entity].mask;
}

int EntityContainsMask(Entity entity, ComponentMask mask){
	return (entity.mask.mask & mask.mask) == mask.mask;
}

//Engine initialization function
//Return 1 if suceeded, 0 if failed
int InitEngine(){
	if(!initializedECS){
		printf("ECS not initialized! Initialize and configure ECS before initializing the engine!\n");
		return 0;
	}

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
	
	//Inicializações gerais

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

	//Define resolução interna do jogo
	Screen.gameWidth = Screen.windowWidth/Screen.gameScale;
	Screen.gameHeight = Screen.windowHeight/Screen.gameScale;

	//Inicializa o renderizador e a textura da tela
	Core.renderer = SDL_CreateRenderer(Core.window, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderSetLogicalSize(Core.renderer, Screen.gameWidth, Screen.gameHeight);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	//Call initialization function of all systems

	ListCellPointer current = GetFirstCell(ECS.SystemList);
	while(current){
		System curSystem = *((System *)GetElement(*current));
		curSystem.systemInit();

		current = GetNextCell(current);
	}

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

	//Run systems updates
	int i;
	for(i=0;i<ECS.maxEntities;i++){

		ListCellPointer currentSystem = GetFirstCell(ECS.SystemList);
		while(currentSystem){
			//Entity contains needed components
			if(EntityContainsMask(ECS.Entities[i],((System*)GetElement(*currentSystem))->required) ){
				//Entity doesn't contains the excluded components
				if(!EntityContainsMask(ECS.Entities[i],((System*)GetElement(*currentSystem))->excluded) ){
					//Execute system update in the entity
					((System*)GetElement(*currentSystem)) -> systemUpdate(i);
				}
			}

			currentSystem = GetNextCell(currentSystem);
		}
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
			free(ECS.Components[i][j].data);
		}
		free(ECS.Components[i]);
	}
	free(ECS.Components);
	FreeList(&ECS.ComponentTypes);

	FreeList(&ECS.AvaliableEntitiesIndexes);
	free(ECS.Entities);

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

    if(errorOcurred) system("pause");
}