#include "EditorUI.h"

/////////////////////////////////External data//////////////////////////////////

//Engine data
extern engineScreen Screen;
extern engineInput Input;

//Color definitions from Editor.c
extern Vector3 fieldColor;
extern Vector3 fieldEditingColor;
extern Vector3 lightWhite;

//Data from Editor.c
extern Vector3 mousePos;
extern int editingField;
extern char *textFieldString;

/////////////////////////////////////////////////////////////////////////////////

#define NUMBER_OF_ICONS 20
GLuint iconsTex[NUMBER_OF_ICONS];
int iconsSize[NUMBER_OF_ICONS];

TTF_Font* gizmosFont;
TTF_Font* gizmosFontSmall;

//Internal functions
void LoadUITexture(char *path,int index);

//-------------------------- Initialization function --------------------------

void InitEditorUI(){

    gizmosFont = TTF_OpenFont("Interface/Fonts/gros/Gros.ttf",16);
	if(!gizmosFont){
		printf("Font: Error loading font!");
	}

    gizmosFontSmall= TTF_OpenFont("Interface/Fonts/coolthre/CoolThre.ttf",12);
    if(!gizmosFontSmall){
		printf("Font: Error loading small font!");
	}

    //Load UI icons
    glGenTextures(NUMBER_OF_ICONS, iconsTex);
    LoadUITexture("Interface/IconsUI/add.png",0);
    LoadUITexture("Interface/IconsUI/remove.png",1);
    LoadUITexture("Interface/IconsUI/bin.png",2);
    LoadUITexture("Interface/IconsUI/play.png",3);
    LoadUITexture("Interface/IconsUI/pause.png",4);
    LoadUITexture("Interface/IconsUI/stop.png",5);
    LoadUITexture("Interface/IconsUI/home.png",6);
    LoadUITexture("Interface/IconsUI/reload.png",7);
    LoadUITexture("Interface/IconsUI/next.png",8);
    LoadUITexture("Interface/IconsUI/previous.png",9);
    LoadUITexture("Interface/IconsUI/folder.png",10);
    LoadUITexture("Interface/IconsUI/file.png",11);
    LoadUITexture("Interface/IconsUI/up.png",12);
    LoadUITexture("Interface/IconsUI/down.png",13);
    LoadUITexture("Interface/IconsUI/toggleOn.png",14);
    LoadUITexture("Interface/IconsUI/toggleOff.png",15);
    LoadUITexture("Interface/IconsUI/toggleUndefined.png",16);
    LoadUITexture("Interface/IconsUI/voxel.png",17);
    LoadUITexture("Interface/IconsUI/x.png",18);
    LoadUITexture("Interface/IconsUI/save.png",19);
}

void FreeEditorUI(){
    TTF_CloseFont(gizmosFont);
    TTF_CloseFont(gizmosFontSmall);
    glDeleteTextures(NUMBER_OF_ICONS,iconsTex);
}


//-------------------------- UI elements drawing and interaction --------------------------

//Returns 1 if pressed, 2 if mouse is over and 0 if neither case
int PointButton(Vector3 pos,int iconID, int scale, Vector3 defaultColor, Vector3 mouseOverColor, Vector3 pressedColor){
    int state = 0;

    Vector3 color = defaultColor;

    if(MouseOverPointGizmos(mousePos, pos, scale * iconsSize[iconID])){
        color = mouseOverColor;

        //Pressed
        if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
            color = pressedColor;
            state = 1;
        }else{
            //Mouse over only
            state = 2;
        }
    }else{
        state = 0;
    }

    DrawPoint((Vector3){roundf(pos.x) + 0.375, roundf(pos.y) + 0.375,0},iconsSize[iconID]*scale, iconsTex[iconID], color.x,color.y,color.z);

    return state;
}

//Returns 1 if pressed, 2 if mouse is over and 0 if neither case
int PointToggle(int *data,Vector3 pos,int onIconID, int offIconID, int undefinedIconID, int scale, Vector3 onColor, Vector3 offColor, Vector3 undefinedColor, Vector3 mouseOverColor){
    int state = 0;
    int iconSize = max(max(iconsSize[onIconID],iconsSize[offIconID]),iconsSize[undefinedIconID]);
    GLuint usedIcon;
    Vector3 color;

    if(*data == 1){
        color = onColor;
        usedIcon = iconsTex[onIconID];
    }else if(*data == -1){
        color = undefinedColor;
        usedIcon = iconsTex[undefinedIconID];
    }else{
        color = offColor;
        usedIcon = iconsTex[offIconID];
    }

        if(MouseOverPointGizmos(mousePos, pos, scale * iconSize)){
            color = mouseOverColor;

            //Pressed
            if(GetMouseButtonDown(SDL_BUTTON_LEFT)){
                state = 1;
                *data = *data<0? 1:!(*data);
            }else{
                //Mouse over only
                state = 2;
            }
        }else{
            //Mouse not over
            state = 0;
        }

    DrawPoint((Vector3){roundf(pos.x) + 0.375, roundf(pos.y) + 0.375,0},iconSize*scale, usedIcon, color.x,color.y,color.z);

    return state;
}

void DrawPointIcon(Vector3 pos,int iconID, int scale, Vector3 color){
    DrawPoint(pos,iconsSize[iconID]*scale, iconsTex[iconID], color.x,color.y,color.z);
}

void Vector3Field(char *title, Vector3 *data,int ommitX,int ommitY,int ommitZ,int x, int w, int fieldsSpacing, int* curField, int* curHeight){
    *curHeight -= 2;
    DrawTextColored(title, lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
    *curHeight -= 2 + TTF_FontHeight(gizmosFontSmall);

    int fieldW = w/3;

    //1 is X field, 2 is Y field and 3 is Z field
    Vector3 min1 = { x,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max1 = { x+fieldW - fieldsSpacing,*curHeight,0};
    Vector3 min2 = { x+fieldW + fieldsSpacing,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max2 = { x+fieldW*2 - fieldsSpacing,*curHeight,0};
    Vector3 min3 = { x+fieldW*2 + fieldsSpacing,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max3 = { x+fieldW*3 - fieldsSpacing,*curHeight,0};

    if(editingField == *curField || editingField == (*curField)+1 || editingField == (*curField)+2)
    {
        //String edit field background
        DrawRectangle(min1,max3,fieldEditingColor.x, fieldEditingColor.y, fieldEditingColor.z);

        //Get the cursor position by creating a string containing the characters until the cursor
        //and getting his size when rendered with the used font
        char buff[13];
        strncpy(buff,textFieldString,Input.textInputCursorPos);
        memset(buff+Input.textInputCursorPos,'\0',1);
        int cursorPos,h;
        TTF_SizeText(gizmosFont,buff,&cursorPos,&h);
        cursorPos+=min1.x;

        //Cursor line
        DrawLine((Vector3){cursorPos, min1.y},
                 (Vector3){cursorPos, max1.y},
                 2,0.7,0.7,0.7);

        //Render the string
        DrawTextColored(textFieldString, lightWhite, min1.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

        //Pass the string as float data
        if(editingField == *curField){
            data->x = strtof(textFieldString, NULL);
        }else if(editingField == (*curField)+1){
            data->y = strtof(textFieldString, NULL);
        }else{
            data->z = strtof(textFieldString, NULL);
        }

    }else{
        //Not editing any of the three fields, just draw the three boxes and floats normally
        static char valueString[5] = "  0.0";

        //Fields selection
        if(editingField<0 && GetMouseButtonDown(SDL_BUTTON_LEFT)){
            if(MouseOverBox(mousePos,min1,max1,0)){
                textFieldString = (char*)calloc(13,sizeof(char));

                if(!ommitX) snprintf(textFieldString, 12, "%6.6f", data->x);
                else snprintf(textFieldString, 4, "0.0");
                
                GetTextInput(textFieldString, 12, strlen(textFieldString));
                editingField = *curField;
            }

            if(MouseOverBox(mousePos,min2,max2,0)){
                textFieldString = (char*)calloc(13,sizeof(char));
                if(!ommitY) snprintf(textFieldString, 12, "%6.6f", data->y);
                else snprintf(textFieldString, 4, "0.0");

                GetTextInput(textFieldString, 12, strlen(textFieldString));
                editingField = (*curField)+1;
            }

            if(MouseOverBox(mousePos,min3,max3,0)){
                textFieldString = (char*)calloc(13,sizeof(char));
                if(!ommitX) snprintf(textFieldString, 12, "%6.6f", data->z);
                else snprintf(textFieldString, 4, "0.0");

                GetTextInput(textFieldString, 12, strlen(textFieldString));
                editingField = (*curField)+2;
            }
        }

        //Check if no field was selected before rendering the individual fields
        if(editingField != *curField && editingField != (*curField)+1 && editingField != (*curField)+2){

            //X field
            DrawRectangle(min1,max1,fieldColor.x, fieldColor.y, fieldColor.z);
            if(!ommitX) snprintf(valueString,5,"%3.1f",data->x);
            else snprintf(valueString,5,"---");
            DrawTextColored(valueString, lightWhite, min1.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

            //Y field
            DrawRectangle(min2,max2,fieldColor.x, fieldColor.y, fieldColor.z);
            if(!ommitY) snprintf(valueString,5,"%3.1f",data->y);
            else snprintf(valueString,5,"---");
            DrawTextColored(valueString, lightWhite, min2.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

            //Z field
            DrawRectangle(min3,max3,fieldColor.x, fieldColor.y, fieldColor.z);
            if(!ommitZ) snprintf(valueString,5,"%3.1f",data->z);
            else snprintf(valueString,5,"---");
            DrawTextColored(valueString, lightWhite, min3.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);
        }
    }

    //Mark as used ID fields
    *curField +=3;

    *curHeight -= 2 + TTF_FontHeight(gizmosFont);
}

void FloatField(char *title, float *data,int ommit,int x, int w, int* curField, int* curHeight){
    *curHeight -= 4;
    DrawTextColored(title, lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
    *curHeight -= 2 + TTF_FontHeight(gizmosFontSmall);

    Vector3 min = { x,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max = { x+w,*curHeight,0};

    if(editingField == *curField)
    {
        //Field background
        DrawRectangle(min,max,fieldEditingColor.x, fieldEditingColor.y, fieldEditingColor.z);

        //Get the cursor position by creating a string containing the characters until the cursor
        //and getting his size when rendered with the used font
        char buff[13];
        strncpy(buff,textFieldString,Input.textInputCursorPos);
        memset(buff+Input.textInputCursorPos,'\0',1);
        int cursorPos,h;
        TTF_SizeText(gizmosFont,buff,&cursorPos,&h);
        cursorPos+=min.x;

        //Cursor line
        DrawLine((Vector3){cursorPos, min.y},
                 (Vector3){cursorPos, max.y},
                 2,0.7,0.7,0.7);

        //Render the string
        DrawTextColored(textFieldString, lightWhite, min.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

        //Pass the string as float data
        *data = strtof(textFieldString, NULL);

    }else{
        //Not editing, just draw the box and float normally
        static char valueString[12] = "  0.0";

        //Field background
        DrawRectangle(min,max,fieldColor.x, fieldColor.y, fieldColor.z);
        //Data text
        if(!ommit) snprintf(valueString,12,"%6.6f",*data);
        else snprintf(valueString,4,"---");
        DrawTextColored(valueString, lightWhite, min.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

        //Fields selection
        if(editingField<0 && GetMouseButtonDown(SDL_BUTTON_LEFT)){
            if(MouseOverBox(mousePos,min,max,0)){
                textFieldString = (char*)calloc(13,sizeof(char));

                if(!ommit) snprintf(textFieldString, 12, "%6.6f", *data);
                else snprintf(textFieldString, 4, "0.0");
                
                GetTextInput(textFieldString, 12, strlen(textFieldString));
                editingField = *curField;
            }
        }
    }

    //Mark as used ID field
    *curField +=1;

    *curHeight -= 2 + TTF_FontHeight(gizmosFont);
}

void IntField(char *title, int *data,int ommit,int x, int w, int* curField, int* curHeight){
    *curHeight -= 4;
    DrawTextColored(title, lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
    *curHeight -= 2 + TTF_FontHeight(gizmosFontSmall);

    Vector3 min = { x,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 max = { x+w,*curHeight,0};

    if(editingField == *curField)
    {
        //Field background
        DrawRectangle(min,max,fieldEditingColor.x, fieldEditingColor.y, fieldEditingColor.z);

        //Get the cursor position by creating a string containing the characters until the cursor
        //and getting his size when rendered with the used font
        char buff[13];
        strncpy(buff,textFieldString,Input.textInputCursorPos);
        memset(buff+Input.textInputCursorPos,'\0',1);
        int cursorPos,h;
        TTF_SizeText(gizmosFont,buff,&cursorPos,&h);
        cursorPos+=min.x;

        //Cursor line
        DrawLine((Vector3){cursorPos, min.y},
                 (Vector3){cursorPos, max.y},
                 2,0.7,0.7,0.7);

        //Render the string
        DrawTextColored(textFieldString, lightWhite, min.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

        //Pass the string as float data
        *data = (int) strtol(textFieldString, NULL,0);

    }else{
        //Not editing, just draw the box and float normally
        static char valueString[12] = "  0.0";

        //Field background
        DrawRectangle(min,max,fieldColor.x, fieldColor.y, fieldColor.z);
        //Data text
        if(!ommit) snprintf(valueString,12,"%d",*data);
        else snprintf(valueString,4,"---");
        DrawTextColored(valueString, lightWhite, min.x, *curHeight - TTF_FontHeight(gizmosFont), gizmosFont);

        //Fields selection
        if(editingField<0 && GetMouseButtonDown(SDL_BUTTON_LEFT)){
            if(MouseOverBox(mousePos,min,max,0)){
                textFieldString = (char*)calloc(13,sizeof(char));

                if(!ommit) snprintf(textFieldString, 12, "%d", *data);
                else textFieldString[0] = '\0';
                
                GetTextInput(textFieldString, 12, strlen(textFieldString));
                editingField = *curField;
            }
        }
    }

    //Mark as used ID field
    *curField +=1;

    *curHeight -= 2 + TTF_FontHeight(gizmosFont);
}

//-------------------------- Helper functions --------------------------

int MouseOverLineGizmos(Vector3 mousePos, Vector3 originPos, Vector3 handlePos,int mouseOverDistance){
    return DistanceFromPointToLine2D(handlePos,originPos,mousePos)<mouseOverDistance && Distance(mousePos,handlePos)<Distance(handlePos, originPos) && Distance(mousePos,originPos)<Distance(handlePos, originPos);
}

int MouseOverPointGizmos(Vector3 mousePos, Vector3 originPos,int mouseOverDistance){
    return Distance(mousePos,originPos)<mouseOverDistance;
}

int MouseOverBox(Vector3 mousePos, Vector3 min, Vector3 max,int mouseOverDistance){
    return (mousePos.x>min.x-mouseOverDistance && mousePos.x<max.x+mouseOverDistance) && 
           (mousePos.y>min.y-mouseOverDistance && mousePos.y<max.y+mouseOverDistance);
}

Vector3 WorldVectorToScreenVector(Vector3 v){
    Vector3 screenPos;
	screenPos.x = (int)(v.x - v.y)*2 + 0.375;
    screenPos.y = (int)(v.x + v.y) + (v.z)*2 + 0.375;
    screenPos.z = 0;
    return screenPos;
}

void LoadUITexture(char *path,int index){
    SDL_Surface *img = IMG_Load(path);
    if(!img){ printf("Failed to load UI Icon! (%s)\n",path); return; }
    iconsSize[index] = max(img->w,img->h);

    glBindTexture(GL_TEXTURE_2D, iconsTex[index]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[4] = {0,0,0,0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    SDL_FreeSurface(img);
}