#include "MenuWindow.h"

/////////////////////////////////External data//////////////////////////////////

//Engine data
extern engineScreen Screen;
extern engineECS ECS;
extern engineScene Scene;

//Data from EditorUI.c
extern int iconsSize[];
extern TTF_Font* gizmosFont;
extern TTF_Font* gizmosFontSmall;

//Color definitions from Editor.c
extern Vector3 bgPanelColor;
extern Vector3 bgLightColor;
extern Vector3 bgMediumColor;
extern Vector3 menuActiveTabColor;
extern Vector3 menuTabColor;
extern Vector3 buttonOverColor;
extern Vector3 scrollbarOverColor;
extern Vector3 brightWhite;
extern Vector3 lightWhite;

//Data from Editor.c
extern Vector3 mousePos;
extern List SelectedEntities;

//Data from FileBrowser.c
extern FileBrowserData fileBrowser;

/////////////////////////////////////////////////////////////////////////////////

void FBLoadScene();
void FBImportPrefab();
void FBImportSceneEntities();
void FBSaveScene();
void FBExportPrefab();

void NewSceneDontSaveOption();
void NewSceneSaveOption();
void NewSceneCancelOption();
void NewSceneSaveScene();


int menuOpened = 0;

int selectedTab = 0;
int currentField = -1;
void DrawMenuWindow(){
    Vector3 bgMin = {Screen.windowWidth/2 -300,Screen.windowHeight/2 -100};
    Vector3 bgMax = {Screen.windowWidth/2 +300,Screen.windowHeight/2 +100};
    Vector3 optionsBgMin = {Screen.windowWidth/2 -300,Screen.windowHeight/2 -100};
    Vector3 optionsBgMax = {Screen.windowWidth/2 +300,Screen.windowHeight/2 +35};
    Vector3 headerMin = {Screen.windowWidth/2 -300,Screen.windowHeight/2 +70};
    Vector3 headerMax = {Screen.windowWidth/2 +300,Screen.windowHeight/2 +100};
    DrawRectangle(bgMin,bgMax,bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);
    DrawRectangle(headerMin,headerMax,bgMediumColor.x, bgMediumColor.y, bgMediumColor.z);
    DrawRectangle(optionsBgMin,optionsBgMax,menuActiveTabColor.x, menuActiveTabColor.y, menuActiveTabColor.z);

    if(PointButton((Vector3){headerMax.x - iconsSize[8] - 6,headerMin.y+(headerMax.y - headerMin.y)/2,0},18, 1, (Vector3){0.75,0.75,0.75}, (Vector3){1,0.2,0.2}, (Vector3){scrollbarOverColor.x, scrollbarOverColor.y, scrollbarOverColor.z}) == 1){
        menuOpened = 0;
        return;
    }

    int w,h;
    TTF_SizeText(gizmosFont,"Menu",&w,&h);
    DrawTextColored("Menu", lightWhite, headerMin.x + 10, headerMin.y+ ((headerMax.y-headerMin.y)-h)/2, gizmosFont);

    Vector3 tabMin = {Screen.windowWidth/2 -300,Screen.windowHeight/2 +35};
    Vector3 tabMax = {Screen.windowWidth/2 -300,Screen.windowHeight/2 +65};
    int tabIndex;
    for(tabIndex = 0; tabIndex<3;tabIndex++){
        switch(tabIndex){
            case 0: TTF_SizeText(gizmosFont,"Scene",&w,&h);
            break;
            case 1: TTF_SizeText(gizmosFont,"Entities",&w,&h);
            break;
            case 2: TTF_SizeText(gizmosFont,"Options",&w,&h);
            break;
        }

        tabMax.x += w + 6;

        if(selectedTab == tabIndex){
            DrawRectangle(tabMin,tabMax,menuActiveTabColor.x, menuActiveTabColor.y, menuActiveTabColor.z);
        }else{
            if(MouseOverBox(mousePos,tabMin,tabMax,0)){
                DrawRectangle(tabMin,tabMax,bgMediumColor.x, bgMediumColor.y, bgMediumColor.z);
                if(GetMouseButtonUp(SDL_BUTTON_LEFT)){
                    currentField = -1;
                    selectedTab = tabIndex;
                }
            }else{
                DrawRectangle(tabMin,tabMax,menuTabColor.x, menuTabColor.y, menuTabColor.z);
            }
        }

        //Tabs title
        switch(tabIndex){
            case 0:
                DrawTextColored("Scene", lightWhite, tabMin.x + 3, tabMin.y+ ((tabMax.y-tabMin.y)-h)/2, gizmosFont);
            break;
            case 1:
                DrawTextColored("Entities", lightWhite, tabMin.x + 3, tabMin.y+ ((tabMax.y-tabMin.y)-h)/2, gizmosFont);
            break;
            case 2:
                DrawTextColored("Options", lightWhite, tabMin.x + 3, tabMin.y+ ((tabMax.y-tabMin.y)-h)/2, gizmosFont);
            break;
        }

        tabMin.x += w + 8;
        tabMax.x += 2;

        //Tabs content
        int btw,bth,spacing,i;
        Vector3 bMin;
        Vector3 bMax;
        switch(selectedTab){
            case 0:
                DrawTextColored("file", lightWhite, optionsBgMin.x + 10, optionsBgMax.y-20, gizmosFontSmall);

                //New Scene button
                bMin = (Vector3){optionsBgMin.x + 10,optionsBgMax.y-55};
                bMax = (Vector3){optionsBgMin.x + 160,optionsBgMax.y-25};
                if(MouseOverBox(mousePos, bMin, bMax,0)){
                    DrawRectangle(bMin,bMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
                    if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                        //Check if scene is empty
                        int i, sceneIsEmpty = 1;
                        for(i=0; i<=ECS.maxUsedIndex; i++){
                            if(IsValidEntity(i)){
                                sceneIsEmpty = 0;
                                break;
                            }
                        }
                        if(sceneIsEmpty){
                            //Just close the menu, as there is no need to clear the current scene
                            menuOpened = 0;
                        }else{
                            OpenDialogWindow("Do you want to save\nthe scene first?", "Save", "Don't save", "Cancel", &NewSceneSaveOption, &NewSceneDontSaveOption, &NewSceneCancelOption);
                        }

                    }
                }else{
                    DrawRectangle(bMin,bMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);
                }
                TTF_SizeText(gizmosFont,"New Scene",&btw,&bth);
                DrawTextColored("New Scene", brightWhite, bMin.x+ ((bMax.x-bMin.x)-btw)/2, bMin.y+ ((bMax.y-bMin.y)-bth)/2, gizmosFont);

                //Open Scene button
                spacing = bMax.y-bMin.y + 4;
                bMin.y -= spacing;
                bMax.y -= spacing;
                if(MouseOverBox(mousePos, bMin, bMax,0)){
                    DrawRectangle(bMin,bMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
                    if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                        if(Scene.path[0] != '\0'){
                            OpenFileBrowser(0,Scene.path,FBLoadScene);
                        }else{
                            OpenFileBrowser(0,NULL,FBLoadScene);
                        }
                        FileBrowserExtension("scene");
                    }
                }else{
                    DrawRectangle(bMin,bMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);
                }
                TTF_SizeText(gizmosFont,"Open Scene",&btw,&bth);
                DrawTextColored("Open Scene", brightWhite, bMin.x+ ((bMax.x-bMin.x)-btw)/2, bMin.y+ ((bMax.y-bMin.y)-bth)/2, gizmosFont);

                //Save Scene
                spacing = bMax.y-bMin.y + 4;
                bMin.y -= spacing;
                bMax.y -= spacing;
                if(MouseOverBox(mousePos, bMin, bMax,0)){
                    DrawRectangle(bMin,bMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
                    if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                        if(Scene.path[0] != '\0'){
                            OpenFileBrowser(1,Scene.path,FBSaveScene);
                        }else{
                            OpenFileBrowser(1,NULL,FBSaveScene);
                        }
                        FileBrowserExtension("scene");
                        
                    }
                }else{
                    DrawRectangle(bMin,bMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);
                }
                TTF_SizeText(gizmosFont,"Save Scene",&btw,&bth);
                DrawTextColored("Save Scene", brightWhite, bMin.x+ ((bMax.x-bMin.x)-btw)/2, bMin.y+ ((bMax.y-bMin.y)-bth)/2, gizmosFont);

                //Line separating the buttons category
                DrawRectangle((Vector3){bMax.x + 10,bMin.y}, (Vector3){bMax.x + 12,optionsBgMax.y-10},bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);
                
                //Add some spacing between different categories
                spacing = bMax.x-bMin.x + 22;
                bMin.x += spacing;
                bMax.x += spacing * 9.0/10.0;

                //Reduce the width of the columns
                spacing *= 8.0/10.0;

                //Reset button height to top
                bMin.y = optionsBgMax.y-55;
                bMax.y = optionsBgMax.y-25;

                DrawTextColored("settings", lightWhite, bMin.x, optionsBgMax.y-20, gizmosFontSmall);

                currentField++;
                int currentHeight = bMax.y-50;
                DrawTextColored("Sun direction", lightWhite, bMin.x, optionsBgMax.y-60, gizmosFontSmall);

                Vector3 sunDir = GetTrieElementAs_Vector3(Scene.data, "sunDirection", (Vector3){0.75,0.2,-1.5});
                Vector3 sunDirOld = sunDir;
                XYZSliderField("X","Y","Z", &sunDir, (Vector3){-1,1},0,0,0, bMin.x, bMax.x - bMin.x, &currentField, &currentHeight);
                if(sunDir.x != sunDirOld.x || sunDir.y != sunDirOld.y || sunDir.z != sunDirOld.z){
                    InsertTrie(&Scene.data, "sunDirection", sunDir);
                }


                bMin.x += spacing;
                bMax.x += spacing;
                currentHeight = optionsBgMax.y-10;

                Vector3 sunColor = GetTrieElementAs_Vector3(Scene.data, "sunColor", (Vector3){1,1,1});
                Vector3 sunColorOld = sunColor;
                RGBField("Sun color", &sunColor, 0,0,0, bMin.x , bMax.x - bMin.x, &currentField, &currentHeight);
                if(sunColor.x != sunColorOld.x || sunColor.y != sunColorOld.y || sunColor.z != sunColorOld.z){
                    InsertTrie(&Scene.data, "sunColor", sunColor);
                }

                bMin.x += spacing;
                bMax.x += spacing;
                currentHeight = optionsBgMax.y-10;

                Vector3 bgColor = GetTrieElementAs_Vector3(Scene.data, "backgroundColor", (Vector3){0.01,0.01,0.01});
                Vector3 bgColorOld = bgColor;
                RGBField("Background col", &bgColor, 0,0,0, bMin.x , bMax.x - bMin.x, &currentField, &currentHeight);
                if(bgColor.x != bgColorOld.x || bgColor.y != bgColorOld.y || bgColor.z != bgColorOld.z){
                    InsertTrie(&Scene.data, "backgroundColor", bgColor);
                }

            break;
            case 1:
                DrawTextColored("file", lightWhite, optionsBgMin.x + 10, optionsBgMax.y-20, gizmosFontSmall);

                //Export selected button
                bMin = (Vector3){optionsBgMin.x + 10,optionsBgMax.y-55};
                bMax = (Vector3){optionsBgMin.x + 270,optionsBgMax.y-25};
                if(!IsListEmpty(SelectedEntities)){
                    if(MouseOverBox(mousePos, bMin, bMax,0)){
                        DrawRectangle(bMin,bMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
                        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                            OpenFileBrowser(1,NULL,FBExportPrefab);
                            FileBrowserExtension("prefab");
                        }
                    }else{
                        DrawRectangle(bMin,bMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);
                    }
                }else{
                    DrawRectangle(bMin,bMax,0.2,0.2,0.2);
                }
                TTF_SizeText(gizmosFont,"Export selected",&btw,&bth);
                DrawTextColored("Export selected", brightWhite, bMin.x+ ((bMax.x-bMin.x)-btw)/2, bMin.y+ ((bMax.y-bMin.y)-bth)/2, gizmosFont);

                //Import prefab button
                spacing = bMax.y-bMin.y + 4;
                bMin.y -= spacing;
                bMax.y -= spacing;
                if(MouseOverBox(mousePos, bMin, bMax,0)){
                    DrawRectangle(bMin,bMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
                    if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                        OpenFileBrowser(0,NULL,FBImportPrefab);
                        FileBrowserExtension("prefab");
                    }
                }else{
                    DrawRectangle(bMin,bMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);
                }
                TTF_SizeText(gizmosFont,"Import prefab",&btw,&bth);
                DrawTextColored("Import prefab", brightWhite, bMin.x+ ((bMax.x-bMin.x)-btw)/2, bMin.y+ ((bMax.y-bMin.y)-bth)/2, gizmosFont);

                //Import scene entities button
                spacing = bMax.y-bMin.y + 4;
                bMin.y -= spacing;
                bMax.y -= spacing;
                if(MouseOverBox(mousePos, bMin, bMax,0)){
                    DrawRectangle(bMin,bMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
                    if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                        if(Scene.path[0] != '\0'){
                            OpenFileBrowser(0,Scene.path,FBImportSceneEntities);
                        }else{
                            OpenFileBrowser(0,NULL,FBImportSceneEntities);
                        }
                        FileBrowserExtension("scene");
                    }
                }else{
                    DrawRectangle(bMin,bMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);
                }
                TTF_SizeText(gizmosFont,"Import scene entities",&btw,&bth);
                DrawTextColored("Import scene entities", brightWhite, bMin.x+ ((bMax.x-bMin.x)-btw)/2, bMin.y+ ((bMax.y-bMin.y)-bth)/2, gizmosFont);

                //Line separating the buttons category
                DrawRectangle((Vector3){bMax.x + 10,bMin.y}, (Vector3){bMax.x + 12,optionsBgMax.y-10},bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);
                
                //Add some spacing between different categories and change button width
                spacing = bMax.x-bMin.x + 22;
                bMin.x = optionsBgMin.x + 10 + spacing;
                bMax.x = optionsBgMin.x + 220 + spacing;
                
                //Reset button height to top
                bMin.y = optionsBgMax.y-55;
                bMax.y = optionsBgMax.y-25;

                DrawTextColored("Selection", lightWhite, bMin.x, optionsBgMax.y-20, gizmosFontSmall);

                //Select all button
                if(MouseOverBox(mousePos, bMin, bMax,0)){
                    DrawRectangle(bMin,bMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
                    if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                        //Select all entities
                        FreeList(&SelectedEntities);
                        for(i=0;i<=ECS.maxUsedIndex;i++){
                            if(IsValidEntity(i)){
                                InsertListEnd(&SelectedEntities,&i);
                            }
                        }
                    }
                }else{
                    DrawRectangle(bMin,bMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);
                }
                TTF_SizeText(gizmosFont,"Select all",&btw,&bth);
                DrawTextColored("Select all", brightWhite, bMin.x+ ((bMax.x-bMin.x)-btw)/2, bMin.y+ ((bMax.y-bMin.y)-bth)/2, gizmosFont);

                //Deselect all button
                spacing = bMax.y-bMin.y + 4;
                bMin.y -= spacing;
                bMax.y -= spacing;
                if(MouseOverBox(mousePos, bMin, bMax,0)){
                    DrawRectangle(bMin,bMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
                    if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                        //Deselect entities
                        FreeList(&SelectedEntities);
                    }
                }else{
                    DrawRectangle(bMin,bMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);
                }
                TTF_SizeText(gizmosFont,"Deselect all",&btw,&bth);
                DrawTextColored("Deselect all", brightWhite, bMin.x+ ((bMax.x-bMin.x)-btw)/2, bMin.y+ ((bMax.y-bMin.y)-bth)/2, gizmosFont);

                //Remove selected button
                spacing = bMax.y-bMin.y + 4;
                bMin.y -= spacing;
                bMax.y -= spacing;
                if(MouseOverBox(mousePos, bMin, bMax,0)){
                    DrawRectangle(bMin,bMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
                    if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                        //Remove selected entities
                        ListCellPointer sEntity;
                        ListForEach(sEntity,SelectedEntities){
                            DestroyEntity(GetElementAsType(sEntity,EntityID));
                        }
                        FreeList(&SelectedEntities);
                    }
                }else{
                    DrawRectangle(bMin,bMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);
                }
                TTF_SizeText(gizmosFont,"Remove selected",&btw,&bth);
                DrawTextColored("Remove selected", brightWhite, bMin.x+ ((bMax.x-bMin.x)-btw)/2, bMin.y+ ((bMax.y-bMin.y)-bth)/2, gizmosFont);

            break;
            case 2:
                
            break;
        }
    }
}

void FBLoadScene(){
    LoadScene(fileBrowser.filePath,fileBrowser.fileName);
    menuOpened = 0;
}

void FBImportSceneEntities(){
    LoadSceneAdditive(fileBrowser.filePath,fileBrowser.fileName, 0);
    menuOpened = 0;
}

void FBSaveScene(){
    strcpy(Scene.path,fileBrowser.filePath);
    strcpy(Scene.filename,fileBrowser.fileName);
    ExportScene(fileBrowser.filePath,fileBrowser.fileName);
    menuOpened = 0;
}

void FBExportPrefab(){
    ExportEntityPrefab(GetElementAsType(GetFirstCell(SelectedEntities),EntityID), fileBrowser.filePath, fileBrowser.fileName);
    menuOpened = 0;
}

void FBImportPrefab(){
    ImportEntityPrefab(fileBrowser.filePath, fileBrowser.fileName);
    menuOpened = 0;
}

void NewSceneDontSaveOption(){
    //Destroy all entities from the scene
    int i;
    for(i=0; i<=ECS.maxUsedIndex; i++){
        if(IsValidEntity(i)){
            DestroyEntity(i);
        }
    }
    FreeList(&SelectedEntities);

    //Close menu
    menuOpened = 0;

    //Reset the current scene path and name
    Scene.path[0] = '\0';
    Scene.filename[0] = '\0';
}

void NewSceneSaveOption(){
    if(Scene.path[0] != '\0'){
        OpenFileBrowser(1,Scene.path,NewSceneSaveScene);
    }else{
        OpenFileBrowser(1,NULL,NewSceneSaveScene);
    }
    FileBrowserExtension("scene");
}

void NewSceneCancelOption(){
    CloseDialogWindow();
}

void NewSceneSaveScene(){
    strcpy(Scene.path,fileBrowser.filePath);
    strcpy(Scene.filename,fileBrowser.fileName);
    ExportScene(fileBrowser.filePath,fileBrowser.fileName);

    //Execute the same steps from the don't save option
    NewSceneDontSaveOption();
}