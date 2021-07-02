#ifndef LEVELCONF_H
#define LEVELCONF_H

#include <SDL2/SDL.h>
#include "inttypes.h"

typedef struct LevelConfig {
    uint8_t *scatterChasePeriodSeconds;
    uint8_t numScatterChasePeriods;
    SDL_Point ghostPenTile;
    SDL_Point pacStartingTile;
    float baseSpeed;
    SDL_Point powerPelletTiles[ 4 ];
} LevelConfig;

#endif