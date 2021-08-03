#ifndef LEVELCONF_H
#define LEVELCONF_H

#include <SDL2/SDL.h>
#include "inttypes.h"
#include "pickup.h"

#define MAX_PICKUPS_PER_LEVEL 4

typedef struct PickupConfig {
    unsigned int textureAtlasId;
    unsigned int numDots;
    float activeTime;
    unsigned int scoreReward;
    PickupType pickupType;
} PickupConfig;

typedef struct LevelConfig {
    uint32_t *scatterChasePeriodSeconds;
    uint8_t numScatterChasePeriods;
    SDL_Point ghostPenTile;
    SDL_Point pacStartingTile;
    float baseSpeed;
    SDL_Point powerPelletTiles[ 4 ];
    PickupConfig pickupConfigs[ MAX_PICKUPS_PER_LEVEL ];
    unsigned int numPickupConfigs;
    int ghostVulnerableDuration;
} LevelConfig;

#endif