#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#include "soloud_c.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "SDL_FontCache.h"

#include "utils.h"
#include "voxelLoader.h"
#include "voxelRenderer.h"
#include "voxelLogic.h"

#define FRAMES_PER_SECOND 60

//Resolução interna, utilizada na renderização
int GAME_SCREEN_WIDTH = 640;
int GAME_SCREEN_HEIGHT = 360;

//Resolução da janela do jogo
int SCREEN_WIDTH = 1280;
int SCREEN_HEIGHT = 720;

int SCREEN_SCALE = 2;

char *fpscounter;

FC_Font* font = NULL;

//Tempo entre frames, calculado ao fim do loop principal
double deltaTime = 0;
int ExitGame = 0;

//Array de ponteiros com os objetos
VoxelObject **SceneShadowCasters;
int SceneShadowCastersSize;
VoxelObject **EnemiesAndBullets;
int EnemiesAndBulletsSize;
VoxelObject **scene;
int sceneObjectCount;

//Pool de objetos
PoolObject Pool[POOLSIZE];

extern VoxelObject model;

int main(int argc, char *argv[]){
	int SDLIMGFailed = 0;
	int ErrorOcurred = 0;
	unsigned int frameTicks;
	unsigned int mstime = 0;

	SDL_Renderer * renderer = NULL;
	SDL_Texture * render = NULL;
	SDL_Window* window = NULL;	
	
	//Inicializações gerais

	srand( (unsigned)time(NULL) );

	Soloud *soloud = NULL;
	soloud = Soloud_create();
	if(Soloud_initEx(soloud,SOLOUD_CLIP_ROUNDOFF | SOLOUD_ENABLE_VISUALIZATION, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO)<0){
		printf("SoLoud could not initialize! \n");
		ErrorOcurred = 1;
		goto EndProgram;
	}

	if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG){
		printf("SDL Image could not initialize! \n");
		SDLIMGFailed = 1;
		ErrorOcurred = 1;
		goto EndProgram;
	}
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
        printf("SDL could not initialize! SLD_Error: %s\n", SDL_GetError());
		ErrorOcurred = 1;
		goto EndProgram;
    }
	window = SDL_CreateWindow( "Vopix Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if(window == NULL){
		printf("Window could not be created! SDL_Error %s\n", SDL_GetError() );
		ErrorOcurred = 1;
		goto EndProgram;
	}	

	//Define resolução interna do jogo
	GAME_SCREEN_WIDTH = SCREEN_WIDTH/SCREEN_SCALE;
	GAME_SCREEN_HEIGHT = SCREEN_HEIGHT/SCREEN_SCALE;

	//Inicializa o renderizador e a textura da tela
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	render = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
	SDL_RenderSetLogicalSize(renderer, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
	
	//Define o vetor que irá receber os pixels da tela e o que recebe os valores de profundidade
	Pixel *pix;
	int pitch = GAME_SCREEN_WIDTH * sizeof(unsigned int);
	Uint16 *depth = (Uint16*) calloc(GAME_SCREEN_HEIGHT*GAME_SCREEN_WIDTH,sizeof(Uint16));
	
	//Define e carrega texturas da UI
	SDL_Rect topUIRect = { 0, 0, 640, 32 };
	SDL_Rect topUIDest = { 0, 0, 640, 32 };
	SDL_Surface * topUISurf = IMG_Load("Interface/UIGameTop.png");
	SDL_Texture * topUITex = SDL_CreateTextureFromSurface(renderer, topUISurf);

	//Inicializa string para o contador de frames
	fpscounter = (char*)calloc(50,sizeof(char));

	//Inicializa contadores de FPS
	InitFPS();
	Uint64 NOW = SDL_GetPerformanceCounter();
	Uint64 LAST = 0;

	//Inicializa fonte
	font = FC_CreateFont();  
	if(!FC_LoadFont(font, renderer, "Interface/Fonts/Visitor.ttf",18, FC_MakeColor(255,255,255,255), TTF_STYLE_NORMAL)){
		printf("Font: Error loading font!");
	}
	
	//Carrega o mapa
	if(LoadMap("Maps/Test.txt")<0){
		ErrorOcurred = 1;
		ExitGame = 1;
	}

	//Inicializações do jogo
	InitRenderer(depth);
	InputStart();
	GameStart();

	VoxelObject *threadObjs1[1] = {&model};

	//Cria as listas de ponteiros de objetos a renderizar
	SceneShadowCasters = VoxelPointerArrayUnion(Pool[0].numberOfInstances+Pool[1].numberOfInstances+1,3, &(*threadObjs1),1, &(*Pool[0].objs),Pool[0].numberOfInstances,&(*Pool[1].objs),Pool[1].numberOfInstances );
	SceneShadowCastersSize = Pool[0].numberOfInstances+1+Pool[1].numberOfInstances;

	EnemiesAndBullets = VoxelPointerArrayUnion(Pool[0].numberOfInstances+Pool[1].numberOfInstances,2, &(*Pool[0].objs),Pool[0].numberOfInstances, &(*Pool[1].objs),Pool[1].numberOfInstances );
	EnemiesAndBulletsSize = Pool[0].numberOfInstances+Pool[1].numberOfInstances;

	//Loop do jogo
	while (!ExitGame)
	{
		//Tick atual do loop (ms)
		frameTicks = SDL_GetTicks();
		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();
		deltaTime = (double)((NOW - LAST)*1000 / SDL_GetPerformanceFrequency() )*0.001;

		//Updates
		InputUpdate();
		GameUpdate();
		PoolUpdate();

		//Trava a textura da tela na memória e inicia renderização
		SDL_LockTexture(render, NULL, (void**)&pix, &pitch);
			UpdateScreenPointer(pix);
			ClearScreen();

			pthread_t tID1;
			RendererArguments renderArguments1 = {threadObjs1,1,NULL,0};
			pthread_create(&tID1, NULL, &RenderThread, (void *)&renderArguments1);

			pthread_t tID2;
			RendererArguments renderArguments2 = {scene,sceneObjectCount,SceneShadowCasters,SceneShadowCastersSize};
			pthread_create(&tID2, NULL, &RenderThread, (void *)&renderArguments2);

			pthread_join(tID1, NULL);

			RendererArguments renderArguments3 = {EnemiesAndBullets,EnemiesAndBulletsSize,NULL,0};
			pthread_create(&tID1, NULL, &RenderThread, (void *)&renderArguments3);

			pthread_join(tID2, NULL);

			PostProcess();

		SDL_UnlockTexture(render);

		//Limpa a tela e "Blita" a tela renderizada
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, render, NULL, NULL);

		//Renderiza UI
				
		float barMultiplier = (model.voxelsRemaining-(model.voxelCount/2.0f))/(model.voxelCount/2.0f);
		SDL_Rect topUIHealthBarRect = { 108, 32, 153*barMultiplier, 32 };
		SDL_Rect topUIHealthBarDest = { 108, 0, 153*barMultiplier, 32 };

		SDL_RenderCopy(renderer, topUITex, &topUIRect, &topUIDest);
		SDL_RenderCopy(renderer, topUITex, &topUIHealthBarRect, &topUIHealthBarDest);
		
		FC_DrawAlign(font, renderer, GAME_SCREEN_WIDTH,0,FC_ALIGN_RIGHT, "%4.2f :FPS\n%3d : MS\n%5.4lf : DT", GetFPS(), mstime, deltaTime); 

		SDL_RenderPresent(renderer);

		//Conta MS e FPS gastos e coloca como título da tela
		mstime = SDL_GetTicks()-frameTicks;
		ProcessFPS();
		
		while( SDL_GetTicks()-frameTicks <  (1000/FRAMES_PER_SECOND) ){ }
	}
	
	//Fim do programa, onde ocorre as dealocações
	EndProgram:
	free(fpscounter);

	FreeRenderer();
	FreeInput();
	FreeScene();
	FreePool();

	if(font!=NULL)
		FC_FreeFont(font);

	free(SceneShadowCasters);
	free(EnemiesAndBullets);

	if(soloud!=NULL){
		Soloud_deinit(soloud);
		Soloud_destroy(soloud);
	}
	
	if(render!=NULL)
		SDL_DestroyTexture(render);
			
	if(renderer!=NULL)
		SDL_DestroyRenderer(renderer);

    if(window!=NULL)
		SDL_DestroyWindow( window );

	if(SDLIMGFailed)
		IMG_Quit();

	if(SDL_WasInit(SDL_INIT_EVERYTHING)!=0)
    	SDL_Quit();

	if(ErrorOcurred)
		system("pause");
    return 0;
}