#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef __unix__
#include "soloud_c.h"
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

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

SDL_Renderer * renderer = NULL;
SDL_Window* window = NULL;	
SDL_GLContext gGlContext;

//Tempo entre frames, calculado ao fim do loop principal
double deltaTime = 0;
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

int main(int argc, char *argv[]){
	int SDLIMGFailed = 0;
	int ErrorOcurred = 0;
	unsigned int frameTicks;
	unsigned int mstime = 0;
	
	//Inicializações gerais

	srand( (unsigned)time(NULL) );
	
	#ifndef __unix__
	Soloud *soloud = NULL;
	soloud = Soloud_create();
	if(Soloud_initEx(soloud,SOLOUD_CLIP_ROUNDOFF | SOLOUD_ENABLE_VISUALIZATION, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO)<0){
		printf("SoLoud could not initialize! \n");
		ErrorOcurred = 1;
		goto EndProgram;
	}
	#endif

	if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG){
		printf("SDL Image could not initialize! \n");
		SDLIMGFailed = 1;
		ErrorOcurred = 1;
		goto EndProgram;
	}

	if(TTF_Init()==-1) {
    	printf("TTF_Init could not initialize! %s\n", TTF_GetError());
		ErrorOcurred = 1;
		goto EndProgram;
	}

    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
        printf("SDL could not initialize! SLD_Error: %s\n", SDL_GetError());
		ErrorOcurred = 1;
		goto EndProgram;
    }
	window = SDL_CreateWindow( "Vopix Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if(window == NULL){
		printf("Window could not be created! SDL_Error %s\n", SDL_GetError() );
		ErrorOcurred = 1;
		goto EndProgram;
	}	

	//
	//OpenGl initializations
	//

	//Setting OpenGL version
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

	//Creating OpenGL context
	gGlContext = SDL_GL_CreateContext(window);
    if (gGlContext == NULL)
    {
        printf("Cannot create OpenGL context with error: %s\n",SDL_GetError());
        ErrorOcurred = 1;
		goto EndProgram;
    }

	 //Initialize GLEW
	glewExperimental = GL_TRUE; 
	GLenum glewError = glewInit();
	if( glewError != GLEW_OK )
	{
		printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
		ErrorOcurred = 1;
		goto EndProgram;
	}

	//Use Vsync
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
	GAME_SCREEN_WIDTH = SCREEN_WIDTH/SCREEN_SCALE;
	GAME_SCREEN_HEIGHT = SCREEN_HEIGHT/SCREEN_SCALE;

	//Inicializa o renderizador e a textura da tela
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderSetLogicalSize(renderer, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);

	//Define e carrega texturas da UI
	//SDL_Rect topUIRect = { 0, 0, 640, 32 };
	//SDL_Rect topUIDest = { 0, 0, 640, 32 };
	//SDL_Surface * topUISurf = IMG_Load("Interface/UIGameTop.png");
	//SDL_Texture * topUITex = SDL_CreateTextureFromSurface(renderer, topUISurf);

	//Inicializa string para o contador de frames
	fpscounter = (char*)calloc(50,sizeof(char));

	//Inicializa contadores de FPS
	InitFPS();
	Uint64 NOW = SDL_GetPerformanceCounter();
	Uint64 LAST = 0;

	//Inicializa fonte
	font = TTF_OpenFont("Interface/Fonts/Visitor.ttf",18);
	if(!font){
		printf("Font: Error loading font!");
	}
	
	//Carrega o mapa
	Rooms = InitList(sizeof(MultiVoxelObject));
	MultiVoxelObject testRoom = LoadMultiVoxelModel("Models/dune.vox");
	InsertListStart(&Rooms,&testRoom);
	//Inicializações do jogo
	InitRenderer();
	InputStart();
	GameStart();

	VoxelObjectList Players =  InitializeObjectList();
	AddObjectInList(&Players,&model);

	//Cria as listas de ponteiros de objetos a renderizar
	SceneShadowCasters = InitializeObjectList();
	EnemiesAndBullets = InitializeObjectList();
	CombineObjectLists(&SceneShadowCasters,3,Players,Pool[0].objs,Pool[1].objs);
	CombineObjectLists(&EnemiesAndBullets,2,Pool[0].objs,Pool[1].objs);

	VoxelObject ob = LoadVoxelModel("Models/Tests/glock.vox");

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	SDL_Texture *test = RenderIcon(&ob);
	SDL_SetTextureBlendMode(test,SDL_BLENDMODE_BLEND);

	SDL_Rect texture_rect;
	texture_rect.x = 270;  //the x coordinate
	texture_rect.y = 0; // the y coordinate
	SDL_QueryTexture(test, NULL, NULL, &texture_rect.w, &texture_rect.h);

	FreeObject(&ob);

	//Loop do jogo
	while (!ExitGame)
	{
		
		//Tick atual do loop (ms)
		frameTicks = SDL_GetTicks();
		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();
		deltaTime = (double)((NOW - LAST)*1000 / SDL_GetPerformanceFrequency() )*0.001;

		SDL_Color bgColor = {0,38,75,0};
		ClearRender(bgColor);





		RenderObjectList(Players, (VoxelObjectList){NULL,0});
		RenderObjectList( ((MultiVoxelObject*) GetFirst(Rooms))->objects , SceneShadowCasters);
		RenderObjectList(EnemiesAndBullets, (VoxelObjectList){NULL,0});

		//Updates
		InputUpdate();
		GameUpdate();
		PoolUpdate();

		RenderToScreen();

		//Renderiza UI
				
		//float barMultiplier = (model.voxelsRemaining-(model.voxelCount/2.0f))/(model.voxelCount/2.0f);
		//SDL_Rect topUIHealthBarRect = { 108, 32, 153*barMultiplier, 32 };
		//SDL_Rect topUIHealthBarDest = { 108, 0, 153*barMultiplier, 32 };

		//SDL_RenderCopy(renderer, topUITex, &topUIRect, &topUIDest);
		//SDL_RenderCopy(renderer, topUITex, &topUIHealthBarRect, &topUIHealthBarDest);
		static char perfInfo[100];
		sprintf(perfInfo,"%4.2f  :FPS\n  %3d : MS\n%5.4lf : DT", GetFPS(), mstime, deltaTime);
		//FC_DrawAlign(font, renderer, GAME_SCREEN_WIDTH,0,FC_ALIGN_RIGHT, "%4.2f :FPS\n%3d : MS\n%5.4lf : DT", GetFPS(), mstime, deltaTime); 
		SDL_Color fontColor = {255,255,255,255};
		RenderText(perfInfo, fontColor, SCREEN_WIDTH-100, SCREEN_HEIGHT-50, font);
		//SDL_RenderCopy(renderer, test, NULL, &texture_rect);

		SDL_GL_SwapWindow(window);

		//Conta MS e FPS gastos e coloca como título da tela
		mstime = SDL_GetTicks()-frameTicks;
		ProcessFPS();

		while( SDL_GetTicks()-frameTicks <  (1000/FRAMES_PER_SECOND) ){ }
	}
	SDL_DestroyTexture(test);
	//Fim do programa, onde ocorre as dealocações
	EndProgram:
	free(fpscounter);

	//FreeMultiObject((MultiVoxelObject*) GetFirst(Rooms));
	FreeList(&Rooms);

	FreeRenderer();
	FreeInput();
	FreePool();

	if(font!=NULL)
		TTF_CloseFont(font);

	FreeObjectList(&SceneShadowCasters);
	FreeObjectList(&EnemiesAndBullets);

	#ifndef __unix__
	if(soloud!=NULL){
		Soloud_deinit(soloud);
		Soloud_destroy(soloud);
	}
	#endif
	
	//if(render!=NULL)
		//SDL_DestroyTexture(render);
			
	if(renderer!=NULL)
		SDL_DestroyRenderer(renderer);

    if(window!=NULL)
		SDL_DestroyWindow( window );

	if(SDLIMGFailed)
		IMG_Quit();

	if(TTF_WasInit())
		TTF_Quit();

	if(SDL_WasInit(SDL_INIT_EVERYTHING)!=0)
    	SDL_Quit();

	if(ErrorOcurred)
		system("pause");
    return 0;
}
