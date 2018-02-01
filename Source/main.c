#include "Engine.h"
#include "utils.h"
#include "Components/VoxelModel.h"
#include "Components/Transform.h"
#include "Components/Rigidbody.h"
#include "Components/ParentChild.h"
#include "Systems/VoxelRenderer.h"
#include "Systems/VoxelModification.h"
#include "Systems/VoxelPhysics.h"
#include "Systems/EditorGizmos.h"

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
	ComponentID parentChildComponent = RegisterNewComponent("ParentChild", &ParentChildConstructor, &ParentChildDestructor);

	if(RegisterNewSystem(1,CreateComponentMaskByName(3,"Transform", "VoxelModel","RigidBody"),(ComponentMask){0},&VoxelPhysicsInit,&VoxelPhysicsUpdate,&VoxelPhysicsFree) < 0) printf("Failed to register VoxelPhysics system!\n");
	if(RegisterNewSystem(0,CreateComponentMaskByName(2,"Transform", "VoxelModel"),(ComponentMask){0},&VoxelRendererInit,&VoxelRendererUpdate,&VoxelRendererFree) < 0) printf("Failed to register VoxelRender system!\n");
	if(RegisterNewSystem(2,CreateComponentMaskByName(1,"VoxelModel"),(ComponentMask){0},&VoxelModificationInit,&VoxelModificationUpdate,&VoxelModificationFree) < 0) printf("Failed to register VoxelModification system!\n");
	if(RegisterNewSystem(-1,CreateComponentMaskByName(0),(ComponentMask){0},&EditorGizmosInit,&EditorGizmosUpdate,&EditorGizmosFree) < 0) printf("Failed to register Editor system!\n");

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

		RenderText(fpsInfo, fontColor, 10, TTF_FontHeight(font)*2 + 10, 0, font);
		RenderText(msInfo, fontColor, 10, TTF_FontHeight(font) + 10, 0, font);
		RenderText(dtInfo, fontColor, 10, 10, 0, font);

		EngineUpdateEnd();
		ProcessFPS();
	}

	if(font!=NULL)
	 	TTF_CloseFont(font);

	EndEngine(0);
    return 0;
}