#include "Editor.h"

static System *ThisSystem;

/////////////////////////////////External data//////////////////////////////////

extern engineECS ECS;
extern engineScreen Screen;
extern engineRendering Rendering;
extern engineInput Input;
extern engineTime Time;

//Editor external files data

//FileBrowser.c
extern FileBrowserData fileBrowser;

//EditorUI.c
extern GLuint iconsTex[];
extern int iconsSize[];
extern TTF_Font* gizmosFont;
extern TTF_Font* gizmosFontSmall;

//DialogWindow.c
extern DialogWindowData dialog;

//ComponentsPanel.c
extern int componentWindowLength;

//MenuWindow.c
extern int menuOpened;

/////////////////////////////////////////////////////////////////////////////////

//Color definitions
Vector3 bgPanelColor = {0.02,0.02,0.05};
Vector3 bgLightColor = {0.2,0.2,0.35};
Vector3 bgMediumColor = {0.1,0.1,0.15};

Vector3 fieldColor = {0.2, 0.2, 0.2};
Vector3 fieldEditingColor = {0.3, 0.3, 0.3};
Vector3 buttonOverColor = {0.3,0.3,0.4};

Vector3 scrollbarInactiveColor = {0.3,0.3,0.3};
Vector3 scrollbarOverColor = {0.5,0.5,0.5};

Vector3 menuTabColor = {0.05,0.05,0.10};
Vector3 menuActiveTabColor = {0.15,0.15,0.2};

Vector3 brightWhite = {250.0f/255.0f, 250.0f/255.0f, 250.0f/255.0f};
Vector3 lightWhite = {200.0f/255.0f, 200.0f/255.0f, 200.0f/255.0f};


//Scrollbar configs
const int scrollbarMouseOverDistance = 8;
const int scrollbarMouseWheelSpeed = 25;
const double scrollbarWidth = 12;

char *textFieldString = NULL;

//Current opened scene
char scenePath[_TINYDIR_PATH_MAX] = "";
char sceneName[_TINYDIR_FILENAME_MAX] = "";

//Input data
Vector3 mousePos = {0,0,0};
Vector3 deltaMousePos = {0,0,0};

//List of selected EntityIDs
List SelectedEntities;

//Runs on engine start
void EditorInit(){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("Editor"));

    SelectedEntities = InitList(sizeof(EntityID));

    AllocatePlayModeData();
    DisablePlayModeSystems();

    InitEditorUI();
}

//Runs at engine finish
void EditorFree(){
    FreeList(&SelectedEntities);

    if(fileBrowser.opened) CloseFileBrowser();
    
    FreePlayModeData();
    FreeEditorUI();
}


//Runs each GameLoop iteration
int editingField = -1;
int hideGizmos = 0;
void ExecuteShortcuts();
void EditorUpdate(){
    //Update mouse data
    mousePos = (Vector3){Input.mouseX,Screen.windowHeight - Input.mouseY,0};
    deltaMousePos = (Vector3){Input.deltaMouseX,-Input.deltaMouseY,0};

    //Remove selection of text field
    if(GetMouseButtonDown(SDL_BUTTON_LEFT) || GetKeyDown(SDL_SCANCODE_RETURN) || GetKeyDown(SDL_SCANCODE_KP_ENTER)){
        if(SDL_IsTextInputActive()){
            StopTextInput();
            free(textFieldString);
            textFieldString = NULL;
            editingField = -1;
        }
    }

    glClear(GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, Rendering.frameBuffer);
    glViewport(0,0,Screen.gameWidth,Screen.gameHeight);

    //Draw UI windows and gizmos
    if(!menuOpened){
        if(!fileBrowser.opened && !hideGizmos){
            DrawTransformGizmos();
        }

        DrawPlayModeWidget();

        DrawComponentsPanel();
        DrawEntitiesPanel();

        //Draw camera position text
        int w, h;
        static char camPosX[] = "0000000000:X";
        static char camPosY[] = "0000000000:Y";
        snprintf(camPosX, 13, "%10d:X", (int)Rendering.cameraPosition.x);
        snprintf(camPosY, 13, "%10d:Y", (int)Rendering.cameraPosition.y);
        TTF_SizeText(gizmosFont, camPosX, &w, &h);
        DrawTextColored(camPosX, brightWhite, Screen.windowWidth - (IsListEmpty(SelectedEntities)? 0:componentWindowLength) - w - 8, TTF_FontHeight(gizmosFont)*2, gizmosFont);
        TTF_SizeText(gizmosFont, camPosY, &w, &h);
        DrawTextColored(camPosY, brightWhite, Screen.windowWidth - (IsListEmpty(SelectedEntities)? 0:componentWindowLength) - w - 8, TTF_FontHeight(gizmosFont), gizmosFont);

        ExecuteShortcuts();
        
    }else{
        if(!fileBrowser.opened && !dialog.opened){
            DrawMenuWindow();
        }
    }

    if(fileBrowser.opened && !dialog.opened){
        DrawFileBrowser();
    }

    if(dialog.opened){
        DrawDialogWindow();
    }
}

void ExecuteShortcuts(){
    //Delete shortcut
        if(GetKeyDown(SDL_SCANCODE_DELETE) && editingField<0  && !fileBrowser.opened){
            ListCellPointer sEntity;
            ListForEach(sEntity,SelectedEntities){
                DestroyEntity(GetElementAsType(sEntity,EntityID));
            }
            FreeList(&SelectedEntities);
        }

        //Pan shortcut
        if(GetMouseButton(SDL_BUTTON_RIGHT)){
            MoveCamera(-Input.deltaMouseX/(Time.deltaTime * Screen.gameScale),Input.deltaMouseY/(Time.deltaTime * Screen.gameScale),0);
        }

        //Parent shortcut
        if(GetKeyDown(SDL_SCANCODE_P)){
            ListCellPointer cellp = GetFirstCell(SelectedEntities);
            if(GetLength(SelectedEntities) == 1){
                UnsetParent(GetElementAsType(cellp,EntityID));
            }else{
                while(cellp != GetLastCell(SelectedEntities)){
                    SetEntityParent(GetElementAsType(cellp,EntityID),GetElementAsType(GetLastCell(SelectedEntities),EntityID));
                    cellp = GetNextCell(cellp);
                }
            }
        }

        //Center camera on selected object
        if(GetKey(SDL_SCANCODE_F) && !IsListEmpty(SelectedEntities)){
            EntityID lastSelected = GetElementAsType(GetLastCell(SelectedEntities),EntityID);
            if(EntityContainsComponent(lastSelected, GetComponentID("Transform"))){
                Vector3 screenPos = PositionToCameraCoords(GetPosition(lastSelected));
                TranslateCamera(screenPos.x, screenPos.y, 0);
            }
        }

        //Hide gizmos shortcut
        if(GetKeyDown(SDL_SCANCODE_H)){
            hideGizmos = !hideGizmos;
        }

        //Test shortcuts
        if(GetKeyDown(SDL_SCANCODE_E)){
            if(GetLength(SelectedEntities) == 1){
                ListCellPointer cellp = GetFirstCell(SelectedEntities);
                ExportEntityPrefab(GetElementAsType(cellp,EntityID), "Assets", "newPrefab");
            }
            //ExportScene("Assets", "newScene");
        }

        if(GetKeyDown(SDL_SCANCODE_G)){
            EntityID newEntity = ImportEntityPrefab("Assets", "newPrefab.prefab");
            FreeList(&SelectedEntities);
            InsertListEnd(&SelectedEntities,&newEntity);
            printf("Created entity %d!\n",newEntity);
            //LoadScene("Assets", "newScene.scene");
        }
}