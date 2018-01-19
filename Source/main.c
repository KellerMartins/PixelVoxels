#include "Engine.h"
#include "utils.h"
#include "Components/voxelModel.h"
#include "Systems/voxelLoader.h"
#include "Systems/voxelRenderer.h"
#include "Systems/voxelLogic.h"

int ExitGame = 0;

char *fpscounter;
TTF_Font* font = NULL;

//Array de ponteiros com os objetos
VoxelObjectList SceneShadowCasters;
VoxelObjectList EnemiesAndBullets;
List Rooms;

//Pool de objetos
PoolObject Pool[POOLSIZE];

extern VoxelObject model;
extern engineTime Time;
extern engineCore Core;
extern engineScreen Screen;


int main(int argc, char *argv[]){

	InitECS(10);

	if(!InitEngine()) return 1;

	//Inicializações do jogo
	InitRenderer();
	InputStart();
	GameStart();

	//Inicializa string para o contador de frames
	fpscounter = (char*)calloc(50,sizeof(char));

	//Inicializa contadores de FPS
	InitFPS();

	//Inicializa fonte
	font = TTF_OpenFont("Interface/Fonts/Visitor.ttf",18);
	if(!font){
		printf("Font: Error loading font!");
	}
	
	//Carrega o mapa
	Rooms = InitList(sizeof(MultiVoxelObject));
	MultiVoxelObject testRoom = LoadMultiVoxelModel("Models/dune.vox");
	InsertListStart(&Rooms,&testRoom);

	VoxelObjectList Players =  InitializeObjectList();
	AddObjectInList(&Players,&model);

	//Cria as listas de ponteiros de objetos a renderizar
	SceneShadowCasters = InitializeObjectList();
	EnemiesAndBullets = InitializeObjectList();
	CombineObjectLists(&SceneShadowCasters,3,Players,Pool[0].objs,Pool[1].objs);
	CombineObjectLists(&EnemiesAndBullets,2,Pool[0].objs,Pool[1].objs);

	//Loop do jogo
	while (!ExitGame)
	{
		EngineUpdate();

		SDL_Color bgColor = {0,38,75,0};
		ClearRender(bgColor);

		RenderObjectList(Players, (VoxelObjectList){NULL,0});
		RenderObjectList( ((MultiVoxelObject*) GetFirstElement(Rooms))->objects , SceneShadowCasters);
		RenderObjectList(EnemiesAndBullets, (VoxelObjectList){NULL,0});

		//Updates
		InputUpdate();
		GameUpdate();
		PoolUpdate();

		RenderToScreen();

		static char perfInfo[100];
		sprintf(perfInfo,"%4.2f  :FPS\n  %3d : MS\n%5.4lf : DT", GetFPS(), Time.msTime, Time.deltaTime);
		SDL_Color fontColor = {255,255,255,255};
		RenderText(perfInfo, fontColor, Screen.windowWidth-100, Screen.windowHeight-50, font);

		EngineUpdateEnd();
		ProcessFPS();

	}
	free(fpscounter);

	FreeList(&Rooms);

	FreeRenderer();
	FreeInput();
	FreePool();

	if(font!=NULL)
	 	TTF_CloseFont(font);

	FreeObjectList(&SceneShadowCasters);
	FreeObjectList(&EnemiesAndBullets);

	EndEngine(0);

    return 0;
}
