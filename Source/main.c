#include "Engine.h"
#include "utils.h"

#include "Components/VoxelModel.h"
#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/PointLight.h"

#include "Systems/VoxelRenderer.h"
#include "Systems/PointLighting.h"
#include "Systems/VoxelModification.h"
#include "Systems/VoxelPhysics.h"
#include "Systems/Editor.h"
#include "Systems/UIRenderer.h"

extern engineTime Time;
extern engineCore Core;
extern engineScreen Screen;
extern engineRendering Rendering;
extern engineECS ECS;

TTF_Font* font = NULL;

void* f(void* p){
	return NULL;
}

int main(int argc, char *argv[]){

	InitECS(110);

	ComponentID transformComponent = RegisterNewComponent("Transform", &TransformConstructor, &TransformDestructor,&TransformCopy, &TransformEncode, &TransformDecode);
	ComponentID voxelModelComponent = RegisterNewComponent("VoxelModel", &VoxelModelConstructor, &VoxelModelDestructor,&VoxelModelCopy, &VoxelModelEncode, &VoxelModelDecode);
	ComponentID rigidBodyComponent = RegisterNewComponent("RigidBody", &RigidBodyConstructor, &RigidBodyDestructor,&RigidBodyCopy, &RigidBodyEncode, &RigidBodyDecode);
	ComponentID pointLightComponent = RegisterNewComponent("PointLight", &PointLightConstructor, &PointLightDestructor,&PointLightCopy, &PointLightEncode, &PointLightDecode);

	if(RegisterNewSystem("VoxelPhysics",2,CreateComponentMaskByID(3,transformComponent, voxelModelComponent,rigidBodyComponent),(ComponentMask){0},&VoxelPhysicsInit,&VoxelPhysicsUpdate,&VoxelPhysicsFree) < 0) printf("Failed to register VoxelPhysics system!\n");
	if(RegisterNewSystem("PointLighting",1,CreateComponentMaskByID(2,transformComponent, pointLightComponent),(ComponentMask){0},&PointLightingInit,&PointLightingUpdate,&PointLightingFree) < 0) printf("Failed to register PointLighting system!\n");
	if(RegisterNewSystem("VoxelRenderer",0,CreateComponentMaskByID(2,transformComponent, voxelModelComponent),(ComponentMask){0},&VoxelRendererInit,&VoxelRendererUpdate,&VoxelRendererFree) < 0) printf("Failed to register VoxelRender system!\n");
	if(RegisterNewSystem("VoxelModification",3,CreateComponentMaskByID(2,voxelModelComponent, transformComponent),(ComponentMask){0},&VoxelModificationInit,&VoxelModificationUpdate,&VoxelModificationFree) < 0) printf("Failed to register VoxelModification system!\n");
	if(RegisterNewSystem("Editor",-1,CreateComponentMaskByID(0),(ComponentMask){0},&EditorInit,&EditorUpdate,&EditorFree) < 0) printf("Failed to register Editor system!\n");
	if(RegisterNewSystem("UIRenderer",-2,CreateComponentMaskByID(0),(ComponentMask){0},&UIRendererInit,&UIRendererUpdate,&UIRendererFree) < 0) printf("Failed to register UIRenderer system!\n");
	if(!InitEngine()) return 1;

	Rendering.clearScreenColor = (SDL_Color){0,38,75,0};

	InitFPS();
	
	//Initialize font
	font = TTF_OpenFont("Interface/Fonts/Visitor.ttf",18);
	if(!font){
		printf("Font: Error loading font!");
	}

	printf("GameLoop Initialized\n");
	//Game Loop
	while (!GameExited())
	{

		EngineUpdate();
		
		// if (GetKey(SDL_SCANCODE_UP))
		// {
		// 	MoveCamera(0,-150,0);
		// }
		// else if (GetKey(SDL_SCANCODE_DOWN))
		// {
		// 	MoveCamera(0,150,0);
		// }
		// if (GetKey(SDL_SCANCODE_RIGHT))
		// {
		// 	MoveCamera(150,0,0);
		// }
		// else if (GetKey(SDL_SCANCODE_LEFT))
		// {
		// 	MoveCamera(-150,0,0);
		// }
		// if (GetKey(SDL_SCANCODE_RSHIFT))
		// {
		// 	MoveCamera(0,0,50);
		// }
		// else if (GetKey(SDL_SCANCODE_RCTRL))
		// {
		// 	MoveCamera(0,0,-50);
		// }
		if (GetKey(SDL_SCANCODE_ESCAPE))
		{
			ExitGame();
		}
		if (GetKeyDown(SDL_SCANCODE_R))
		{
			ReloadShaders();
		}

		RenderToScreen();

		static char fpsInfo[20];
		static char msInfo[20];
		static char dtInfo[20];
		sprintf(fpsInfo,"FPS: %4.2f", GetFPS());
		sprintf(msInfo,"MS : %3d" ,Time.msTime);
		sprintf(dtInfo,"DT : %5.4lf", Time.deltaTime);
		SDL_Color fontColor = {255,255,255,255};

		RenderTextDebug(fpsInfo, fontColor, 110, TTF_FontHeight(font)*2 + 10, font);
		RenderTextDebug(msInfo, fontColor, 110, TTF_FontHeight(font) + 10, font);
		RenderTextDebug(dtInfo, fontColor, 110, 10, font);
		
		EngineUpdateEnd();
		ProcessFPS();
	}

	if(font!=NULL)
	 	TTF_CloseFont(font);

	EndEngine(0);
    return 0;
}