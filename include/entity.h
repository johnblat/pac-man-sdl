#ifndef ENTITY_H
#define ENTITY_H

#include <SDL2/SDL_gamecontroller.h>
#include "actor.h"
#include "ghostStates.h"
#include "animation.h"
#include "levelConfig.h"
#include "render.h"
#include "inttypes.h"

#define MAX_NUM_ENTITIES 13

typedef unsigned int EntityId;
extern unsigned int g_NumEntities;


typedef struct Entities {
    Position                    *positions        [ MAX_NUM_ENTITIES ];
    Actor                       *actors           [ MAX_NUM_ENTITIES ]; 
    AnimatedSprite              *animatedSprites  [ MAX_NUM_ENTITIES ]; // pac-man, ghosts, power-pellets, eyes, etc
    RenderData                  *renderDatas     [ MAX_NUM_ENTITIES ]; // for render textures, 5 thru 9 only different is the Rect x and y 
    GhostState                  *ghostStates      [ MAX_NUM_ENTITIES ];
    TargetingBehavior           *targetingBehaviors[ MAX_NUM_ENTITIES ]; 
    float                       *chargeTimers     [ MAX_NUM_ENTITIES ];
    float                       *dashTimers       [ MAX_NUM_ENTITIES ];
    float                       *slowTimers       [ MAX_NUM_ENTITIES ];
    uint8_t                     *inputMasks       [ MAX_NUM_ENTITIES ];
    SDL_GameController          *gameControllers  [ MAX_NUM_ENTITIES ];
} Entities;

EntityId createPlayer( Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite );

EntityId createGhost(  Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite, TargetingBehavior targetingBehavior );

EntityId createPowerPellet( Entities *entities, AnimatedSprite *animatedSprite, RenderData *renderClip );

void collectDotProcess( Entities *entities, char dots[ TILE_ROWS ][ TILE_COLS ], unsigned int *num_dots, Score *score, SDL_Renderer *renderer );

#endif