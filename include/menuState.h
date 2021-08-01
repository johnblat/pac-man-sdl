#ifndef MENU_STATE_H
#define MENU_STATE_H

#include <SDL2/SDL_mixer.h>
#include "programState.h"
#include "sounds.h"
#include "globalData.h"


typedef enum MenuState {
    TITLE_SCREEN_MENU_STATE,
    MAIN_MENU_SCREEN_MENU_STATE,
    JOIN_GAME_MENU_STATE
} MenuState;


void mainMenuProgramStateEnter();

void menuProgramStateProcess(SDL_Event *event, Entities *entities, LevelConfig *levelConfig, TileMap *tilemap, Blink *startMenuBlink, float deltaTime  );

#endif