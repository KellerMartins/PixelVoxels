#include "Engine.h"

/////////////////////////////////External data//////////////////////////////////

//From Engine/EngineCore.c
extern engineCore Core;
extern engineTime Time;
extern engineScreen Screen;

//From Engine/EngineECS.c
extern engineECS ECS;
extern unsigned char initializedECS;

//From Engine/EngineRendering.c
extern engineRendering Rendering;

//From Engine/EngineInput.c
extern engineInput Input;

////////////////////////////////////////////////////////////////////////////////

unsigned char initializedEngine = 0;


//-------- Engine Functions called from main -------------

//Engine initialization function
//Return 1 if suceeded, 0 if failed
int InitEngine(){
	if(initializedEngine){
		printf("InitEngine: Engine already initialized\n");
	}
	if(!initializedECS){
		printf("InitEngine: ECS not initialized! Initialize and configure ECS before initializing the engine!\n");
		return 0;
	}

	printf("Initializing Engine...\n");

	InitTime();
	InitScreen(1280,720,2,60);

	if(!InitCore() || !InitInput() || !InitRenderer()){
		EndEngine(1);
		return 0;
	}
	
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
	UpdateTime();

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
	WaitUntilNextFrame();
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
	int c,j;
	for(c=0; c<ECS.numberOfComponents; c++){
		for(j=0;j<ECS.maxEntities;j++){
			if(ECS.Components[c][j].data){
				ECS.ComponentTypes[c].destructor(&ECS.Components[c][j].data);
			}
		}
		free(ECS.Components[c]);
	}
	free(ECS.Components);
	free(ECS.ComponentTypes);

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