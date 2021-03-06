#include "EditorGizmos.h"

/////////////////////////////////External data//////////////////////////////////

//Engine data
extern engineScreen Screen;
extern engineECS ECS;

//Color definitions from Editor.c
extern Vector3 scrollbarOverColor;

//Data from Editor.c
extern Vector3 mousePos;
extern Vector3 deltaMousePos;

//Data from EntitiesPanel.c
extern List SelectedEntities;

/////////////////////////////////////////////////////////////////////////////////

float positionGizmosLength = 10;
const int selectMouseOverDistance = 10;
const int axisMouseOverDistance = 20;

int movingX = 0;
int movingY = 0;
int movingZ = 0;
void DrawTransformGizmos(){
    EntityID entity;
    for(entity = 0; entity <= ECS.maxUsedIndex; entity++){
        
        //Run for all entities with the transform component
        if(IsValidEntity(entity) && EntityContainsComponent(entity,GetComponentID("Transform"))){

            Vector3 position;
            Matrix3x3 rotation;
            GetGlobalTransform(entity,&position,NULL,&rotation);
            rotation = MultiplyMatrix3x3(Transpose(GetRotationMatrix(entity)),rotation);
            Vector3 screenPos = PositionToGameScreenCoords(position);

            Vector3 originPos = (Vector3){screenPos.x,screenPos.y,0};

            if(!IsSelected(entity)){
                //Entity not selected, show selection point
                Vector3 selPointColor = {scrollbarOverColor.x, scrollbarOverColor.y, scrollbarOverColor.z};
                if(MouseOverPoint(mousePos, originPos, selectMouseOverDistance)){
                    selPointColor = (Vector3){1,1,1};
                    
                    if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                        if(GetKey(SDL_SCANCODE_LSHIFT)){
                            InsertListEnd(&SelectedEntities,&entity);
                        }else{
                            FreeList(&SelectedEntities);
                            InsertListEnd(&SelectedEntities,&entity);
                        }
                    }
                }
                DrawPoint(screenPos, 5, 0, selPointColor.x, selPointColor.y, selPointColor.z);

            }else{
                //Entity selected, show transform gizmos and deselection point
                //Forward (X) gizmos
                Vector3 lineXEndPos = PositionToGameScreenCoords(Add(position,RotateVector((Vector3){positionGizmosLength * 2/Screen.gameScale,0,0},rotation)));
                lineXEndPos.z = 0;

                Vector3 gizmosColor = {1,0.75,0.75};

                if(!movingX){
                    if(MouseOverLine(mousePos, originPos, lineXEndPos, axisMouseOverDistance) && !MouseOverPoint(mousePos, originPos, selectMouseOverDistance)){
                        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                            if(!movingZ && !movingY)
                                movingX = 1;
                        }
                    }else{
                        gizmosColor = (Vector3){1,0,0};
                    }
                }
                //Forward (X) line
                DrawLine(screenPos,lineXEndPos,4, gizmosColor.x, gizmosColor.y, gizmosColor.z);
                //Forward (X) point
                DrawPoint(lineXEndPos, 5, 0, gizmosColor.x, gizmosColor.y, gizmosColor.z);



                //Left (Y) gizmos
                Vector3 lineYEndPos = PositionToGameScreenCoords(Add(position,RotateVector((Vector3){0,positionGizmosLength * 2/Screen.gameScale,0},rotation)));
                lineYEndPos.z = 0;

                gizmosColor = (Vector3){0.75,1,0.75};

                if(!movingY){
                    if(MouseOverLine(mousePos, originPos, lineYEndPos, axisMouseOverDistance) && !MouseOverPoint(mousePos, originPos, selectMouseOverDistance)){
                        
                        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                            if(!movingX && !movingZ)
                                movingY = 1;
                        }
                    }else{
                        gizmosColor = (Vector3){0,1,0};
                    }
                }

                //Left (Y) line
                DrawLine(screenPos,lineYEndPos,4, gizmosColor.x, gizmosColor.y, gizmosColor.z);
                //Left (Y) point
                DrawPoint(lineYEndPos, 5, 0, gizmosColor.x, gizmosColor.y, gizmosColor.z);



                //Up (Z) gizmos
                Vector3 lineZEndPos = PositionToGameScreenCoords(Add(position,RotateVector((Vector3){0,0,positionGizmosLength * 2/Screen.gameScale},rotation)));
                lineZEndPos.z = 0;

                gizmosColor = (Vector3){0.75,0.75,1};
                
                if(!movingZ){
                    if(MouseOverLine(mousePos, originPos, lineZEndPos, axisMouseOverDistance) && !MouseOverPoint(mousePos, originPos, selectMouseOverDistance)){
                        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                            if(!movingX && !movingY)
                                movingZ = 1;
                        }
                    }else{
                        gizmosColor = (Vector3){0,0,1};
                    }
                }

                //Left (Z) line
                DrawLine(screenPos,lineZEndPos,4, gizmosColor.x, gizmosColor.y, gizmosColor.z);
                //Up (Z) point
                DrawPoint(lineZEndPos, 5, 0, gizmosColor.x, gizmosColor.y, gizmosColor.z);

                //Origin point
                if(MouseOverPoint(mousePos, originPos, selectMouseOverDistance)){
                    gizmosColor = (Vector3){1,1,0};

                    if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                        RemoveFromSelected(entity);
                    }
                }else{
                    gizmosColor = (Vector3){1,1,1};
                }
                DrawPoint(screenPos, 5, 0, gizmosColor.x, gizmosColor.y, gizmosColor.z);

                //Arrow movement
                if(movingX){
                    if(GetMouseButton(SDL_BUTTON_LEFT)){
                        Vector3 arrowScreen = WorldVectorToScreenVector(RotateVector(VECTOR3_FORWARD,rotation));
                        double mouseMovementX = norm(deltaMousePos) * dot(NormalizeVector(arrowScreen), NormalizeVector(deltaMousePos))/(norm(arrowScreen)*2);
                        SetPosition(entity,Add(GetPosition(entity),(Vector3){mouseMovementX,0,0}));
                    }else{
                        movingX = 0;
                    }
                }

                if(movingY){
                    if(GetMouseButton(SDL_BUTTON_LEFT)){
                        Vector3 arrowScreen = WorldVectorToScreenVector(RotateVector(VECTOR3_LEFT,rotation));
                        double mouseMovementY = norm(deltaMousePos) * dot(NormalizeVector(arrowScreen), NormalizeVector(deltaMousePos))/(norm(arrowScreen)*2);
                        SetPosition(entity,Add(GetPosition(entity),(Vector3){0,mouseMovementY,0}));
                    }else{
                        movingY = 0;
                    }
                }

                if(movingZ){
                    if(GetMouseButton(SDL_BUTTON_LEFT)){
                        Vector3 arrowScreen = WorldVectorToScreenVector(RotateVector(VECTOR3_UP,rotation));
                        double mouseMovementZ = norm(deltaMousePos) * dot(NormalizeVector(arrowScreen), NormalizeVector(deltaMousePos))/(norm(arrowScreen)*2);
                        SetPosition(entity,Add(GetPosition(entity),(Vector3){0,0,mouseMovementZ}));
                    }else{
                        movingZ = 0;
                    }
                }
                //int w, h;
                //TTF_SizeText(gizmosFont,"Some random text",&w,&h);
                //DrawTextColored("Some random text", lightWhite, screenPos.x-w/2, screenPos.y-h*1.5,gizmosDepth, gizmosFont);  
            }
        }
    }
}