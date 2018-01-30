#include "EditorGizmos.h"

static System *ThisSystem;

extern engineECS ECS;
extern engineScreen Screen;
extern engineRendering Rendering;
extern engineInput Input;

static const int positionGizmosLength = 20;
static const int lineGizmosMouseOverDistance = 10;
static const float lineGizmosMouseMovement = 4.50;
static const int gizmosDepth = 1;

int MouseOverLineGizmos(Vector3 mousePos, Vector3 originPos, Vector3 handlePos);
Vector3 WorldVectorToScreenVector(Vector3 v);

TTF_Font* gizmosFont;

//Runs on engine start
void EditorGizmosInit(System *systemObject){
    ThisSystem = systemObject;

    gizmosFont = TTF_OpenFont("Interface/Fonts/gros/GROS.ttf",16);
	if(!gizmosFont){
		printf("Font: Error loading font!");
	}
}


int movingX = 0;
int movingY = 0;
int movingZ = 0;

//Runs each GameLoop iteration
void EditorGizmosUpdate(){
    EntityID entity;


    //Runs for all entities
	for(entity = 0; entity <= ECS.maxUsedIndex; entity++){
        
        //Check if entity contains needed components
        //If no component is required , run for all
        if(!IsEmptyComponentMask(ThisSystem->required) && !EntityContainsMask(entity,ThisSystem->required) ) continue;
        //Check if entity doesn't contains the excluded components
        //If there is no restriction, run all with the required components
        if(!IsEmptyComponentMask(ThisSystem->excluded) && EntityContainsMask(entity,ThisSystem->excluded) ) continue;

        if(EntityContainsComponent(entity,GetComponentID("Transform"))){

            Vector3 position = GetPosition(entity);
            Vector3 screenPos = PositionToGameScreenCoords(position);

            //Transform to screen coordinates
            screenPos.x = Screen.windowWidth/2 + (screenPos.x/(float)Screen.gameWidth) * Screen.windowWidth;
            screenPos.y = Screen.windowHeight/2 + (screenPos.y/(float)Screen.gameHeight) * Screen.windowHeight;

            Vector3 mousePos = (Vector3){Input.mouseX,Screen.windowHeight - Input.mouseY,0};
            Vector3 deltaMousePos = (Vector3){Input.deltaMouseX,-Input.deltaMouseY,0};
            Vector3 originPos = (Vector3){screenPos.x,screenPos.y,0};
            
            glPointSize(5);
            glLineWidth(2);
            glClear(GL_DEPTH_BUFFER_BIT);

            //Render UI objects in the [0,0.01] depth range
            glDepthRange(0, 0.01);

            //Forward (X) line
            glBegin(GL_LINES);

                Vector3 lineXEndPos = PositionToGameScreenCoords(Add(position,(Vector3){positionGizmosLength,0,0}));

                lineXEndPos.x = Screen.windowWidth/2 + (lineXEndPos.x/(float)Screen.gameWidth) * Screen.windowWidth;
                lineXEndPos.y = Screen.windowHeight/2 + (lineXEndPos.y/(float)Screen.gameHeight) * Screen.windowHeight;
                lineXEndPos.z = 0;

                if(!movingX){
                    if(MouseOverLineGizmos(mousePos, originPos, lineXEndPos)){
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

                Vector3 lineYEndPos = PositionToGameScreenCoords(Add(position,(Vector3){0,positionGizmosLength,0}));

                lineYEndPos.x = Screen.windowWidth/2 + (lineYEndPos.x/(float)Screen.gameWidth) * Screen.windowWidth;
                lineYEndPos.y = Screen.windowHeight/2 + (lineYEndPos.y/(float)Screen.gameHeight) * Screen.windowHeight;
                lineYEndPos.z = 0;

                if(!movingY){
                    if(MouseOverLineGizmos(mousePos, originPos, lineYEndPos)){
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

                Vector3 lineZEndPos = PositionToGameScreenCoords(Add(position,(Vector3){0,0,positionGizmosLength}));

                lineZEndPos.x = Screen.windowWidth/2 + (lineZEndPos.x/(float)Screen.gameWidth) * Screen.windowWidth;
                lineZEndPos.y = Screen.windowHeight/2 + (lineZEndPos.y/(float)Screen.gameHeight) * Screen.windowHeight;
                lineZEndPos.z = 0;
                
                if(!movingZ){
                    if(MouseOverLineGizmos(mousePos, originPos, lineZEndPos)){
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

                glColor3f(1,1,0);
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

            //Return depth to default values
            glDepthRange(0, 1.0);
        }
    }
}

//Runs at engine finish
void EditorGizmosFree(){
    TTF_CloseFont(gizmosFont);
}

int MouseOverLineGizmos(Vector3 mousePos, Vector3 originPos, Vector3 handlePos){
    return DistanceFromPointToLine2D(handlePos,originPos,mousePos)<lineGizmosMouseOverDistance && Distance(mousePos,handlePos)<Distance(handlePos, originPos) && Distance(mousePos,originPos)<Distance(handlePos, originPos);
}

Vector3 WorldVectorToScreenVector(Vector3 v){
    Vector3 screenPos;
	screenPos.x = (int)(v.x - v.y)*2 + 0.375;
    screenPos.y = (int)(v.x + v.y) + (v.z)*2 + 0.375;
    screenPos.z = 0;
    return screenPos;
}