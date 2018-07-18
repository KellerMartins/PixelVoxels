#include "EngineCore.h"

engineCore Core;
engineTime Time;
engineScreen Screen;

int exitGame = 0;

void ExitGame(){
	exitGame = 1;
}

int GameExited(){
	return exitGame;
}

int InitCore(){
    exitGame = 0;

	srand( (unsigned)time(NULL) );

	if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG){
		PrintLog(Error,"SDL Image could not initialize! \n");
        return 0;
	}

	if(TTF_Init()==-1) {
    	PrintLog(Error,"TTF_Init could not initialize! %s\n", TTF_GetError());
        return 0;
	}

    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
        PrintLog(Error,"SDL could not initialize! SLD_Error: %s\n", SDL_GetError());
        return 0;
    }
	Core.window = SDL_CreateWindow( "Vopix Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,  Screen.windowWidth,  Screen.windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if(Core.window == NULL){
		PrintLog(Error,"Window could not be created! SDL_Error %s\n", SDL_GetError() );
        return 0;
	}

    return 1;
}

void InitTime(){
    Time.frameTicks = 0;
	Time.msTime = 0;
    Time.deltaTime = 0;

    Time.nowCounter = SDL_GetPerformanceCounter();
	Time.lastCounter = 0;
}

void InitScreen(int windowWidth, int windowHeight, int scale, int maxFPS){
    Screen.windowWidth = windowWidth;
    Screen.windowHeight = windowHeight;
    Screen.gameScale = scale;
    Screen.maxFPS = maxFPS;

    //Define internal game resolution
	Screen.gameWidth = Screen.windowWidth/Screen.gameScale;
	Screen.gameHeight = Screen.windowHeight/Screen.gameScale;
}

void UpdateTime(){
	//Start elapsed ms time and delta time calculation
    Time.frameTicks = SDL_GetTicks();
	Time.lastCounter = Time.nowCounter;
	Time.nowCounter = SDL_GetPerformanceCounter();
	Time.deltaTime = (double)((Time.nowCounter - Time.lastCounter)*1000 / SDL_GetPerformanceFrequency() )*0.001;
}

void WaitUntilNextFrame(){
	if(Screen.maxFPS == 0) return;

	Time.msTime = SDL_GetTicks()-Time.frameTicks;
    while( SDL_GetTicks()-Time.frameTicks <  (1000/Screen.maxFPS) ){ }
}

