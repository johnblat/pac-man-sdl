#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL.h>

#define MAX_NUM_GAME_CONTROLLERS 4

unsigned int g_NumJoysticks = 0;
unsigned int g_NumGamepads = 0;

typedef enum InputMask {
    g_INPUT_UP =                0b10000,
    g_INPUT_LEFT =              0b01000,
    g_INPUT_RIGHT =             0b00100,
    g_INPUT_DOWN =              0b00010,
    g_INPUT_ACTION =            0b00001
} InputMask;

typedef struct SDL_ScancodeToInputMask {
    SDL_Scancode scancode;
    InputMask inputMask;
} SDL_ScancodeToInputMask;

SDL_ScancodeToInputMask gkeyBindings[ 5 ];

SDL_GameController *g_GameControllers[] = {NULL, NULL, NULL, NULL};

typedef unsigned int GameControllerId;

void addKeyBinding(unsigned int keyBindId, SDL_Scancode scancode, InputMask inputMask ) {
    gkeyBindings[ keyBindId ].scancode = scancode;
    gkeyBindings[ keyBindId ].inputMask = inputMask;
}


// void GameControllersInputProcess( SDL_GameController **gameControllers, uint8_t *inputMasks ) {
//     for( int i = 0; i < MAX_NUM_ENTITIES; ++i ) {

//     }
// }

#endif