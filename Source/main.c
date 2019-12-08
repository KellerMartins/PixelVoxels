#include "Engine.h"
#include "utils.h"

#include "Components/VoxelModel.h"
#include "Components/Transform.h"
#include "Components/RigidBody.h"
#include "Components/PointLight.h"
#include "Components/LuaScript.h"

#include "Systems/VoxelRenderer.h"
#include "Systems/PointLighting.h"
#include "Systems/Shadows.h"
#include "Systems/VoxelModification.h"
#include "Systems/VoxelPhysics.h"
#include "Systems/Editor.h"
#include "Systems/UIRenderer.h"
#include "Systems/LuaSystem.h"

extern engineTime Time;
extern engineCore Core;
extern engineScreen Screen;
extern engineRendering Rendering;
extern engineECS ECS;
extern engineScene Scene;

TTF_Font* font = NULL;

void* f(void* p){
	return NULL;
}

int main(int argc, char *argv[]){

	OpenLogFile("gameLog.txt");
	InitECS(110);

	//Register ECS Components
	ComponentID transformComponent = RegisterNewComponent("Transform", &TransformConstructor, &TransformDestructor,&TransformCopy, &TransformEncode, &TransformDecode);
	ComponentID voxelModelComponent = RegisterNewComponent("VoxelModel", &VoxelModelConstructor, &VoxelModelDestructor,&VoxelModelCopy, &VoxelModelEncode, &VoxelModelDecode);
	ComponentID rigidBodyComponent = RegisterNewComponent("RigidBody", &RigidBodyConstructor, &RigidBodyDestructor,&RigidBodyCopy, &RigidBodyEncode, &RigidBodyDecode);
	ComponentID pointLightComponent = RegisterNewComponent("PointLight", &PointLightConstructor, &PointLightDestructor,&PointLightCopy, &PointLightEncode, &PointLightDecode);
	ComponentID luaScriptComponent = RegisterNewComponent("LuaScript", &LuaScriptConstructor, &LuaScriptDestructor,&LuaScriptCopy, &LuaScriptEncode, &LuaScriptDecode);

	//Register ECS Systems
	if(RegisterNewSystem("VoxelPhysics",3,CreateComponentMaskByID(3,transformComponent, voxelModelComponent,rigidBodyComponent),(ComponentMask){0},&VoxelPhysicsInit,&VoxelPhysicsUpdate,&VoxelPhysicsFree) < 0) PrintLog(Error,"Main: Failed to register VoxelPhysics system!\n");
	if(RegisterNewSystem("PointLighting",2,CreateComponentMaskByID(2,transformComponent, pointLightComponent),(ComponentMask){0},&PointLightingInit,&PointLightingUpdate,&PointLightingFree) < 0) PrintLog(Error,"Main: Failed to register PointLighting system!\n");
	if(RegisterNewSystem("Shadows",2,CreateComponentMaskByID(2,transformComponent, voxelModelComponent),(ComponentMask){0},&ShadowsInit,&ShadowsUpdate,&ShadowsFree) < 0) PrintLog(Error,"Main: Failed to register Shadows system!\n");
	if(RegisterNewSystem("VoxelRenderer",0,CreateComponentMaskByID(2,transformComponent, voxelModelComponent),(ComponentMask){0},&VoxelRendererInit,&VoxelRendererUpdate,&VoxelRendererFree) < 0) PrintLog(Error,"Main: Failed to register VoxelRender system!\n");
	if(RegisterNewSystem("VoxelModification",4,CreateComponentMaskByID(2,voxelModelComponent, transformComponent),(ComponentMask){0},&VoxelModificationInit,&VoxelModificationUpdate,&VoxelModificationFree) < 0) PrintLog(Error,"Main: Failed to register VoxelModification system!\n");
	if(RegisterNewSystem("Editor",-1,CreateComponentMaskByID(0),(ComponentMask){0},&EditorInit,&EditorUpdate,&EditorFree) < 0) PrintLog(Error,"Main: Failed to register Editor system!\n");
	if(RegisterNewSystem("UIRenderer",-2,CreateComponentMaskByID(0),(ComponentMask){0},&UIRendererInit,&UIRendererUpdate,&UIRendererFree) < 0) PrintLog(Error,"Main: Failed to register UIRenderer system!\n");
	if(RegisterNewSystem("LuaSystem",1,CreateComponentMaskByID(1,luaScriptComponent),(ComponentMask){0},&LuaSystemInit,&LuaSystemUpdate,&LuaSystemFree) < 0) PrintLog(Error,"Main: Failed to register LuaSystem system!\n");
	if(!InitEngine()) return 1;

	InitFPS();
	
	//Initialize font
	font = TTF_OpenFont("Assets/Interface/Fonts/Visitor.ttf",18);
	if(!font){
		PrintLog(Error,"Main: Error loading font!");
	}

	LoadScene("Assets", "default.scene");

	PrintLog(Info,"GameLoop Initialized\n");
	//Game Loop
	while (!GameExited())
	{
		
		EngineUpdate();
		/*
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
		}*/
		if (GetKey(SDL_SCANCODE_ESCAPE))
		{
			ExitGame();
		}
		if (GetKeyDown(SDL_SCANCODE_R))
		{
			if(ReloadAllScripts()){
				PrintLog(Info,"Reloaded Lua scripts without errors!\n");
			}
		}
		if (GetKeyDown(SDL_SCANCODE_T))
		{
			ReloadShaders();
		}
		
		//Debug shadow key
		if(!GetKey(SDL_SCANCODE_V))
			RenderToScreen();

		static char fpsInfo[20];
		static char msInfo[20];
		static char dtInfo[20];
		sprintf(fpsInfo,"FPS: %4.2f", GetFPS());
		sprintf(msInfo,"MS : %3u" ,Time.msTime);
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
	CloseLogFile();
    return 0;
}