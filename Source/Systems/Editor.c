#include "Editor.h"

static System *ThisSystem;

static SystemID VoxRenderSystem ;
static SystemID EditorSystem;

extern engineECS ECS;
extern engineScreen Screen;
extern engineRendering Rendering;
extern engineInput Input;
extern engineTime Time;

static float positionGizmosLength = 20;
static const int selectMouseOverDistance = 10;
static const int axisMouseOverDistance = 20;
static const int scrollbarMouseOverDistance = 8;
static const float lineGizmosMouseMovement = 4.50;

static int componentWindowLength = 200;
static int componentWindowWidthSpacing = 14;
static int componentWindowBottomSpacing = 50;
static int componentNameLeftSpacing = 5;
static int componentBetweenSpacing = 3;

static int entityWindowLength = 100;
static int entityWindowWidthSpacing = 14;
static int entityWindowTopHeightSpacing = 60;
static int entityNameLeftSpacing = 18;
static int entityBetweenSpacing = 2;

static int scrollbarMouseWheelSpeed = 25;

Vector3 bgPanelColor = {0.02,0.02,0.05};

SDL_Color brightWhite = {250,250,250};
SDL_Color lightWhite = {200,200,200};
TTF_Font* gizmosFont;
TTF_Font* gizmosFontSmall;

GLuint iconsTex[19];
int iconsSize = 9;

char *textFieldString = NULL;

//File browser data;
//Lists of tinydir_file
List Folders;
List Files;
//List of char*
List Paths;
int indexPath;
char filePath[_TINYDIR_PATH_MAX];
char fileName[_TINYDIR_FILENAME_MAX];
char fileExtension[_TINYDIR_FILENAME_MAX] = "";
void (*onOpenFunction)();
//File browser open status (0 = not opened, 1 = opened valid folder, -1 failed to open)
int fileBrowserOpened = 0;
//Line of items scrolled
int fbItemsScroll = 0;

//Input data
static Vector3 mousePos = {0,0,0};
static Vector3 deltaMousePos = {0,0,0};

//List of selected EntityIDs
List SelectedEntities;
//Array of existing entities
int *existingEntities = NULL;
//Copy of the components and entities data to be reseted when exiting play mode
Component** componentsPlaymodeCopy;
Entity* entitiesPlaymodeCopy;

//Internal functions
int MouseOverLineGizmos(Vector3 mousePos, Vector3 originPos, Vector3 handlePos,int mouseOverDistance);
int MouseOverPointGizmos(Vector3 mousePos, Vector3 originPos,int mouseOverDistance);
int MouseOverBox(Vector3 mousePos, Vector3 min, Vector3 max,int mouseOverDistance);
Vector3 WorldVectorToScreenVector(Vector3 v);
int IsSelected(EntityID entity);
void RemoveFromSelected(EntityID entity);
int DrawComponentHeader(ComponentID component, int* curHeight);
void DrawRectangle(Vector3 min, Vector3 max, float r, float g, float b);
void DrawPointIcon(Vector3 pos,int iconID, int scale, Vector3 color);
void Vector3Field(char *title, Vector3 *data,int ommitX,int ommitY,int ommitZ,int x, int w, int fieldsSpacing, int* curField, int* curHeight);
void FloatField(char *title, float *data,int ommit,int x, int w, int* curField, int* curHeight);
void IntField(char *title, int *data,int ommit,int x, int w, int* curField, int* curHeight);
void IntListField(char *title, List *list,int x, int w, int* curField, int* curHeight);
int PointButton(Vector3 pos,int iconID, int scale, Vector3 defaultColor, Vector3 mouseOverColor, Vector3 pressedColor);
int PointToggle(int *data,Vector3 pos,int onIconID, int offIconID, int undefinedIconID, int scale, Vector3 onColor, Vector3 offColor, Vector3 undefinedColor, Vector3 mouseOverColor);
void LoadUITexture(char *path,int index);
void OpenFileBrowser(char *initialPath,void(*onOpen)());
void FileBrowserExtension(char *ext);
void CloseFileBrowser();

void LoadModel();

void EnterPlayMode();
void ExitPlayMode();

//Runs on engine start
void EditorInit(System *systemObject){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("Editor"));

    existingEntities = calloc(ECS.maxEntities,sizeof(int));
    SelectedEntities = InitList(sizeof(EntityID));
    int i;

    //Allocate the playmode components and entities copy
    componentsPlaymodeCopy = malloc(GetLength(ECS.ComponentTypes) * sizeof(Component*));
    for(i=0;i<GetLength(ECS.ComponentTypes);i++){
        componentsPlaymodeCopy[i] = calloc(ECS.maxEntities,sizeof(Component));
    }

    entitiesPlaymodeCopy = calloc(ECS.maxEntities,sizeof(Entity));

    VoxRenderSystem = GetSystemID("VoxelRenderer");
    EditorSystem = GetSystemID("Editor");

    //Disable all game systems, except the rendering
    for(i=0;i<GetLength(ECS.SystemList);i++){
        if(i != EditorSystem && i!= VoxRenderSystem){
            DisableSystem(i);
        }
    }
    
    gizmosFont = TTF_OpenFont("Interface/Fonts/gros/GROS.ttf",16);
	if(!gizmosFont){
		printf("Font: Error loading font!");
	}

    gizmosFontSmall= TTF_OpenFont("Interface/Fonts/coolthre/COOLTHRE.ttf",12);
    if(!gizmosFontSmall){
		printf("Font: Error loading small font!");
	}

    //Load UI icons
    glGenTextures(19, iconsTex);
    LoadUITexture("Interface/IconsUI/add.png",0);
    LoadUITexture("Interface/IconsUI/remove.png",1);
    LoadUITexture("Interface/IconsUI/bin.png",2);
    LoadUITexture("Interface/IconsUI/play.png",3);
    LoadUITexture("Interface/IconsUI/pause.png",4);
    LoadUITexture("Interface/IconsUI/stop.png",5);
    LoadUITexture("Interface/IconsUI/home.png",6);
    LoadUITexture("Interface/IconsUI/reload.png",7);
    LoadUITexture("Interface/IconsUI/next.png",8);
    LoadUITexture("Interface/IconsUI/previous.png",9);
    LoadUITexture("Interface/IconsUI/folder.png",10);
    LoadUITexture("Interface/IconsUI/file.png",11);
    LoadUITexture("Interface/IconsUI/up.png",12);
    LoadUITexture("Interface/IconsUI/down.png",13);
    LoadUITexture("Interface/IconsUI/toggleOn.png",14);
    LoadUITexture("Interface/IconsUI/toggleOff.png",15);
    LoadUITexture("Interface/IconsUI/toggleUndefined.png",16);
    LoadUITexture("Interface/IconsUI/voxel.png",17);
    LoadUITexture("Interface/IconsUI/x.png",18);
}

int movingX = 0;
int movingY = 0;
int movingZ = 0;


int entityStartHeight = 0;
int componentStartHeight = 0;
int movingScrollbar = 0;
int editingField = -1;
int addComponentWindowOpened = 0;
int addComponentScroll = 0;


//0 = Editor, 1 = Play, 2 = Paused
int playMode = 0;

//Runs each GameLoop iteration
void EditorUpdate(){
    EntityID entity;

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
    //Render UI objects in the [0,0.01] depth range
    glDepthRange(0, 0.01);

    glBindFramebuffer(GL_FRAMEBUFFER, Rendering.frameBuffer);
    glViewport(0,0,Screen.gameWidth,Screen.gameHeight);

    //-------------------------- Play Mode --------------------------
    Vector3 playBGMin = (Vector3){Screen.windowWidth/2 - iconsSize * 4  -2,  Screen.windowHeight-iconsSize * 4  -1 };
    Vector3 playBGMax = (Vector3){Screen.windowWidth/2 + iconsSize * 4  +2,  Screen.windowHeight};

    if(playMode == 1){
        //Play mode
        DrawRectangle(playBGMin,playBGMax,0.5,0.5,0.5);

        //Pause button
        if(1 == PointButton((Vector3){Screen.windowWidth/2 - iconsSize * 2 -2,  Screen.windowHeight-iconsSize * 2 -1 },4, 2, (Vector3){0.1,0.1,0.1}, (Vector3){1,1,1}, (Vector3){1,1,1})){
            playMode = 2;

            //Disable all game systems
            int i;
            for(i=0;i<GetLength(ECS.SystemList);i++){
                if(i != EditorSystem && i!= VoxRenderSystem){
                    DisableSystem(i);
                }
            }
        }
        //Stop button
        if(1 == PointButton((Vector3){Screen.windowWidth/2 + iconsSize * 2 +2,  Screen.windowHeight-iconsSize * 2 -1 },5, 2, (Vector3){0.1,0.1,0.1}, (Vector3){1,1,1}, (Vector3){1,1,1})){
            playMode = 0;
            ExitPlayMode();
            //Disable all game systems
            int i;
            for(i=0;i<GetLength(ECS.SystemList);i++){
                if(i != EditorSystem && i!= VoxRenderSystem){
                    DisableSystem(i);
                }
            }
        }
    }else if(playMode == 2){
        //Paused
        DrawRectangle(playBGMin,playBGMax,0.5,0.5,0.5);

        //Play button
        if(1 == PointButton((Vector3){Screen.windowWidth/2 - iconsSize * 2 -2,  Screen.windowHeight-iconsSize * 2 -1 },3, 2, (Vector3){0.1,0.1,0.1}, (Vector3){1,1,1}, (Vector3){1,1,1})){
            playMode = 1;
            //Enable all game systems
            int i;
            for(i=0;i<GetLength(ECS.SystemList);i++){
                if(i != EditorSystem && i!= VoxRenderSystem){
                    EnableSystem(i);
                }
            }
        }
        //Stop button enabled
        if(1 == PointButton((Vector3){Screen.windowWidth/2 + iconsSize * 2 +2,  Screen.windowHeight-iconsSize * 2 -1 },5, 2, (Vector3){0.1,0.1,0.1}, (Vector3){1,1,1}, (Vector3){1,1,1})){
            playMode = 0;
            ExitPlayMode();

            //Disable all game systems
            int i;
            for(i=0;i<GetLength(ECS.SystemList);i++){
                if(i != EditorSystem && i!= VoxRenderSystem){
                    DisableSystem(i);
                }
            }
        }
    }else{
        //In editor
        DrawRectangle(playBGMin,playBGMax,0.1,0.1,0.15);

        //Play button
        if(1 == PointButton((Vector3){Screen.windowWidth/2 - iconsSize * 2 -2,  Screen.windowHeight-iconsSize * 2 -1 },3, 2, (Vector3){0.7,0.7,0.7}, (Vector3){1,1,1}, (Vector3){1,1,1})){
            playMode = 1;
            EnterPlayMode();
            //Enable all game systems
            int i;
            for(i=0;i<GetLength(ECS.SystemList);i++){
                if(i != EditorSystem && i!= VoxRenderSystem){
                    EnableSystem(i);
                }
            }
        }
        //Stop button disabled
        PointButton((Vector3){Screen.windowWidth/2 + iconsSize * 2 +2,  Screen.windowHeight-iconsSize * 2 -1 },5, 2, (Vector3){0.2,0.2,0.2}, (Vector3){0.2,0.2,0.2}, (Vector3){0.2,0.2,0.2});
    }

    //-------------------------- Transform gizmos --------------------------
    if(!fileBrowserOpened){
        glPointSize(5 * 2/Screen.gameScale);
        glLineWidth(2* 2/Screen.gameScale);
        for(entity = 0; entity <= ECS.maxUsedIndex; entity++){
            
            //Run for all entities with the transform component
            if(EntityContainsComponent(entity,GetComponentID("Transform"))){

                Vector3 position = GetPosition(entity);
                Vector3 screenPos = PositionToGameScreenCoords(position);

                //Transform to screen coordinates
                screenPos.x = Screen.windowWidth/2 + (screenPos.x/(float)Screen.gameWidth) * Screen.windowWidth;
                screenPos.y = Screen.windowHeight/2 + (screenPos.y/(float)Screen.gameHeight) * Screen.windowHeight;

                Vector3 originPos = (Vector3){screenPos.x,screenPos.y,0};

                if(!IsSelected(entity)){
                    //Entity not selected, show selection point
                    glBegin(GL_POINTS);
                        if(MouseOverPointGizmos(mousePos, originPos, selectMouseOverDistance)){
                            glColor3f(1,1,1);
                            
                            if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                                if(GetKey(SDL_SCANCODE_LSHIFT)){
                                    InsertListEnd(&SelectedEntities,&entity);
                                }else{
                                    FreeList(&SelectedEntities);
                                    InsertListEnd(&SelectedEntities,&entity);
                                }
                            }
                        }else{
                            glColor3f(0.5,0.5,0.5);
                        }
                        glVertex2f(screenPos.x,screenPos.y);
                    glEnd();

                }else{
                    //Entity selected, show transform gizmos and deselection point
                    //Forward (X) line
                    glBegin(GL_LINES);

                        Vector3 lineXEndPos = PositionToGameScreenCoords(Add(position,(Vector3){positionGizmosLength * 2/Screen.gameScale,0,0}));

                        lineXEndPos.x = Screen.windowWidth/2 + (lineXEndPos.x/(float)Screen.gameWidth) * Screen.windowWidth;
                        lineXEndPos.y = Screen.windowHeight/2 + (lineXEndPos.y/(float)Screen.gameHeight) * Screen.windowHeight;
                        lineXEndPos.z = 0;

                        if(!movingX){
                            if(MouseOverLineGizmos(mousePos, originPos, lineXEndPos, axisMouseOverDistance) && !MouseOverPointGizmos(mousePos, originPos, selectMouseOverDistance)){
                                glColor3f(1,0.75,0.75);
                                
                                if(GetMouseButton(SDL_BUTTON_LEFT)){
                                    if(!movingZ && !movingY)
                                        movingX = 1;
                                }
                            }else{
                                glColor3f(1,0,0);
                            }
                        }else{
                            glColor3f(1,0.75,0.75);
                        }

                        glVertex2f(screenPos.x,screenPos.y);
                        glVertex2f(lineXEndPos.x,lineXEndPos.y);
                    glEnd();
                    //Forward (X) point
                    glBegin(GL_POINTS);
                        glVertex2f(lineXEndPos.x,lineXEndPos.y);
                    glEnd();

                    //Left (Y) line
                    glBegin(GL_LINES);

                        Vector3 lineYEndPos = PositionToGameScreenCoords(Add(position,(Vector3){0,positionGizmosLength* 2/Screen.gameScale,0}));

                        lineYEndPos.x = Screen.windowWidth/2 + (lineYEndPos.x/(float)Screen.gameWidth) * Screen.windowWidth;
                        lineYEndPos.y = Screen.windowHeight/2 + (lineYEndPos.y/(float)Screen.gameHeight) * Screen.windowHeight;
                        lineYEndPos.z = 0;

                        if(!movingY){
                            if(MouseOverLineGizmos(mousePos, originPos, lineYEndPos, axisMouseOverDistance) && !MouseOverPointGizmos(mousePos, originPos, selectMouseOverDistance)){
                                glColor3f(0.75,1,0.75);
                                if(GetMouseButton(SDL_BUTTON_LEFT)){
                                    if(!movingX && !movingZ)
                                        movingY = 1;
                                }
                            }else{
                                glColor3f(0,1,0);
                            }
                        }else{
                            glColor3f(0.75,1,0.75);
                        }

                        glVertex2f(screenPos.x,screenPos.y);
                        glVertex2f(lineYEndPos.x,lineYEndPos.y);
                    glEnd();
                    //Left (Y) point
                    glBegin(GL_POINTS);
                        glVertex2f(lineYEndPos.x,lineYEndPos.y);
                    glEnd();

                    //Up (Z) line
                    glBegin(GL_LINES);

                        Vector3 lineZEndPos = PositionToGameScreenCoords(Add(position,(Vector3){0,0,positionGizmosLength* 2/Screen.gameScale}));

                        lineZEndPos.x = Screen.windowWidth/2 + (lineZEndPos.x/(float)Screen.gameWidth) * Screen.windowWidth;
                        lineZEndPos.y = Screen.windowHeight/2 + (lineZEndPos.y/(float)Screen.gameHeight) * Screen.windowHeight;
                        lineZEndPos.z = 0;
                        
                        if(!movingZ){
                            if(MouseOverLineGizmos(mousePos, originPos, lineZEndPos, axisMouseOverDistance) && !MouseOverPointGizmos(mousePos, originPos, selectMouseOverDistance)){
                                glColor3f(0.75,0.75,1);
                                
                                if(GetMouseButton(SDL_BUTTON_LEFT)){
                                    if(!movingX && !movingY)
                                        movingZ = 1;
                                }
                            }else{
                                glColor3f(0,0,1);
                            }
                        }else{
                            glColor3f(0.75,0.75,1);
                        }

                        glVertex2f(screenPos.x,screenPos.y);
                        glVertex2f(lineZEndPos.x,lineZEndPos.y);
                    glEnd();
                    //Up (Z) point and origin point
                    glBegin(GL_POINTS);

                        glVertex2f(lineZEndPos.x,lineZEndPos.y);

                        if(MouseOverPointGizmos(mousePos, originPos, selectMouseOverDistance)){
                            glColor3f(1,1,0);

                            if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                                RemoveFromSelected(entity);
                            }
                        }else{
                            glColor3f(1,1,1);
                        }
                        glVertex2f(screenPos.x,screenPos.y);
                    glEnd();

                    //Arrow movement
                    if(movingX){
                        if(GetMouseButton(SDL_BUTTON_LEFT)){
                            double mouseMovementX = norm(VectorProjection(deltaMousePos,Subtract(lineXEndPos,originPos)))/(norm(WorldVectorToScreenVector(VECTOR3_FORWARD))*2) * sign(deltaMousePos.y);
                            SetPosition(entity,Add(GetPosition(entity),(Vector3){mouseMovementX,0,0}));
                        }else{
                            movingX = 0;
                        }
                    }

                    if(movingY){
                        if(GetMouseButton(SDL_BUTTON_LEFT)){
                            double mouseMovementY = norm(VectorProjection(deltaMousePos,Subtract(lineYEndPos,originPos)))/(norm(WorldVectorToScreenVector(VECTOR3_LEFT))*2) * sign(deltaMousePos.y);
                            SetPosition(entity,Add(GetPosition(entity),(Vector3){0,mouseMovementY,0}));
                        }else{
                            movingY = 0;
                        }
                    }

                    if(movingZ){
                        if(GetMouseButton(SDL_BUTTON_LEFT)){
                            double mouseMovementZ = norm(VectorProjection(deltaMousePos,Subtract(lineZEndPos,originPos)))/(norm(WorldVectorToScreenVector(VECTOR3_UP))*2)* sign(deltaMousePos.y);
                            SetPosition(entity,Add(GetPosition(entity),(Vector3){0,0,mouseMovementZ}));
                        }else{
                            movingZ = 0;
                        }
                    }
                    //int w, h;
                    //TTF_SizeText(gizmosFont,"Some random text",&w,&h);
                    //RenderText("Some random text", lightWhite, screenPos.x-w/2, screenPos.y-h*1.5,gizmosDepth, gizmosFont);  
                }
            }
        }
    }

    //-------------------------- Components panel --------------------------
    int currentComponentField = 0;
    if(!IsListEmpty(SelectedEntities)){

        //Panel background
        glBegin(GL_QUADS);
            glColor3f(bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);
            glVertex2f( Screen.windowWidth-componentWindowLength,  Screen.windowHeight);
            glVertex2f( Screen.windowWidth,  Screen.windowHeight);
            glVertex2f( Screen.windowWidth, 0);
            glVertex2f( Screen.windowWidth-componentWindowLength, 0);
        glEnd();

        //Set mask as the components of the first entity selected
        ComponentMask mask = GetEntityComponents(*((EntityID*)GetFirstElement(SelectedEntities)));

        ListCellPointer current;
        ListForEach(current,SelectedEntities){
            EntityID e = GetElementAsType(current, EntityID);
            mask = IntersectComponentMasks(mask,GetEntityComponents(e));
        }

        //Show components of the selected entities
        if(!addComponentWindowOpened){

            //Show the panel of the components contained by the selected entities
            int c,componentHeight = Screen.windowHeight + componentStartHeight;
            for(c=0;c<GetLength(ECS.ComponentTypes);c++){

                if(MaskContainsComponent(mask,c)){
                    //Drawing component element

                    //Only render internal fields if the component is not removed (returned 1)
                    if(DrawComponentHeader(c, &componentHeight)){
                        //Component specific drawing
                        if(c == GetComponentID("Transform")){
                            glBegin(GL_QUADS);
                                //Component background
                                glColor3f(0.1,0.1,0.15);
                                glVertex2f( Screen.windowWidth-componentWindowLength,  componentHeight);
                                glVertex2f( Screen.windowWidth-componentWindowWidthSpacing,  componentHeight);
                                glVertex2f( Screen.windowWidth-componentWindowWidthSpacing, componentHeight-95);
                                glVertex2f( Screen.windowWidth-componentWindowLength, componentHeight-95);
                            glEnd();

                            int ommitPosX = 0, ommitPosY = 0, ommitPosZ = 0;
                            int ommitRotX = 0, ommitRotY = 0, ommitRotZ = 0;
                            ListCellPointer selEntity = GetFirstCell(SelectedEntities);
                            Vector3 lastPos = GetPosition(GetElementAsType(selEntity,int));
                            Vector3 lastRot = GetRotation(GetElementAsType(selEntity,int));
                            
                            ListForEach(selEntity, SelectedEntities){
                                Vector3 curPos = GetPosition(GetElementAsType(selEntity,int));
                                Vector3 curRot = GetRotation(GetElementAsType(selEntity,int));
                                if(curPos.x != lastPos.x){
                                    ommitPosX = 1;
                                    lastPos.x = 0;
                                }
                                if(curPos.y != lastPos.y){
                                    ommitPosY = 1;
                                    lastPos.y = 0;
                                }
                                if(curPos.z != lastPos.z){
                                    ommitPosZ = 1;
                                    lastPos.z = 0;
                                }

                                if(curRot.x != lastRot.x){
                                    ommitRotX = 1;
                                    lastRot.x = 0;
                                }
                                if(curRot.y != lastRot.y){
                                    ommitRotY = 1;
                                    lastRot.y = 0;
                                }
                                if(curRot.z != lastRot.z){
                                    ommitRotZ = 1;
                                    lastRot.z = 0;
                                }
                            }
                            Vector3 newPos = lastPos;
                            Vector3 newRot = lastRot;
                            Vector3Field("position",&newPos,ommitPosX,ommitPosY,ommitPosZ,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1,4, &currentComponentField, &componentHeight);
                            Vector3Field("rotation",&newRot,ommitRotX,ommitRotY,ommitRotZ,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1,4, &currentComponentField, &componentHeight);

                            int changedPosX = 0, changedPosY = 0, changedPosZ = 0;
                            int changedRotX = 0, changedRotY = 0, changedRotZ = 0;
                            if(lastPos.x != newPos.x) changedPosX = 1;
                            if(lastPos.y != newPos.y) changedPosY = 1;
                            if(lastPos.z != newPos.z) changedPosZ = 1;

                            if(lastRot.x != newRot.x) changedRotX = 1;
                            if(lastRot.y != newRot.y) changedRotY = 1;
                            if(lastRot.z != newRot.z) changedRotZ = 1;
                            
                            if(changedPosX || changedPosY || changedPosZ || changedRotX || changedRotY || changedRotZ){
                                ListForEach(selEntity, SelectedEntities){
                                    Vector3 pos = GetPosition(GetElementAsType(selEntity,int));
                                    Vector3 rot = GetRotation(GetElementAsType(selEntity,int));

                                    if(changedPosX) pos.x = newPos.x;
                                    if(changedPosY) pos.y = newPos.y;
                                    if(changedPosZ) pos.z = newPos.z;

                                    if(changedRotX) rot.x = newRot.x;
                                    if(changedRotY) rot.y = newRot.y;
                                    if(changedRotZ) rot.z = newRot.z;

                                    SetPosition(GetElementAsType(selEntity,int),pos);
                                    SetRotation(GetElementAsType(selEntity,int),rot);
                                }
                            }
                            componentHeight -= 7;
                        }else if(c == GetComponentID("RigidBody")){
                            
                            //Check if the selected entities are static and use gravity
                            ListCellPointer selEntity = GetFirstCell(SelectedEntities);
                            int isStatic = IsStaticRigidbody(GetElementAsType(selEntity,int));
                            int useGravity = UsesGravity(GetElementAsType(selEntity,int)); 
                            ListForEach(selEntity, SelectedEntities){
                                if(IsStaticRigidbody(GetElementAsType(selEntity,int)) != isStatic){
                                    isStatic = -1;
                                }
                                if(UsesGravity(GetElementAsType(selEntity,int)) != useGravity){
                                    useGravity = -1;
                                }
                                if(isStatic == -1 && useGravity == -1) break;
                            }

                            if(!isStatic){
                                glBegin(GL_QUADS);
                                    //Component background
                                    glColor3f(0.1,0.1,0.15);
                                    glVertex2f( Screen.windowWidth-componentWindowLength,  componentHeight);
                                    glVertex2f( Screen.windowWidth-componentWindowWidthSpacing,  componentHeight);
                                    glVertex2f( Screen.windowWidth-componentWindowWidthSpacing, componentHeight-225);
                                    glVertex2f( Screen.windowWidth-componentWindowLength, componentHeight-225);
                                glEnd();

                                componentHeight-=2;
                                if(1 == PointToggle(&isStatic,(Vector3){Screen.windowWidth-componentWindowLength + 12,componentHeight - 10},14,15,16,1, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1})){
                                    ListForEach(selEntity, SelectedEntities){
                                        SetStaticRigidbody(GetElementAsType(selEntity,int),isStatic);
                                    }
                                }
                                RenderText("Is Static", lightWhite, Screen.windowWidth-componentWindowLength + 25, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                                componentHeight-=20;

                                if(1 == PointToggle(&useGravity,(Vector3){Screen.windowWidth-componentWindowLength + 12,componentHeight - 10},14,15,16,1, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1})){
                                    ListForEach(selEntity, SelectedEntities){
                                        SetUseGravity(GetElementAsType(selEntity,int),useGravity);
                                    }
                                }
                                RenderText("use gravity", lightWhite, Screen.windowWidth-componentWindowLength + 25, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                                componentHeight-=20;

                                int ommitVelX = 0, ommitVelY = 0, ommitVelZ = 0;
                                int ommitAccelX = 0, ommitAccelY = 0, ommitAccelZ = 0;
                                int ommitBounc = 0, ommitMass = 0;

                                selEntity = GetFirstCell(SelectedEntities);
                                Vector3 lastVel = GetVelocity(GetElementAsType(selEntity,int));
                                Vector3 lastAccel = GetAcceleration(GetElementAsType(selEntity,int));
                                float lastBounc = GetBounciness(GetElementAsType(selEntity,int));
                                float lastMass = GetMass(GetElementAsType(selEntity,int));
                                
                                ListForEach(selEntity, SelectedEntities){
                                    Vector3 curVel = GetVelocity(GetElementAsType(selEntity,int));
                                    Vector3 curAccel = GetAcceleration(GetElementAsType(selEntity,int));
                                    float curBounc = GetBounciness(GetElementAsType(selEntity,int));
                                    float curMass = GetMass(GetElementAsType(selEntity,int));

                                    if(curVel.x != lastVel.x){
                                        ommitVelX = 1;
                                        lastVel.x = 0;
                                    }
                                    if(curVel.y != lastVel.y){
                                        ommitVelY = 1;
                                        lastVel.y = 0;
                                    }
                                    if(curVel.z != lastVel.z){
                                        ommitVelZ = 1;
                                        lastVel.z = 0;
                                    }

                                    if(curAccel.x != lastAccel.x){
                                        ommitAccelX = 1;
                                        lastAccel.x = 0;
                                    }
                                    if(curAccel.y != lastAccel.y){
                                        ommitAccelY = 1;
                                        lastAccel.y = 0;
                                    }
                                    if(curBounc != lastBounc){
                                        ommitBounc = 1;
                                        lastBounc = 0;
                                    }
                                    if(curMass != lastMass){
                                        ommitMass = 1;
                                        lastMass = 0;
                                    }
                                }
                                Vector3 newVel = lastVel;
                                Vector3 newAccel = lastAccel;
                                float newBounc = lastBounc;
                                float newMass = lastMass;

                                FloatField("mass",&newMass,ommitMass,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1, &currentComponentField, &componentHeight);
                                FloatField("bounciness",&newBounc,ommitBounc,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1, &currentComponentField, &componentHeight);
                                Vector3Field("initial velocity",&newVel,ommitVelX,ommitVelY,ommitVelZ,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1,4, &currentComponentField, &componentHeight);
                                Vector3Field("const. accel.",&newAccel,ommitAccelX,ommitAccelY,ommitAccelZ,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1,4, &currentComponentField, &componentHeight);

                                int changedVelX = 0, changedVelY = 0, changedVelZ = 0;
                                int changedAccelX = 0, changedAccelY = 0, changedAccelZ = 0;
                                int changedBounc = 0, changedMass = 0;

                                if(lastVel.x != newVel.x) changedVelX = 1;
                                if(lastVel.y != newVel.y) changedVelY = 1;
                                if(lastVel.z != newVel.z) changedVelZ = 1;

                                if(lastAccel.x != newAccel.x) changedAccelX = 1;
                                if(lastAccel.y != newAccel.y) changedAccelY = 1;
                                if(lastAccel.z != newAccel.z) changedAccelZ = 1;

                                if(lastBounc != newBounc) changedBounc = 1;
                                if(lastMass != newMass) changedMass = 1;
                                
                                if(changedVelX || changedVelY || changedVelZ || changedAccelX || changedAccelY || changedAccelZ || changedBounc || changedMass){
                                    ListForEach(selEntity, SelectedEntities){
                                        Vector3 pos = GetVelocity(GetElementAsType(selEntity,int));
                                        Vector3 rot = GetAcceleration(GetElementAsType(selEntity,int));

                                        if(changedVelX) pos.x = newVel.x;
                                        if(changedVelY) pos.y = newVel.y;
                                        if(changedVelZ) pos.z = newVel.z;

                                        if(changedAccelX) rot.x = newAccel.x;
                                        if(changedAccelY) rot.y = newAccel.y;
                                        if(changedAccelZ) rot.z = newAccel.z;

                                        SetVelocity(GetElementAsType(selEntity,int),pos);
                                        SetAcceleration(GetElementAsType(selEntity,int),rot);

                                        if(changedBounc) SetBounciness(GetElementAsType(selEntity,int),newBounc);
                                        if(changedMass) SetMass(GetElementAsType(selEntity,int),newMass);
                                    }
                                }
                                componentHeight -= 7;
                            }else{
                                glBegin(GL_QUADS);
                                    //Component background
                                    glColor3f(0.1,0.1,0.15);
                                    glVertex2f( Screen.windowWidth-componentWindowLength,  componentHeight);
                                    glVertex2f( Screen.windowWidth-componentWindowWidthSpacing,  componentHeight);
                                    glVertex2f( Screen.windowWidth-componentWindowWidthSpacing, componentHeight-22);
                                    glVertex2f( Screen.windowWidth-componentWindowLength, componentHeight-22);
                                glEnd();

                                componentHeight-=2;
                                if(1 == PointToggle(&isStatic,(Vector3){Screen.windowWidth-componentWindowLength + 12,componentHeight - 10},14,15,16,1, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1})){
                                    ListForEach(selEntity, SelectedEntities){
                                        SetStaticRigidbody(GetElementAsType(selEntity,int),isStatic);
                                    }
                                }
                                RenderText("Is Static", lightWhite, Screen.windowWidth-componentWindowLength + 25, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                                componentHeight-=20;
                            }
                        }else if(c == GetComponentID("VoxelModel")){
                            
                            
                            ListCellPointer selEntity = GetFirstCell(SelectedEntities);
                            int isEnabled = IsVoxelModelEnabled(GetElementAsType(selEntity,EntityID));

                            glBegin(GL_QUADS);
                                //Component background
                                glColor3f(0.1,0.1,0.15);
                                glVertex2f( Screen.windowWidth-componentWindowLength,  componentHeight);
                                glVertex2f( Screen.windowWidth-componentWindowWidthSpacing,  componentHeight);
                                glVertex2f( Screen.windowWidth-componentWindowWidthSpacing, componentHeight-92);
                                glVertex2f( Screen.windowWidth-componentWindowLength, componentHeight-92);
                            glEnd();

                            VoxelModel* m = GetVoxelModelPointer(GetElementAsType(selEntity,EntityID));
                            if(1 == PointToggle(&isEnabled,(Vector3){Screen.windowWidth-componentWindowLength + 12,componentHeight - 10},14,15,16,1, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1})){
                                ListForEach(selEntity, SelectedEntities){
                                    SetVoxelModelEnabled(GetElementAsType(selEntity,int),isEnabled);
                                }
                            }
                            RenderText("Enabled", lightWhite, Screen.windowWidth-componentWindowLength + 25, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                            componentHeight-=22;

                            if(1 == PointButton((Vector3){Screen.windowWidth-componentWindowLength + 15,componentHeight - 10},10,1, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1}, (Vector3){0.8,0.8,0.8})){
                                if(!fileBrowserOpened){
                                    if(m->modelPath[0] != '\0'){
                                        OpenFileBrowser(GetVoxelModelPointer(GetElementAsType(selEntity,EntityID))->modelPath,LoadModel);
                                    }else{
                                        OpenFileBrowser(NULL,LoadModel);
                                    }
                                    FileBrowserExtension("vox");
                                }
                            }
                            
                            if(m->modelName[0] != '\0'){
                                RenderText(m->modelName, lightWhite, Screen.windowWidth-componentWindowLength + 28, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                            }else{
                                RenderText("No model", lightWhite, Screen.windowWidth-componentWindowLength + 28, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                            }
                            componentHeight-=22;
                            
                            //Center field
                            int ommitCenterX = 0, ommitCenterY = 0, ommitCenterZ = 0;

                            selEntity = GetFirstCell(SelectedEntities);
                            Vector3 lastCenter = GetVoxelModelCenter(GetElementAsType(selEntity,int));
                            
                            ListForEach(selEntity, SelectedEntities){
                                Vector3 curCenter = GetVoxelModelCenter(GetElementAsType(selEntity,int));

                                if(curCenter.x != lastCenter.x){
                                    ommitCenterX = 1;
                                    lastCenter.x = 0;
                                }
                                if(curCenter.y != lastCenter.y){
                                    ommitCenterY = 1;
                                    lastCenter.y = 0;
                                }
                                if(curCenter.z != lastCenter.z){
                                    ommitCenterZ = 1;
                                    lastCenter.z = 0;
                                }
                            }
                            Vector3 newCenter = lastCenter;

                            Vector3Field("Center",&newCenter,ommitCenterX,ommitCenterY,ommitCenterZ,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1,4, &currentComponentField, &componentHeight);
                            int changedCenterX = 0, changedCenterY = 0, changedCenterZ = 0;

                            if(lastCenter.x != newCenter.x) changedCenterX = 1;
                            if(lastCenter.y != newCenter.y) changedCenterY = 1;
                            if(lastCenter.z != newCenter.z) changedCenterZ = 1;
                            
                            if(changedCenterX || changedCenterY || changedCenterZ){
                                ListForEach(selEntity, SelectedEntities){
                                    Vector3 center = GetVoxelModelCenter(GetElementAsType(selEntity,int));

                                    if(changedCenterX) center.x = newCenter.x;
                                    if(changedCenterY) center.y = newCenter.y;
                                    if(changedCenterZ) center.z = newCenter.z;

                                    SetVoxelModelCenter(GetElementAsType(selEntity,int),center);
                                }
                            }
                            componentHeight -= 7;
                        }
                    }
                    componentHeight -= componentBetweenSpacing;
                }
            }
            
            //Scrollbar
            
            int mouseOverComponentPanel = MouseOverBox(mousePos, (Vector3){Screen.windowWidth-componentWindowLength,0,0}, (Vector3){Screen.windowWidth,Screen.windowHeight,0},0);
            glLineWidth(clamp((componentWindowWidthSpacing/2 - 2) * 2/Screen.gameScale,1,Screen.windowWidth));
            int offscreenPixels = -clamp(componentHeight-componentStartHeight-componentWindowBottomSpacing,-(Screen.windowHeight-10),0);
            glBegin(GL_LINES);  
                Vector3 scrollbarStart = {Screen.windowWidth - componentWindowWidthSpacing/2 ,Screen.windowHeight-componentStartHeight -2,0};
                Vector3 scrollbarEnd = {Screen.windowWidth - componentWindowWidthSpacing/2 ,offscreenPixels - componentStartHeight +componentWindowBottomSpacing+ 1,0};
                
                if(mouseOverComponentPanel){
                    if(!movingScrollbar){
                        if(MouseOverLineGizmos(mousePos, scrollbarStart, scrollbarEnd, scrollbarMouseOverDistance)){
                            glColor3f(0.5,0.5,0.5);

                            if(GetMouseButton(SDL_BUTTON_LEFT)){
                                movingScrollbar = 1;
                            }
                        }else{
                            glColor3f(0.3,0.3,0.4);  
                        }
                    }else if (movingScrollbar == 1){
                        glColor3f(0.55,0.55,0.55);
                    }else{
                        glColor3f(0.3,0.3,0.4); 
                    }
                }else{
                    glColor3f(0.3,0.3,0.3);
                }

                glVertex2f(scrollbarStart.x,scrollbarStart.y);
                glVertex2f(scrollbarEnd.x,scrollbarEnd.y);
            glEnd();

            if(mouseOverComponentPanel){
                if(Input.mouseWheelY!=0) movingScrollbar = 2;
                if(movingScrollbar){

                    //Moving with mouse click
                    if(movingScrollbar == 1){
                        if(GetMouseButton(SDL_BUTTON_LEFT)){
                            if(offscreenPixels>0){
                                double scrollbarMovement = norm(VectorProjection(deltaMousePos,(Vector3){0,1,0})) * sign(deltaMousePos.y);
                                componentStartHeight = clamp(componentStartHeight-scrollbarMovement,0,offscreenPixels);
                            }else{
                                componentStartHeight = 0;
                            }
                        }else{
                            movingScrollbar = 0;
                        }
                    }else{
                        //Moving with mouse wheel
                        if(Input.mouseWheelY!=0){
                            if(offscreenPixels>0){
                                double scrollbarMovement = Input.mouseWheelY*scrollbarMouseWheelSpeed;
                                componentStartHeight = clamp(componentStartHeight-scrollbarMovement,0,offscreenPixels);
                            }else{
                                componentStartHeight = 0;
                            } 
                        }else{
                            movingScrollbar = 0;
                        }
                    }
                }
            }
        }else{
            //Show components to be added
            int buttonHeight = Screen.windowHeight - (addComponentScroll>0? 22:0);
            int w,h,i = 0;
            ListCellPointer cellComp;
            ListForEach(cellComp,ECS.ComponentTypes){
                if(i-addComponentScroll<0 || buttonHeight<componentWindowBottomSpacing ){i++; continue;}

                Vector3 cbMin = {Screen.windowWidth - componentWindowLength+1,buttonHeight-40};
                Vector3 cbMax = {Screen.windowWidth-2,buttonHeight};
                if(!MaskContainsComponent(mask,i)){
                    if(MouseOverBox(mousePos,cbMin,cbMax,0)){
                        DrawRectangle(cbMin, cbMax,0.3,0.3,0.45);
                        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                            ListCellPointer cellSel;
                            ListForEach(cellSel,SelectedEntities){
                                if(!EntityContainsComponent(GetElementAsType(cellSel,EntityID),i)){
                                    AddComponentToEntity(i,GetElementAsType(cellSel,EntityID));
                                    addComponentWindowOpened = 0;
                                }
                            }
                        }
                    }else{
                        DrawRectangle(cbMin, cbMax,0.2,0.2,0.35);
                    }
                }else{
                    DrawRectangle(cbMin, cbMax,0.1,0.1,0.1);
                }
                TTF_SizeText(gizmosFont,GetElementAsType(cellComp,ComponentType).name,&w,&h);
                RenderText(GetElementAsType(cellComp,ComponentType).name, lightWhite, cbMin.x + ((cbMax.x-cbMin.x)-w)/2, cbMin.y + ((cbMax.y-cbMin.y)-h)/2, gizmosFont);

                buttonHeight-= 42;
                i++;
            }

            //Scrollbar
            //'i' is now the number of component types
            if(GetLength(ECS.ComponentTypes) > 16){
                Vector3 scrollbarDownMin = {Screen.windowWidth - componentWindowLength,componentWindowBottomSpacing+2};
                Vector3 scrollbarDownMax = {Screen.windowWidth,componentWindowBottomSpacing+22};
                Vector3 scrollbarUpMin = {Screen.windowWidth - componentWindowLength,Screen.windowHeight-22};
                Vector3 scrollbarUpMax = {Screen.windowWidth,Screen.windowHeight-2};

                if(addComponentScroll < GetLength(ECS.ComponentTypes)-15){
                    if(MouseOverBox(mousePos,scrollbarDownMin,scrollbarDownMax,0)){
                        DrawRectangle(scrollbarDownMin,scrollbarDownMax,0.1,0.1,0.125);
                        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                            addComponentScroll++;
                        }
                    }else{
                        DrawRectangle(scrollbarDownMin,scrollbarDownMax,0.05,0.05,0.075);
                    }
                    DrawPointIcon((Vector3){scrollbarDownMin.x + (scrollbarDownMax.x - scrollbarDownMin.x)/2 - 1,
                                            scrollbarDownMin.y + (scrollbarDownMax.y - scrollbarDownMin.y)/2},13, 1, (Vector3){0.2,0.2,0.2});

                    //Mouse scroll
                    if(Input.mouseWheelY<0){
                        addComponentScroll++;
                    }
                }
                if(addComponentScroll>0){
                    if(MouseOverBox(mousePos,scrollbarUpMin,scrollbarUpMax,0)){
                        DrawRectangle(scrollbarUpMin,scrollbarUpMax,0.1,0.1,0.125);
                        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                            addComponentScroll--;
                        }
                    }else{
                        DrawRectangle(scrollbarUpMin,scrollbarUpMax,0.05,0.05,0.075);
                    }
                    DrawPointIcon((Vector3){scrollbarUpMin.x + (scrollbarUpMax.x - scrollbarUpMin.x)/2 - 1,
                                            scrollbarUpMin.y + (scrollbarUpMax.y - scrollbarUpMin.y)/2},12, 1, (Vector3){0.2,0.2,0.2});
                    //Mouse scroll
                    if(Input.mouseWheelY>0){
                        addComponentScroll--;
                    }
                }    
            }else{
                fbItemsScroll = 0;
            }
        }

        //Line separating the add component buttom from the components
        glLineWidth(2/Screen.gameScale);
        glColor3f(bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);
        glBegin(GL_LINES);  
            glVertex2f(Screen.windowWidth - componentWindowLength,componentWindowBottomSpacing+2);
            glVertex2f(Screen.windowWidth,componentWindowBottomSpacing+2);
            glVertex2f(Screen.windowWidth - componentWindowLength,1);
            glVertex2f(Screen.windowWidth,1);
        glEnd();

        //Add component button
        glBegin(GL_QUADS);

            Vector3 cbMin = {Screen.windowWidth - componentWindowLength,2,0};
            Vector3 cbMax = {Screen.windowWidth-2,componentWindowBottomSpacing,0};
            if(MouseOverBox(mousePos, cbMin, cbMax,0)){
                glColor3f(0.3,0.3,0.4);

                if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                    addComponentWindowOpened = !addComponentWindowOpened;
                }
            }else{
                glColor3f(0.2,0.2,0.35);
            }
            glVertex2f( cbMax.x, cbMax.y);
            glVertex2f( cbMin.x, cbMax.y);
            glVertex2f( cbMin.x, cbMin.y);
            glVertex2f( cbMax.x, cbMin.y);
        glEnd();
        int w,h;
        if(!addComponentWindowOpened){
            TTF_SizeText(gizmosFont,"Add Component",&w,&h);
            RenderText("Add Component", brightWhite, cbMin.x + ((cbMax.x-cbMin.x)-w)/2, cbMin.y + ((cbMax.y-cbMin.y)-h)/2, gizmosFont);
        }else{
            TTF_SizeText(gizmosFont,"Back",&w,&h);
            RenderText("Back", brightWhite, cbMin.x + ((cbMax.x-cbMin.x)-w)/2, cbMin.y + ((cbMax.y-cbMin.y)-h)/2, gizmosFont);
        }
    }

    //-------------------------- Entities panel --------------------------

    //Entities Panel background
    glBegin(GL_QUADS);
        glColor3f(bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);
        glVertex2f( entityWindowLength,  Screen.windowHeight);
        glVertex2f( 0,  Screen.windowHeight);
        glVertex2f( 0, 0);
        glVertex2f( entityWindowLength, 0);
    glEnd();

    //Update array of instantiated entities
    memset(existingEntities, 1, ECS.maxEntities * sizeof(int));
    ListCellPointer curEntity;
    ListForEach(curEntity,ECS.AvaliableEntitiesIndexes){
        EntityID id = GetElementAsType(curEntity,EntityID);
        existingEntities[id] = 0;
    }

    int entityHeight = Screen.windowHeight + entityStartHeight-entityWindowTopHeightSpacing;
    for(entity=0;entity<=ECS.maxUsedIndex;entity++){
        if(existingEntities[entity]){

            if(entityHeight>0 && entityHeight<Screen.windowHeight-entityWindowTopHeightSpacing+TTF_FontHeight(gizmosFont)+2){

                int isSelected = IsSelected(entity);
                Vector3 elMin = {entityWindowWidthSpacing,entityHeight-TTF_FontHeight(gizmosFont)-2,0};
                Vector3 elMax = {entityWindowLength,entityHeight,0};
                
                //Entity element
                if(MouseOverBox(mousePos, elMin, elMax,0)){
                    if(isSelected){
                        DrawRectangle(elMin,elMax,0.3,0.3,0.3);

                        if(GetMouseButtonDown(SDL_BUTTON_LEFT) && !fileBrowserOpened){
                            RemoveFromSelected(entity);
                        }
                        if(GetMouseButtonDown(SDL_BUTTON_RIGHT) && !fileBrowserOpened){
                            EntityID newEntity = DuplicateEntity(entity);
                            FreeList(&SelectedEntities);
                            InsertListEnd(&SelectedEntities,&newEntity);
                        }
                    }else{
                        DrawRectangle(elMin,elMax,0.3,0.3,0.4);

                        if(GetMouseButtonDown(SDL_BUTTON_LEFT) && !fileBrowserOpened){
                            if(GetKey(SDL_SCANCODE_LSHIFT)){
                                InsertListEnd(&SelectedEntities,&entity);
                            }else{
                                FreeList(&SelectedEntities);
                                InsertListEnd(&SelectedEntities,&entity);
                            }
                        }
                    }
                }else{
                    if(isSelected){
                        DrawRectangle(elMin,elMax,0.35,0.35,0.5);
                    }else{
                        DrawRectangle(elMin,elMax,0.2,0.2,0.35);
                    }
                }
                static char entityName[12];
                snprintf(entityName,11,"%d",entity);
                RenderText(entityName, lightWhite, entityNameLeftSpacing, entityHeight - TTF_FontHeight(gizmosFont), gizmosFont);
            }
            entityHeight -= TTF_FontHeight(gizmosFont)+2 + entityBetweenSpacing;
        }
    }

    //Scrollbar
    int mouseOverEntityPanel = MouseOverBox(mousePos, (Vector3){0,0,0}, (Vector3){entityWindowLength,Screen.windowHeight-entityWindowTopHeightSpacing,0},0);
    glLineWidth(clamp((entityWindowWidthSpacing/2 - 2) * 2/Screen.gameScale,1,Screen.windowWidth));
    int offscreenPixels = -clamp(entityHeight-entityStartHeight,-(Screen.windowHeight-10),0);
    glBegin(GL_LINES);  
    
        Vector3 scrollbarStart = {entityWindowWidthSpacing/2 ,Screen.windowHeight-entityStartHeight - entityWindowTopHeightSpacing,0};
        Vector3 scrollbarEnd = {entityWindowWidthSpacing/2 ,offscreenPixels - entityStartHeight + 1,0};
        
        if(mouseOverEntityPanel){
            if(!movingScrollbar){
                if(MouseOverLineGizmos(mousePos, scrollbarStart, scrollbarEnd, scrollbarMouseOverDistance)){
                    glColor3f(0.5,0.5,0.5);

                    if(GetMouseButton(SDL_BUTTON_LEFT)){
                        movingScrollbar = 1;
                    }
                }else{
                    glColor3f(0.3,0.3,0.4);  
                }
            }else if (movingScrollbar == 1){
                glColor3f(0.55,0.55,0.55);
            }else{
                glColor3f(0.3,0.3,0.4); 
            }
        }else{
            glColor3f(0.3,0.3,0.3);
        }

        glVertex2f(scrollbarStart.x,scrollbarStart.y);
        glVertex2f(scrollbarEnd.x,scrollbarEnd.y);
    glEnd();

    if(mouseOverEntityPanel){
        if(Input.mouseWheelY!=0) movingScrollbar = 2;
        if(movingScrollbar){

            //Moving with mouse click
            if(movingScrollbar == 1){
                if(GetMouseButton(SDL_BUTTON_LEFT)){
                    if(offscreenPixels>0){
                        double scrollbarMovement = norm(VectorProjection(deltaMousePos,(Vector3){0,1,0})) * sign(deltaMousePos.y);
                        entityStartHeight = clamp(entityStartHeight-scrollbarMovement,0,offscreenPixels);
                    }else{
                        entityStartHeight = 0;
                    }
                }else{
                    movingScrollbar = 0;
                }
            }else{
                //Moving with mouse wheel
                if(Input.mouseWheelY!=0){
                    if(offscreenPixels>0){
                        double scrollbarMovement = Input.mouseWheelY*scrollbarMouseWheelSpeed;
                        entityStartHeight = clamp(entityStartHeight-scrollbarMovement,0,offscreenPixels);
                    }else{
                        entityStartHeight = 0;
                    } 
                }else{
                    movingScrollbar = 0;
                }
            }
        }
    }

    //New Entity button
    glLineWidth(2 * 2/Screen.gameScale);
    glColor3f(bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);
    glBegin(GL_LINES);  
        //Line separating new entity button from the entities and the scrollbar
        glVertex2f(0,Screen.windowHeight-entityWindowTopHeightSpacing);
        glVertex2f(entityWindowLength,Screen.windowHeight-entityWindowTopHeightSpacing);
    glEnd();
    glBegin(GL_QUADS);
        //Entity element
        Vector3 bMin = {0,Screen.windowHeight-entityWindowTopHeightSpacing+2,0};
        Vector3 bMax = {entityWindowLength,Screen.windowHeight-(entityWindowTopHeightSpacing/2),0};
        if(MouseOverBox(mousePos, bMin, bMax,0)){
            glColor3f(0.3,0.3,0.4);
            if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                CreateEntity();
                glColor3f(0.3,0.3,0.4);
            }
        }else{
            glColor3f(0.2,0.2,0.35);
        }
        glVertex2f( bMax.x, bMax.y);
        glVertex2f( bMin.x, bMax.y);
        glVertex2f( bMin.x, bMin.y);
        glVertex2f( bMax.x, bMin.y);
    glEnd();
    RenderText("+ Entity", brightWhite, 5, Screen.windowHeight-(entityWindowTopHeightSpacing)+5, gizmosFont);

    //-------------------------- File browser --------------------------
    if(fileBrowserOpened){
        int w,h;
        //Backgrounds
        Vector3 fbMin = {Screen.windowWidth/2 -300,Screen.windowHeight/2 -200};
        Vector3 fbMax = {Screen.windowWidth/2 +300,Screen.windowHeight/2 +200};
        Vector3 fbFootMin = {Screen.windowWidth/2 -299,Screen.windowHeight/2 -199};
        Vector3 fbFootMax = {Screen.windowWidth/2 +299,Screen.windowHeight/2 -150};
        Vector3 fbHeaderMin = {Screen.windowWidth/2 -299,Screen.windowHeight/2 +169};
        Vector3 fbHeaderMax = {Screen.windowWidth/2 +299,Screen.windowHeight/2 +199};

        DrawRectangle(fbMin,fbMax,bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);
        DrawRectangle(fbFootMin,fbFootMax,0.1,0.1,0.15);
        DrawRectangle(fbHeaderMin,fbHeaderMax,0.1,0.1,0.15);

        //Header
        Vector3 filepathBgMin = {Screen.windowWidth/2-180,Screen.windowHeight/2 +172};
        Vector3 filepathBgMax = {Screen.windowWidth/2 +297,Screen.windowHeight/2 +195};
        DrawRectangle(filepathBgMin,filepathBgMax,0.2, 0.2, 0.2);

        //Header buttons
        //Previous button
        if(indexPath>0){
            if(PointButton((Vector3){fbHeaderMin.x+ iconsSize * 2 ,fbHeaderMin.y+(fbHeaderMax.y - fbHeaderMin.y)/2,0},9, 1, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1}, (Vector3){0.5,0.5,0.5}) == 1){
                indexPath--;
                OpenFileBrowser(*((char**)GetElementAt(Paths,indexPath)),onOpenFunction);
            }
        }else{
            PointButton((Vector3){fbHeaderMin.x+ iconsSize * 2 ,fbHeaderMin.y+(fbHeaderMax.y - fbHeaderMin.y)/2,0},9, 1, (Vector3){0.25,0.25,0.25}, (Vector3){0.25,0.25,0.25}, (Vector3){0.25,0.25,0.25});
        }
        //Next button
        if(indexPath<GetLength(Paths)-1){
            if(PointButton((Vector3){fbHeaderMin.x+ iconsSize * 6 ,fbHeaderMin.y+(fbHeaderMax.y - fbHeaderMin.y)/2,0},8, 1, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1}, (Vector3){0.5,0.5,0.5}) == 1){
                indexPath++;
                OpenFileBrowser(*((char**)GetElementAt(Paths,indexPath)),onOpenFunction);
            }
        }else{
            PointButton((Vector3){fbHeaderMin.x+ iconsSize * 6 ,fbHeaderMin.y+(fbHeaderMax.y - fbHeaderMin.y)/2,0},8, 1, (Vector3){0.25,0.25,0.25}, (Vector3){0.25,0.25,0.25}, (Vector3){0.25,0.25,0.25});
        }
        //Home
        if(PointButton((Vector3){fbHeaderMin.x+ iconsSize * 10,fbHeaderMin.y+(fbHeaderMax.y - fbHeaderMin.y)/2,0},6, 1, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1}, (Vector3){0.5,0.5,0.5})==1){
            OpenFileBrowser(NULL,onOpenFunction);
        }
        //File path
        if(strlen(filePath)>34){
            char minifiedPath[] = "0000000000000000000000000000000000...";
            memcpy(minifiedPath,filePath,34*sizeof(char));
            TTF_SizeText(gizmosFont,minifiedPath,&w,&h);
            RenderText(minifiedPath, lightWhite, filepathBgMin.x + 6, filepathBgMin.y+ ((filepathBgMax.y-filepathBgMin.y)-h)/2 -1, gizmosFont);    
        }else{
            TTF_SizeText(gizmosFont,filePath,&w,&h);
            RenderText(filePath, lightWhite, filepathBgMin.x + 6, filepathBgMin.y+ ((filepathBgMax.y-filepathBgMin.y)-h)/2 -1, gizmosFont);    
        }


        //Foot
        //Cancel Button
        Vector3 cancelButtonMin = {Screen.windowWidth/2 +207,Screen.windowHeight/2 -197};
        Vector3 cancelButtonMax = {Screen.windowWidth/2 +297,Screen.windowHeight/2 -152};
        if(MouseOverBox(mousePos,cancelButtonMin,cancelButtonMax,0)){
            DrawRectangle(cancelButtonMin,cancelButtonMax,0.3,0.3,0.45);
            if(GetMouseButtonUp(SDL_BUTTON_LEFT)){
                CloseFileBrowser();
            }
        }else{
            DrawRectangle(cancelButtonMin,cancelButtonMax,0.2,0.2,0.35);
        }
        TTF_SizeText(gizmosFont,"Cancel",&w,&h);
        RenderText("Cancel", lightWhite, cancelButtonMin.x + ((cancelButtonMax.x-cancelButtonMin.x)-w)/2, cancelButtonMin.y+ ((cancelButtonMax.y-cancelButtonMin.y)-h)/2, gizmosFont);
        //Open button
        Vector3 openButtonMin = {Screen.windowWidth/2 +207 - (cancelButtonMax.x-cancelButtonMin.x) - 10,Screen.windowHeight/2 -197};
        Vector3 openButtonMax = {Screen.windowWidth/2 +297 - (cancelButtonMax.x-cancelButtonMin.x) - 10,Screen.windowHeight/2 -152};
        if(!strcmp(fileName,"")){
            DrawRectangle(openButtonMin,openButtonMax,0.1,0.1,0.1);
        }else{
            if(MouseOverBox(mousePos,openButtonMin,openButtonMax,0)){
                DrawRectangle(openButtonMin,openButtonMax,0.3,0.3,0.45);
                if(GetMouseButtonUp(SDL_BUTTON_LEFT)){
                    onOpenFunction();
                    CloseFileBrowser();
                }
            }else{
                DrawRectangle(openButtonMin,openButtonMax,0.2,0.2,0.35);
            }
        }
        TTF_SizeText(gizmosFont,"Open",&w,&h);
        RenderText("Open", lightWhite, openButtonMin.x + ((openButtonMax.x-openButtonMin.x)-w)/2, openButtonMin.y+ ((openButtonMax.y-openButtonMin.y)-h)/2, gizmosFont);
        //File name
        Vector3 filenameBgMin = {Screen.windowWidth/2 -295,Screen.windowHeight/2 -195};
        Vector3 filenameBgMax = {openButtonMin.x -10,Screen.windowHeight/2 -164};
        DrawRectangle(filenameBgMin,filenameBgMax,0.2, 0.2, 0.2);
        RenderText("file name", lightWhite, filenameBgMin.x, filenameBgMax.y +1, gizmosFontSmall);
        TTF_SizeText(gizmosFont,fileName,&w,&h);
        RenderText(fileName, lightWhite, filenameBgMin.x + 5, filenameBgMin.y+ ((filenameBgMax.y-filenameBgMin.y)-h)/2 +1, gizmosFont);

        //Browser Items
        if(fileBrowserOpened == 1){
            int i=0,startx = fbHeaderMin.x + iconsSize * 3 + 12,starty = fbHeaderMin.y - iconsSize*3 -28;
            int x = startx, y = starty;
            ListCellPointer cell;
            //Folders
            ListForEach(cell,Folders){
                tinydir_file file = GetElementAsType(cell,tinydir_file);
                if(!strcmp(file.name,".") || !strcmp(file.name,"..")) continue;

                if((i - 7*fbItemsScroll)<0 || (i - 7*fbItemsScroll)>=(7 * 3)){
                    i++;
                    continue;
                }

                //Only jump if not the first line (end of the line, not the first item to render)
                if(i%7==0 && (i - 7*fbItemsScroll)!=0){
                    x = startx;
                    y-= iconsSize * 6 + 40;
                }
                //Folder icon/button
                if(PointButton((Vector3){x,y,0},10,3, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1}, (Vector3){0.5,0.5,0.5}) == 1){
                    char *path = calloc(_TINYDIR_PATH_MAX,sizeof(char));
                    strncpy(path,file.path,_TINYDIR_PATH_MAX);

                    OpenFileBrowser(path,onOpenFunction);
                    memcpy(filePath,path,_TINYDIR_PATH_MAX*sizeof(char));

                    //If there is any folder as next folder, remove from the list before adding the new folder
                    while(indexPath+1 < GetLength(Paths)){
                        RemoveListEnd(&Paths);
                    }

                    InsertListEnd(&Paths,&path);
                    indexPath++;
                }
                if(strlen(file.name)>6){
                    char minifiedName[] = "00000...";
                    memcpy(minifiedName,file.name,5*sizeof(char));
                    TTF_SizeText(gizmosFont,minifiedName,&w,&h);
                    RenderText(minifiedName, lightWhite, x-(iconsSize * 3) +((iconsSize * 6) - w)/2, y - (iconsSize * 3) - h, gizmosFont);
                }else{
                    TTF_SizeText(gizmosFont,file.name,&w,&h);
                    RenderText(file.name, lightWhite, x-(iconsSize * 3) +((iconsSize * 6) - w)/2, y - (iconsSize * 3) - h, gizmosFont);
                }
                x += iconsSize * 6 + 30;
                i++;
            }

            //Files
            int specificExtension = fileExtension[0] == '\0'? 0:(!strcmp(fileExtension,"vox")? 1:2);
            ListForEach(cell,Files){
                tinydir_file file = GetElementAsType(cell,tinydir_file);

                if(specificExtension){
                    int isEqual = 1;
                    int chr;
                    for(chr=0;chr<_TINYDIR_FILENAME_MAX && fileExtension[chr] != '\0';chr++){
                        if(tolower(fileExtension[chr]) != tolower(file.extension[chr])){isEqual = 0; break;}
                    }
                    if(!isEqual) continue;
                }

                if((i - 7*fbItemsScroll)<0 || (i - 7*fbItemsScroll)>=(7 * 3)){
                    i++;
                    continue;
                }
                

                //Only jump if not the first line (end of the line, not the first item to render)
                if(i%7==0 && (i - 7*fbItemsScroll)!=0){
                    x = startx;
                    y-= iconsSize * 6 + 40;
                }
                int icon = specificExtension==0? 11:(specificExtension == 1? 17:11);
                if(PointButton((Vector3){x,y,0},icon,3, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1}, (Vector3){0.5,0.5,0.5}) == 1){
                    memcpy(fileName,file.name,_TINYDIR_FILENAME_MAX*sizeof(char));
                }
                if(strlen(file.name)>6){
                    char minifiedName[] = "00000...";
                    memcpy(minifiedName,file.name,5*sizeof(char));
                    TTF_SizeText(gizmosFont,minifiedName,&w,&h);
                    RenderText(minifiedName, lightWhite, x-(iconsSize * 3) +((iconsSize * 6) - w)/2, y - (iconsSize * 3) - h, gizmosFont);
                }else{
                    TTF_SizeText(gizmosFont,file.name,&w,&h);
                    RenderText(file.name, lightWhite, x-(iconsSize * 3) +((iconsSize * 6) - w)/2, y - (iconsSize * 3) - h, gizmosFont);
                }
                x += iconsSize * 6 + 30;
                i++;
            }

            //Scrollbar
            //'i' is now the number of items
            if( i/(7*3) > 0){
                Vector3 scrollbarDownMin = {fbMin.x+1,fbFootMax.y+2};
                Vector3 scrollbarDownMax = {fbMax.x-1,fbFootMax.y+22};
                Vector3 scrollbarUpMin = {fbMin.x+1,fbHeaderMin.y-22};
                Vector3 scrollbarUpMax = {fbMax.x-1,fbHeaderMin.y-2};

                if(fbItemsScroll<(i/7)-2){
                    if(MouseOverBox(mousePos,scrollbarDownMin,scrollbarDownMax,0)){
                        DrawRectangle(scrollbarDownMin,scrollbarDownMax,0.1,0.1,0.125);
                        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                            fbItemsScroll++;
                        }
                    }else{
                        DrawRectangle(scrollbarDownMin,scrollbarDownMax,0.05,0.05,0.075);
                    }
                    DrawPointIcon((Vector3){scrollbarDownMin.x + (scrollbarDownMax.x - scrollbarDownMin.x)/2 - 1,
                                            scrollbarDownMin.y + (scrollbarDownMax.y - scrollbarDownMin.y)/2},13, 1, (Vector3){0.2,0.2,0.2});

                    //Mouse scroll
                    if(Input.mouseWheelY<0){
                        fbItemsScroll++;
                    }
                }
                if(fbItemsScroll>0){
                    if(MouseOverBox(mousePos,scrollbarUpMin,scrollbarUpMax,0)){
                        DrawRectangle(scrollbarUpMin,scrollbarUpMax,0.1,0.1,0.125);
                        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                            fbItemsScroll--;
                        }
                    }else{
                        DrawRectangle(scrollbarUpMin,scrollbarUpMax,0.05,0.05,0.075);
                    }
                    DrawPointIcon((Vector3){scrollbarUpMin.x + (scrollbarUpMax.x - scrollbarUpMin.x)/2 - 1,
                                            scrollbarUpMin.y + (scrollbarUpMax.y - scrollbarUpMin.y)/2},12, 1, (Vector3){0.2,0.2,0.2});
                    //Mouse scroll
                    if(Input.mouseWheelY>0){
                        fbItemsScroll--;
                    }
                }    
                
            }else{
                fbItemsScroll = 0;
            }
        }else if (fileBrowserOpened == -1){
            //Invalid folder message
            TTF_SizeText(gizmosFont,"Invalid or nonexistent path!",&w,&h);
            RenderText("Invalid or nonexistent path!", lightWhite, fbMin.x+ ((fbMax.x-fbMin.x)-w)/2, fbMin.y+ ((fbMax.y-fbMin.y)-h)/2, gizmosFont);
        }
    }
    //-------------------------- Shortcuts --------------------------

    //Delete shortcut
    if(GetKeyDown(SDL_SCANCODE_DELETE) && editingField<0  && !fileBrowserOpened){
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

    if(GetMouseButtonDown(SDL_BUTTON_MIDDLE)){
        fileBrowserOpened? CloseFileBrowser():OpenFileBrowser(NULL,NULL);
        FileBrowserExtension(NULL);
    }

    if(GetKeyDown(SDL_SCANCODE_P)){
        ListCellPointer cellp = GetFirstCell(SelectedEntities);
        while(cellp != GetLastCell(SelectedEntities)){
            printf("Parenting");
            SetEntityParent(GetElementAsType(cellp,EntityID),GetElementAsType(GetLastCell(SelectedEntities),EntityID));
            printf("Parented");
            cellp = GetNextCell(cellp);
        }
    }

    //Return depth to default values
    glDepthRange(0, 1.0);

}

//Runs at engine finish
void EditorFree(){
    FreeList(&SelectedEntities);
    free(existingEntities);

    if(fileBrowserOpened) CloseFileBrowser();
    
    int c=0,i;
    //Free entities backup
    for(i=0;i<ECS.maxEntities;i++){
        FreeList(&entitiesPlaymodeCopy[i].childs);
    }

    //Free components backup
    ListCellPointer comp;
    ListForEach(comp,ECS.ComponentTypes){
        for(i=0;i<ECS.maxEntities;i++){
            if(componentsPlaymodeCopy[c][i].data){
                GetElementAsType(comp,ComponentType).destructor(&componentsPlaymodeCopy[c][i].data);
            }
        }
        free(componentsPlaymodeCopy[c]);
        c++;
    }
    free(componentsPlaymodeCopy);

    TTF_CloseFont(gizmosFont);
}

int MouseOverLineGizmos(Vector3 mousePos, Vector3 originPos, Vector3 handlePos,int mouseOverDistance){
    return DistanceFromPointToLine2D(handlePos,originPos,mousePos)<mouseOverDistance && Distance(mousePos,handlePos)<Distance(handlePos, originPos) && Distance(mousePos,originPos)<Distance(handlePos, originPos);
}

int MouseOverPointGizmos(Vector3 mousePos, Vector3 originPos,int mouseOverDistance){
    return Distance(mousePos,originPos)<mouseOverDistance;
}

int MouseOverBox(Vector3 mousePos, Vector3 min, Vector3 max,int mouseOverDistance){
    return (mousePos.x>min.x-mouseOverDistance && mousePos.x<max.x+mouseOverDistance) && 
           (mousePos.y>min.y-mouseOverDistance && mousePos.y<max.y+mouseOverDistance);
}

Vector3 WorldVectorToScreenVector(Vector3 v){
    Vector3 screenPos;
	screenPos.x = (int)(v.x - v.y)*2 + 0.375;
    screenPos.y = (int)(v.x + v.y) + (v.z)*2 + 0.375;
    screenPos.z = 0;
    return screenPos;
}

int IsSelected(EntityID entity){
    ListCellPointer current = GetFirstCell(SelectedEntities);
    while(current){
        if(*((EntityID*)GetElement(*current)) == entity){
            return 1;
        }
        current = GetNextCell(current);
    }
    return 0;
}

void RemoveFromSelected(EntityID entity){
    int index = 0;
    ListCellPointer current = GetFirstCell(SelectedEntities);
    while(current){
        if(*((EntityID*)GetElement(*current)) == entity){
            RemoveListIndex(&SelectedEntities, index);
            return;
        }
        index++;
        current = GetNextCell(current);
    }
    return;
}

//Return 0 if removed, 1 if not
int DrawComponentHeader(ComponentID component, int* curHeight){
    glBegin(GL_QUADS);
        //Top panel
        glColor3f(0.2,0.2,0.35);
        glVertex2f( Screen.windowWidth-componentWindowLength,  *curHeight);
        glVertex2f( Screen.windowWidth-componentWindowWidthSpacing,  *curHeight);
        glVertex2f( Screen.windowWidth-componentWindowWidthSpacing, *curHeight-TTF_FontHeight(gizmosFont)-2);
        glVertex2f( Screen.windowWidth-componentWindowLength, *curHeight-TTF_FontHeight(gizmosFont)-2);
    glEnd();


    //Remove component button
    Vector3 removePos = {Screen.windowWidth-componentWindowWidthSpacing - (TTF_FontHeight(gizmosFont)-2)/2 -2,*curHeight - (TTF_FontHeight(gizmosFont))/2};
    if(PointButton(removePos,2, 1, (Vector3){0.9,0.9,0.9}, (Vector3){1,0.2,0.2}, (Vector3){1,0.5,0.5}) == 1){
        //Remove all components of this type from the selected entities
        ListCellPointer comp;
        ListForEach(comp,SelectedEntities){
            EntityID e = *((EntityID*)GetElement(*comp));
            RemoveComponentFromEntity(component,e);
        }
        *curHeight -= TTF_FontHeight(gizmosFont);
        return 0;
    }

    //Component Name
    ComponentType *type = ((ComponentType*)GetElementAt(ECS.ComponentTypes,component));
    //Cut names that are too big to be shown
    if(strlen(type->name)>11){
        char cName[14] = "OOOOOOOOOO...";
        strncpy(cName,type->name,10);
        RenderText(cName, brightWhite, Screen.windowWidth-componentWindowLength+componentNameLeftSpacing, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);
    }else{
        RenderText(type->name, brightWhite, Screen.windowWidth-componentWindowLength+componentNameLeftSpacing, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);
    }

    *curHeight -= TTF_FontHeight(gizmosFont)+2;
    return 1;
}

//Returns 1 if pressed, 2 if mouse is over and 0 if neither case
int PointButton(Vector3 pos,int iconID, int scale, Vector3 defaultColor, Vector3 mouseOverColor, Vector3 pressedColor){
    int state = 0;

    glPointSize(scale * iconsSize * 2/Screen.gameScale);
    glEnable(GL_TEXTURE_2D);
    glAlphaFunc (GL_NOTEQUAL, 0.0f);
    glEnable(GL_ALPHA_TEST);
    glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, iconsTex[iconID]);
    glEnable(GL_POINT_SPRITE);

    glBegin(GL_POINTS);
        if(MouseOverPointGizmos(mousePos, pos, scale * iconsSize)){
            glColor3f(mouseOverColor.x,mouseOverColor.y,mouseOverColor.z);

            //Pressed
            if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                glColor3f(pressedColor.x,pressedColor.y,pressedColor.z);
                state = 1;
            }else{
                //Mouse over only
                glColor3f(mouseOverColor.x,mouseOverColor.y,mouseOverColor.z);
                state = 2;
            }
        }else{
            glColor3f(defaultColor.x,defaultColor.y,defaultColor.z);
            state = 0;
        }

        glVertex2f( roundf(pos.x), roundf(pos.y));
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    return state;
}

//Returns 1 if pressed, 2 if mouse is over and 0 if neither case
int PointToggle(int *data,Vector3 pos,int onIconID, int offIconID, int undefinedIconID, int scale, Vector3 onColor, Vector3 offColor, Vector3 undefinedColor, Vector3 mouseOverColor){
    int state = 0;

    glPointSize(scale * iconsSize * 2/Screen.gameScale);
    glEnable(GL_TEXTURE_2D);
    glAlphaFunc (GL_NOTEQUAL, 0.0f);
    glEnable(GL_ALPHA_TEST);
    glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    if(*data == 1){
        glColor3f(onColor.x,onColor.y,onColor.z);
        glBindTexture(GL_TEXTURE_2D, iconsTex[onIconID]);
    }else if(*data == -1){
        glColor3f(offColor.x,offColor.y,offColor.z);
        glBindTexture(GL_TEXTURE_2D, iconsTex[undefinedIconID]);
    }else{
        glColor3f(offColor.x,offColor.y,offColor.z);
        glBindTexture(GL_TEXTURE_2D, iconsTex[offIconID]);
    }
    glEnable(GL_POINT_SPRITE);

    glBegin(GL_POINTS);
        if(MouseOverPointGizmos(mousePos, pos, scale * iconsSize)){
            glColor3f(mouseOverColor.x,mouseOverColor.y,mouseOverColor.z);

            //Pressed
            if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                state = 1;
                *data = *data<0? 1:!(*data);
            }else{
                //Mouse over only
                state = 2;
            }
        }else{
            //Mouse not over
            state = 0;
        }

        glVertex2f( roundf(pos.x), roundf(pos.y));
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    return state;
}

void DrawPointIcon(Vector3 pos,int iconID, int scale, Vector3 color){
    glPointSize(scale * iconsSize * 2/Screen.gameScale);
    glEnable(GL_TEXTURE_2D);
    glAlphaFunc (GL_NOTEQUAL, 0.0f);
    glEnable(GL_ALPHA_TEST);
    glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, iconsTex[iconID]);
    glEnable(GL_POINT_SPRITE);

    glBegin(GL_POINTS);
        glColor3f(color.x,color.y,color.z);
        glVertex2f( roundf(pos.x), roundf(pos.y));
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

void Vector3Field(char *title, Vector3 *data,int ommitX,int ommitY,int ommitZ,int x, int w, int fieldsSpacing, int* curField, int* curHeight){
    *curHeight -= 2;
    RenderText(title, lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
    *curHeight -= 2 + TTF_FontHeight(gizmosFontSmall);

    int fieldW = w/3;

    //1 is X field, 2 is Y field and 3 is Z field
    Vector3 min1 = { x,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max1 = { x+fieldW - fieldsSpacing,*curHeight,0};
    Vector3 min2 = { x+fieldW + fieldsSpacing,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max2 = { x+fieldW*2 - fieldsSpacing,*curHeight,0};
    Vector3 min3 = { x+fieldW*2 + fieldsSpacing,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max3 = { x+fieldW*3 - fieldsSpacing,*curHeight,0};

    if(editingField == *curField || editingField == (*curField)+1 || editingField == (*curField)+2)
    {
        //String edit field background
        DrawRectangle(min1,max3,0.3, 0.3, 0.3);

        //Get the cursor position by creating a string containing the characters until the cursor
        //and getting his size when rendered with the used font
        char buff[13];
        strncpy(buff,textFieldString,Input.textInputCursorPos);
        memset(buff+Input.textInputCursorPos,'\0',1);
        int cursorPos,h;
        TTF_SizeText(gizmosFont,buff,&cursorPos,&h);
        cursorPos+=min1.x;

        //Cursor line
        glColor3f(0.7,0.7,0.7);
        glLineWidth(2/Screen.gameScale);
        glBegin(GL_LINES);
            glVertex2f( cursorPos, min1.y);
            glVertex2f( cursorPos, max1.y);
        glEnd();

        //Render the string
        RenderText(textFieldString, lightWhite, min1.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

        //Pass the string as float data
        if(editingField == *curField){
            data->x = strtof(textFieldString, NULL);
        }else if(editingField == (*curField)+1){
            data->y = strtof(textFieldString, NULL);
        }else{
            data->z = strtof(textFieldString, NULL);
        }

    }else{
        //Not editing any of the three fields, just draw the three boxes and floats normally
        static char valueString[5] = "  0.0";

        //Fields selection
        if(editingField<0 && GetMouseButtonDown(SDL_BUTTON_LEFT)){
            if(MouseOverBox(mousePos,min1,max1,0)){
                textFieldString = (char*)calloc(13,sizeof(char));

                if(!ommitX) snprintf(textFieldString, 12, "%6.6f", data->x);
                else snprintf(textFieldString, 4, "0.0");
                
                GetTextInput(textFieldString, 12, strlen(textFieldString));
                editingField = *curField;
            }

            if(MouseOverBox(mousePos,min2,max2,0)){
                textFieldString = (char*)calloc(13,sizeof(char));
                if(!ommitY) snprintf(textFieldString, 12, "%6.6f", data->y);
                else snprintf(textFieldString, 4, "0.0");

                GetTextInput(textFieldString, 12, strlen(textFieldString));
                editingField = (*curField)+1;
            }

            if(MouseOverBox(mousePos,min3,max3,0)){
                textFieldString = (char*)calloc(13,sizeof(char));
                if(!ommitX) snprintf(textFieldString, 12, "%6.6f", data->z);
                else snprintf(textFieldString, 4, "0.0");

                GetTextInput(textFieldString, 12, strlen(textFieldString));
                editingField = (*curField)+2;
            }
        }

        //Check if no field was selected before rendering the individual fields
        if(editingField != *curField && editingField != (*curField)+1 && editingField != (*curField)+2){

            //X field
            DrawRectangle(min1,max1,0.2, 0.2, 0.2);
            if(!ommitX) snprintf(valueString,5,"%3.1f",data->x);
            else snprintf(valueString,5,"---");
            RenderText(valueString, lightWhite, min1.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

            //Y field
            DrawRectangle(min2,max2,0.2, 0.2, 0.2);
            if(!ommitY) snprintf(valueString,5,"%3.1f",data->y);
            else snprintf(valueString,5,"---");
            RenderText(valueString, lightWhite, min2.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

            //Z field
            DrawRectangle(min3,max3,0.2, 0.2, 0.2);
            if(!ommitZ) snprintf(valueString,5,"%3.1f",data->z);
            else snprintf(valueString,5,"---");
            RenderText(valueString, lightWhite, min3.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);
        }
    }

    //Mark as used ID fields
    *curField +=3;

    *curHeight -= 2 + TTF_FontHeight(gizmosFont);
}

void FloatField(char *title, float *data,int ommit,int x, int w, int* curField, int* curHeight){
    *curHeight -= 4;
    RenderText(title, lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
    *curHeight -= 2 + TTF_FontHeight(gizmosFontSmall);

    Vector3 min = { x,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max = { x+w,*curHeight,0};

    if(editingField == *curField)
    {
        //Field background
        DrawRectangle(min,max,0.3, 0.3, 0.3);

        //Get the cursor position by creating a string containing the characters until the cursor
        //and getting his size when rendered with the used font
        char buff[13];
        strncpy(buff,textFieldString,Input.textInputCursorPos);
        memset(buff+Input.textInputCursorPos,'\0',1);
        int cursorPos,h;
        TTF_SizeText(gizmosFont,buff,&cursorPos,&h);
        cursorPos+=min.x;

        //Cursor line
        glColor3f(0.7,0.7,0.7);
        glLineWidth(2/Screen.gameScale);
        glBegin(GL_LINES);
            glVertex2f( cursorPos, min.y);
            glVertex2f( cursorPos, max.y);
        glEnd();

        //Render the string
        RenderText(textFieldString, lightWhite, min.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

        //Pass the string as float data
        *data = strtof(textFieldString, NULL);

    }else{
        //Not editing, just draw the box and float normally
        static char valueString[12] = "  0.0";

        //Field background
        DrawRectangle(min,max,0.2, 0.2, 0.2);
        //Data text
        if(!ommit) snprintf(valueString,12,"%6.6f",*data);
        else snprintf(valueString,4,"---");
        RenderText(valueString, lightWhite, min.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

        //Fields selection
        if(editingField<0 && GetMouseButtonDown(SDL_BUTTON_LEFT)){
            if(MouseOverBox(mousePos,min,max,0)){
                textFieldString = (char*)calloc(13,sizeof(char));

                if(!ommit) snprintf(textFieldString, 12, "%6.6f", *data);
                else snprintf(textFieldString, 4, "0.0");
                
                GetTextInput(textFieldString, 12, strlen(textFieldString));
                editingField = *curField;
            }
        }
    }

    //Mark as used ID field
    *curField +=1;

    *curHeight -= 2 + TTF_FontHeight(gizmosFont);
}

void IntField(char *title, int *data,int ommit,int x, int w, int* curField, int* curHeight){
    *curHeight -= 4;
    RenderText(title, lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
    *curHeight -= 2 + TTF_FontHeight(gizmosFontSmall);

    Vector3 min = { x,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max = { x+w,*curHeight,0};

    if(editingField == *curField)
    {
        //Field background
        DrawRectangle(min,max,0.3, 0.3, 0.3);

        //Get the cursor position by creating a string containing the characters until the cursor
        //and getting his size when rendered with the used font
        char buff[13];
        strncpy(buff,textFieldString,Input.textInputCursorPos);
        memset(buff+Input.textInputCursorPos,'\0',1);
        int cursorPos,h;
        TTF_SizeText(gizmosFont,buff,&cursorPos,&h);
        cursorPos+=min.x;

        //Cursor line
        glColor3f(0.7,0.7,0.7);
        glLineWidth(2/Screen.gameScale);
        glBegin(GL_LINES);
            glVertex2f( cursorPos, min.y);
            glVertex2f( cursorPos, max.y);
        glEnd();

        //Render the string
        RenderText(textFieldString, lightWhite, min.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

        //Pass the string as float data
        *data = (int) strtol(textFieldString, NULL,0);

    }else{
        //Not editing, just draw the box and float normally
        static char valueString[12] = "  0.0";

        //Field background
        DrawRectangle(min,max,0.2, 0.2, 0.2);
        //Data text
        if(!ommit) snprintf(valueString,12,"%d",*data);
        else snprintf(valueString,4,"---");
        RenderText(valueString, lightWhite, min.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

        //Fields selection
        if(editingField<0 && GetMouseButtonDown(SDL_BUTTON_LEFT)){
            if(MouseOverBox(mousePos,min,max,0)){
                textFieldString = (char*)calloc(13,sizeof(char));

                if(!ommit) snprintf(textFieldString, 12, "%d", *data);
                else textFieldString[0] = '\0';
                
                GetTextInput(textFieldString, 12, strlen(textFieldString));
                editingField = *curField;
            }
        }
    }

    //Mark as used ID field
    *curField +=1;

    *curHeight -= 2 + TTF_FontHeight(gizmosFont);
}

//WIP
void IntListField(char *title, List *list,int x, int w, int* curField, int* curHeight){
    *curHeight -= 4;
    int titleLen = strlen(title);
    titleLen += 8;
    char *fieldTitle = calloc(titleLen,sizeof(char));
    snprintf(fieldTitle,titleLen,"%s [%d]",title,GetLength(*list));

    RenderText(fieldTitle, lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
    free(fieldTitle);

    *curHeight -= 2 + TTF_FontHeight(gizmosFontSmall);
    
    Vector3 min = { x,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max = { x+w,*curHeight,0};

    ListCellPointer cell;
    ListForEach(cell,*list){
        if(editingField == *curField)
        {
            //Field background
            DrawRectangle(min,max,0.3, 0.3, 0.3);

            //Get the cursor position by creating a string containing the characters until the cursor
            //and getting his size when rendered with the used font
            /*char buff[13];
            strncpy(buff,textFieldString,Input.textInputCursorPos);
            memset(buff+Input.textInputCursorPos,'\0',1);
            int cursorPos,h;
            TTF_SizeText(gizmosFont,buff,&cursorPos,&h);
            cursorPos+=min.x;

            //Cursor line
            glColor3f(0.7,0.7,0.7);
            glLineWidth(2/Screen.gameScale);
            glBegin(GL_LINES);
                glVertex2f( cursorPos, min.y);
                glVertex2f( cursorPos, max.y);
            glEnd();

            //Render the string
            RenderText(textFieldString, lightWhite, min.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

            //Pass the string as float data
            *data = (int) strtol(textFieldString, NULL,0);*/

        }else{
            //Not editing, just draw the box and float normally
            //static char valueString[12] = "  0.0";

            //Field background
            DrawRectangle(min,max,0.2, 0.2, 0.2);
            /*
            //Data text
            if(!ommit) snprintf(valueString,12,"%d",*data);
            else snprintf(valueString,4,"---");
            RenderText(valueString, lightWhite, min.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

            //Fields selection
            if(editingField<0 && GetMouseButtonDown(SDL_BUTTON_LEFT)){
                if(MouseOverBox(mousePos,min,max,0)){
                    textFieldString = (char*)calloc(13,sizeof(char));

                    if(!ommit) snprintf(textFieldString, 12, "%d", *data);
                    else textFieldString[0] = '\0';
                    
                    GetTextInput(textFieldString, 12, strlen(textFieldString));
                    editingField = *curField;
                }
            }*/
        }

        //Mark as used ID field
        *curField +=1;

        min.y -= 5 + TTF_FontHeight(gizmosFont);
        max.y -= 5 + TTF_FontHeight(gizmosFont);
        *curHeight -= 5 + TTF_FontHeight(gizmosFont);
    }
}

void DrawRectangle(Vector3 min, Vector3 max, float r, float g, float b){
    glColor3f(r,g,b);
    glBegin(GL_QUADS);
        glVertex2f( min.x, max.y);
        glVertex2f( max.x, max.y);
        glVertex2f( max.x, min.y);
        glVertex2f( min.x, min.y);
    glEnd();
}

void LoadUITexture(char *path,int index){
    SDL_Surface *img = IMG_Load(path);
    if(!img){ printf("Failed to load UI Icon! (%s)\n",path); return; }
    iconsSize = img->w;

    glBindTexture(GL_TEXTURE_2D, iconsTex[index]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    SDL_FreeSurface(img);
}

void EnterPlayMode(){
    int c=0,i;
    for(i=0;i<=ECS.maxUsedIndex;i++){
        //Copy entities data
        entitiesPlaymodeCopy[i] =  ECS.Entities[i];
        entitiesPlaymodeCopy[i].childs = InitList(sizeof(EntityID));

        ListCellPointer chld;
        ListForEach(chld,ECS.Entities[i].childs){
            EntityID chldID = GetElementAsType(chld,EntityID);
            InsertListEnd(&entitiesPlaymodeCopy[i].childs,&chldID);
        }
        
        c=0;
        //Copy component data
        ListCellPointer comp;
        ListForEach(comp,ECS.ComponentTypes){
            if(ECS.Components[c][i].data){
                componentsPlaymodeCopy[c][i].data = GetElementAsType(comp,ComponentType).copy(ECS.Components[c][i].data);
            }
            c++;
        }
    }
    
}

void ExitPlayMode(){
    int c,i;
    for(i=0;i<ECS.maxEntities;i++){
        //Destroy game modified entity before recreate
        if(IsValidEntity(i)){
            DestroyEntity(i);
        }

        ECS.Entities[i] = entitiesPlaymodeCopy[i];
        ECS.Entities[i].childs = InitList(sizeof(EntityID));
        
        ListCellPointer chld;
        ListForEach(chld,entitiesPlaymodeCopy[i].childs){
            EntityID chldID = GetElementAsType(chld,EntityID);
            
            InsertListEnd(&ECS.Entities[i].childs,&chldID);
        }
        //Destroy backup
        FreeList(&entitiesPlaymodeCopy[i].childs);

        c=0;
        ListCellPointer comp;
        ListForEach(comp,ECS.ComponentTypes){
            if(componentsPlaymodeCopy[c][i].data){
                //Copy the backup data
                ECS.Components[c][i].data = GetElementAsType(comp,ComponentType).copy(componentsPlaymodeCopy[c][i].data);
                ECS.Entities[i].mask.mask |= (1<<c);

                ListCellPointer ent;
                int indx = 0;
                ListForEach(ent,ECS.AvaliableEntitiesIndexes){
                    if(GetElementAsType(ent,EntityID) == i){
                        RemoveListIndex(&ECS.AvaliableEntitiesIndexes,indx);
                        break;
                    }
                    indx++;
                }

                //Destroy backup
                GetElementAsType(comp,ComponentType).destructor(&componentsPlaymodeCopy[c][i].data);
            }
            c++;
        }
        
    }
}

void OpenFileBrowser(char *initialPath,void (*onOpen)()){
    //Free the folder and files lists before opening another path
    if(fileBrowserOpened){
        FreeList(&Folders);
        FreeList(&Files);
    }
    onOpenFunction = *onOpen;
    Folders = InitList(sizeof(tinydir_file));
    Files = InitList(sizeof(tinydir_file));
    if(initialPath){
        memcpy(filePath,initialPath,_TINYDIR_PATH_MAX*sizeof(char));
    }else{
        char DefaultPath[] = "Assets";
        memcpy(filePath,DefaultPath,sizeof(DefaultPath));

        if(fileBrowserOpened){
            //Free the paths list when returning to default path
            ListCellPointer cell;
            ListForEach(cell,Paths){
                free(GetElement(*cell));
            }
            FreeList(&Paths);
        }
    }
    //Insert the initial path to the list when opening/returning to default path
    if(!fileBrowserOpened || !initialPath){
        Paths = InitList(sizeof(char*));
        char *firstPath = malloc(_TINYDIR_PATH_MAX*sizeof(char));
        memcpy(firstPath,filePath,_TINYDIR_PATH_MAX*sizeof(char));
        InsertListEnd(&Paths,&firstPath);
        indexPath = 0;
    }
    memset(fileName,'\0',_TINYDIR_FILENAME_MAX*sizeof(char));

    tinydir_dir dir;
    if(tinydir_open(&dir, filePath) < 0){
        //Set as invalid folder path
        fileBrowserOpened = -1;
        return;
    }
    while(dir.has_next){
        tinydir_file file;
        tinydir_readfile(&dir, &file);
        if (file.is_dir)
        {
            InsertListEnd(&Folders,&file);
        }else{
            
            InsertListEnd(&Files,&file);
            char ** extStr = &((tinydir_file*)GetLastElement(Files))->extension;
            *extStr = malloc(strlen(file.extension) * sizeof(char));
            strcpy(*extStr,file.extension);
        }

        tinydir_next(&dir);
    }
    tinydir_close(&dir);

    fileBrowserOpened = 1;
}

void FileBrowserExtension(char *ext){
    if(ext){
        strncpy(fileExtension,ext,_TINYDIR_FILENAME_MAX);
    }else{
        strncpy(fileExtension,"",_TINYDIR_FILENAME_MAX);
    }
}

void CloseFileBrowser(){
    fileBrowserOpened = 0;
    FreeList(&Folders);
    FreeList(&Files);

    ListCellPointer cell;
    ListForEach(cell,Paths){
        free(*((char**)GetElement(*cell)));
    }
    FreeList(&Paths);
}

void LoadModel(){
    char pathString[_TINYDIR_PATH_MAX];
    strcpy(pathString,filePath);
    strcat(pathString,fileName);
    printf("(%s)(%s)\n",filePath,fileName);
    LoadVoxelModel(GetElementAsType(GetFirstCell(SelectedEntities),EntityID),filePath,fileName);
}