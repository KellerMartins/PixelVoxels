#include "EditorUI.h"

/////////////////////////////////External data//////////////////////////////////

//Engine data
extern engineScreen Screen;
extern engineInput Input;
extern engineCore Core;

//Color definitions from Editor.c
extern Vector3 fieldColor;
extern Vector3 fieldEditingColor;
extern Vector3 lightWhite;
extern Vector3 brightWhite;

//Data from Editor.c
extern Vector3 mousePos;
extern Vector3 deltaMousePos;
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

    if(MouseOverPoint(mousePos, pos, scale * iconsSize[iconID])){
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

        if(MouseOverPoint(mousePos, pos, scale * iconSize)){
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

void Vector3Field(char *title, Vector3 *data, double dragAmount,int ommitX,int ommitY,int ommitZ,int x, int w, int fieldsSpacing, int* curField, int* curHeight){
    *curHeight -= 2;
    DrawTextColored(title, lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
    *curHeight -= 2 + TTF_FontHeight(gizmosFontSmall);

    double fieldW = w/3.0 - fieldsSpacing/1.5;


    Vector3 size = { fieldW, TTF_FontHeight(gizmosFont)+2,0};
    Vector3 posX = { x,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 posY = { posX.x + size.x + fieldsSpacing,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 posZ = { posY.x + size.x + fieldsSpacing,*curHeight-TTF_FontHeight(gizmosFont)-2,0};

    Vector3 activePos = { x,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 activeSize = { w,TTF_FontHeight(gizmosFont)+2,0};

    int fieldDelta = editingField - *curField;
    int lastActiveField = editingField;
    switch(fieldDelta){
        //X is being edited
        case (0): FloatBoxActive(&(data->x),ommitX, activePos, activeSize, 5,6,dragAmount);
        break;

        //Y is being edited
        case (1): FloatBoxActive(&(data->y),ommitY, activePos, activeSize, 5,6,dragAmount);
        break;

        //Z is being edited
        case (2): FloatBoxActive(&(data->z),ommitZ, activePos, activeSize, 5,6,dragAmount);
        break;

        //None of the three are being edited
        default:
            FloatBoxInactive(*curField + 0,&(data->x),ommitX, posX, size, 1,2 , 5,6);
            FloatBoxInactive(*curField + 1,&(data->y),ommitY, posY, size, 1,2 , 5,6);
            FloatBoxInactive(*curField + 2,&(data->z),ommitZ, posZ, size, 1,2 , 5,6);

            //Draw an empty box in case any of the fields has been selected, to make an smoother transition to the next frame with the text
            if(lastActiveField != editingField){
                DrawRectangle(activePos, Add(activeSize,activePos), fieldEditingColor.x, fieldEditingColor.y, fieldEditingColor.z);
            }
        break;
    }

    *curHeight -= 2 + TTF_FontHeight(gizmosFont);
    *curField +=3;
}

void RGBField(char *title, Vector3 *data,int ommitR,int ommitG,int ommitB,int x, int w, int* curField, int* curHeight){
    
    *curHeight -= 4;
    DrawTextColored(title, lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
    *curHeight -= 2 + TTF_FontHeight(gizmosFontSmall);


    *curHeight -= 4;
    DrawRectangle((Vector3){x,*curHeight-TTF_FontHeight(gizmosFont)-2}, 
                  (Vector3){x+w,*curHeight},
                  data->x,data->y,data->z);
    *curHeight -= 16 + TTF_FontHeight(gizmosFontSmall)*2;


    DrawTextColored("r", lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall)/2, gizmosFontSmall);
    double sliderVal = Slider(data->x, x+12, *curHeight, w-17, 14, 1, lightWhite, brightWhite, brightWhite);
    if(sliderVal > 0){
        data->x = sliderVal;
    }
    *curHeight -= 20;

    DrawTextColored("G", lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall)/2, gizmosFontSmall);
    sliderVal = Slider(data->y, x+12, *curHeight, w-17, 14, 1, lightWhite, brightWhite, brightWhite);
    if(sliderVal > 0){
        data->y = sliderVal;
    }
    *curHeight -= 20;

    DrawTextColored("b", lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall)/2, gizmosFontSmall);
    sliderVal = Slider(data->z, x+12, *curHeight, w-17, 14, 1, lightWhite, brightWhite, brightWhite);
    if(sliderVal > 0){
        data->z = sliderVal;
    }
    *curHeight -= 12;
}

void SliderField(char *title, float *data, Vector3 range, int ommit, int x, int w, int* curField, int* curHeight){
    
    *curHeight -= 4;
    DrawTextColored(title, lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
    *curHeight -= 8 + TTF_FontHeight(gizmosFontSmall);

    if(editingField == *curField){
            FloatBoxActive(data, ommit, (Vector3){x, *curHeight - iconsSize[14]}, (Vector3){w, iconsSize[14]*2}, 5,5,(range.y - range.x)*100);
    }else{
        double sliderVal = Slider((*data-range.x)/(range.y - range.x), x+2, *curHeight, w-7, 14, 1, lightWhite, brightWhite, brightWhite);
        if(sliderVal > 0){
            *data = (sliderVal * (range.y - range.x)) + range.x;
        }

        if(editingField == -1 && MouseOverBox(mousePos, (Vector3){x, *curHeight - iconsSize[14]}, (Vector3){x+w, *curHeight + iconsSize[14]}, 0) && GetMouseButtonDown(SDL_BUTTON_RIGHT)){
            textFieldString = (char*)calloc(FIELD_MAX_CHARS+1,sizeof(char));

            if(!ommit) snprintf(textFieldString, FIELD_MAX_CHARS, "%*.*f", 5, 5, *data);
            else snprintf(textFieldString, 4, "0.0");
            
            GetTextInput(textFieldString, FIELD_MAX_CHARS, strlen(textFieldString));
            editingField = *curField;
        }
    }

    *curHeight -= 12;
    *curField +=1;
}

void FloatField(char *title, float *data, double dragAmount,int ommit,int x, int w, int* curField, int* curHeight){
    *curHeight -= 4;
    DrawTextColored(title, lightWhite, x, *curHeight - TTF_FontHeight(gizmosFontSmall), gizmosFontSmall);
    *curHeight -= 2 + TTF_FontHeight(gizmosFontSmall);

    Vector3 pos = { x,*curHeight-TTF_FontHeight(gizmosFont)-2,0};
    Vector3 size = { w,TTF_FontHeight(gizmosFont)+2,0};

    if(*curField == editingField){
        FloatBoxActive(data, ommit, pos, size, 6,6,dragAmount);
    }else{
        FloatBoxInactive(*curField, data, ommit, pos, size, 6,6, 6,6);
    }
    *curHeight -= 2 + TTF_FontHeight(gizmosFont);
    *curField +=1;
}

void FloatBoxActive(float *data,int ommit, Vector3 pos, Vector3 size,int intDigits,int decDigits, double dragAmount){

    //Limits the number of decimal and integer digits to fit into the maximum field size
    intDigits = intDigits > FIELD_MAX_CHARS? FIELD_MAX_CHARS: intDigits;

    //Calculates the number of decimal digits if the requested amount overflows the string. The 1 represents the decimal dot character
    decDigits = intDigits + decDigits + 1 > FIELD_MAX_CHARS? FIELD_MAX_CHARS-(intDigits+1) : decDigits;

    //Remove a decimal digit if the number has a sign, to make the positive and negative numbers have about the same length
    decDigits -= (*data < 0) && decDigits>0 ? 1 : 0;

    //Define the rect
    Vector3 min = pos;
    Vector3 max = { pos.x + size.x, pos.y + size.y};


    //Field background
    DrawRectangle(min,max,fieldEditingColor.x, fieldEditingColor.y, fieldEditingColor.z);

    if(GetMouseButton(SDL_BUTTON_LEFT) && !GetMouseButtonDown(SDL_BUTTON_LEFT)){
        
        //Loops the mouse movement after passing the edge of the screen
        int x, y;
        SDL_CaptureMouse(SDL_TRUE);
        SDL_GetMouseState(&x,&y);
        SDL_WarpMouseInWindow(Core.window, Modulus(x, Screen.windowWidth) , y);

        //Ignore the jump of the mouse position caused from the loop from one edge to the other
        int mouseLooped = (x == Modulus(x, Screen.windowWidth) && abs(deltaMousePos.x) < Screen.gameWidth);
        *data += ( mouseLooped ? deltaMousePos.x : 0 ) / dragAmount;

        
        const size_t fieldLength = intDigits + decDigits + 1 // 1 for the \n
                                    + (*data < pow(10,intDigits)) //+1 for the dot, if the int. part fits in the requested int. size
                                    + (*data < 0); //+1 for the minus, if negative

        snprintf(textFieldString,fieldLength,"%*.*f",intDigits, decDigits,*data);


    }else{
        SDL_CaptureMouse(SDL_FALSE);
    }

    //Get the cursor position by creating a string containing the characters until the cursor
    //and getting his size when rendered with the used font
    char buff[FIELD_MAX_CHARS];
    strncpy(buff,textFieldString,Input.textInputCursorPos);
    memset(buff+Input.textInputCursorPos,'\0',1);
    int cursorPos,h;
    TTF_SizeText(gizmosFont,buff,&cursorPos,&h);
    cursorPos+=min.x+1;

    //Cursor line
    DrawLine((Vector3){cursorPos, min.y},
                (Vector3){cursorPos, max.y},
                2,0.7,0.7,0.7);

    //Render the string
    DrawTextColored(textFieldString, lightWhite, min.x + 1, min.y + (size.y - TTF_FontHeight(gizmosFont))/2 +1, gizmosFont);

    //Pass the string as float data
    *data = strtof(textFieldString, NULL);
}

void FloatBoxInactive(int fieldID, float *data,int ommit, Vector3 pos, Vector3 size,int intDigits,int decDigits, int activeIntDigits,int activeDecDigits){

    //Limits the number of decimal and integer digits to fit into the maximum field size
    intDigits = intDigits > FIELD_MAX_CHARS? FIELD_MAX_CHARS: intDigits;

    //Calculates the number of decimal digits if the requested amount overflows the string. The 1 represents the decimal dot character
    decDigits = intDigits + decDigits + 1 > FIELD_MAX_CHARS? FIELD_MAX_CHARS-(intDigits+1) : decDigits;
    //Remove a decimal digit if the number has a sign, to make the positive and negative numbers have about the same length
    decDigits -= (*data < 0) && decDigits>0 ? 1 : 0;
    

    //Define the rect
    Vector3 min = pos;
    Vector3 max = { pos.x + size.x, pos.y + size.y};

    //Not editing, just draw the box and float normally
    static char valueString[FIELD_MAX_CHARS+1] = "0.0";

    //Field background
    DrawRectangle(min,max,fieldColor.x, fieldColor.y, fieldColor.z);

    //Data text
    if(!ommit){
        const size_t fieldLength = intDigits + decDigits + 1 // 1 for the \n
                                    + (*data < pow(10,intDigits)) //+1 for the dot, if the int. part fits in the requested int. size
                                    + (*data < 0); //+1 for the minus, if negative

        snprintf(valueString,fieldLength,"%*.*f",intDigits, decDigits,*data);
    }
    else{
        snprintf(valueString,4,"---");
    }
    DrawTextColored(valueString, lightWhite, min.x + 1, min.y + (size.y - TTF_FontHeight(gizmosFont))/2 +1, gizmosFont);


    //Field selection
    if(editingField<0 && GetMouseButtonDown(SDL_BUTTON_LEFT)){
        if(MouseOverBox(mousePos,min,max,0)){
            textFieldString = (char*)calloc(FIELD_MAX_CHARS+1,sizeof(char));

            if(!ommit) snprintf(textFieldString, FIELD_MAX_CHARS, "%*.*f", activeIntDigits, activeDecDigits, *data);
            else snprintf(textFieldString, 4, "0.0");
            
            GetTextInput(textFieldString, FIELD_MAX_CHARS, strlen(textFieldString));
            editingField = fieldID;
        }
    }

}

double Slider(double t, int x, int y, int w,int iconID, int scale, Vector3 defaultColor, Vector3 mouseOverColor, Vector3 pressedColor){
    DrawLine((Vector3){x, y+1}, (Vector3){x+w, y+1}, (iconsSize[iconID]-4)*2*scale, fieldColor.x, fieldColor.y, fieldColor.z);

    if( MouseOverLine(mousePos, (Vector3){x, y}, (Vector3){x+w+1, y},(iconsSize[iconID]-2)*scale) && GetMouseButton(SDL_BUTTON_LEFT)){
        t = ( (mousePos.x - x) / (double) w );
        t = clamp(t, 0, 1);
        DrawPoint((Vector3){(x+5)+t*(w-5), y}, iconsSize[iconID], iconsTex[iconID]*scale, defaultColor.x, defaultColor.y, defaultColor.z);

        return t;
    }else{
        DrawPoint((Vector3){clamp((x+5)+t*(w-5),x,x+w), y}, iconsSize[iconID], iconsTex[iconID]*scale, defaultColor.x, defaultColor.y, defaultColor.z);
    }


    return -1;
}

//-------------------------- Helper functions --------------------------

int MouseOverLine(Vector3 mousePos, Vector3 originPos, Vector3 handlePos,int mouseOverDistance){
    return DistanceFromPointToLine2D(handlePos,originPos,mousePos)<mouseOverDistance && Distance(mousePos,handlePos)<Distance(handlePos, originPos) && Distance(mousePos,originPos)<Distance(handlePos, originPos);
}

int MouseOverPoint(Vector3 mousePos, Vector3 originPos,int mouseOverDistance){
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