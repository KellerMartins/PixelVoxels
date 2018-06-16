#include "EntitiesPanel.h"

/////////////////////////////////External data//////////////////////////////////

//Engine data
extern engineScreen Screen;
extern engineInput Input;
extern engineECS ECS;

//Data from EditorUI.c
extern TTF_Font* gizmosFont;

//Color definitions from Editor.c
extern Vector3 scrollbarInactiveColor;
extern Vector3 scrollbarOverColor;
extern Vector3 buttonOverColor;
extern Vector3 bgPanelColor;
extern Vector3 bgLightColor;
extern Vector3 brightWhite;
extern Vector3 lightWhite;

//Data from Editor.c
extern const int scrollbarMouseOverDistance;
extern const int scrollbarMouseWheelSpeed;
extern const double scrollbarWidth;
extern List SelectedEntities;
extern Vector3 mousePos;
extern Vector3 deltaMousePos;

//Data from FileBrowser.c
extern FileBrowserData fileBrowser;

//Data from MenuWindow.c
extern int menuOpened;

/////////////////////////////////////////////////////////////////////////////////


int entityWindowLength = 100;
int entityWindowWidthSpacing = 14;
int entityWindowTopHeightSpacing = 90;
int entityNameLeftSpacing = 18;
int entityBetweenSpacing = 2;

//Internal functions
static int IsSelected(EntityID entity);
static void RemoveFromSelected(EntityID entity);

int entityStartHeight = 0;
int movingEntitiesScrollbar = 0;
void DrawEntitiesPanel(){

    EntityID entity;
    //Entities Panel background
    Vector3 panelMin = {0,0,0};
    Vector3 panelMax = {entityWindowLength,Screen.windowHeight,0};
    DrawRectangle(panelMin, panelMax, bgPanelColor.x, bgPanelColor.y, bgPanelColor.z);

    //Open menu button
    Vector3 bMin = {0,Screen.windowHeight-(entityWindowTopHeightSpacing * 2.0/3.0)+2,0};
    Vector3 bMax = {entityWindowLength,Screen.windowHeight-2,0};
    if(MouseOverBox(mousePos, bMin, bMax,0)){
        DrawRectangle(bMin,bMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
            menuOpened = 1;
            CloseFileBrowser();
        }
    }else{
        DrawRectangle(bMin,bMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);
    }
    DrawTextColored("Menu", brightWhite, 16, Screen.windowHeight-(entityWindowTopHeightSpacing/2)+5, gizmosFont);





    //Draw Entities boxes
    int entityHeight = Screen.windowHeight + entityStartHeight-entityWindowTopHeightSpacing;
    for(entity=0;entity<=ECS.maxUsedIndex;entity++){
        if(IsValidEntity(entity) && !EntityIsChild(entity)){
            DrawEntityElement(entity, &entityHeight,0);
        }
    }

    //Scrollbar
    int mouseOverEntityPanel = MouseOverBox(mousePos, (Vector3){-1,-1,0}, (Vector3){entityWindowLength,Screen.windowHeight-entityWindowTopHeightSpacing,0},0);
    int offscreenPixels = -clamp(entityHeight-entityStartHeight,-INFINITY,0);
    
        Vector3 scrollbarStart = {entityWindowWidthSpacing/2 ,Screen.windowHeight-entityStartHeight - entityWindowTopHeightSpacing,0};
        Vector3 scrollbarEnd = {entityWindowWidthSpacing/2 ,offscreenPixels - entityStartHeight + 1,0};
        Vector3 scrollbarColor = {scrollbarInactiveColor.x, scrollbarInactiveColor.y, scrollbarInactiveColor.z};

        //If the scrollbar is only 10 pixels high, make the bar lerp between the top and bottom height, instead of
        //showing exactly the pixel movement of the bar
        if(scrollbarEnd.y > scrollbarStart.y-10){
            float frac = fabs(entityStartHeight/(float)offscreenPixels);
            scrollbarStart.y = Lerp(frac, Screen.windowHeight- entityWindowTopHeightSpacing - 5, 8) - 5;
            scrollbarEnd.y = Lerp(frac, Screen.windowHeight- entityWindowTopHeightSpacing - 5, 8) + 5;
        }
        
        if(mouseOverEntityPanel){
            if(!movingEntitiesScrollbar){
                if(MouseOverLine(mousePos, scrollbarStart, scrollbarEnd, scrollbarMouseOverDistance)){
                    scrollbarColor = (Vector3){scrollbarOverColor.x, scrollbarOverColor.y, scrollbarOverColor.z};

                    if(GetMouseButton(SDL_BUTTON_LEFT)){
                        movingEntitiesScrollbar = 1;
                    }
                }else{
                    scrollbarColor = (Vector3){buttonOverColor.x, buttonOverColor.y, buttonOverColor.z};
                }
            }else if (movingEntitiesScrollbar == 1){
                scrollbarColor = (Vector3){0.55,0.55,0.55};
            }else{
                scrollbarColor = (Vector3){buttonOverColor.x, buttonOverColor.y, buttonOverColor.z};
            }
        }

    //Scrollbar bar
    DrawRectangle((Vector3){scrollbarEnd.x-scrollbarWidth/2.0, scrollbarEnd.y},
                  (Vector3){scrollbarStart.x+scrollbarWidth/2.0, scrollbarStart.y},
                   scrollbarColor.x, scrollbarColor.y, scrollbarColor.z);

    if(mouseOverEntityPanel){
        if(Input.mouseWheelY!=0) movingEntitiesScrollbar = 2;
        if(movingEntitiesScrollbar){

            //Moving with mouse click
            if(movingEntitiesScrollbar == 1){
                if(GetMouseButton(SDL_BUTTON_LEFT)){
                    if(offscreenPixels>0){
                        float scrollMultiplier = clamp(offscreenPixels/(float)(Screen.windowHeight-entityWindowTopHeightSpacing),1,INFINITY);
                        double scrollbarMovement = norm(VectorProjection(deltaMousePos,(Vector3){0,1,0})) * sign(deltaMousePos.y)*scrollMultiplier;
                        entityStartHeight = clamp(entityStartHeight-scrollbarMovement,0,offscreenPixels);
                    }else{
                        entityStartHeight = 0;
                    }
                }else{
                    movingEntitiesScrollbar = 0;
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
                    movingEntitiesScrollbar = 0;
                }
            }
        }
    }

    //New Entity button

    //Line separating new entity button from the entities and the scrollbar
    DrawLine((Vector3){0,Screen.windowHeight-entityWindowTopHeightSpacing},
             (Vector3){entityWindowLength,Screen.windowHeight-entityWindowTopHeightSpacing},
              4,bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);

    //Entity element
    bMin = (Vector3){0,Screen.windowHeight-entityWindowTopHeightSpacing+2,0};
    bMax = (Vector3){entityWindowLength,Screen.windowHeight-(entityWindowTopHeightSpacing * 2.0/3.0),0};
    Vector3 bColor = {bgLightColor.x, bgLightColor.y, bgLightColor.z};
    if(MouseOverBox(mousePos, bMin, bMax,0)){
        bColor = (Vector3){buttonOverColor.x, buttonOverColor.y, buttonOverColor.z};
        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
            CreateEntity();
        }
    }
    DrawRectangle(bMin, bMax, bColor.x, bColor.y, bColor.z);
    DrawTextColored("+ Entity", brightWhite, 5, Screen.windowHeight-(entityWindowTopHeightSpacing)+5, gizmosFont);
}

void DrawEntityElement(EntityID entity, int *entityHeight, int depth){

    //Draw this entity rect
    if((*entityHeight)>0 && (*entityHeight)<Screen.windowHeight-entityWindowTopHeightSpacing+TTF_FontHeight(gizmosFont)+2){

        int isSelected = IsSelected(entity);
        Vector3 elMin = {entityWindowWidthSpacing +depth*10,(*entityHeight)-TTF_FontHeight(gizmosFont)-2,0};
        Vector3 elMax = {entityWindowLength,(*entityHeight),0};
        
        //Entity element
        //If mouse is over the item and is inside the entityWindow (Condition to avoid overlapping with the add entity button)
        if(MouseOverBox(mousePos, elMin, elMax,0) && mousePos.y<Screen.windowHeight-entityWindowTopHeightSpacing){
            if(isSelected){
                DrawRectangle(elMin,elMax,scrollbarInactiveColor.x, scrollbarInactiveColor.y, scrollbarInactiveColor.z);

                if(GetMouseButtonDown(SDL_BUTTON_LEFT) && !fileBrowser.opened){
                    RemoveFromSelected(entity);
                }
                if(GetMouseButtonDown(SDL_BUTTON_RIGHT) && !fileBrowser.opened){
                    EntityID newEntity = DuplicateEntity(entity);
                    FreeList(&SelectedEntities);
                    InsertListEnd(&SelectedEntities,&newEntity);
                }
            }else{
                DrawRectangle(elMin,elMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);

                if(GetMouseButtonDown(SDL_BUTTON_LEFT) && !fileBrowser.opened){
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
                DrawRectangle(elMin,elMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
            }else{
                DrawRectangle(elMin,elMax,bgLightColor.x*clamp(1 - depth*0.15,0.5,1),bgLightColor.y*clamp(1 - depth*0.15,0.5,1),bgLightColor.z*clamp(1 - depth*0.15,0.5,1));
            }
        }
        static char entityName[12];
        snprintf(entityName,11,"%d",entity);
        int w,h;
        TTF_SizeText(gizmosFont,entityName,&w,&h);
        if(entityWindowLength - (entityNameLeftSpacing +depth*10)>= w){
            DrawTextColored(entityName, lightWhite, entityNameLeftSpacing +depth*10, (*entityHeight) - TTF_FontHeight(gizmosFont), gizmosFont);
        }
    }

    //Add entities spacing
    (*entityHeight) -= TTF_FontHeight(gizmosFont)+2 + entityBetweenSpacing;

    //Draw child entities boxes
    if(EntityIsParent(entity)){
        ListCellPointer childCell;
        ListForEach(childCell, *GetChildsList(entity)){
            DrawEntityElement(GetElementAsType(childCell,EntityID), entityHeight, depth+1);
        }
    }
}



static int IsSelected(EntityID entity){
    ListCellPointer current = GetFirstCell(SelectedEntities);
    while(current){
        if(*((EntityID*)GetElement(*current)) == entity){
            return 1;
        }
        current = GetNextCell(current);
    }
    return 0;
}

static void RemoveFromSelected(EntityID entity){
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
