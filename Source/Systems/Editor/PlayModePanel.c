#include "PlayModePanel.h"

/////////////////////////////////External data//////////////////////////////////

//Engine data
extern engineScreen Screen;
extern engineECS ECS;

//Data from EditorUI.c
extern int iconsSize[];

//Color definitions from Editor.c
extern Vector3 bgMediumColor;
extern Vector3 scrollbarOverColor;

//Data from Editor.c
extern List SelectedEntities;

/////////////////////////////////////////////////////////////////////////////////

static SystemID VoxPhysicsSystem;
static SystemID LuaSystem;

//Copy of the components and entities data to be reseted when exiting play mode
Component** componentsPlaymodeCopy;
Entity* entitiesPlaymodeCopy;

void AllocatePlayModeData(){

    //Cache the ID of the dynamic systems to be disabled
    VoxPhysicsSystem = GetSystemID("VoxelPhysics");
    LuaSystem = GetSystemID("LuaSystem");

    int i;
    //Allocate the playmode components and entities copy
    componentsPlaymodeCopy = malloc(GetLength(ECS.ComponentTypes) * sizeof(Component*));
    for(i=0;i<GetLength(ECS.ComponentTypes);i++){
        componentsPlaymodeCopy[i] = calloc(ECS.maxEntities,sizeof(Component));
    }

    entitiesPlaymodeCopy = calloc(ECS.maxEntities,sizeof(Entity));
}

void FreePlayModeData(){
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
}

//0 = Editor, 1 = Play, 2 = Paused
int playMode = 0;
void DrawPlayModeWidget(){
    int iconSize = max(max(iconsSize[3],iconsSize[4]),iconsSize[5]);
    Vector3 playBGMin = (Vector3){Screen.windowWidth/2 - iconSize * 4  -2,  Screen.windowHeight-iconSize * 4  -1 };
    Vector3 playBGMax = (Vector3){Screen.windowWidth/2 + iconSize * 4  +2,  Screen.windowHeight};

    if(playMode == 1){
        //Play mode
        DrawRectangle(playBGMin,playBGMax,scrollbarOverColor.x, scrollbarOverColor.y, scrollbarOverColor.z);

        //Pause button
        if(1 == PointButton((Vector3){Screen.windowWidth/2 - iconSize * 2 -2,  Screen.windowHeight-iconSize * 2 -1 },4, 2, (Vector3){0.1,0.1,0.1}, (Vector3){1,1,1}, (Vector3){1,1,1})){
            playMode = 2;
            DisablePlayModeSystems();
        }
        //Stop button
        if(1 == PointButton((Vector3){Screen.windowWidth/2 + iconSize * 2 +2,  Screen.windowHeight-iconSize * 2 -1 },5, 2, (Vector3){0.1,0.1,0.1}, (Vector3){1,1,1}, (Vector3){1,1,1})){
            playMode = 0;
            ExitPlayMode();

        }
    }else if(playMode == 2){
        //Paused
        DrawRectangle(playBGMin,playBGMax,scrollbarOverColor.x, scrollbarOverColor.y, scrollbarOverColor.z);

        //Play button
        if(1 == PointButton((Vector3){Screen.windowWidth/2 - iconSize * 2 -2,  Screen.windowHeight-iconSize * 2 -1 },3, 2, (Vector3){0.1,0.1,0.1}, (Vector3){1,1,1}, (Vector3){1,1,1})){
            playMode = 1;
            EnablePlayModeSystems();
        }
        //Stop button enabled
        if(1 == PointButton((Vector3){Screen.windowWidth/2 + iconSize * 2 +2,  Screen.windowHeight-iconSize * 2 -1 },5, 2, (Vector3){0.1,0.1,0.1}, (Vector3){1,1,1}, (Vector3){1,1,1})){
            playMode = 0;
            ExitPlayMode();
        }
    }else{
        //In editor
        DrawRectangle(playBGMin,playBGMax,bgMediumColor.x, bgMediumColor.y, bgMediumColor.z);

        //Play button
        if(1 == PointButton((Vector3){Screen.windowWidth/2 - iconSize * 2 -2,  Screen.windowHeight-iconSize * 2 -1 },3, 2, (Vector3){0.7,0.7,0.7}, (Vector3){1,1,1}, (Vector3){1,1,1})){
            playMode = 1;
            EnterPlayMode();
            
        }
        //Stop button disabled
        PointButton((Vector3){Screen.windowWidth/2 + iconSize * 2 +2,  Screen.windowHeight-iconSize * 2 -1 },5, 2, (Vector3){0.2,0.2,0.2}, (Vector3){0.2,0.2,0.2}, (Vector3){0.2,0.2,0.2});
    }
}



//Function responsible for copying the game state to another structure before starting the play mode
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

    EnablePlayModeSystems();
    
}

//Function responsible to restore the state of the game stored before starting the play mode
void ExitPlayMode(){
    int c,i;
    for(i=0;i<ECS.maxEntities;i++){
        //Destroy game modified entity before recreate
        if(IsValidEntity(i)){
            //This code is similar to the DestroyEntity function, but without destroying the child objects
            int mask = ECS.Entities[i].mask.mask;
            c = 0;
            ListCellPointer compCell;
            ListForEach(compCell,ECS.ComponentTypes){
                if(mask & 1){
                    ((ComponentType*)(GetElement(*compCell)))->destructor(&ECS.Components[c][i].data);
                }
                mask >>=1;
                c++;
            }

            ECS.Entities[i].mask.mask = 0;
            ECS.Entities[i].isSpawned = 0;
            FreeList(&ECS.Entities[i].childs);
            ECS.Entities[i].isChild = 0;
            ECS.Entities[i].isParent = 0;

            InsertListStart(&ECS.AvaliableEntitiesIndexes, (void*)&i);
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

    //Clear selected entities list
    FreeList(&SelectedEntities);

    DisablePlayModeSystems();
}

void EnablePlayModeSystems(){
    //Enable all dynamic game systems
    int i;
    for(i=0;i<GetLength(ECS.SystemList);i++){
        if(i == VoxPhysicsSystem || i == LuaSystem){
            EnableSystem(i);
        }
    }
}

void DisablePlayModeSystems(){
    //Disable all dynamic game systems
    int i;
    for(i=0;i<GetLength(ECS.SystemList);i++){
        if(i == VoxPhysicsSystem || i == LuaSystem){
            DisableSystem(i);
        }
    }
}