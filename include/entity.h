#ifndef ENTITY_H
#define ENTITY_H

#include "actor.h"
#include "targeting.h"
#include "animation.h"
#include "resources.h"
#include "render.h"
#include "inttypes.h"

#define MAX_NUM_ENTITIES 13

typedef unsigned int EntityId;
extern unsigned int g_NumEntities;


typedef struct {
    Position                    *positions        [ MAX_NUM_ENTITIES ];
    Actor                       *actors           [ MAX_NUM_ENTITIES ]; 
    AnimatedSprite              *animatedSprites  [ MAX_NUM_ENTITIES ]; // pac-man, ghosts, power-pellets, eyes, etc
    RenderClipFromTextureAtlas  *render_clips     [ MAX_NUM_ENTITIES ]; // for render textures, 5 thru 9 only different is the Rect x and y 
    GhostState                  *ghostStates      [ MAX_NUM_ENTITIES ];
    TargetingBehavior           *targetingBehaviors[ MAX_NUM_ENTITIES ]; 
    float                       *chargeTimers     [ MAX_NUM_ENTITIES ];
    float                       *dashTimers       [ MAX_NUM_ENTITIES ];
    float                       *slowTimers       [ MAX_NUM_ENTITIES ];
    uint8_t                     *inputMasks       [ MAX_NUM_ENTITIES ];
} Entities;

EntityId createPlayer( Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite, RenderClipFromTextureAtlas *renderClip );

EntityId createGhost(  Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite, RenderClipFromTextureAtlas *renderClip, TargetingBehavior targetingBehavior );

EntityId createPowerPellet( Entities *entities, AnimatedSprite *animatedSprite, RenderClipFromTextureAtlas *renderClip );

#endif