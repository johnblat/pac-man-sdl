#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL_gamecontroller.h>

SDL_GameController *g_GameController = NULL;
unsigned int g_NumJoysticks = 0;
unsigned int g_NumGamepads = 0;
unsigned int g_GameControllerButtonMask = 0b00000;
unsigned int g_GAMEPAD_UP =               0b10000;
unsigned int g_GAMEPAD_LEFT =             0b01000;
unsigned int g_GAMEPAD_RIGHT =            0b00100;
unsigned int g_GAMEPAD_DOWN =             0b00010;
unsigned int g_GAMEPAD_ACTION =           0b00001;

#endif