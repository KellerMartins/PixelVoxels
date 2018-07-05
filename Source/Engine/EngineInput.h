#ifndef ENGINEINPUT_H
#define ENGINEINPUT_H

#include <SDL2/SDL.h>
#include "../utils.h"

#include "EngineCore.h"

typedef struct engineInput{
    //Keyboard key state arrays
    const Uint8 *keyboardCurrent;
    Uint8 *keyboardLast;

    int mouseX;
    int mouseY;
    int deltaMouseX;
    int deltaMouseY;

    int mouseButtonCurrent[3];
    int mouseButtonLast[3];

    int mouseWheelX;
    int mouseWheelY;

    char *textInput;
    unsigned textInputMax;
    unsigned textInputLength;
    unsigned textInputCursorPos;

    SDL_Event event;
}engineInput;


//Input functions
int InitInput();
int FreeInput();
void InputUpdate();
int GetKey(SDL_Scancode key);
int GetKeyDown(SDL_Scancode key);
int GetKeyUp(SDL_Scancode key);

int GetMouseButton(int button);
int GetMouseButtonDown(int button);
int GetMouseButtonUp(int button);

void GetTextInput(char* outputTextPointer, int maxLength, int currentLength);
void StopTextInput();


#endif