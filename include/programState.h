#ifndef PROG_STATE_H
#define PROG_STATE_H

#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include "entity.h"
#include "levelConfig.h"
#include "tiles.h"
#include "sounds.h"
#include "gamePlayingState.h"

typedef enum ProgramState {
    MENU_PROGRAM_STATE,
    GAME_PLAYING_PROGRAM_STATE,
    EXIT_STATE
} ProgramState;


void gamePlayProgramStateEnter( Entities *entities, TileMap *tilemap, LevelConfig *levelConfig );

#endif