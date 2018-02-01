#include "EditorGizmos.h"

static System *ThisSystem;

extern engineECS ECS;
extern engineScreen Screen;
extern engineRendering Rendering;
extern engineInput Input;

static float positionGizmosLength = 20;
static const int selectMouseOverDistance = 10;
static const int axisMouseOverDistance = 20;
static const int scrollbarMouseOverDistance = 8;
static const float lineGizmosMouseMovement = 4.50;
static const int gizmosDepth = 1;

static float componentWindowLenght = 200;
static float componentWindowWidthSpacing = 14;
static float componentNameLeftSpacing = 5;
static float componentBetweenSpacing = 5;

static float entityWindowLenght = 100;
static float entityWindowWidthSpacing = 14;
static float entityNameLeftSpacing = 5;
static float entityBetweenSpacing = 5;

static int scrollbarMouseWheelSpeed = 25;

//List of selected EntityIDs
List SelectedEntities;

int MouseOverLineGizmos(Vector3 mousePos, Vector3 originPos, Vector3 handlePos,int mouseOverDistance);
int MouseOverPointGizmos(Vector3 mousePos, Vector3 originPos,int mouseOverDistance);
Vector3 WorldVectorToScreenVector(Vector3 v);
int IsSelected(EntityID entity);
void RemoveFromSelected(EntityID entity);

TTF_Font* gizmosFont;
GLuint iconsTex[3];
int iconsSize = 9;

//Runs on engine start
void EditorGizmosInit(System *systemObject){
    ThisSystem = systemObject;

    SelectedEntities = InitList(sizeof(EntityID));

    gizmosFont = TTF_OpenFont("Interface/Fonts/gros/GROS.ttf",16);
	if(!gizmosFont){
		printf("Font: Error loading font!");
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


int componentStartHeight = 0;
int movingScrollbar = 0;

//Runs each GameLoop iteration
void EditorGizmosUpdate(){
    EntityID entity;

    Vector3 mousePos = (Vector3){Input.mouseX,Screen.windowHeight - Input.mouseY,0};
    Vector3 deltaMousePos = (Vector3){Input.deltaMouseX,-Input.deltaMouseY,0};

    glPointSize(5 * 2/Screen.gameScale);
    glLineWidth(2* 2/Screen.gameScale);
    glClear(GL_DEPTH_BUFFER_BIT);
    //Render UI objects in the [0,0.01] depth range
    glDepthRange(0, 0.01);

    glBindFramebuffer(GL_FRAMEBUFFER, Rendering.frameBuffer);
    glViewport(0,0,Screen.gameWidth,Screen.gameHeight);

    //Entities Panel background
    glBegin(GL_QUADS);
        glColor3f(0.02,0.02,0.05);
        glVertex2f( entityWindowLenght,  Screen.windowHeight);
        glVertex2f( 0,  Screen.windowHeight);
        glVertex2f( 0, 0);
        glVertex2f( entityWindowLenght, 0);
    glEnd();

    //Runs for all entities
	for(entity = 0; entity <= ECS.maxUsedIndex; entity++){
        
        //Check if entity contains needed components
        //If no component is required , run for all
        if(!IsEmptyComponentMask(ThisSystem->required) && !EntityContainsMask(entity,ThisSystem->required) ) continue;
        //Check if entity doesn't contains the excluded components
        //If there is no restriction, run all with the required components
        if(!IsEmptyComponentMask(ThisSystem->excluded) && EntityContainsMask(entity,ThisSystem->excluded) ) continue;
        
        //-------------------------- Transform gizmos --------------------------
        if(EntityContainsComponent(entity,GetComponentID("Transform"))){

            Vector3 position = GetPosition(entity);
            Vector3 screenPos = PositionToGameScreenCoords(position);

            //Transform to screen coordinates
            screenPos.x = Screen.windowWidth/2 + (screenPos.x/(float)Screen.gameWidth) * Screen.windowWidth;
            screenPos.y = Screen.windowHeight/2 + (screenPos.y/(float)Screen.gameHeight) * Screen.windowHeight;

            Vector3 originPos = (Vector3){screenPos.x,screenPos.y,0};

            if(!IsSelected(entity)){
                //Element not selected
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
                    glVertex3f(screenPos.x,screenPos.y,gizmosDepth);
                glEnd();

            }else{

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

                    glVertex3f(screenPos.x,screenPos.y,gizmosDepth);
                    glVertex3f(lineXEndPos.x,lineXEndPos.y,gizmosDepth);
                glEnd();
                //Forward (X) point
                glBegin(GL_POINTS);
                    glVertex3f(lineXEndPos.x,lineXEndPos.y,gizmosDepth);
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

                    glVertex3f(screenPos.x,screenPos.y,gizmosDepth);
                    glVertex3f(lineYEndPos.x,lineYEndPos.y,gizmosDepth);
                glEnd();
                //Left (Y) point
                glBegin(GL_POINTS);
                    glVertex3f(lineYEndPos.x,lineYEndPos.y,gizmosDepth);
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

                    glVertex3f(screenPos.x,screenPos.y,gizmosDepth);
                    glVertex3f(lineZEndPos.x,lineZEndPos.y,gizmosDepth);
                glEnd();
                //Up (Z) point and origin point
                glBegin(GL_POINTS);

                    glVertex3f(lineZEndPos.x,lineZEndPos.y,gizmosDepth);

                    if(MouseOverPointGizmos(mousePos, originPos, selectMouseOverDistance)){
                        glColor3f(1,1,0);

                        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                            RemoveFromSelected(entity);
                        }
                    }else{
                        glColor3f(1,1,1);
                    }
                    glVertex3f(screenPos.x,screenPos.y,gizmosDepth);
                glEnd();

                //printf("%f %f %f \n",norm(WorldVectorToScreenVector(VECTOR3_FORWARD)),norm(WorldVectorToScreenVector(VECTOR3_LEFT)),norm(WorldVectorToScreenVector(VECTOR3_UP)));

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
                SDL_Color fontColor = {255,255,255};
                int w, h;
                TTF_SizeText(gizmosFont,"Some random text",&w,&h);
                RenderText("Some random text", fontColor, screenPos.x-w/2, screenPos.y-h*1.5,gizmosDepth, gizmosFont);  
            }
        }
    }

    //-------------------------- Components panel --------------------------
    if(!IsListEmpty(SelectedEntities)){

        
        //Set mask as the components of the first entity selected
        ComponentMask mask = GetEntityComponents(*((EntityID*)GetFirstElement(SelectedEntities)));

        ListCellPointer current;
        ListForEach(current,SelectedEntities){
            EntityID e = *((EntityID*)GetElement(*current));
            mask = IntersectComponentMasks(mask,GetEntityComponents(e));
        }

        SDL_Color fontColor = {255,255,255};

        //Panel background
        glBegin(GL_QUADS);
            glColor3f(0.02,0.02,0.05);
            glVertex2f( Screen.windowWidth-componentWindowLenght,  Screen.windowHeight);
            glVertex2f( Screen.windowWidth,  Screen.windowHeight);
            glVertex2f( Screen.windowWidth, 0);
            glVertex2f( Screen.windowWidth-componentWindowLenght, 0);
        glEnd();

        //Show the panel of the components contained by the selected entities
        int c,componentHeight = Screen.windowHeight + componentStartHeight;
        for(c=0;c<GetLength(ECS.ComponentTypes);c++){
            if(MaskContainsComponent(mask,c)){

                glBegin(GL_QUADS);
                    //Top panel
                    glColor3f(0.2,0.2,0.35);
                    glVertex2f( Screen.windowWidth-componentWindowLenght,  componentHeight);
                    glVertex2f( Screen.windowWidth-componentWindowWidthSpacing,  componentHeight);
                    glVertex2f( Screen.windowWidth-componentWindowWidthSpacing, componentHeight-TTF_FontHeight(gizmosFont)-2);
                    glVertex2f( Screen.windowWidth-componentWindowLenght, componentHeight-TTF_FontHeight(gizmosFont)-2);
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
                    Vector3 removePos = {Screen.windowWidth-componentWindowWidthSpacing - (TTF_FontHeight(gizmosFont)-2)/2 -2,componentHeight - (TTF_FontHeight(gizmosFont))/2};
                    if(MouseOverPointGizmos(mousePos, removePos, iconsSize * 2/Screen.gameScale)){
                        glColor3f(1,0.2,0.2);

                        //Pressed to remove
                        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                            //Remove all components of this type from the selected entities
                            ListCellPointer comp;
                            ListForEach(comp,SelectedEntities){
                                EntityID e = *((EntityID*)GetElement(*comp));
                                RemoveComponentFromEntity(c,e);
                                
                            }
                            break;
                        }
                    }else{
                        glColor3f(0.9,0.9,0.9);
                    }
                    glVertex2f( roundf(removePos.x), roundf(removePos.y));
                glEnd();
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);

                
                ComponentType *type = ((ComponentType*)GetElementAt(ECS.ComponentTypes,c));
                //Cut names that are too big to be shown
                if(type->nameSize>11){
                    char cName[14] = "OOOOOOOOOO...";
                    strncpy(cName,type->name,10);
                    RenderText(cName, fontColor, Screen.windowWidth-componentWindowLenght+componentNameLeftSpacing, componentHeight - TTF_FontHeight(gizmosFont),gizmosDepth, gizmosFont);
                }else{
                    RenderText(type->name, fontColor, Screen.windowWidth-componentWindowLenght+componentNameLeftSpacing, componentHeight - TTF_FontHeight(gizmosFont),gizmosDepth, gizmosFont);
                }
                componentHeight -= TTF_FontHeight(gizmosFont);

                //Component specific drawing
                if(c == GetComponentID("Transform")){
                    glBegin(GL_QUADS);
                        //Component background
                        glColor3f(0.1,0.1,0.15);
                        glVertex2f( Screen.windowWidth-componentWindowLenght,  componentHeight);
                        glVertex2f( Screen.windowWidth-componentWindowWidthSpacing,  componentHeight);
                        glVertex2f( Screen.windowWidth-componentWindowWidthSpacing, componentHeight-100);
                        glVertex2f( Screen.windowWidth-componentWindowLenght, componentHeight-100);
                    glEnd();

                    componentHeight -= 100;
                }
                componentHeight -= componentBetweenSpacing;
            }
        }
        
        //Scrollbar
        glLineWidth(clamp((componentWindowWidthSpacing/2 - 2) * 2/Screen.gameScale,1,Screen.windowWidth));
        int offscreenPixels = -clamp(componentHeight-componentStartHeight,-(Screen.windowHeight-10),0);
        glBegin(GL_LINES);  
            Vector3 scrollbarStart = {Screen.windowWidth - componentWindowWidthSpacing/2 ,Screen.windowHeight-componentStartHeight -2,0};
            Vector3 scrollbarEnd = {Screen.windowWidth - componentWindowWidthSpacing/2 ,offscreenPixels - componentStartHeight + 1,0};

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

            glVertex3f(scrollbarStart.x,scrollbarStart.y,gizmosDepth);
            glVertex3f(scrollbarEnd.x,scrollbarEnd.y,gizmosDepth);
        glEnd();

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

    //Return depth to default values
    glDepthRange(0, 1.0);

}

//Runs at engine finish
void EditorGizmosFree(){
    FreeList(&SelectedEntities);
    TTF_CloseFont(gizmosFont);
}

int MouseOverLineGizmos(Vector3 mousePos, Vector3 originPos, Vector3 handlePos,int mouseOverDistance){
    return DistanceFromPointToLine2D(handlePos,originPos,mousePos)<mouseOverDistance && Distance(mousePos,handlePos)<Distance(handlePos, originPos) && Distance(mousePos,originPos)<Distance(handlePos, originPos);
}

int MouseOverPointGizmos(Vector3 mousePos, Vector3 originPos,int mouseOverDistance){
    return Distance(mousePos,originPos)<mouseOverDistance;
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