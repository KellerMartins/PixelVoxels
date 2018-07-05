#include "EngineInput.h"

/////////////////////////////////External data//////////////////////////////////

//From Engine/EngineCore.c
extern engineTime Time;

////////////////////////////////////////////////////////////////////////////////

engineInput Input;
unsigned char initializedInput = 0;

// ----------- Input functions ---------------

int InitInput(){
	if(initializedInput){
		printf("Input already initialized!\n");
		return 0;
	}
	
	Input.keyboardLast = (Uint8 *)calloc(SDL_NUM_SCANCODES,sizeof(Uint8));
	Input.keyboardCurrent = SDL_GetKeyboardState(NULL);

	Input.mouseX = 0;
	Input.mouseY = 0;
	Input.deltaMouseX = 0;
	Input.deltaMouseY = 0;

	Input.mouseWheelX = 0;
	Input.mouseWheelY = 0;

	Input.textInput = NULL;
	Input.textInputMax = 0;
	Input.textInputLength = 0;
	Input.textInputCursorPos = 0;

	initializedInput = 1;
	return 1;
}

int FreeInput(){
	if(!initializedInput){
		printf("FreeInput: Input not initialized!\n");
		return 0;
	}
	
	free(Input.keyboardLast);
	initializedInput = 0;

	return 1;
}

void InputUpdate(){
    memcpy(Input.keyboardLast,Input.keyboardCurrent,SDL_NUM_SCANCODES*sizeof(Uint8));

	int oldMouseX = Input.mouseX;
	int oldMouseY = Input.mouseY;

	SDL_GetMouseState(&Input.mouseX,&Input.mouseY);

	Input.deltaMouseX = Input.mouseX - oldMouseX;
	Input.deltaMouseY = Input.mouseY - oldMouseY;

	Input.mouseButtonLast[0] = Input.mouseButtonCurrent[0];
	Input.mouseButtonLast[1] = Input.mouseButtonCurrent[1];
	Input.mouseButtonLast[2] = Input.mouseButtonCurrent[2];

	Input.mouseWheelX = 0;
	Input.mouseWheelY = 0;

	int maxl = Input.textInputMax-Input.textInputLength;

    while (SDL_PollEvent(&Input.event)) {

        switch (Input.event.type)
        {
			case SDL_MOUSEWHEEL:			
				Input.mouseWheelX = Input.event.wheel.x;
				Input.mouseWheelY = Input.event.wheel.y;
			break;
            case SDL_QUIT:
                ExitGame();
                break;

			case SDL_MOUSEBUTTONDOWN:
				switch (Input.event.button.button)
				{
					case SDL_BUTTON_LEFT:
						Input.mouseButtonCurrent[0] = 1;
						break;
					case SDL_BUTTON_RIGHT:
						Input.mouseButtonCurrent[2] = 1;
						break;
					case SDL_BUTTON_MIDDLE:
						Input.mouseButtonCurrent[1] = 1;
						break;
				}
			break;

			case SDL_MOUSEBUTTONUP:
				switch (Input.event.button.button)
				{
					case SDL_BUTTON_LEFT:
						Input.mouseButtonCurrent[0] = 0;
						break;
					case SDL_BUTTON_RIGHT:
						Input.mouseButtonCurrent[2] = 0;
						break;
					case SDL_BUTTON_MIDDLE:
						Input.mouseButtonCurrent[1] = 0;
						break;
				}
			break;

			case SDL_TEXTINPUT:
				if(maxl>0){
					char buff[100];
					strncpy(buff, Input.textInput+Input.textInputCursorPos,Input.textInputLength-Input.textInputCursorPos);
					strncpy(Input.textInput+Input.textInputCursorPos, Input.event.text.text,1);
					strncpy(Input.textInput+Input.textInputCursorPos+1, buff,Input.textInputLength-Input.textInputCursorPos);
					Input.textInputLength += 1;
					Input.textInputCursorPos +=1;
				}
			break;
        }
    }

	
	if(SDL_IsTextInputActive()){

		//Repeat action if the key is hold
		static double keyboardHold = 0;
		keyboardHold+= Time.deltaTime;

		//Backspace delete
		if(GetKeyDown(SDL_SCANCODE_BACKSPACE) || (GetKey(SDL_SCANCODE_BACKSPACE) && keyboardHold >= 0.05)){
			if(Input.textInputCursorPos>0){
				memmove(Input.textInput + Input.textInputCursorPos-1,Input.textInput + Input.textInputCursorPos,Input.textInputLength - Input.textInputCursorPos);
				memset(Input.textInput + Input.textInputLength-1, '\0',1);
				Input.textInputLength -= 1;
				Input.textInputCursorPos -=1;
			}
			keyboardHold = 0;
		}
		
		//Del delete
		if(GetKeyDown(SDL_SCANCODE_DELETE) || (GetKey(SDL_SCANCODE_DELETE) && keyboardHold >= 0.05)){//string
			if(Input.textInputLength-Input.textInputCursorPos>0){
				//Deletes the character in the cursor position by moving the sucessing characters to the left and setting the last character as a '\0'
				memmove(Input.textInput + Input.textInputCursorPos,Input.textInput + Input.textInputCursorPos+1,Input.textInputLength - (Input.textInputCursorPos + 1));
				memset(Input.textInput + Input.textInputLength-1, '\0',1);
				Input.textInputLength -= 1;
			}
			keyboardHold = 0;
		}
		
		//Cursor movement
		if(GetKeyDown(SDL_SCANCODE_LEFT) || (GetKey(SDL_SCANCODE_LEFT) && keyboardHold >= 0.05)){
			Input.textInputCursorPos = Input.textInputCursorPos<1? 0:Input.textInputCursorPos-1;
			keyboardHold = 0;
		}
		if(GetKeyDown(SDL_SCANCODE_RIGHT) || (GetKey(SDL_SCANCODE_RIGHT) && keyboardHold >= 0.05)){
			Input.textInputCursorPos = Input.textInputCursorPos<Input.textInputLength? Input.textInputCursorPos+1:Input.textInputLength;
			keyboardHold = 0;
		}

		//Home and End shortcuts
		if(GetKeyDown(SDL_SCANCODE_HOME)){
			Input.textInputCursorPos = 0;
		}
		if(GetKeyDown(SDL_SCANCODE_END)){
			Input.textInputCursorPos = Input.textInputLength;
		}

		//Increase delay between the key press and key repetition
		if(GetKeyDown(SDL_SCANCODE_BACKSPACE) || GetKeyDown(SDL_SCANCODE_DELETE) || GetKeyDown(SDL_SCANCODE_LEFT) || GetKeyDown(SDL_SCANCODE_RIGHT))
			keyboardHold = -0.5;
	}
}

int GetKey(SDL_Scancode key){
	if(!initializedInput){
		printf("GetKey: Input not initialized!\n");
		return 0;
	}
	if(key >= SDL_NUM_SCANCODES){
		printf("GetKey: Key out of bounds! (%d)\n",key);
		return 0;
	}
	return Input.keyboardCurrent[key];
}
int GetKeyDown(SDL_Scancode key){
	if(!initializedInput){
		printf("GetKeyDown: Input not initialized!\n");
		return 0;
	}
	if(key >= SDL_NUM_SCANCODES){
		printf("GetKeyDown: Key out of bounds! (%d)\n",key);
		return 0;
	}
	return (Input.keyboardCurrent[key] && !Input.keyboardLast[key]);
} 
int GetKeyUp(SDL_Scancode key){
	if(!initializedInput){
		printf("GetKeyUp: Input not initialized!\n");
		return 0;
	}
	if(key >= SDL_NUM_SCANCODES){
		printf("GetKeyUp: Key out of bounds! (%d)\n",key);
		return 0;
	}
	return (!Input.keyboardCurrent[key] && Input.keyboardLast[key]);
} 


int GetMouseButton(int button){
	if(!initializedInput){
		printf("GetMouseButton: Input not initialized!\n");
		return 0;
	}
	if(button < SDL_BUTTON_LEFT || button > SDL_BUTTON_RIGHT){
		printf("GetMouseButton: Button out of bounds! (%d)\n",button);
		return 0;
	}
	return (Input.mouseButtonCurrent[button-SDL_BUTTON_LEFT]);
}
int GetMouseButtonDown(int button){
	if(!initializedInput){
		printf("GetMouseButtonDown: Input not initialized!\n");
		return 0;
	}
	if(button < SDL_BUTTON_LEFT || button > SDL_BUTTON_RIGHT){
		printf("GetMouseButtonDown: Button out of bounds! (%d)\n",button);
		return 0;
	}
	return (Input.mouseButtonCurrent[button-SDL_BUTTON_LEFT] && !Input.mouseButtonLast[button-SDL_BUTTON_LEFT]);
}
int GetMouseButtonUp(int button){
	if(!initializedInput){
		printf("GetMouseButtonUp: Input not initialized!\n");
		return 0;
	}
	if(button < SDL_BUTTON_LEFT || button > SDL_BUTTON_RIGHT){
		printf("GetMouseButtonUp: Button out of bounds! (%d)\n",button);
		return 0;
	}
	return (!Input.mouseButtonCurrent[button-SDL_BUTTON_LEFT] && Input.mouseButtonLast[button-SDL_BUTTON_LEFT]);
}

void GetTextInput(char* outputTextPointer, int maxLength, int currentLength){
	if(SDL_IsTextInputActive()){
		printf("GetTextInput: Text input active, stop with StopTextInput() before calling again.\n");
		return;
	}

    SDL_StartTextInput();
	Input.textInput = outputTextPointer;
	Input.textInputMax = maxLength;
	Input.textInputLength = currentLength;
	Input.textInputCursorPos = currentLength; 
}

void StopTextInput(){
	if(!SDL_IsTextInputActive()){
		printf("StopTextInput: Text input already disabled.\n");
		return;
	}
    SDL_StopTextInput();
	Input.textInput = NULL;
	Input.textInputMax = 0;
	Input.textInputLength = 0;
	Input.textInputCursorPos = 0;
}