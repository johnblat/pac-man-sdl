#ifndef ENTITY_H
#define ENTITY_H

#include <SDL2/SDL_gamecontroller.h>
#include "render.h"
#include "actor.h"
#include "ghostStates.h"
#include "animation.h"
#include "levelConfig.h"
#include "render.h"
#include "inttypes.h"

#define MAX_NUM_ENTITIES 13

typedef unsigned int EntityId;
extern unsigned int g_NumEntities;

typedef enum {
    POWER_PELLET_PICKUP,
    FRUIT_PICKUP
} PickupType;

typedef struct Entities {
    Position           *positions         [ MAX_NUM_ENTITIES ];
    Actor              *actors            [ MAX_NUM_ENTITIES ]; 
    AnimatedSprite     *animatedSprites   [ MAX_NUM_ENTITIES ]; 
    RenderData         *renderDatas       [ MAX_NUM_ENTITIES ]; 
    GhostState         *ghostStates       [ MAX_NUM_ENTITIES ];
    TargetingBehavior  *targetingBehaviors[ MAX_NUM_ENTITIES ]; 
    float              *chargeTimers      [ MAX_NUM_ENTITIES ];
    float              *dashTimers        [ MAX_NUM_ENTITIES ];
    float              *slowTimers        [ MAX_NUM_ENTITIES ];
    uint8_t            *inputMasks        [ MAX_NUM_ENTITIES ];
    SDL_GameController *gameControllers   [ MAX_NUM_ENTITIES ];
    PickupType         *pickupTypes       [ MAX_NUM_ENTITIES ];
} Entities;

EntityId createPlayer( Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite );

EntityId createGhost(  Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite, TargetingBehavior targetingBehavior );

EntityId createPowerPellet(Entities *entities, AnimatedSprite *animatedSprite, SDL_Point tile );

void collectDotProcess( Entities *entities, char dots[ TILE_ROWS ][ TILE_COLS ], unsigned int *num_dots, Score *score, SDL_Renderer *renderer );

#endif