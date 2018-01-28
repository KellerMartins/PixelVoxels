#include "Engine.h"
#include "utils.h"
#include "Components/VoxelModel.h"
#include "Components/Transform.h"
#include "Components/Rigidbody.h"
#include "Components/ParentChild.h"
#include "Systems/VoxelRenderer.h"
#include "Systems/VoxelModification.h"
#include "Systems/VoxelPhysics.h"

extern engineTime Time;
extern engineCore Core;
extern engineScreen Screen;
extern engineRendering Rendering;
extern engineECS ECS;

TTF_Font* font = NULL;

int main(int argc, char *argv[]){

	InitECS(10);

	ComponentID transformComponent = RegisterNewComponent("Transform", &TransformConstructor, &TransformDestructor);
	ComponentID voxelModelComponent = RegisterNewComponent("VoxelModel", &VoxelModelConstructor, &VoxelModelDestructor);
	ComponentID rigidBodyComponent = RegisterNewComponent("RigidBody", &RigidBodyConstructor, &RigidBodyDestructor);
	ComponentID ParentChildComponent = RegisterNewComponent("ParentChild", &ParentChildConstructor, &ParentChildDestructor);

	if(RegisterNewSystem(1,CreateComponentMask(3,"Transform", "VoxelModel","RigidBody"),(ComponentMask){0},&VoxelPhysicsInit,&VoxelPhysicsUpdate,&VoxelPhysicsFree) < 0) printf("Failed to register VoxelPhysics system!\n");
	if(RegisterNewSystem(0,CreateComponentMask(2,"Transform", "VoxelModel"),(ComponentMask){0},&VoxelRendererInit,&VoxelRendererUpdate,&VoxelRendererFree) < 0) printf("Failed to register VoxelRender system!\n");
	if(RegisterNewSystem(2,CreateComponentMask(1,"VoxelModel"),(ComponentMask){0},&VoxelModificationInit,&VoxelModificationUpdate,&VoxelModificationFree) < 0) printf("Failed to register VoxelModification system!\n");

	if(!InitEngine()) return 1;

	Rendering.clearScreenColor = (SDL_Color){0,38,75,0};

	InitFPS();

	//Initialize font
	font = TTF_OpenFont("Interface/Fonts/Visitor.ttf",18);
	if(!font){
		printf("Font: Error loading font!");
	}

	static EntityID e = -1;
	e = CreateEntity();
	LoadVoxelModel(e,"Models/dune.vox");
	AddComponentToEntity(GetComponentID("RigidBody"),e);
	SetStaticRigidbody(e,1);

	static EntityID e1 = -1;
	e1 = CreateEntity();
	LoadVoxelModel(e1,"Models/bullet.vox");
	AddComponentToEntity(GetComponentID("RigidBody"),e1);
	SetUseGravity(e1,1);
	SetPosition(e1,(Vector3){5,50,80});
	SetVelocity(e1,(Vector3){0.1,0,0});
	SetMass(e1,1);
	SetBounciness(e1, 0.7);

	static EntityID e2 = -1;
	e2 = CreateEntity();
	LoadVoxelModel(e2,"Models/Spaceship.vox");
	SetPosition(e2,(Vector3){15,50,80});
	SetParent(e2,e1);

	printf("GameLoop Initialized\n");
	//Game Loop
	while (!GameExited())
	{

		EngineUpdate();

		if (GetKey(SDL_SCANCODE_UP))
		{
			MoveCamera(0,-150,0);
		}
		else if (GetKey(SDL_SCANCODE_DOWN))
		{
			MoveCamera(0,150,0);
		}
		if (GetKey(SDL_SCANCODE_RIGHT))
		{
			MoveCamera(150,0,0);
		}
		else if (GetKey(SDL_SCANCODE_LEFT))
		{
			MoveCamera(-150,0,0);
		}
		if (GetKey(SDL_SCANCODE_RSHIFT))
		{
			MoveCamera(0,0,50);
		}
		else if (GetKey(SDL_SCANCODE_RCTRL))
		{
			MoveCamera(0,0,-50);
		}
		if (GetKey(SDL_SCANCODE_ESCAPE))
		{
			ExitGame();
		}
		if (GetKey(SDL_SCANCODE_R))
		{
			ReloadShaders();
		}
		if (GetKeyDown(SDL_SCANCODE_Q))
		{
			if(IsChild(e2)){
				UnsetParent(e2);
			}else{
				SetParent(e2,e1);
			}
		}

		RenderToScreen();

		static char perfInfo[100];
		sprintf(perfInfo,"%4.2f  :FPS\n  %3d : MS\n%5.4lf : DT", GetFPS(), Time.msTime, Time.deltaTime);
		SDL_Color fontColor = {255,255,255,255};
		RenderText(perfInfo, fontColor, Screen.windowWidth-100, Screen.windowHeight-50, font);

		EngineUpdateEnd();
		ProcessFPS();
	}

	if(font!=NULL)
	 	TTF_CloseFont(font);

	EndEngine(0);
    return 0;
}