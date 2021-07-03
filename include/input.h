#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL_gamecontroller.h>



SDL_GameController *g_GameControllers[] = {NULL, NULL};


unsigned int g_NumJoysticks = 0;
unsigned int g_NumGamepads = 0;
unsigned int g_INPUT_UP =                0b10000;
unsigned int g_INPUT_LEFT =              0b01000;
unsigned int g_INPUT_RIGHT =             0b00100;
unsigned int g_INPUT_DOWN =              0b00010;
unsigned int g_INPUT_ACTION =            0b00001;

// void GameControllersInputProcess( SDL_GameController **gameControllers, uint8_t *inputMasks ) {
//     for( int i = 0; i < MAX_NUM_ENTITIES; ++i ) {

//     }
// }

#endif