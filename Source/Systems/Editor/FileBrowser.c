#include "FileBrowser.h"

/////////////////////////////////External data//////////////////////////////////

//Engine data
extern engineScreen Screen;
extern engineInput Input;

//Data from EditorUI.c
extern int iconsSize[];
extern TTF_Font* gizmosFont;
extern TTF_Font* gizmosFontSmall;

//Color definitions from Editor.c
extern Vector3 bgPanelColor;
extern Vector3 bgLightColor;
extern Vector3 bgMediumColor;
extern Vector3 fieldColor;
extern Vector3 fieldEditingColor;
extern Vector3 buttonOverColor;
extern Vector3 scrollbarOverColor;
extern Vector3 lightWhite;

//Data from Editor.c
extern Vector3 mousePos;

/////////////////////////////////////////////////////////////////////////////////


//File browser data struct
FileBrowserData fileBrowser = {.fileExtension = "", .fileName = "", .filePath = "", .fileExtension = "", .itemsScroll = 0, .opened = 0};


//Mode: 0 = open file, 1 = save file
void OpenFileBrowser(int mode, char *initialPath,void (*onOpen)()){
    //Free the folder and files lists before opening another path
    if(fileBrowser.opened){
        FreeList(&fileBrowser.folders);
        FreeList(&fileBrowser.files);
    }

    fileBrowser.onConfirmFunction = *onOpen;
    fileBrowser.folders = InitList(sizeof(tinydir_file));
    fileBrowser.files = InitList(sizeof(tinydir_file));

    if(initialPath){
        int pathSize = strlen(initialPath);
        memcpy(fileBrowser.filePath,initialPath,(pathSize+1)*sizeof(char));
    }else{
        char DefaultPath[] = "Assets";
        memcpy(fileBrowser.filePath,DefaultPath,sizeof(DefaultPath));
        if(fileBrowser.opened){
            //Free the paths list when returning to default path
            ListCellPointer cell;
            ListForEach(cell,fileBrowser.paths){
                free(GetElementAsType(cell,char*));
            }
            FreeList(&fileBrowser.paths);
        }
    }
    //Insert the initial path to the list when opening/returning to default path
    if(!fileBrowser.opened || !initialPath){
        fileBrowser.paths = InitList(sizeof(char*));

        int pathLen = strlen(fileBrowser.filePath);

        char *firstPath = malloc((pathLen+1)*sizeof(char));
        memcpy(firstPath,fileBrowser.filePath,(pathLen+1)*sizeof(char));

        InsertListEnd(&fileBrowser.paths,&firstPath);
        fileBrowser.indexPath = 0;
    }
    memset(fileBrowser.fileName,'\0',FILENAME_MAX*sizeof(char));

    tinydir_dir dir;
    if(tinydir_open(&dir, fileBrowser.filePath) < 0){
        //Set as invalid folder path
        fileBrowser.opened = -1;
        return;
    }
    while(dir.has_next){
        tinydir_file file;
        tinydir_readfile(&dir, &file);
        if (file.is_dir)
        {
            InsertListEnd(&fileBrowser.folders,&file);
        }else{
            
            InsertListEnd(&fileBrowser.files,&file);
            char ** extStr = &((tinydir_file*)GetLastElement(fileBrowser.files))->extension;
            int extLen = strlen(file.extension)+1;
            *extStr = malloc(extLen * sizeof(char));
            strncpy(*extStr,file.extension, extLen);
        }

        tinydir_next(&dir);
    }
    tinydir_close(&dir);

    fileBrowser.opened = 1+mode;
}

void FileBrowserExtension(char *ext){
    if(ext){
        strncpy(fileBrowser.fileExtension,ext,FILENAME_MAX);
    }else{
        strncpy(fileBrowser.fileExtension,"",FILENAME_MAX);
    }
}

void CloseFileBrowser(){
    fileBrowser.opened = 0;
    FreeList(&fileBrowser.folders);
    FreeList(&fileBrowser.files);

    ListCellPointer cell;
    ListForEach(cell,fileBrowser.paths){
        free(GetElementAsType(cell,char*));
    }
    FreeList(&fileBrowser.paths);
}

void DrawFileBrowser(){
    int w,h;
    //Backgrounds
    Vector3 fbMin = {Screen.windowWidth/2 -300,Screen.windowHeight/2 -200};
    Vector3 fbMax = {Screen.windowWidth/2 +300,Screen.windowHeight/2 +200};
    Vector3 fbFootMin = {Screen.windowWidth/2 -299,Screen.windowHeight/2 -199};
    Vector3 fbFootMax = {Screen.windowWidth/2 +299,Screen.windowHeight/2 -150};
    Vector3 fbHeaderMin = {Screen.windowWidth/2 -299,Screen.windowHeight/2 +169};
    Vector3 fbHeaderMax = {Screen.windowWidth/2 +299,Screen.windowHeight/2 +199};

    DrawRectangle(fbMin,fbMax,bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);
    DrawRectangle(fbFootMin,fbFootMax,bgMediumColor.x, bgMediumColor.y, bgMediumColor.z);
    DrawRectangle(fbHeaderMin,fbHeaderMax,bgMediumColor.x, bgMediumColor.y, bgMediumColor.z);

    //Header
    Vector3 filepathBgMin = {Screen.windowWidth/2-180,Screen.windowHeight/2 +172};
    Vector3 filepathBgMax = {Screen.windowWidth/2 +297,Screen.windowHeight/2 +195};
    DrawRectangle(filepathBgMin,filepathBgMax,fieldColor.x, fieldColor.y, fieldColor.z);

    //0 = open mode, 1 = save mode
    int mode = fileBrowser.opened-1;

    //Header buttons
    //Previous button
    if(fileBrowser.indexPath>0){
        if(PointButton((Vector3){fbHeaderMin.x+ iconsSize[9] * 2 ,fbHeaderMin.y+(fbHeaderMax.y - fbHeaderMin.y)/2,0},9, 1, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1}, (Vector3){scrollbarOverColor.x, scrollbarOverColor.y, scrollbarOverColor.z}) == 1){
            fileBrowser.indexPath--;
            OpenFileBrowser(mode,*((char**)GetElementAt(fileBrowser.paths,fileBrowser.indexPath)),fileBrowser.onConfirmFunction);
        }
    }else{
        PointButton((Vector3){fbHeaderMin.x+ iconsSize[9] * 2 ,fbHeaderMin.y+(fbHeaderMax.y - fbHeaderMin.y)/2,0},9, 1, (Vector3){0.25,0.25,0.25}, (Vector3){0.25,0.25,0.25}, (Vector3){0.25,0.25,0.25});
    }
    //Next button
    if(fileBrowser.indexPath<GetLength(fileBrowser.paths)-1){
        if(PointButton((Vector3){fbHeaderMin.x+ iconsSize[8] * 6 ,fbHeaderMin.y+(fbHeaderMax.y - fbHeaderMin.y)/2,0},8, 1, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1}, (Vector3){scrollbarOverColor.x, scrollbarOverColor.y, scrollbarOverColor.z}) == 1){
            fileBrowser.indexPath++;
            OpenFileBrowser(mode,*((char**)GetElementAt(fileBrowser.paths,fileBrowser.indexPath)),fileBrowser.onConfirmFunction);
        }
    }else{
        PointButton((Vector3){fbHeaderMin.x+ iconsSize[8] * 6 ,fbHeaderMin.y+(fbHeaderMax.y - fbHeaderMin.y)/2,0},8, 1, (Vector3){0.25,0.25,0.25}, (Vector3){0.25,0.25,0.25}, (Vector3){0.25,0.25,0.25});
    }
    //Home
    if(PointButton((Vector3){fbHeaderMin.x+ iconsSize[6] * 10,fbHeaderMin.y+(fbHeaderMax.y - fbHeaderMin.y)/2,0},6, 1, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1}, (Vector3){scrollbarOverColor.x, scrollbarOverColor.y, scrollbarOverColor.z})==1){
        OpenFileBrowser(mode,NULL,fileBrowser.onConfirmFunction);
    }
    //File path
    if(strlen(fileBrowser.filePath)>34){
        char minifiedPath[] = "0000000000000000000000000000000000...";
        memcpy(minifiedPath,fileBrowser.filePath,34*sizeof(char));
        TTF_SizeText(gizmosFont,minifiedPath,&w,&h);
        DrawTextColored(minifiedPath, lightWhite, filepathBgMin.x + 6, filepathBgMin.y+ ((filepathBgMax.y-filepathBgMin.y)-h)/2 -1, gizmosFont);    
    }else{
        TTF_SizeText(gizmosFont,fileBrowser.filePath,&w,&h);
        DrawTextColored(fileBrowser.filePath, lightWhite, filepathBgMin.x + 6, filepathBgMin.y+ ((filepathBgMax.y-filepathBgMin.y)-h)/2 -1, gizmosFont);    
    }


    //Foot
    //Cancel Button
    Vector3 cancelButtonMin = {Screen.windowWidth/2 +207,Screen.windowHeight/2 -197};
    Vector3 cancelButtonMax = {Screen.windowWidth/2 +297,Screen.windowHeight/2 -152};
    if(MouseOverBox(mousePos,cancelButtonMin,cancelButtonMax,0)){
        DrawRectangle(cancelButtonMin,cancelButtonMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
        if(GetMouseButtonUp(SDL_BUTTON_LEFT)){
            CloseFileBrowser();
        }
    }else{
        DrawRectangle(cancelButtonMin,cancelButtonMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);
    }
    TTF_SizeText(gizmosFont,"Cancel",&w,&h);
    DrawTextColored("Cancel", lightWhite, cancelButtonMin.x + ((cancelButtonMax.x-cancelButtonMin.x)-w)/2, cancelButtonMin.y+ ((cancelButtonMax.y-cancelButtonMin.y)-h)/2, gizmosFont);

    //Open/Save button
    Vector3 openButtonMin = {Screen.windowWidth/2 +207 - (cancelButtonMax.x-cancelButtonMin.x) - 10,Screen.windowHeight/2 -197};
    Vector3 openButtonMax = {Screen.windowWidth/2 +297 - (cancelButtonMax.x-cancelButtonMin.x) - 10,Screen.windowHeight/2 -152};

    if(StringCompareEqual(fileBrowser.fileName,"")){
        //No file selected, disable button
        DrawRectangle(openButtonMin,openButtonMax,0.1,0.1,0.1);
    }else{
        if(MouseOverBox(mousePos,openButtonMin,openButtonMax,0)){
            DrawRectangle(openButtonMin,openButtonMax,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
            if(GetMouseButtonUp(SDL_BUTTON_LEFT)){
                if(mode == 0){
                    fileBrowser.onConfirmFunction();
                    CloseFileBrowser();
                }else if(mode == 1){
                    //Check if file already exists
                    
                    char nameBuff[FILENAME_MAX];
                    strcpy(nameBuff,fileBrowser.fileName);
                    strcat(nameBuff,".");
                    strcat(nameBuff,fileBrowser.fileExtension);
                    
                    int willOverride = 0;

                    ListCellPointer fileCell;
                    ListForEach(fileCell,fileBrowser.files){
                        if(StringCompareEqual(nameBuff,GetElementAsType(fileCell,tinydir_file).name)){
                            willOverride = 1;
                            break;
                        }
                    }
                    if(willOverride){
                        OpenDialogWindow("Do you want override the file ?", "Cancel", "Confirm", "",OverrideFileDialogCancel, OverrideFileDialogConfirmation, NULL);
                    }else{
                        fileBrowser.onConfirmFunction();
                        CloseFileBrowser();
                    }
                }
            }
        }else{
            DrawRectangle(openButtonMin,openButtonMax,bgLightColor.x, bgLightColor.y, bgLightColor.z);
        }
    }
    TTF_SizeText(gizmosFont,mode?"Save":"Open",&w,&h);
    DrawTextColored(mode?"Save":"Open", lightWhite, openButtonMin.x + ((openButtonMax.x-openButtonMin.x)-w)/2, openButtonMin.y+ ((openButtonMax.y-openButtonMin.y)-h)/2, gizmosFont);

    //File name
    if(mode == 1){
        Vector3 filenameBgMin = {Screen.windowWidth/2 -295,Screen.windowHeight/2 -195};
        Vector3 filenameBgMax = {openButtonMin.x -10,Screen.windowHeight/2 -164};
        if(MouseOverBox(mousePos,filenameBgMin,filenameBgMax,0)){
            DrawRectangle(filenameBgMin,filenameBgMax,fieldEditingColor.x, fieldEditingColor.y, fieldEditingColor.z);
            if(GetMouseButtonUp(SDL_BUTTON_LEFT) && !SDL_IsTextInputActive()){
                //Files saved have a limit of 27 characters
                int curLen = strlen(fileBrowser.fileName);
                curLen = curLen>=27? 27:curLen;
                GetTextInput(fileBrowser.fileName, 27, curLen);
                memset(fileBrowser.fileName+curLen,'\0',FILENAME_MAX-curLen);
            }
        }else{
            DrawRectangle(filenameBgMin,filenameBgMax,fieldColor.x, fieldColor.y, fieldColor.z);
        }

        if(SDL_IsTextInputActive()){
            //Get the cursor position by creating a string containing the characters until the cursor
            //and getting his size when rendered with the used font
            char buff[FILENAME_MAX];
            strncpy(buff,fileBrowser.fileName,Input.textInputCursorPos);
            memset(buff+Input.textInputCursorPos,'\0',1);
            int cursorPos,h;
            TTF_SizeText(gizmosFont,buff,&cursorPos,&h);
            cursorPos += filenameBgMin.x + 5;

            //Cursor line
            DrawLine((Vector3){cursorPos, filenameBgMin.y+2},
                     (Vector3){cursorPos, filenameBgMin.y-2},
                      2,0.7,0.7,0.7);
        }
        
        DrawTextColored("file name", lightWhite, filenameBgMin.x, filenameBgMax.y +1, gizmosFontSmall);
        TTF_SizeText(gizmosFont,fileBrowser.fileName,&w,&h);
        DrawTextColored(fileBrowser.fileName, lightWhite, filenameBgMin.x + 5, filenameBgMin.y+ ((filenameBgMax.y-filenameBgMin.y)-h)/2 +1, gizmosFont);
    }else{
        Vector3 filenameBgMin = {Screen.windowWidth/2 -295,Screen.windowHeight/2 -195};
        Vector3 filenameBgMax = {openButtonMin.x -10,Screen.windowHeight/2 -164};
        DrawRectangle(filenameBgMin,filenameBgMax,fieldColor.x, fieldColor.y, fieldColor.z);
        DrawTextColored("file name", lightWhite, filenameBgMin.x, filenameBgMax.y +1, gizmosFontSmall);
        TTF_SizeText(gizmosFont,fileBrowser.fileName,&w,&h);
        DrawTextColored(fileBrowser.fileName, lightWhite, filenameBgMin.x + 5, filenameBgMin.y+ ((filenameBgMax.y-filenameBgMin.y)-h)/2 +1, gizmosFont);
    }

    //Browser Items
    if(fileBrowser.opened>0){
        int i=0,startx = fbHeaderMin.x + iconsSize[10] * 3 + 12,starty = fbHeaderMin.y - iconsSize[10]*3 -28;
        int x = startx, y = starty;
        int foldersUpdated = 0;
        ListCellPointer cell;
        //Folders
        ListForEach(cell,fileBrowser.folders){
            tinydir_file file = GetElementAsType(cell,tinydir_file);
            if(StringCompareEqual(file.name,".") || StringCompareEqual(file.name,"..")) continue;

            //Skip rendering the items out of view
            if((i - 7*fileBrowser.itemsScroll)<0 || (i - 7*fileBrowser.itemsScroll)>=(7 * 3)){
                i++;
                continue;
            }

            //Only jump if not the first line (end of the line, not the first item to render)
            if(i%7==0 && (i - 7*fileBrowser.itemsScroll)!=0){
                x = startx;
                y-= iconsSize[10] * 6 + 40;
            }

            //Render folder name
            if(strlen(file.name)>6){
                //Cut name if too long
                char minifiedName[] = "00000...";
                memcpy(minifiedName,file.name,5*sizeof(char));
                TTF_SizeText(gizmosFont,minifiedName,&w,&h);
                DrawTextColored(minifiedName, lightWhite, x-(iconsSize[10] * 3) +((iconsSize[10] * 6) - w)/2, y - (iconsSize[10] * 3) - h, gizmosFont);
            }else{
                TTF_SizeText(gizmosFont,file.name,&w,&h);
                DrawTextColored(file.name, lightWhite, x-(iconsSize[10] * 3) +((iconsSize[10] * 6) - w)/2, y - (iconsSize[10] * 3) - h, gizmosFont);
            }

            //Folder icon/button
            if(PointButton((Vector3){x,y,0},10,3, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1}, (Vector3){scrollbarOverColor.x, scrollbarOverColor.y, scrollbarOverColor.z}) == 1){
                char *path = calloc(PATH_MAX,sizeof(char));
                strncpy(path,file.path,PATH_MAX);
                PrintLog(Info,"(%s)\n", path);
                OpenFileBrowser(mode,path,fileBrowser.onConfirmFunction);
                memcpy(fileBrowser.filePath,path,PATH_MAX*sizeof(char));

                //If there is any folder as next folder, remove from the list before adding the new folder
                while(fileBrowser.indexPath+1 < GetLength(fileBrowser.paths)){
                    RemoveListEnd(&fileBrowser.paths);
                }

                InsertListEnd(&fileBrowser.paths,&path);
                fileBrowser.indexPath++;

                //As the contents of the fileBrowser.folders have been modified, and we are iterating
                //over it, break the loop
                foldersUpdated = 1;
                break;
            }
            x += iconsSize[10] * 6 + 30;
            i++;
        }

        //Files
        //0 = No specific extension, show all; 1 = vox; 2 = Other extensions
        int specificExtension = fileBrowser.fileExtension[0] == '\0'? 0:(StringCompareEqual(fileBrowser.fileExtension,"vox")? 1:2);

        ListForEach(cell,fileBrowser.files){
            tinydir_file file = GetElementAsType(cell,tinydir_file);

            //Skip rendering the files with different extensions
            if(specificExtension && !StringCompareEqualCaseInsensitive(fileBrowser.fileExtension,file.extension)) continue;

            //Skip rendering the items out of view
            if((i - 7*fileBrowser.itemsScroll)<0 || (i - 7*fileBrowser.itemsScroll)>=(7 * 3)){
                i++;
                continue;
            }
            
            //Select the icon ID
            int icon = specificExtension==0? 11:(specificExtension == 1? 17:11);

            //Only jump to the next line if not the first line (if end of the line && not the first item to render)
            if(i%7==0 && (i - 7*fileBrowser.itemsScroll)!=0){
                x = startx;
                y-= iconsSize[icon] * 6 + 40;
            }
            
            //File icon/button
            //Ignore misclick caused by a change in the folders structure
            if(!foldersUpdated && PointButton((Vector3){x,y,0},icon,3, (Vector3){0.75,0.75,0.75}, (Vector3){1,1,1}, (Vector3){scrollbarOverColor.x, scrollbarOverColor.y, scrollbarOverColor.z}) == 1){
                //Remove the extension from the name in the save mode
                if(mode == 1){
                    int extLen = strlen(file.extension) + 1; //Extension name length + dot
                    int filenameLen = strlen(file.name);

                    memcpy(fileBrowser.fileName,file.name,FILENAME_MAX*sizeof(char));
                    fileBrowser.fileName[filenameLen - extLen] = '\0';

                }else{
                    memcpy(fileBrowser.fileName,file.name,FILENAME_MAX*sizeof(char));
                }
            }

            //Render file name
            if(strlen(file.name)>6){
                //Cut name if too long
                char minifiedName[] = "00000...";
                memcpy(minifiedName,file.name,5*sizeof(char));
                TTF_SizeText(gizmosFont,minifiedName,&w,&h);
                DrawTextColored(minifiedName, lightWhite, x-(iconsSize[icon] * 3) +((iconsSize[icon] * 6) - w)/2, y - (iconsSize[icon] * 3) - h, gizmosFont);
            }else{
                TTF_SizeText(gizmosFont,file.name,&w,&h);
                DrawTextColored(file.name, lightWhite, x-(iconsSize[icon] * 3) +((iconsSize[icon] * 6) - w)/2, y - (iconsSize[icon] * 3) - h, gizmosFont);
            }
            x += iconsSize[icon] * 6 + 30;
            i++;
        }

        //Scrollbar
        //'i' is now the number of items
        if( i/(7*3) > 0){
            Vector3 scrollbarDownMin = {fbMin.x+1,fbFootMax.y+2};
            Vector3 scrollbarDownMax = {fbMax.x-1,fbFootMax.y+22};
            Vector3 scrollbarUpMin = {fbMin.x+1,fbHeaderMin.y-22};
            Vector3 scrollbarUpMax = {fbMax.x-1,fbHeaderMin.y-2};

            if(fileBrowser.itemsScroll<(i/7)-2){
                if(MouseOverBox(mousePos,scrollbarDownMin,scrollbarDownMax,0)){
                    DrawRectangle(scrollbarDownMin,scrollbarDownMax,0.1,0.1,0.125);
                    if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                        fileBrowser.itemsScroll++;
                    }
                }else{
                    DrawRectangle(scrollbarDownMin,scrollbarDownMax,0.05,0.05,0.075);
                }
                DrawPointIcon((Vector3){scrollbarDownMin.x + (scrollbarDownMax.x - scrollbarDownMin.x)/2 - 1,
                                        scrollbarDownMin.y + (scrollbarDownMax.y - scrollbarDownMin.y)/2},13, 1, (Vector3){0.2,0.2,0.2});

                //Mouse scroll
                if(Input.mouseWheelY<0){
                    fileBrowser.itemsScroll++;
                }
            }
            if(fileBrowser.itemsScroll>0){
                if(MouseOverBox(mousePos,scrollbarUpMin,scrollbarUpMax,0)){
                    DrawRectangle(scrollbarUpMin,scrollbarUpMax,0.1,0.1,0.125);
                    if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                        fileBrowser.itemsScroll--;
                    }
                }else{
                    DrawRectangle(scrollbarUpMin,scrollbarUpMax,0.05,0.05,0.075);
                }
                DrawPointIcon((Vector3){scrollbarUpMin.x + (scrollbarUpMax.x - scrollbarUpMin.x)/2 - 1,
                                        scrollbarUpMin.y + (scrollbarUpMax.y - scrollbarUpMin.y)/2},12, 1, (Vector3){0.2,0.2,0.2});
                //Mouse scroll
                if(Input.mouseWheelY>0){
                    fileBrowser.itemsScroll--;
                }
            }    
            
        }else{
            fileBrowser.itemsScroll = 0;
        }
    }else if (fileBrowser.opened == -1){
        //Invalid folder message
        TTF_SizeText(gizmosFont,"Invalid or nonexistent path!",&w,&h);
        DrawTextColored("Invalid or nonexistent path!", lightWhite, fbMin.x+ ((fbMax.x-fbMin.x)-w)/2, fbMin.y+ ((fbMax.y-fbMin.y)-h)/2, gizmosFont);
    }
}

void OverrideFileDialogConfirmation(){
    fileBrowser.onConfirmFunction();
    CloseFileBrowser();
}

void OverrideFileDialogCancel(){
    //As NULL functions are identitied as not existent options in the dialog window
    //use this empty function as cancel option
}