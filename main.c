#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
//#include <SDL2/SDL_opengl.h>

#include "utils.h"
#include "voxelLoader.h"
#include "voxelRenderer.h"
#include "voxelLogic.h"

#define FRAMES_PER_SECOND 60

//Resolução interna, utilizada na renderização
const int GAME_SCREEN_WIDTH = 640;
const int GAME_SCREEN_HEIGHT = 360;

//Resolução da janela do jogo
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

//Variáveis do contador de FPS
//Número de frames armazenados
#define FRAME_VALUES 10
Uint32 frametimes[FRAME_VALUES];
Uint32 frametimelast;
Uint32 framecount;
float framespersecond;
char *fpscounter;
void fpsinit();
void fpsthink();

//Tempo entre frames, calculado ao fim do loop principal
double deltaTime = 0;

//Array com o estado do teclado (atual e do frame anterior)
const Uint8 *keyboard_current = NULL;
Uint8 *keyboard_last;

//Array de ponteiros com os objetos da cena
VoxelObject **scene;
unsigned int sceneObjectCount;

//Pool de objetos
PoolObject Pool[POOLSIZE];

int main(int argc, char *argv[]){
	int quit = 0;
	int SDLIMGFailed = 0;
	int ErrorOcurred = 0;
	unsigned int frameTicks;
	unsigned int mstime;

	SDL_Event event;
	SDL_Renderer * renderer = NULL;
	SDL_Texture * render = NULL;
	SDL_Window* window = NULL;	

	//Inicializa as vars do teclado
	keyboard_last = (Uint8 *)calloc(284,sizeof(Uint8));
	keyboard_current = SDL_GetKeyboardState(NULL);
	
	fpscounter = (char*)calloc(50,sizeof(char));

	//Carrega modelo da nave do player
	VoxelObject model;
	FILE *voxelFile = fopen("Models/Spaceship.vox","rb");
	model = FromMagica(voxelFile);
	model.position = (Vector3){0,30,20};
	fclose(voxelFile);

	//Carrega modelo da bala no pool
	voxelFile = fopen("Models/Bullet.vox","rb");
	Pool[0].baseObj = FromMagica(voxelFile);
	fclose(voxelFile);
	//Define o número de instâncias disponíveis
	Pool[0].numberOfInstances = 60;
	Pool[0].type = BULLET;

	//Carrega modelo da nave inimiga no pool
	voxelFile = fopen("Models/SpaceshipEnemy.vox","rb");
	Pool[1].baseObj = FromMagica(voxelFile);
	fclose(voxelFile);
	//Define o número de instâncias disponíveis
	Pool[1].numberOfInstances = 5;
	Pool[1].type = ENEMY;
	
	//Inicializa o pool
	InitializePool(Pool);
	//Inicializa rand
	srand( (unsigned)time(NULL) );


	if(LoadMap("Maps/Test.txt")<0){
		ErrorOcurred = 1;
	}

	VoxelObject *threadObjs1[1] = {&model};

	//Cria um ponteiro de ponteiros contendo os elementos de outros ponteiros de ponteiros. 
	//VoxelPointerArrayUnion(int [total size of pointer],int [number of pointers to join], VoxelObject **[Pointers],int [pointerSize],...)
	VoxelObject **SceneShadowCasters = VoxelPointerArrayUnion(Pool[0].numberOfInstances+Pool[1].numberOfInstances+1,3, &(*threadObjs1),1, &(*Pool[0].objs),Pool[0].numberOfInstances,&(*Pool[1].objs),Pool[1].numberOfInstances );
	int SceneShadowCastersSize = Pool[0].numberOfInstances+1+Pool[1].numberOfInstances;

	VoxelObject **EnemiesAndBullets = VoxelPointerArrayUnion(Pool[0].numberOfInstances+Pool[1].numberOfInstances,2, &(*Pool[0].objs),Pool[0].numberOfInstances, &(*Pool[1].objs),Pool[1].numberOfInstances );
	int EnemiesAndBulletsSize = Pool[0].numberOfInstances+Pool[1].numberOfInstances;

	if(ErrorOcurred){
		goto EndProgram;
	}
	
	if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG){
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

	window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
	if(window == NULL){
		printf("Window could not be created! SDL_Error %s\n", SDL_GetError() );
		ErrorOcurred = 1;
		goto EndProgram;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	render = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
	SDL_RenderSetLogicalSize(renderer, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);

	SDL_Rect topUIRect = { 0, 0, 640, 32 };
	SDL_Rect topUIDest = { 0, 0, 640, 32 };
	SDL_Surface * topUISurf = IMG_Load("Interface/UIGameTop.png");
	SDL_Texture * topUITex = SDL_CreateTextureFromSurface(renderer, topUISurf);

	Pixel *pix;
	int pitch = GAME_SCREEN_WIDTH * sizeof(unsigned int);
	fpsinit();
	Uint64 NOW = SDL_GetPerformanceCounter();
	Uint64 LAST = 0;

	while (!quit)
	{
		//fps.start();
		frameTicks = SDL_GetTicks();
		
		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();
		deltaTime = (double)((NOW - LAST)*1000 / SDL_GetPerformanceFrequency() )*0.001;

		SDL_LockTexture(render, NULL, (void**)&pix, &pitch);

			ClearScreen(pix);

			pthread_t tID1;
			RendererArguments renderArguments1 = {pix,threadObjs1,1,NULL,0};
			pthread_create(&tID1, NULL, &RenderThread, (void *)&renderArguments1);

			pthread_t tID2;
			RendererArguments renderArguments2 = {pix,scene,sceneObjectCount,SceneShadowCasters,SceneShadowCastersSize};
			pthread_create(&tID2, NULL, &RenderThread, (void *)&renderArguments2);

			pthread_join(tID1, NULL);

			RendererArguments renderArguments3 = {pix,EnemiesAndBullets,EnemiesAndBulletsSize,NULL,0};
			pthread_create(&tID1, NULL, &RenderThread, (void *)&renderArguments3);

			pthread_join(tID2, NULL);

			FillBackground(pix);
			PostProcess(pix);

		SDL_UnlockTexture(render);

		
		if(keyboard_current!=NULL){
			memcpy(keyboard_last,keyboard_current,284*sizeof(Uint8));
		}
		while (SDL_PollEvent(&event)) {
			switch (event.type)
			{
				case SDL_QUIT:
					quit = 1;
					break;
			}
		}

		if (keyboard_current[SDL_SCANCODE_UP])
		{
			MoveObject(&model,0,-100,0,&(*scene),sceneObjectCount,5,2);
		}
		else if (keyboard_current[SDL_SCANCODE_DOWN])
		{
			MoveObject(&model,0,100,0,&(*scene),sceneObjectCount,5,2);
		}

		if (keyboard_current[SDL_SCANCODE_RIGHT])
		{
			MoveObject(&model,100,0,0,&(*scene),sceneObjectCount,5,2);
		}
		else if (keyboard_current[SDL_SCANCODE_LEFT])
		{
			MoveObject(&model,-100,0,0,&(*scene),sceneObjectCount,5,2);
		}

		if (keyboard_current[SDL_SCANCODE_RSHIFT])
		{
			MoveObject(&model,0,0,100,&(*scene),sceneObjectCount,5,2);
		}
		else if (keyboard_current[SDL_SCANCODE_RCTRL])
		{
			MoveObject(&model,0,0,-100,&(*scene),sceneObjectCount,5,2);
		}

		if (keyboard_current[SDL_SCANCODE_KP_0])
		{
			model.rotation.z-=1;
		}
		else if (keyboard_current[SDL_SCANCODE_KP_1])
		{
			model.rotation.z+=1;
		}
		

		if (keyboard_current[SDL_SCANCODE_W])
		{
			MoveCamera(0,-50,0);
		}
		else if (keyboard_current[SDL_SCANCODE_S])
		{
			MoveCamera(0,50,0);
		}

		if (keyboard_current[SDL_SCANCODE_D])
		{
			MoveCamera(50,0,0);
		}
		else if (keyboard_current[SDL_SCANCODE_A])
		{
			MoveCamera(-50,0,0);
		}
		if (keyboard_current[SDL_SCANCODE_E])
		{
			MoveCamera(0,0,50);
		}
		else if (keyboard_current[SDL_SCANCODE_Q])
		{
			MoveCamera(0,0,-50);
		}
		if (keyboard_current[SDL_SCANCODE_ESCAPE])
		{
			quit = 1;
			break;
		}
		if (keyboard_current[SDL_SCANCODE_SPACE] && !keyboard_last[SDL_SCANCODE_SPACE])
		{
			if(model.numberOfPoints !=0){
				for(int i=0;i<model.numberOfPoints;i++){
					if(model.points[i].type == 0){
						Spawn(0,model.points[i].x+model.position.x,model.points[i].y+model.position.y,model.points[i].z+model.position.z);
					}
				}
			}
		}

		PoolUpdate();

		SDL_RenderClear(renderer);
		//Voxel to render
		SDL_RenderCopy(renderer, render, NULL, NULL);
		//UI Rendering
		SDL_RenderCopy(renderer, topUITex, &topUIRect, &topUIDest);
		
		float barMultiplier = (model.voxelsRemaining-(model.voxelCount/2.0f))/(model.voxelCount/2.0f);
		SDL_Rect topUIHealthBarRect = { 108, 32, 153*barMultiplier, 32 };
		SDL_Rect topUIHealthBarDest = { 108, 0, 153*barMultiplier, 32 };
		SDL_RenderCopy(renderer, topUITex, &topUIHealthBarRect, &topUIHealthBarDest);

		SDL_RenderPresent(renderer);

		mstime = SDL_GetTicks()-frameTicks;
		fpsthink();
		sprintf(fpscounter, "FPS: %0.2f |Render MS: %d |DeltaTime: %7.6lf", framespersecond, mstime, deltaTime);
		SDL_SetWindowTitle(window,fpscounter);

		while( SDL_GetTicks()-frameTicks <  (1000/FRAMES_PER_SECOND) ){ }
	}
	
	//Fim do programa, onde ocorre as dealocações
	EndProgram:
	free(fpscounter);
	free(keyboard_last);

	FreeScene();

	FreePool();
	free(SceneShadowCasters);
	
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

void fpsinit() {

        // Set all frame times to 0ms.
        memset(frametimes, 0, sizeof(frametimes));
        framecount = 0;
        framespersecond = 0;
        frametimelast = SDL_GetTicks();

}

void fpsthink() {

        Uint32 frametimesindex;
        Uint32 getticks;
        Uint32 count;
        Uint32 i;

        // frametimesindex is the position in the array. It ranges from 0 to FRAME_VALUES.
        // This value rotates back to 0 after it hits FRAME_VALUES.
        frametimesindex = framecount % FRAME_VALUES;

        // store the current time
        getticks = SDL_GetTicks();

        // save the frame time value
        frametimes[frametimesindex] = getticks - frametimelast;

        // save the last frame time for the next fpsthink
        frametimelast = getticks;

        // increment the frame count
        framecount++;

        // Work out the current framerate

        // The code below could be moved into another function if you don't need the value every frame.

        // I've included a test to see if the whole array has been written to or not. This will stop
        // strange values on the first few (FRAME_VALUES) frames.
        if (framecount < FRAME_VALUES) {

                count = framecount;

        } else {

                count = FRAME_VALUES;

        }

        // add up all the values and divide to get the average frame time.
        framespersecond = 0;
        for (i = 0; i < count; i++) {

                framespersecond += frametimes[i];

        }

        framespersecond /= count;

        // now to make it an actual frames per second value...
        framespersecond = 1000.f / framespersecond;

}
