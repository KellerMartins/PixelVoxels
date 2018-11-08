#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include "../../Engine.h"
#include "../UIRenderer.h"
#include "../../Libs/tinydir.h"

#include "DialogWindow.h"
#include "EditorUI.h"

//File browser "namespace"
typedef struct {
    List folders; //List of tinydir_file
    List files;   //List of tinydir_file
    List paths;   //List of char*
    int indexPath;
    char filePath[PATH_MAX];
    char fileName[FILENAME_MAX];
    char fileExtension[FILENAME_MAX];
    void (*onConfirmFunction)();
    int opened; //File browser open status (0 = not opened, 1 = opened (load mode), 2 = opened (save mode), -1 failed to open)
    int itemsScroll;     //Line of items scrolled
}FileBrowserData;


void DrawFileBrowser();

void OpenFileBrowser(int mode, char *initialPath,void(*onOpen)());
void FileBrowserExtension(char *ext);
void CloseFileBrowser();
void OverrideFileDialogConfirmation();
void OverrideFileDialogCancel();

#endif