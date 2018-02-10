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

static float componentWindowLength = 200;
static float componentWindowWidthSpacing = 14;
static float componentNameLeftSpacing = 5;
static float componentBetweenSpacing = 5;

static float entityWindowLength = 100;
static float entityWindowWidthSpacing = 14;
static float entityWindowTopHeightSpacing = 60;
static float entityNameLeftSpacing = 18;
static float entityBetweenSpacing = 2;

static int scrollbarMouseWheelSpeed = 25;

SDL_Color fontColor = {255,255,255};
TTF_Font* gizmosFont;
TTF_Font* gizmosFontSmall;

GLuint iconsTex[6];
int iconsSize = 9;

char *textFieldString = NULL;

//Input data
static Vector3 mousePos = {0,0,0};
static Vector3 deltaMousePos = {0,0,0};

//List of selected EntityIDs
List SelectedEntities;
//Array of existing entities
int *existingEntities = NULL;

//Internal functions
int MouseOverLineGizmos(Vector3 mousePos, Vector3 originPos, Vector3 handlePos,int mouseOverDistance);
int MouseOverPointGizmos(Vector3 mousePos, Vector3 originPos,int mouseOverDistance);
int MouseOverBox(Vector3 mousePos, Vector3 min, Vector3 max,int mouseOverDistance);
Vector3 WorldVectorToScreenVector(Vector3 v);
int IsSelected(EntityID entity);
void RemoveFromSelected(EntityID entity);
int DrawComponentHeader(ComponentID component, int* curHeight);
void DrawRectangle(Vector3 min, Vector3 max, float r, float g, float b);
void Vector3Field(char *title, Vector3 *data,int ommitX,int ommitY,int ommitZ,int x, int w, int fieldsSpacing, int* curField, int* curHeight);
int PointButton(Vector3 pos,int iconID, int scale, Vector3 defaultColor, Vector3 mouseOverColor, Vector3 pressedColor);
void LoadUITexture(char *path,int index);

//Runs on engine start
void EditorInit(System *systemObject){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("Editor"));

    existingEntities = calloc(ECS.maxEntities,sizeof(int));
    SelectedEntities = InitList(sizeof(EntityID));

    VoxRenderSystem = GetSystemID("VoxelRenderer");
    EditorSystem = GetSystemID("Editor");

    //Disable all game systems, except the rendering
    int i;
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
    glGenTextures(5, iconsTex);
    LoadUITexture("Interface/IconsUI/add.png",0);
    LoadUITexture("Interface/IconsUI/remove.png",1);
    LoadUITexture("Interface/IconsUI/bin.png",2);
    LoadUITexture("Interface/IconsUI/play.png",3);
    LoadUITexture("Interface/IconsUI/pause.png",4);
    LoadUITexture("Interface/IconsUI/stop.png",5);
}

int movingX = 0;
int movingY = 0;
int movingZ = 0;


int entityStartHeight = 0;
int componentStartHeight = 0;
int movingScrollbar = 0;
int editingField = -1;

//0 = Editor, 1 = Play, 2 = Paused
int playMode = 0;

//Runs each GameLoop iteration
void EditorUpdate(){
    EntityID entity;

    //printf("%d\n",GetLength(SelectedEntities));

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
    Vector3 playBGMin = (Vector3){Screen.windowWidth/2 - iconsSize * 4 * 2/Screen.gameScale -2,  Screen.windowHeight-iconsSize * 4 * 2/Screen.gameScale -1 };
    Vector3 playBGMax = (Vector3){Screen.windowWidth/2 + iconsSize * 4 * 2/Screen.gameScale +2,  Screen.windowHeight};

    if(playMode == 1){
        //Play mode
        DrawRectangle(playBGMin,playBGMax,0.5,0.5,0.5);

        //Pause button
        if(1 == PointButton((Vector3){Screen.windowWidth/2 - iconsSize * 2 * 2/Screen.gameScale -2,  Screen.windowHeight-iconsSize * 2 * 2/Screen.gameScale -1 },4, 2, (Vector3){0.1,0.1,0.1}, (Vector3){1,1,1}, (Vector3){1,1,1})){
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
        if(1 == PointButton((Vector3){Screen.windowWidth/2 + iconsSize * 2 * 2/Screen.gameScale +2,  Screen.windowHeight-iconsSize * 2 * 2/Screen.gameScale -1 },5, 2, (Vector3){0.1,0.1,0.1}, (Vector3){1,1,1}, (Vector3){1,1,1})){
            playMode = 0;

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
        if(1 == PointButton((Vector3){Screen.windowWidth/2 - iconsSize * 2 * 2/Screen.gameScale -2,  Screen.windowHeight-iconsSize * 2 * 2/Screen.gameScale -1 },3, 2, (Vector3){0.1,0.1,0.1}, (Vector3){1,1,1}, (Vector3){1,1,1})){
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
        if(1 == PointButton((Vector3){Screen.windowWidth/2 + iconsSize * 2 * 2/Screen.gameScale +2,  Screen.windowHeight-iconsSize * 2 * 2/Screen.gameScale -1 },5, 2, (Vector3){0.1,0.1,0.1}, (Vector3){1,1,1}, (Vector3){1,1,1})){
            playMode = 0;

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
        if(1 == PointButton((Vector3){Screen.windowWidth/2 - iconsSize * 2 * 2/Screen.gameScale -2,  Screen.windowHeight-iconsSize * 2 * 2/Screen.gameScale -1 },3, 2, (Vector3){0.7,0.7,0.7}, (Vector3){1,1,1}, (Vector3){1,1,1})){
            playMode = 1;

            //Enable all game systems
            int i;
            for(i=0;i<GetLength(ECS.SystemList);i++){
                if(i != EditorSystem && i!= VoxRenderSystem){
                    EnableSystem(i);
                }
            }
        }
        //Stop button disabled
        PointButton((Vector3){Screen.windowWidth/2 + iconsSize * 2 * 2/Screen.gameScale +2,  Screen.windowHeight-iconsSize * 2 * 2/Screen.gameScale -1 },5, 2, (Vector3){0.2,0.2,0.2}, (Vector3){0.2,0.2,0.2}, (Vector3){0.2,0.2,0.2});
    }

    //-------------------------- Transform gizmos --------------------------
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
                //RenderText("Some random text", fontColor, screenPos.x-w/2, screenPos.y-h*1.5,gizmosDepth, gizmosFont);  
            }
        }
    }

    //-------------------------- Components panel --------------------------
    int currentComponentField = 0;
    if(!IsListEmpty(SelectedEntities)){

        //Set mask as the components of the first entity selected
        ComponentMask mask = GetEntityComponents(*((EntityID*)GetFirstElement(SelectedEntities)));

        ListCellPointer current;
        ListForEach(current,SelectedEntities){
            EntityID e = GetElementAsType(current, EntityID);
            mask = IntersectComponentMasks(mask,GetEntityComponents(e));
        }

        //Panel background
        glBegin(GL_QUADS);
            glColor3f(0.02,0.02,0.05);
            glVertex2f( Screen.windowWidth-componentWindowLength,  Screen.windowHeight);
            glVertex2f( Screen.windowWidth,  Screen.windowHeight);
            glVertex2f( Screen.windowWidth, 0);
            glVertex2f( Screen.windowWidth-componentWindowLength, 0);
        glEnd();

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
                    }
                }
                componentHeight -= componentBetweenSpacing;
            }
        }
        
        //Scrollbar
        int mouseOverComponentPanel = MouseOverBox(mousePos, (Vector3){Screen.windowWidth-componentWindowLength,0,0}, (Vector3){Screen.windowWidth,Screen.windowHeight,0},0);
        glLineWidth(clamp((componentWindowWidthSpacing/2 - 2) * 2/Screen.gameScale,1,Screen.windowWidth));
        int offscreenPixels = -clamp(componentHeight-componentStartHeight,-(Screen.windowHeight-10),0);
        glBegin(GL_LINES);  
            Vector3 scrollbarStart = {Screen.windowWidth - componentWindowWidthSpacing/2 ,Screen.windowHeight-componentStartHeight -2,0};
            Vector3 scrollbarEnd = {Screen.windowWidth - componentWindowWidthSpacing/2 ,offscreenPixels - componentStartHeight + 1,0};
            
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
    }

    //-------------------------- Entities panel --------------------------

    //Entities Panel background
    glBegin(GL_QUADS);
        glColor3f(0.02,0.02,0.05);
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
                glBegin(GL_QUADS);
                    //Entity element
                    Vector3 elMin = {entityWindowWidthSpacing,entityHeight-TTF_FontHeight(gizmosFont)-2,0};
                    Vector3 elMax = {entityWindowLength,entityHeight,0};
                    if(MouseOverBox(mousePos, elMin, elMax,0)){
                        if(isSelected){
                            glColor3f(0.3,0.3,0.3);
                            if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                                RemoveFromSelected(entity);
                            }
                            if(GetMouseButtonDown(SDL_BUTTON_RIGHT)){
                                EntityID newEntity = DuplicateEntity(entity);
                                FreeList(&SelectedEntities);
                                InsertListEnd(&SelectedEntities,&newEntity);
                            }
                        }else{
                            glColor3f(0.3,0.3,0.4);
                            if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
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
                            glColor3f(0.35,0.35,0.5);
                        }else{
                            glColor3f(0.2,0.2,0.35);
                        }
                    }
                    glVertex2f( elMax.x, elMax.y);
                    glVertex2f( elMin.x, elMax.y);
                    glVertex2f( elMin.x, elMin.y);
                    glVertex2f( elMax.x, elMin.y);
                glEnd();
                static char entityName[12];
                snprintf(entityName,11,"%d",entity);
                RenderText(entityName, fontColor, entityNameLeftSpacing, entityHeight - TTF_FontHeight(gizmosFont), gizmosFont);
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
    glColor3f(0.02,0.02,0.05);
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

    RenderText("+ Entity", fontColor, 5, Screen.windowHeight-(entityWindowTopHeightSpacing)+5, gizmosFont);

    //Delete shortcut
    if(GetKeyDown(SDL_SCANCODE_DELETE) && editingField<0){
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

    //Return depth to default values
    glDepthRange(0, 1.0);

}

//Runs at engine finish
void EditorFree(){
    FreeList(&SelectedEntities);
    free(existingEntities);
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
        RenderText(cName, fontColor, Screen.windowWidth-componentWindowLength+componentNameLeftSpacing, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);
    }else{
        RenderText(type->name, fontColor, Screen.windowWidth-componentWindowLength+componentNameLeftSpacing, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);
    }

    *curHeight -= TTF_FontHeight(gizmosFont);
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
        if(MouseOverPointGizmos(mousePos, pos, scale * iconsSize * 2/Screen.gameScale)){
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

void Vector3Field(char *title, Vector3 *data,int ommitX,int ommitY,int ommitZ,int x, int w, int fieldsSpacing, int* curField, int* curHeight){
    *curHeight -= 2;
    RenderText(title, fontColor, x, *curHeight - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
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
        RenderText(textFieldString, fontColor, min1.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

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
            RenderText(valueString, fontColor, min1.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

            //Y field
            DrawRectangle(min2,max2,0.2, 0.2, 0.2);
            if(!ommitY) snprintf(valueString,5,"%3.1f",data->y);
            else snprintf(valueString,5,"---");
            RenderText(valueString, fontColor, min2.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

            //Z field
            DrawRectangle(min3,max3,0.2, 0.2, 0.2);
            if(!ommitZ) snprintf(valueString,5,"%3.1f",data->z);
            else snprintf(valueString,5,"---");
            RenderText(valueString, fontColor, min3.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);
        }
    }

    //Mark as used ID fields
    *curField +=3;

    *curHeight -= 2 + TTF_FontHeight(gizmosFont);
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
    SDL_FreeSurface(img);
}