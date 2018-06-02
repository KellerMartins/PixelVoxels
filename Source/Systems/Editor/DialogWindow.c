#include "DialogWindow.h"

/////////////////////////////////External data//////////////////////////////////

//Engine data
extern engineScreen Screen;
extern engineInput Input;

//Color definitions from Editor.c
extern Vector3 bgPanelColor;
extern Vector3 bgLightColor;
extern Vector3 bgMediumColor;
extern Vector3 buttonOverColor;
extern Vector3 lightWhite;

//Data from Editor.c
extern Vector3 mousePos;
extern TTF_Font* gizmosFont;

/////////////////////////////////////////////////////////////////////////////////

//Dialog window data struct
DialogWindowData dialog = {.opened = 0, .option1Function = NULL, .option2Function = NULL, .option3Function = NULL};



void OpenDialogWindow(char content[], char option1[], char option2[], char option3[], void(*op1Func)(), void(*op2Func)(), void(*op3Func)()){
    dialog.opened = 1;
    //Pass the data to the dialog struct
    if(content){
        strncpy(dialog.contentString,content,sizeof(dialog.contentString)/sizeof(char));
    }else{
        dialog.contentString[0] = '\0';
    }
    //All options have the same size
    int sizeOptions = sizeof(dialog.option1String)/sizeof(char)-1;

    if(option1)
        strncpy(dialog.option1String,option1,sizeOptions);
    if(option2)
        strncpy(dialog.option2String,option2,sizeOptions);
    if(option3)
        strncpy(dialog.option3String,option3,sizeOptions);

    //Ensure the string is zero terminated
    dialog.option1String[sizeOptions] = '\0';
    dialog.option2String[sizeOptions] = '\0';
    dialog.option3String[sizeOptions] = '\0';

    dialog.option1Function = op1Func;
    dialog.option2Function = op2Func;
    dialog.option3Function = op3Func;
}

void CloseDialogWindow(){
    dialog.opened = 0;
    dialog.option1Function = NULL;
    dialog.option2Function = NULL;
    dialog.option3Function = NULL;
}

void DrawDialogWindow(){
    int w,h;
    //Backgrounds
    Vector3 bgMin = {Screen.windowWidth/2 -200,Screen.windowHeight/2 -70};
    Vector3 bgMax = {Screen.windowWidth/2 +200,Screen.windowHeight/2 +70};
    Vector3 footMin = {Screen.windowWidth/2 -200,Screen.windowHeight/2 -69};
    Vector3 footMax = {Screen.windowWidth/2 +200,Screen.windowHeight/2 -30};

    DrawRectangle(bgMin,bgMax,bgPanelColor.x,bgPanelColor.y,bgPanelColor.z);
    DrawRectangle(footMin,footMax,bgMediumColor.x, bgMediumColor.y, bgMediumColor.z);

    Vector3 contentMin = {bgMin.x,footMax.y};
    Vector3 contentMax = {bgMax.x,bgMax.y};

    static char contentBuffer[sizeof(dialog.contentString)/sizeof(char)];
    memcpy(contentBuffer, dialog.contentString,sizeof(dialog.contentString));

    char *contentLine = strtok(contentBuffer,"\n");
    if(contentLine){
        int lineCount = 0;
        //Count the number of lines
        while(contentLine){
            contentLine = strtok(NULL,"\n");
            lineCount++;
        }
        //Reset buffer
        memcpy(contentBuffer, dialog.contentString,sizeof(dialog.contentString));
        contentLine = strtok(contentBuffer,"\n");

        //Get text height
        TTF_SizeText(gizmosFont,contentLine,&w,&h);

        int l,lineHeight = contentMax.y - ((contentMax.y - contentMin.y) - h*lineCount)/2;
        for(l=0; l<lineCount; l++){
            lineHeight -= h;
            //Print the current line
            TTF_SizeText(gizmosFont,contentLine,&w,&h);
            DrawTextColored(contentLine, lightWhite, contentMin.x + 40, lineHeight, gizmosFont);
            
            //Parse the next line
            contentLine = strtok(NULL,"\n");
        }
    }

    //Option 1 Button
    if(dialog.option2Function){
        TTF_SizeText(gizmosFont,dialog.option1String,&w,&h);
    }else{
        w = 0;
        h = 0;
    }

    Vector3 option1Max = {footMax.x-3,footMax.y-3};
    Vector3 option1Min = {option1Max.x-(10 + w),footMin.y+3};
    
    if(dialog.option1Function){
        if(MouseOverBox(mousePos,option1Min,option1Max,0)){
            DrawRectangle(option1Min,option1Max,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
            if(GetMouseButtonUp(SDL_BUTTON_LEFT)){
                dialog.option1Function();
                CloseDialogWindow();
            }
        }else{
            DrawRectangle(option1Min,option1Max,bgLightColor.x, bgLightColor.y, bgLightColor.z);
        }
        
        DrawTextColored(dialog.option1String, lightWhite, option1Min.x + ((option1Max.x-option1Min.x)-w)/2, option1Min.y+ ((option1Max.y-option1Min.y)-h)/2, gizmosFont);
    }

    //Option 2 Button
    if(dialog.option2Function){
        TTF_SizeText(gizmosFont,dialog.option2String,&w,&h);
    }else{
        w = 0;
        h = 0;
    }

    Vector3 option2Min = {option1Min.x - (20 + w),option1Min.y};
    Vector3 option2Max = {option1Min.x - 10,option1Max.y};

    if(dialog.option2Function){
        if(MouseOverBox(mousePos,option2Min,option2Max,0)){
            DrawRectangle(option2Min,option2Max,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
            if(GetMouseButtonUp(SDL_BUTTON_LEFT)){
                dialog.option2Function();
                CloseDialogWindow();
            }
        }else{
            DrawRectangle(option2Min,option2Max,bgLightColor.x, bgLightColor.y, bgLightColor.z);
        }
        
        DrawTextColored(dialog.option2String, lightWhite, option2Min.x + ((option2Max.x-option2Min.x)-w)/2, option2Min.y+ ((option2Max.y-option2Min.y)-h)/2, gizmosFont);
    }

    //Option 3 Button
    if(dialog.option3Function){
        TTF_SizeText(gizmosFont,dialog.option3String,&w,&h);
    }else{
        w = 0;
        h = 0;
    }
    Vector3 option3Min = {option2Min.x - (20 + w),option2Min.y};
    Vector3 option3Max = {option2Min.x - 10,option2Max.y};

    if(dialog.option3Function){
        if(MouseOverBox(mousePos,option3Min,option3Max,0)){
            DrawRectangle(option3Min,option3Max,buttonOverColor.x, buttonOverColor.y, buttonOverColor.z);
            if(GetMouseButtonUp(SDL_BUTTON_LEFT)){
                dialog.option3Function();
                CloseDialogWindow();
            }
        }else{
            DrawRectangle(option3Min,option3Max,bgLightColor.x, bgLightColor.y, bgLightColor.z);
        }
        DrawTextColored(dialog.option3String, lightWhite, option3Min.x + ((option3Max.x-option3Min.x)-w)/2, option3Min.y+ ((option3Max.y-option3Min.y)-h)/2, gizmosFont);
    }
}