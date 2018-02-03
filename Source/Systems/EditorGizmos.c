#include "EditorGizmos.h"

static System *ThisSystem;

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

GLuint iconsTex[3];
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
void Vector3Field(char *title, Vector3 *data,int ommitX,int ommitY,int ommitZ,int x, int w, int fieldsSpacing, int* curField, int* curHeight);

//Runs on engine start
void EditorGizmosInit(System *systemObject){
    ThisSystem = systemObject;

    SelectedEntities = InitList(sizeof(EntityID));

    existingEntities = calloc(ECS.maxEntities,sizeof(int));

    gizmosFont = TTF_OpenFont("Interface/Fonts/gros/GROS.ttf",16);
	if(!gizmosFont){
		printf("Font: Error loading font!");
	}

    gizmosFontSmall= TTF_OpenFont("Interface/Fonts/coolthre/COOLTHRE.ttf",12);
    if(!gizmosFont){
		printf("Font: Error loading small font!");
	}

    //Load UI icons
    glGenTextures(3, iconsTex);
    //add
    SDL_Surface *img = IMG_Load("Interface/IconsUI/add.png");
    if(!img){ printf("Failed to load UI Icon! (add.png)\n"); return; }
    iconsSize = img->w;

    glBindTexture(GL_TEXTURE_2D, iconsTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    SDL_FreeSurface(img);
    //remove
    img = IMG_Load("Interface/IconsUI/remove.png");
    if(!img){ printf("Failed to load UI Icon! (remove.png)\n"); return; }
    glBindTexture(GL_TEXTURE_2D, iconsTex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    SDL_FreeSurface(img);
    //bin
    img = IMG_Load("Interface/IconsUI/bin.png");
    if(!img){ printf("Failed to load UI Icon! (bin.png)\n"); return; }
    glBindTexture(GL_TEXTURE_2D, iconsTex[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    SDL_FreeSurface(img);
}

int movingX = 0;
int movingY = 0;
int movingZ = 0;


int entityStartHeight = 0;
int componentStartHeight = 0;
int movingScrollbar = 0;
int editingField = -1;

//Runs each GameLoop iteration
void EditorGizmosUpdate(){
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

    glPointSize(5 * 2/Screen.gameScale);
    glLineWidth(2* 2/Screen.gameScale);
    glClear(GL_DEPTH_BUFFER_BIT);
    //Render UI objects in the [0,0.01] depth range
    glDepthRange(0, 0.01);

    glBindFramebuffer(GL_FRAMEBUFFER, Rendering.frameBuffer);
    glViewport(0,0,Screen.gameWidth,Screen.gameHeight);

    //-------------------------- Transform gizmos --------------------------
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

                        static Vector3 test = {3.1415,1,2};
                        
                        Vector3Field("position",&test,0,0,0,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1,4, &currentComponentField, &componentHeight);
                        Vector3Field("rotation",&test,0,0,0,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1,4, &currentComponentField, &componentHeight);
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
void EditorGizmosFree(){
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
    glPointSize(iconsSize * 2/Screen.gameScale);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, iconsTex[2]);
    glEnable(GL_POINT_SPRITE);

    glBegin(GL_POINTS);
        Vector3 removePos = {Screen.windowWidth-componentWindowWidthSpacing - (TTF_FontHeight(gizmosFont)-2)/2 -2,*curHeight - (TTF_FontHeight(gizmosFont))/2};
        if(MouseOverPointGizmos(mousePos, removePos, iconsSize * 2/Screen.gameScale)){
            glColor3f(1,0.2,0.2);

            //Pressed to remove
            if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                //Remove all components of this type from the selected entities
                ListCellPointer comp;
                ListForEach(comp,SelectedEntities){
                    EntityID e = *((EntityID*)GetElement(*comp));
                    RemoveComponentFromEntity(component,e);
                    
                }
                *curHeight -= TTF_FontHeight(gizmosFont);
                return 0;
            }
        }else{
            glColor3f(0.9,0.9,0.9);
        }
        glVertex2f( roundf(removePos.x), roundf(removePos.y));
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    //Component Name
    ComponentType *type = ((ComponentType*)GetElementAt(ECS.ComponentTypes,component));
    //Cut names that are too big to be shown
    if(type->nameSize>11){
        char cName[14] = "OOOOOOOOOO...";
        strncpy(cName,type->name,10);
        RenderText(cName, fontColor, Screen.windowWidth-componentWindowLength+componentNameLeftSpacing, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);
    }else{
        RenderText(type->name, fontColor, Screen.windowWidth-componentWindowLength+componentNameLeftSpacing, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);
    }

    *curHeight -= TTF_FontHeight(gizmosFont);
    return 1;
}

void Vector3Field(char *title, Vector3 *data,int ommitX,int ommitY,int ommitZ,int x, int w, int fieldsSpacing, int* curField, int* curHeight){
    *curHeight -= 2;
    RenderText(title, fontColor, x, *curHeight - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
    *curHeight -= 2 + TTF_FontHeight(gizmosFontSmall);

    int fieldW = w/3;

    Vector3 min1 = { x,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max1 = { x+fieldW - fieldsSpacing,*curHeight,0};

    Vector3 min2 = { x+fieldW + fieldsSpacing,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max2 = { x+fieldW*2 - fieldsSpacing,*curHeight,0};

    Vector3 min3 = { x+fieldW*2 + fieldsSpacing,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max3 = { x+fieldW*3 - fieldsSpacing,*curHeight,0};

    static char valueString[5] = "  0.0";
    if(editingField == *curField)
    {
        glColor3f(0.3,0.3,0.3);
        glBegin(GL_QUADS);
            glVertex2f( min1.x, max3.y);
            glVertex2f( max3.x, max3.y);
            glVertex2f( max3.x, min1.y);
            glVertex2f( min1.x, min1.y);
        glEnd();

        data->x = strtof(textFieldString, NULL);
        RenderText(textFieldString, fontColor, min1.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

    }else if(editingField != (*curField)+1 && editingField != (*curField)+2){
        glColor3f(0.2,0.2,0.2);
        glBegin(GL_QUADS);
            glVertex2f( min1.x, max1.y);
            glVertex2f( max1.x, max1.y);
            glVertex2f( max1.x, min1.y);
            glVertex2f( min1.x, min1.y);
        glEnd();

        if(!ommitX)
            snprintf(valueString,5,"%3.1f",data->x);
        else
            snprintf(valueString,5,"---");

        RenderText(valueString, fontColor, min1.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

        if(editingField<0){
            if(MouseOverBox(mousePos,min1,max1,0) && GetMouseButtonDown(SDL_BUTTON_LEFT)){
                textFieldString = (char*)calloc(13,sizeof(char));
                snprintf(textFieldString, 12, "%6.6f", data->x);
                GetTextInput(textFieldString, 12, strlen(textFieldString));
                editingField = *curField;
            }
        }
    }
    *curField +=1;

    if(editingField == *curField)
    {
        glColor3f(0.3,0.3,0.3);
        glBegin(GL_QUADS);
            glVertex2f( min1.x, max3.y);
            glVertex2f( max3.x, max3.y);
            glVertex2f( max3.x, min1.y);
            glVertex2f( min1.x, min1.y);
        glEnd();

        data->y = strtof(textFieldString, NULL);
        RenderText(textFieldString, fontColor, min1.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

    }else if(editingField != (*curField)-1 && editingField != (*curField)+1){
        glColor3f(0.2,0.2,0.2);
        glBegin(GL_QUADS);
            glVertex2f( min2.x, max2.y);
            glVertex2f( max2.x, max2.y);
            glVertex2f( max2.x, min2.y);
            glVertex2f( min2.x, min2.y);
        glEnd();

        if(!ommitY)
            snprintf(valueString,5,"%3.1f",data->y);
        else
            snprintf(valueString,5,"---");

        RenderText(valueString, fontColor, min2.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

        if(editingField<0){
            if(MouseOverBox(mousePos,min2,max2,0) && GetMouseButtonDown(SDL_BUTTON_LEFT)){
                textFieldString = (char*)calloc(13,sizeof(char));
                snprintf(textFieldString, 12, "%6.6f", data->y);
                GetTextInput(textFieldString, 12, strlen(textFieldString));
                editingField = *curField;
            }
        }
    }
    *curField +=1;

    if(editingField == *curField)
    {
        glColor3f(0.3,0.3,0.3);
        glBegin(GL_QUADS);
            glVertex2f( min1.x, max3.y);
            glVertex2f( max3.x, max3.y);
            glVertex2f( max3.x, min1.y);
            glVertex2f( min1.x, min1.y);
        glEnd();

        data->z = strtof(textFieldString, NULL);
        RenderText(textFieldString, fontColor, min1.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

    }else if(editingField != (*curField)-2 && editingField != (*curField)-1){
        glColor3f(0.2,0.2,0.2);
        glBegin(GL_QUADS);
            glVertex2f( min3.x, max3.y);
            glVertex2f( max3.x, max3.y);
            glVertex2f( max3.x, min3.y);
            glVertex2f( min3.x, min3.y);
        glEnd();

        if(!ommitZ)
            snprintf(valueString,5,"%3.1f",data->z);
        else
            snprintf(valueString,5,"---");
        RenderText(valueString, fontColor, min3.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

        if(editingField<0){
            if(MouseOverBox(mousePos,min3,max3,0) && GetMouseButtonDown(SDL_BUTTON_LEFT)){
                textFieldString = (char*)calloc(13,sizeof(char));
                snprintf(textFieldString, 12, "%6.6f", data->z);
                GetTextInput(textFieldString, 12, strlen(textFieldString));
                editingField = *curField;
            }
        }
    }
    *curField +=1;

    *curHeight -= 2 + TTF_FontHeight(gizmosFont);
}