#include "ComponentsPanel.h"

/////////////////////////////////External data//////////////////////////////////

//Engine data
extern engineScreen Screen;
extern engineInput Input;
extern engineECS ECS;

//Data from EditorUI.c
extern int iconsSize[];
extern TTF_Font* gizmosFont;
extern TTF_Font* gizmosFontSmall;

//Color definitions from Editor.c
extern Vector3 bgPanelColor;
extern Vector3 bgLightColor;
extern Vector3 bgMediumColor;
extern Vector3 buttonOverColor;
extern Vector3 scrollbarInactiveColor;
extern Vector3 scrollbarOverColor;
extern Vector3 brightWhite;
extern Vector3 lightWhite;

//Data from Editor.c
extern const int scrollbarMouseOverDistance;
extern const int scrollbarMouseWheelSpeed;
extern Vector3 mousePos;
extern Vector3 deltaMousePos;
extern List SelectedEntities;

//Data from FileBrowser.c
extern FileBrowserData fileBrowser;

/////////////////////////////////////////////////////////////////////////////////



int componentWindowLength = 200;
int componentWindowWidthSpacing = 14;
int componentWindowBottomSpacing = 50;
int componentNameLeftSpacing = 5;
int componentBetweenSpacing = 3;


void FBLoadModel();
void FBLoadScript();


int addComponentWindowOpened = 0;
int addComponentScroll = 0;
int componentStartHeight = 0;
int movingComponentsScrollbar = 0;
void DrawComponentsPanel(){
    int currentComponentField = 0;
    if(!IsListEmpty(SelectedEntities)){
        
        int singlePrefabSelected = 0;
        int ComponentsPanelHeight = Screen.windowHeight;
        if(GetLength(SelectedEntities)==1 && EntityIsPrefab(GetElementAsType(GetFirstCell(SelectedEntities),EntityID))){
            ComponentsPanelHeight -= 50;
            singlePrefabSelected = 1;
        }

        //Panel background
        Vector3 bgMin = {Screen.windowWidth-componentWindowLength, 0,0};
        Vector3 bgMax = {Screen.windowWidth, ComponentsPanelHeight,0};
        DrawRectangle(bgMin, bgMax, bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);

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
            int c,componentHeight = ComponentsPanelHeight + componentStartHeight;
            for(c=0;c<GetLength(ECS.ComponentTypes);c++){

                if(MaskContainsComponent(mask,c)){
                    //Drawing component element

                    //Only render internal fields if the component is not removed (returned 1)
                    if(DrawComponentHeader(c, &componentHeight)){
                        //Component specific drawing
                        if(c == GetComponentID("Transform")){
                            //Component background
                            Vector3 bgMin = {Screen.windowWidth-componentWindowLength, componentHeight-97,0};
                            Vector3 bgMax = {Screen.windowWidth-componentWindowWidthSpacing, componentHeight,0};
                            DrawRectangle(bgMin, bgMax, bgMediumColor.x, bgMediumColor.y, bgMediumColor.z);

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
                            Vector3Field("position",&newPos,10,ommitPosX,ommitPosY,ommitPosZ,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1,4, &currentComponentField, &componentHeight);
                            Vector3Field("rotation",&newRot,1,ommitRotX,ommitRotY,ommitRotZ,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1,4, &currentComponentField, &componentHeight);

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
                            int isStatic = IsStaticRigidBody(GetElementAsType(selEntity,int));
                            int useGravity = UsesGravity(GetElementAsType(selEntity,int)); 
                            ListForEach(selEntity, SelectedEntities){
                                if(IsStaticRigidBody(GetElementAsType(selEntity,int)) != isStatic){
                                    isStatic = -1;
                                }
                                if(UsesGravity(GetElementAsType(selEntity,int)) != useGravity){
                                    useGravity = -1;
                                }
                                if(isStatic == -1 && useGravity == -1) break;
                            }

                            if(!isStatic){
                                //Component background
                                Vector3 bgMin = {Screen.windowWidth-componentWindowLength, componentHeight-230,0};
                                Vector3 bgMax = {Screen.windowWidth-componentWindowWidthSpacing, componentHeight,0};
                                DrawRectangle(bgMin, bgMax, bgMediumColor.x, bgMediumColor.y, bgMediumColor.z);

                                componentHeight-=2;
                                if(1 == PointToggle(&isStatic,(Vector3){Screen.windowWidth-componentWindowLength + 12,componentHeight - 10},14,15,16,1, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1})){
                                    ListForEach(selEntity, SelectedEntities){
                                        SetStaticRigidBody(GetElementAsType(selEntity,int),isStatic);
                                    }
                                }
                                DrawTextColored("Is Static", lightWhite, Screen.windowWidth-componentWindowLength + 25, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                                componentHeight-=20;

                                if(1 == PointToggle(&useGravity,(Vector3){Screen.windowWidth-componentWindowLength + 12,componentHeight - 10},14,15,16,1, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1})){
                                    ListForEach(selEntity, SelectedEntities){
                                        SetUseGravity(GetElementAsType(selEntity,int),useGravity);
                                    }
                                }
                                DrawTextColored("use gravity", lightWhite, Screen.windowWidth-componentWindowLength + 25, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
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

                                FloatField("mass",&newMass,10,ommitMass,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1, &currentComponentField, &componentHeight);
                                FloatField("bounciness",&newBounc,1000,ommitBounc,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1, &currentComponentField, &componentHeight);
                                Vector3Field("initial velocity",&newVel,10,ommitVelX,ommitVelY,ommitVelZ,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1,4, &currentComponentField, &componentHeight);
                                Vector3Field("const. accel.",&newAccel,10,ommitAccelX,ommitAccelY,ommitAccelZ,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1,4, &currentComponentField, &componentHeight);

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
                                Vector3 bgMin = {Screen.windowWidth-componentWindowLength, componentHeight-22,0};
                                Vector3 bgMax = {Screen.windowWidth-componentWindowWidthSpacing, componentHeight,0};
                                DrawRectangle(bgMin, bgMax, bgMediumColor.x, bgMediumColor.y, bgMediumColor.z);

                                componentHeight-=2;
                                if(1 == PointToggle(&isStatic,(Vector3){Screen.windowWidth-componentWindowLength + 12,componentHeight - 10},14,15,16,1, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1})){
                                    ListForEach(selEntity, SelectedEntities){
                                        SetStaticRigidBody(GetElementAsType(selEntity,int),isStatic);
                                    }
                                }
                                DrawTextColored("Is Static", lightWhite, Screen.windowWidth-componentWindowLength + 25, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                                componentHeight-=20;
                            }
                        }else if(c == GetComponentID("VoxelModel")){
                            
                            
                            ListCellPointer selEntity = GetFirstCell(SelectedEntities);
                            int isEnabled = IsVoxelModelEnabled(GetElementAsType(selEntity,EntityID));
                            int isSmall = IsVoxelModelSmallScale(GetElementAsType(selEntity,EntityID));
                            int isSubModel = GetVoxelModelPointer(GetElementAsType(selEntity,EntityID))->objectName[0] == '\0'? 0:1;

                            ListForEach(selEntity, SelectedEntities){
                                if(isEnabled != IsVoxelModelEnabled(GetElementAsType(selEntity,EntityID))){
                                    isEnabled = -1;
                                }
                                if(isSmall != IsVoxelModelSmallScale(GetElementAsType(selEntity,EntityID))){
                                    isSmall = -1;
                                }
                            }
                            int backgroundHeight = (GetLength(SelectedEntities)==1? (isSubModel? 138:118): 96);

                            //Component background
                            Vector3 bgMin = {Screen.windowWidth-componentWindowLength, componentHeight-backgroundHeight,0};
                            Vector3 bgMax = {Screen.windowWidth-componentWindowWidthSpacing, componentHeight,0};
                            DrawRectangle(bgMin, bgMax, bgMediumColor.x, bgMediumColor.y, bgMediumColor.z);
                            
                            if(1 == PointToggle(&isEnabled,(Vector3){Screen.windowWidth-componentWindowLength + 12,componentHeight - 10},14,15,16,1, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1})){
                                ListForEach(selEntity, SelectedEntities){
                                    SetVoxelModelEnabled(GetElementAsType(selEntity,int),isEnabled);
                                }
                            }
                            DrawTextColored("Enabled", lightWhite, Screen.windowWidth-componentWindowLength + 25, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                            componentHeight-=22;

                            if(1 == PointToggle(&isSmall,(Vector3){Screen.windowWidth-componentWindowLength + 12,componentHeight - 10},14,15,16,1, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1})){
                                ListForEach(selEntity, SelectedEntities){
                                    SetVoxelModelSmallScale(GetElementAsType(selEntity,EntityID),isSmall);
                                }
                            }
                            DrawTextColored("small scale", lightWhite, Screen.windowWidth-componentWindowLength + 25, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                            componentHeight-=22;

                            //Only show the model load button and name if only one entity is selected
                            if(GetLength(SelectedEntities)==1){
                                VoxelModel* m = GetVoxelModelPointer(GetElementAsType(GetFirstCell(SelectedEntities),EntityID));

                                if(1 == PointButton((Vector3){Screen.windowWidth-componentWindowLength + 15,componentHeight - 10},10,1, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1}, (Vector3){0.8,0.8,0.8})){
                                    if(!fileBrowser.opened){
                                        if(m->modelPath[0] != '\0'){
                                            OpenFileBrowser(0,GetVoxelModelPointer(GetElementAsType(GetFirstCell(SelectedEntities),EntityID))->modelPath,FBLoadModel);
                                        }else{
                                            OpenFileBrowser(0,NULL,FBLoadModel);
                                        }
                                        FileBrowserExtension("vox");
                                    }
                                }
                                
                                if(m->modelName[0] != '\0'){
                                    DrawTextColored(m->modelName, lightWhite, Screen.windowWidth-componentWindowLength + 28, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                                }else{
                                    DrawTextColored("No model", lightWhite, Screen.windowWidth-componentWindowLength + 28, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                                }
                                componentHeight-=22;

                                if(isSubModel){
                                    DrawTextColored(m->objectName, lightWhite, Screen.windowWidth-componentWindowLength + 25, componentHeight - 6 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                                    componentHeight-=22;
                                }
                            }
                            
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

                            Vector3Field("Center",&newCenter,10,ommitCenterX,ommitCenterY,ommitCenterZ,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1,4, &currentComponentField, &componentHeight);
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
                        }else if(c == GetComponentID("PointLight")){

                            //Component background
                            Vector3 bgMin = {Screen.windowWidth-componentWindowLength, componentHeight-254,0};
                            Vector3 bgMax = {Screen.windowWidth-componentWindowWidthSpacing, componentHeight,0};
                            DrawRectangle(bgMin, bgMax, bgMediumColor.x, bgMediumColor.y, bgMediumColor.z);

                            int ommitColorX = 0, ommitColorY = 0, ommitColorZ = 0;
                            int ommitIntensity = 0, ommitRange = 0, ommitShift = 0;

                            ListCellPointer selEntity = GetFirstCell(SelectedEntities);
                            Vector3 lastColor = GetPointLightColor(GetElementAsType(selEntity,int));
                            float lastIntensity = GetPointLightIntensity(GetElementAsType(selEntity,int));
                            float lastRange = GetPointLightRange(GetElementAsType(selEntity,int));
                            float lastShift = GetPointLightHueShift(GetElementAsType(selEntity,int));
                            
                            ListForEach(selEntity, SelectedEntities){
                                Vector3 curColor = GetPointLightColor(GetElementAsType(selEntity,int));
                                float curIntensity = GetPointLightIntensity(GetElementAsType(selEntity,int));
                                float curRange = GetPointLightRange(GetElementAsType(selEntity,int));
                                float curShift = GetPointLightHueShift(GetElementAsType(selEntity,int));

                                if(curColor.x != lastColor.x){
                                    ommitColorX = 1;
                                    lastColor.x = 0;
                                }
                                if(curColor.y != lastColor.y){
                                    ommitColorY = 1;
                                    lastColor.y = 0;
                                }
                                if(curColor.z != lastColor.z){
                                    ommitColorZ = 1;
                                    lastColor.z = 0;
                                }

                                if(curIntensity != lastIntensity){
                                    ommitIntensity = 1;
                                    lastIntensity = 0;
                                }
                                if(curRange != lastRange){
                                    ommitRange = 1;
                                    lastRange = 0;
                                }
                                if(curShift != lastShift){
                                    ommitShift = 1;
                                    lastShift = 0;
                                }
                            }
                            Vector3 newColor = lastColor;
                            float newIntensity = lastIntensity;
                            float newRange = lastRange;
                            float newShift = lastShift;

                            RGBField("Light color", &newColor,ommitColorX,ommitColorY,ommitColorZ, Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1, &currentComponentField, &componentHeight);
                            SliderField("Hue Shift",&newShift,(Vector3){0,8*PI}, ommitShift,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1, &currentComponentField, &componentHeight);
                            FloatField("Intensity",&newIntensity,100,ommitIntensity,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1, &currentComponentField, &componentHeight);
                            FloatField("Range",&newRange,10,ommitRange,Screen.windowWidth-componentWindowLength + componentNameLeftSpacing, componentWindowLength-componentWindowWidthSpacing-componentNameLeftSpacing*2 +1, &currentComponentField, &componentHeight);

                            int changedColorX = 0, changedColorY = 0, changedColorZ = 0;
                            int changedIntensity = 0, changedRange = 0, changedShift = 0;

                            if(lastColor.x != newColor.x) changedColorX = 1;
                            if(lastColor.y != newColor.y) changedColorY = 1;
                            if(lastColor.z != newColor.z) changedColorZ = 1;

                            if(lastIntensity != newIntensity) changedIntensity = 1;
                            if(lastRange != newRange) changedRange = 1;
                            if(lastShift != newShift) changedShift = 1;
                            
                            if(changedColorX || changedColorY || changedColorZ || changedIntensity || changedRange || changedShift){
                                ListForEach(selEntity, SelectedEntities){
                                    Vector3 color = GetPointLightColor(GetElementAsType(selEntity,int));

                                    if(changedColorX) color.x = newColor.x;
                                    if(changedColorY) color.y = newColor.y;
                                    if(changedColorZ) color.z = newColor.z;

                                    SetPointLightColor(GetElementAsType(selEntity,int),color);

                                    if(changedIntensity) SetPointLightIntensity(GetElementAsType(selEntity,int),newIntensity);
                                    if(changedRange) SetPointLightRange(GetElementAsType(selEntity,int),newRange);
                                    if(changedShift) SetPointLightHueShift(GetElementAsType(selEntity,int),newShift);
                                }
                            }
                        componentHeight -= 7;
                        }else if(c == GetComponentID("LuaScript")){
                            
                            int backgroundHeight = (GetLength(SelectedEntities)==1? 25:0);

                            //Component background
                            Vector3 bgMin = {Screen.windowWidth-componentWindowLength, componentHeight-backgroundHeight,0};
                            Vector3 bgMax = {Screen.windowWidth-componentWindowWidthSpacing, componentHeight,0};
                            DrawRectangle(bgMin, bgMax, bgMediumColor.x, bgMediumColor.y, bgMediumColor.z);

                            //Only show the script load button and name if only one entity is selected
                            if(GetLength(SelectedEntities)==1){
                                EntityID ent = GetElementAsType(GetFirstCell(SelectedEntities),EntityID);
                                
                                if(1 == PointButton((Vector3){Screen.windowWidth-componentWindowLength + 15,componentHeight - 12},10,1, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1}, (Vector3){0.8,0.8,0.8})){
                                    if(!fileBrowser.opened){
                                        char *scriptPath = GetLuaScriptPath(ent);
                                        if(scriptPath[0] != '\0'){
                                            OpenFileBrowser(0,scriptPath,FBLoadScript);
                                        }else{
                                            OpenFileBrowser(0,NULL,FBLoadScript);
                                        }
                                        FileBrowserExtension("lua");
                                    }
                                }

                                char *scriptName = GetLuaScriptName(ent);
                                if(scriptName[0] != '\0'){
                                    DrawTextColored(scriptName, lightWhite, Screen.windowWidth-componentWindowLength + 28, componentHeight - 8 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                                }else{
                                    DrawTextColored("No script", lightWhite, Screen.windowWidth-componentWindowLength + 28, componentHeight - 8 - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
                                }
                                componentHeight-=22;
                            }
                            
                            componentHeight -= 7;
                        }
                    }
                    componentHeight -= componentBetweenSpacing;
                }
            }
            
            //Scrollbar
            
            int mouseOverComponentPanel = MouseOverBox(mousePos, (Vector3){Screen.windowWidth-componentWindowLength,0,0}, (Vector3){Screen.windowWidth,ComponentsPanelHeight,0},0);
            int offscreenPixels = -clamp(componentHeight-componentStartHeight-componentWindowBottomSpacing,-(ComponentsPanelHeight-10),0);

            Vector3 scrollbarStart = {Screen.windowWidth - componentWindowWidthSpacing/2 ,ComponentsPanelHeight-componentStartHeight -2,0};
            Vector3 scrollbarEnd = {Screen.windowWidth - componentWindowWidthSpacing/2 ,offscreenPixels - componentStartHeight +componentWindowBottomSpacing+ 1,0};
            Vector3 scrollbarColor = {scrollbarInactiveColor.x, scrollbarInactiveColor.y, scrollbarInactiveColor.z};

            if(mouseOverComponentPanel){
                if(!movingComponentsScrollbar){
                    if(MouseOverLine(mousePos, scrollbarStart, scrollbarEnd, scrollbarMouseOverDistance)){
                        scrollbarColor = (Vector3){scrollbarOverColor.x, scrollbarOverColor.y, scrollbarOverColor.z};

                        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                            movingComponentsScrollbar = 1;
                        }
                    }else{
                        scrollbarColor = (Vector3){buttonOverColor.x, buttonOverColor.y, buttonOverColor.z};
                    }
                }else if (movingComponentsScrollbar == 1){
                    scrollbarColor = (Vector3){0.55,0.55,0.55};
                }else{
                    scrollbarColor = (Vector3){buttonOverColor.x, buttonOverColor.y, buttonOverColor.z};
                }
            }

            //Scrollbar bar
            float scrollbarWidth = clamp((componentWindowWidthSpacing/2 - 2),1,Screen.windowWidth);
            DrawRectangle((Vector3){scrollbarEnd.x-scrollbarWidth, scrollbarEnd.y},
                          (Vector3){scrollbarStart.x+scrollbarWidth, scrollbarStart.y},
                          scrollbarColor.x, scrollbarColor.y, scrollbarColor.z);

            if(Input.mouseWheelY!=0) movingComponentsScrollbar = 2;
            if(movingComponentsScrollbar){

                //Moving with mouse click
                if(movingComponentsScrollbar == 1){
                    if(GetMouseButton(SDL_BUTTON_LEFT)){
                        if(offscreenPixels>0){
                            double scrollbarMovement = norm(VectorProjection(deltaMousePos,(Vector3){0,1,0})) * sign(deltaMousePos.y);
                            //Skips one pixel if the value is odd, to avoid distortions when downscaling to the game resolution
                            scrollbarMovement += sign(scrollbarMovement) * (abs(scrollbarMovement)%2? 1:0);
                            componentStartHeight = clamp(componentStartHeight-scrollbarMovement,0,offscreenPixels);
                        }else{
                            componentStartHeight = 0;
                        }
                    }else{
                        movingComponentsScrollbar = 0;
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
                        movingComponentsScrollbar = 0;
                    }
                }
            }
        }else{
            //Show components to be added
            int buttonHeight = ComponentsPanelHeight - (addComponentScroll>0? 22:0);
            int w,h,i = 0;
            ListCellPointer cellComp;
            ListForEach(cellComp,ECS.ComponentTypes){
                if(i-addComponentScroll<0 || buttonHeight<componentWindowBottomSpacing ){i++; continue;}

                Vector3 cbMin = {Screen.windowWidth - componentWindowLength+1,buttonHeight-40};
                Vector3 cbMax = {Screen.windowWidth-2,buttonHeight};
                if(!MaskContainsComponent(mask,i)){
                    if(MouseOverBox(mousePos,cbMin,cbMax,0)){
                        DrawRectangle(cbMin, cbMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
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
                        DrawRectangle(cbMin, cbMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);
                    }
                }else{
                    DrawRectangle(cbMin, cbMax,0.1,0.1,0.1);
                }
                TTF_SizeText(gizmosFont,GetElementAsType(cellComp,ComponentType).name,&w,&h);
                DrawTextColored(GetElementAsType(cellComp,ComponentType).name, lightWhite, cbMin.x + ((cbMax.x-cbMin.x)-w)/2, cbMin.y + ((cbMax.y-cbMin.y)-h)/2, gizmosFont);

                buttonHeight-= 42;
                i++;
            }

            //Scrollbar
            //'i' is now the number of component types
            if(GetLength(ECS.ComponentTypes) > 16){
                Vector3 scrollbarDownMin = {Screen.windowWidth - componentWindowLength,componentWindowBottomSpacing+2};
                Vector3 scrollbarDownMax = {Screen.windowWidth,componentWindowBottomSpacing+22};
                Vector3 scrollbarUpMin = {Screen.windowWidth - componentWindowLength,ComponentsPanelHeight-22};
                Vector3 scrollbarUpMax = {Screen.windowWidth,ComponentsPanelHeight-2};

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
                fileBrowser.itemsScroll = 0;
            }
        }

        //Line separating the add component buttom from the components
        //Above button
        DrawLine((Vector3){Screen.windowWidth - componentWindowLength,componentWindowBottomSpacing+2},
                 (Vector3){Screen.windowWidth,componentWindowBottomSpacing+2},
                 2,bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);  
        //Below button  
        DrawLine((Vector3){Screen.windowWidth - componentWindowLength,1},
                 (Vector3){Screen.windowWidth,1},
                 2,bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);

        //Add component button

        Vector3 cbMin = {Screen.windowWidth - componentWindowLength,2,0};
        Vector3 cbMax = {Screen.windowWidth-2,componentWindowBottomSpacing,0};
        Vector3 cbColor = {bgLightColor.x, bgLightColor.y, bgLightColor.z};
        if(MouseOverBox(mousePos, cbMin, cbMax,0)){
            cbColor = (Vector3){buttonOverColor.x, buttonOverColor.y, buttonOverColor.z};

            if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                addComponentWindowOpened = !addComponentWindowOpened;
            }
        }
        DrawRectangle(cbMin, cbMax, cbColor.x, cbColor.y, cbColor.z);

        int w,h;
        if(!addComponentWindowOpened){
            TTF_SizeText(gizmosFont,"Add Component",&w,&h);
            DrawTextColored("Add Component", brightWhite, cbMin.x + ((cbMax.x-cbMin.x)-w)/2, cbMin.y + ((cbMax.y-cbMin.y)-h)/2, gizmosFont);
        }else{
            TTF_SizeText(gizmosFont,"Back",&w,&h);
            DrawTextColored("Back", brightWhite, cbMin.x + ((cbMax.x-cbMin.x)-w)/2, cbMin.y + ((cbMax.y-cbMin.y)-h)/2, gizmosFont);
        }

        
        //Prefab info and options
        if(singlePrefabSelected){
            EntityID entity = GetElementAsType(GetFirstCell(SelectedEntities),EntityID);

            Vector3 prefabBgMin = {Screen.windowWidth-componentWindowLength, ComponentsPanelHeight};
            Vector3 prefabBgMax = {Screen.windowWidth, Screen.windowHeight};
            //Draw the dark background
            DrawRectangle(prefabBgMin,prefabBgMax,bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);

            prefabBgMin.y += 2;
            //Draw the light background above the other
            DrawRectangle(prefabBgMin,prefabBgMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);

            //Draw the prefab name
            TTF_SizeText(gizmosFontSmall, GetPrefabName(entity), &w, &h);
            DrawTextColored(GetPrefabName(entity), brightWhite,prefabBgMin.x + (prefabBgMax.x - prefabBgMin.x - w)/2, prefabBgMin.y + (prefabBgMax.y - prefabBgMin.y - TTF_FontHeight(gizmosFontSmall))*9/10, gizmosFontSmall);


            Vector3 buttonPos = {Screen.windowWidth-componentWindowLength + componentNameLeftSpacing + 2 +iconsSize[7],
                                prefabBgMin.y + (prefabBgMax.y - prefabBgMin.y)*1.25/4};

            //Reload prefab button
            if(PointButton(buttonPos,7, 1, (Vector3){0.9,0.9,0.9}, (Vector3){0.2,1,0.2}, (Vector3){0.5,1,0.5}) == 1){
                EntityID reloadedPrefab = ImportEntityPrefab(GetPrefabPath(entity),GetPrefabName(entity));
                DestroyEntity(entity);
                
                FreeList(&SelectedEntities);
                InsertListStart(&SelectedEntities, &reloadedPrefab);
                entity = reloadedPrefab;
            }
            //Save prefab button
            buttonPos.x += iconsSize[7] + 10 + iconsSize[19];
            if(PointButton(buttonPos,19, 1, (Vector3){0.9,0.9,0.9}, (Vector3){0.2,1,0.2}, (Vector3){0.5,1,0.5}) == 1){
                ExportEntityPrefab(entity,GetPrefabPath(entity),GetPrefabName(entity));
            }
        }
    }
}

//Return 0 if removed, 1 if not
int DrawComponentHeader(ComponentID component, int* curHeight){
    
    //Top panel
    Vector3 panelMin = {Screen.windowWidth - componentWindowLength,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 panelMax = {Screen.windowWidth - componentWindowWidthSpacing,*curHeight,0};
    Vector3 panelColor = {bgLightColor.x, bgLightColor.y, bgLightColor.z};
    DrawRectangle(panelMin, panelMax, panelColor.x, panelColor.y, panelColor.z);

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
        DrawTextColored(cName, brightWhite, Screen.windowWidth-componentWindowLength+componentNameLeftSpacing, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);
    }else{
        DrawTextColored(type->name, brightWhite, Screen.windowWidth-componentWindowLength+componentNameLeftSpacing, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);
    }

    *curHeight -= TTF_FontHeight(gizmosFont)+2;
    return 1;
}

void FBLoadModel(){
    if(IsMultiVoxelModelFile(fileBrowser.filePath,fileBrowser.fileName)){
        LoadMultiVoxelModel(GetElementAsType(GetFirstCell(SelectedEntities),EntityID),fileBrowser.filePath,fileBrowser.fileName);
    }else{
        LoadVoxelModel(GetElementAsType(GetFirstCell(SelectedEntities),EntityID),fileBrowser.filePath,fileBrowser.fileName);
    }
}

void FBLoadScript(){
    SetLuaScript( GetElementAsType(GetFirstCell(SelectedEntities),EntityID) , fileBrowser.filePath, fileBrowser.fileName);
}
