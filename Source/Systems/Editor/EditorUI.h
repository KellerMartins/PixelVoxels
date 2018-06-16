#ifndef EDITORUI_H
#define EDITORUI_H

#include "../../Engine.h"
#include "../UIRenderer.h"

#define FIELD_MAX_CHARS 13

void DrawRectangle(Vector3 min, Vector3 max, float r, float g, float b);
void DrawPointIcon(Vector3 pos,int iconID, int scale, Vector3 color);

void FloatBoxActive(float *data,int ommit, Vector3 pos, Vector3 size,int intDigits,int decDigits);
void FloatBoxInactive(int fieldID, float *data,int ommit, Vector3 pos, Vector3 size,int intDigits,int decDigits, int activeIntDigits,int activeDecDigits);

void StringField(char *title, char *data, int maxChars, int ommit,int x, int w, int* curField, int* curHeight);
void SliderField(char *title, float *data, Vector3 range, int ommit, int x, int w, int* curField, int* curHeight);
void RGBField(char *title, Vector3 *data,int ommitR,int ommitG,int ommitB,int x, int w, int* curField, int* curHeight);
void Vector3Field(char *title, Vector3 *data,int ommitX,int ommitY,int ommitZ,int x, int w, int fieldsSpacing, int* curField, int* curHeight);
void FloatField(char *title, float *data,int ommit,int x, int w, int* curField, int* curHeight);
void IntListField(char *title, List *list,int x, int w, int* curField, int* curHeight);

double Slider(double t, int x, int y, int w,int iconID, int scale, Vector3 defaultColor, Vector3 mouseOverColor, Vector3 pressedColor);
int PointButton(Vector3 pos,int iconID, int scale, Vector3 defaultColor, Vector3 mouseOverColor, Vector3 pressedColor);
int PointToggle(int *data,Vector3 pos,int onIconID, int offIconID, int undefinedIconID, int scale, Vector3 onColor, Vector3 offColor, Vector3 undefinedColor, Vector3 mouseOverColor);

void InitEditorUI();
void FreeEditorUI();
void LoadUITexture(char *path,int index);

int MouseOverLine(Vector3 mousePos, Vector3 originPos, Vector3 handlePos,int mouseOverDistance);
int MouseOverPoint(Vector3 mousePos, Vector3 originPos,int mouseOverDistance);
int MouseOverBox(Vector3 mousePos, Vector3 min, Vector3 max,int mouseOverDistance);
Vector3 WorldVectorToScreenVector(Vector3 v);

#endif