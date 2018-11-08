#ifndef DIALOGWINDOW_H
#define DIALOGWINDOW_H

#include "../../Engine.h"
#include "../UIRenderer.h"

#include "EditorUI.h"

//Dialog window "namespace"
typedef struct {
    int opened;
    char contentString[256];
    char option1String[16];
    char option2String[16];
    char option3String[16];
    void(*option1Function)();
    void(*option2Function)();
    void(*option3Function)();
}DialogWindowData;


void OpenDialogWindow(char content[], char option1[], char option2[], char option3[], void(*op1Func)(), void(*op2Func)(), void(*op3Func)());
void CloseDialogWindow();
void DrawDialogWindow();

#endif