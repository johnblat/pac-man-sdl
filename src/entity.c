#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "sounds.h"
#include "comparisons.h"
#include "stdio.h"
#include "jb_types.h"
#include "animation.h"
#include "ghostStates.h"
#include "states.h"
#include "render.h"
#include "targeting.h"
#include "levelConfig.h"
#include "actor.h"
#include "entity.h"
#include "UI.h"
#include "globalData.h"


void allGhostsVulnerableStateEnter( Entities *entities, LevelConfig *levelConfig ) {
    for( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
        if( entities->ghostStates[ eid ] == NULL ) {
            continue;
        }
        if ( *entities->ghostStates[ eid ] == STATE_NORMAL ) {

            *entities->ghostStates[ eid ] = STATE_VULNERABLE;
            vulnerable_enter( entities, eid );
        }
        
    }   
    gGhostVulnerableTimer = levelConfig->ghostVulnerableDuration;  
    g_NumGhostsEaten = 0;
}

unsigned int g_NumEntities = 0;

EntityId createPlayer( Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite ) {
    EntityId entityId = g_NumEntities;
    g_NumEntities++;

    // allocate
    entities->worldPositions[entityId] = (Position_f *)malloc(sizeof(Position_f));
    entities->currentTiles[entityId] = (SDL_Point *)malloc(sizeof(SDL_Point));
    entities->nextTiles[entityId] = (SDL_Point *)malloc(sizeof(SDL_Point));
    entities->baseSpeeds[entityId] = (float *)malloc(sizeof(float));
    entities->speedMultipliers[entityId] = (float *)malloc(sizeof(float));
    entities->velocities[entityId] = (Vector_f *)malloc(sizeof(Vector_f));
    entities->chargeTimers[entityId] = (float * )malloc(sizeof(float));
    entities->dashTimers[entityId] = (float * )malloc(sizeof(float ) );
    entities->slowTimers[entityId] = ( float * )malloc(sizeof(float ) );
    entities->inputMasks[entityId] = (uint8_t * ) malloc(sizeof( uint8_t ) );
    entities->dashCooldownStocks[entityId] = (CooldownStock *)malloc( sizeof( CooldownStock ) );
    entities->invincibilityTimers[entityId] = (float *)malloc(sizeof(float));
    entities->stopTimers[entityId] = (float *)malloc(sizeof(float));
    entities->isActive[entityId] = (SDL_bool *)malloc(sizeof(SDL_bool));
    entities->deathTimers[entityId] = (float *)malloc(sizeof(float));
    entities->respawnTimers[entityId] = (float *)malloc(sizeof(float));
    entities->collisionRects[entityId] = (SDL_Rect *)malloc(sizeof(SDL_Rect));
    entities->sensors[entityId] = (Sensor *)malloc(sizeof(Sensor));
    entities->directions[entityId] = (Direction *)malloc(sizeof(Direction));

    //initialize

    *entities->worldPositions[entityId] = (Position_f){
        levelConfig->pacStartingTile.x * TILE_SIZE + (ACTOR_SIZE*0.5), 
        levelConfig->pacStartingTile.y * TILE_SIZE + (ACTOR_SIZE*0.5)
    };

    *entities->currentTiles[entityId] = levelConfig->pacStartingTile;

    entities->sensors[entityId]->worldTopSensor = (SDL_Point){
        entities->worldPositions[entityId]->x,
        entities->worldPositions[entityId]->y - (ACTOR_SIZE*0.5)
    };

    entities->sensors[entityId]->worldBottomSensor = (SDL_Point){
        entities->worldPositions[entityId]->x,
        entities->worldPositions[entityId]->y + (ACTOR_SIZE*0.5)
    };

    entities->sensors[entityId]->worldLeftSensor = (SDL_Point){
        entities->worldPositions[entityId]->x - (ACTOR_SIZE*0.5),
        entities->worldPositions[entityId]->y 
    };

    entities->sensors[entityId]->worldRightSensor = (SDL_Point){
        entities->worldPositions[entityId]->x + (ACTOR_SIZE*0.5),
        entities->worldPositions[entityId]->y
    };

    *entities->velocities[entityId] = (Vector_f){0.0f, 0.0f};

    *entities->directions[entityId] = DIR_NONE;

    *entities->nextTiles[entityId] = *entities->currentTiles[entityId];

    *entities->baseSpeeds[entityId] = levelConfig->baseSpeed;

    *entities->speedMultipliers[entityId] = 1.0f;

    entities->animatedSprites[ entityId ] = animatedSprite;

    entities->renderDatas[ entityId ] = renderDataInit();

    *entities->chargeTimers[ entityId ] = 0.0f;

    *entities->dashTimers[ entityId ] = 0.0f;

    *entities->slowTimers[ entityId ] = 0.0f;

    *entities->inputMasks[ entityId ] = 0b00000;

    entities->dashCooldownStocks[entityId]->cooldownDuration = 3.0f;
    entities->dashCooldownStocks[entityId]->cooldownTimer = 0.0f;
    entities->dashCooldownStocks[entityId]->currentNumStock = 3;
    entities->dashCooldownStocks[entityId]->numStockCap = 3;

    *entities->invincibilityTimers[entityId] = 0.0f;

    *entities->stopTimers[entityId] = 0.0f;
    
    *entities->isActive[ entityId ] = SDL_FALSE;

    *entities->deathTimers[entityId] = 0.0f;

    *entities->respawnTimers[entityId] = 0.0f;

    *entities->collisionRects[entityId] = (SDL_Rect){
        entities->worldPositions[entityId]->x - ACTOR_SIZE*0.5,
        entities->worldPositions[entityId]->y - ACTOR_SIZE*0.5,
        ACTOR_SIZE,
        ACTOR_SIZE
    };

    return entityId;
}

EntityId createGhost(  Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite, TargetingBehavior targetingBehavior ) {
    EntityId entityId = g_NumEntities;
    g_NumEntities++;

    // allocate
    entities->worldPositions[entityId] =        (Position_f *)malloc(sizeof(Position_f));
    entities->currentTiles[entityId] =          (SDL_Point *)malloc(sizeof(SDL_Point));
    entities->directions[entityId] =            (Direction *)malloc(sizeof(Direction));
    entities->nextTiles[entityId] =             (SDL_Point *)malloc(sizeof(SDL_Point));
    entities->baseSpeeds[entityId] =            (float *)malloc(sizeof(float));
    entities->speedMultipliers[entityId] =      (float *)malloc(sizeof(float));
    entities->velocities[entityId] =            (Vector_f *)malloc(sizeof(Vector_f));
    entities->targetTiles[entityId] =           (SDL_Point *)malloc(sizeof(SDL_Point));
    entities->targetingBehaviors[ entityId ] =  (TargetingBehavior *)malloc(sizeof(TargetingBehavior));
    entities->ghostStates[ entityId] =          (GhostState *)malloc(sizeof(GhostState));
    entities->stopTimers[entityId] =            (float *) malloc(sizeof(float));
    entities->numDots[entityId] =               (unsigned int *)malloc(sizeof(unsigned int));
    entities->collisionRects[entityId] =        (SDL_Rect *)malloc(sizeof(SDL_Rect));

    //initialize

    *entities->worldPositions[entityId] = (Position_f){
        levelConfig->ghostPenTile.x * TILE_SIZE + (ACTOR_SIZE*0.5),
        levelConfig->ghostPenTile.y * TILE_SIZE + (ACTOR_SIZE*0.5)
    };

    *entities->currentTiles[entityId] = levelConfig->ghostPenTile;

    *entities->directions[entityId] = DIR_NONE;

    *entities->nextTiles[entityId] = *entities->currentTiles[entityId];

    *entities->baseSpeeds[entityId] = levelConfig->baseSpeed;

    *entities->speedMultipliers[entityId] = 0.85f;

    *entities->velocities[entityId] = (Vector_f){
        0.0f,
        0.0f
    };

    *entities->targetTiles[entityId] = *entities->currentTiles[entityId];

    *entities->targetingBehaviors[entityId] = targetingBehavior;

    entities->animatedSprites[ entityId ] = animatedSprite;
    entities->renderDatas[ entityId ] = renderDataInit( );

    *entities->ghostStates[ entityId ] = STATE_LEAVE_PEN;
    leave_pen_enter( entities, entityId );
    *entities->targetingBehaviors[ entityId ] = targetingBehavior;

    *entities->stopTimers[entityId] = 0.0f;

    *entities->numDots[entityId] = 0;

    entities->collisionRects[entityId]->x = entities->worldPositions[entityId]->x - ACTOR_SIZE*0.5;
    entities->collisionRects[entityId]->y = entities->worldPositions[entityId]->y - ACTOR_SIZE*0.5;
    entities->collisionRects[entityId]->w = ACTOR_SIZE;
    entities->collisionRects[entityId]->h = ACTOR_SIZE;

    return entityId;
}

// EntityId createPowerPellet(Entities *entities, AnimatedSprite *animatedSprite, SDL_Point tile ) {

// }

EntityId createInitialTemporaryPickup( Entities *entities, LevelConfig *levelConfig ) {
    EntityId entityId = g_NumEntities;
    g_NumEntities++;

    // allocate
    entities->worldPositions[entityId] = (Position_f *)malloc(sizeof(Position_f));
    entities->currentTiles[entityId] = (SDL_Point *)malloc(sizeof(SDL_Point));
    entities->nextTiles[entityId] = (SDL_Point *)malloc(sizeof(SDL_Point));
    entities->directions[entityId] = (Direction *)malloc(sizeof(Direction));
    entities->baseSpeeds[entityId] = (float *)malloc(sizeof(float));
    entities->speedMultipliers[entityId] = (float *)malloc(sizeof(float));
    entities->activeTimers[ entityId ] = (float *) malloc(sizeof(float));
    entities->pickupTypes[ entityId] = (PickupType *)malloc(sizeof( PickupType )) ;
    entities->numDots[ entityId ] = (unsigned int *) malloc( sizeof( unsigned int ) );
    entities->collisionRects[entityId] = (SDL_Rect *)malloc(sizeof(SDL_Rect));
    entities->scores[ entityId ] = (unsigned int *)malloc( sizeof(unsigned int));

    // initialize

    *entities->worldPositions[entityId] = (Position_f){
        levelConfig->pacStartingTile.x * TILE_SIZE + (TILE_SIZE*0.5),
        levelConfig->pacStartingTile.y * TILE_SIZE + (TILE_SIZE*0.5)
    };

    *entities->currentTiles[entityId] = levelConfig->pacStartingTile;

    *entities->nextTiles[entityId] = *entities->currentTiles[entityId];

    *entities->directions[entityId] = DIR_LEFT;

    *entities->baseSpeeds[entityId] = levelConfig->baseSpeed;

    *entities->speedMultipliers[entityId] = 0.5f;

    *entities->activeTimers[entityId] = 0.0f;

    *entities->pickupTypes[entityId] = NONE_PICKUP;

    *entities->numDots[entityId] = 0;

    *entities->collisionRects[entityId] = (SDL_Rect){
        entities->worldPositions[entityId]->x - (TILE_SIZE*0.5),
        entities->worldPositions[entityId]->y - (TILE_SIZE*0.5),
        TILE_SIZE,
        TILE_SIZE
    };

    entities->animatedSprites[ entityId ] = init_animation( 0, 15, 1, 20 ); // need to set texture atlas later. This is an initial invalid texture atlas id.

    entities->renderDatas[ entityId ] = renderDataInit();
    
    *entities->scores[ entityId ] = 0;

    return entityId;
}


void ghostsProcess( Entities *entities, EntityId *playerIds, unsigned int numPlayers, TileMap *tilemap, float deltaTime, LevelConfig *levelConfig ) {
    TargetingBehavior **targetingBehaviors = entities->targetingBehaviors;
    GhostState **ghostStates = entities->ghostStates;


    for( int eid = 0; eid < g_NumEntities; eid++ ) {
        // skip non ghosts
        if( targetingBehaviors[ eid ] == NULL || ghostStates[ eid ] == NULL ) {
            continue;
        }
        //Ghost state machine processing
        switch( *ghostStates[ eid ] ) {
            case STATE_NORMAL:
                normal_process( entities, eid, playerIds, numPlayers, tilemap, levelConfig );
                break;
            case STATE_VULNERABLE:
                vulnerable_process(entities, eid , tilemap );
                break;
            case STATE_GO_TO_PEN:
                go_to_pen_process(entities, levelConfig, eid, tilemap );
                break;
            case STATE_LEAVE_PEN:
                leave_pen_process( entities, eid, tilemap );
                break;
            case STATE_STAY_PEN:
                stayPenProcess( entities, levelConfig, tilemap, eid );
                break;
            default:
                // something went wrong
                fprintf(stderr, "Entity %d not set to valid state. State: %d\n", eid, *ghostStates[ eid ] );
                break;
        }
        
        set_animation_row( entities->animatedSprites[ eid ], *entities->directions[eid] );

        

    }
}

EntityId createPowerPellet(Entities *entities, AnimatedSprite *animatedSprite, SDL_Point tile ) {
    EntityId entityId = g_NumEntities;
    g_NumEntities++;

    entities->worldPositions[entityId] = (Position_f *)malloc(sizeof(Position_f));
    entities->currentTiles[entityId] = (SDL_Point *)malloc(sizeof(SDL_Point));
    entities->pickupTypes   [ entityId ] = ( PickupType *) malloc( sizeof( PickupType ) );
    entities->scores   [ entityId ] = ( unsigned int *)malloc(sizeof(unsigned int));

    *entities->worldPositions[entityId] = (Position_f){
        tile.x * TILE_SIZE + (TILE_SIZE*0.5),
        tile.y * TILE_SIZE + (TILE_SIZE*0.5),
    };

    *entities->currentTiles[entityId] = tile;

    entities->animatedSprites[ entityId ] = animatedSprite;

    entities->renderDatas[ entityId ] = renderDataInit();

    *entities->pickupTypes [ entityId ] = POWER_PELLET_PICKUP;

    *entities->scores[entityId] = 50;

    return entityId;

}


void collectDotProcess( Entities *entities, char dots[ TILE_ROWS ][ TILE_COLS ], unsigned int *num_dots, Score *score, SDL_Renderer *renderer ) {
    for( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
        if( entities->inputMasks[ eid ] == NULL ) {
            continue; // only entities with input ability should collect dots
        }
        if( *entities->isActive[eid] == SDL_FALSE ) {
            continue;
        }
        if( dots[entities->currentTiles[eid]->y][entities->currentTiles[eid]->x] == 'x') {
            gPacChomp2 = !gPacChomp2;

            if( gPacChomp2 ) {
                Mix_PlayChannel(PAC_CHOMP_CHANNEL, g_PacChompSound2, 0 );
            }
            else {
                Mix_PlayChannel( PAC_CHOMP_CHANNEL2, g_PacChompSound, 0 );
            }

            // get rid of dot marker
            dots[entities->currentTiles[eid]->y][entities->currentTiles[eid]->x] = ' ';

            unsigned int n = *num_dots - 1;

            *num_dots = n;
            
            score->score_number += 10;
        }
    }
}

void tempMirrorPlayerCollectDotProcess( Entities *entities, char dots[ TILE_ROWS ][ TILE_COLS ], Score *score ) {
    for( int eid = 0; eid < g_NumEntities; eid++ ) {
        if( entities->mirrorEntityRefs[ eid ] == NULL ) {
            continue;
        }
        // dont process if inactive
        if( *entities->activeTimers[eid] <= 0.0f ) {
            continue;
        }
        if( dots[entities->currentTiles[eid]->y][entities->currentTiles[eid]->x] == 'x') {
     
            //Mix_PlayChannel( -1, g_PacChompSound, 0 );

            // get rid of dot marker
            dots[entities->currentTiles[eid]->y][entities->currentTiles[eid]->x] = ' ';

            g_NumDots -= 1;
            
            score->score_number += 10;
        }
    }
}

void cooldownProcess( Entities *entities, float deltaTime ) {
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->dashCooldownStocks[ eid ] == NULL ) {
            continue;
        }

        if( entities->dashCooldownStocks[ eid ]->currentNumStock >= entities->dashCooldownStocks[ eid ]->numStockCap ) {
            continue; // no need to charge anything. Player has max stock
        }
        if( entities->dashCooldownStocks[ eid ]->currentNumStock < entities->dashCooldownStocks[ eid ]->numStockCap ) {
            entities->dashCooldownStocks[ eid ]->cooldownTimer -= deltaTime;
        }
        if( entities->dashCooldownStocks[ eid ]->cooldownTimer <= 0.0f ) {
            entities->dashCooldownStocks[ eid ]->currentNumStock++;
            if( entities->dashCooldownStocks[ eid ]->currentNumStock < entities->dashCooldownStocks[ eid ]->numStockCap ) {
                entities->dashCooldownStocks[ eid ]->cooldownTimer = entities->dashCooldownStocks[ eid ]->cooldownDuration;

            }
        }
    }
}

EntityId createTempMirrorPlayer( Entities *entities, EntityId playerId, float activeTime ) {
    EntityId entityId = g_NumEntities++;

    AnimatedSprite *playerAnimatedSprite = entities->animatedSprites[playerId];

    // allocate
    entities->worldPositions[entityId] = (Position_f *)malloc(sizeof(Position_f));
    entities->currentTiles[entityId] = (SDL_Point *)malloc(sizeof(SDL_Point));
    entities->directions[entityId] = (Direction *)malloc(sizeof(Direction));
    entities->velocities[entityId] = (Vector_f *)malloc(sizeof(Vector_f));
    entities->animatedSprites[ entityId ] = (AnimatedSprite *)malloc(sizeof(AnimatedSprite));
    entities->mirrorEntityRefs[ entityId ] = (EntityId *)malloc(sizeof(EntityId));
    entities->activeTimers[entityId]=(float *)malloc(sizeof(float));

    // initialize
    *entities->worldPositions[entityId] = *entities->worldPositions[playerId];
    *entities->currentTiles[entityId] = *entities->currentTiles[playerId];
    *entities->directions[entityId] = *entities->directions[playerId];
    *entities->velocities[entityId] = (Vector_f){0.0f, 0.0f};
    entities->animatedSprites[entityId]->accumulator = playerAnimatedSprite->accumulator;
    entities->animatedSprites[entityId]->current_anim_row = playerAnimatedSprite->current_anim_row;
    entities->animatedSprites[entityId]->current_frame_col = playerAnimatedSprite->current_frame_col;
    entities->animatedSprites[entityId]->default_texture_atlas_id = playerAnimatedSprite->default_texture_atlas_id;
    entities->animatedSprites[entityId]->frame_interval = playerAnimatedSprite->frame_interval;
    entities->animatedSprites[entityId]->num_frames_col = playerAnimatedSprite->num_frames_col;
    entities->animatedSprites[entityId]->texture_atlas_id = playerAnimatedSprite->texture_atlas_id;
    entities->renderDatas[ entityId ] = renderDataInit( );
    entities->renderDatas[entityId]->alphaMod = 150;
    *entities->mirrorEntityRefs[entityId] = playerId;
    *entities->activeTimers[entityId] = activeTime;

    return entityId;
}

/**
 * Overwrite the first inactive entity. creates if none to overwrite
*/
EntityId overwriteInactiveTempMirrorPlayer( Entities *entities, EntityId playerId, float activeTime ) {
    for( int eid = 0 ; eid < g_NumEntities; eid++ ) {
        if( entities->mirrorEntityRefs[eid] == NULL ) {
            continue;
        }
        // can overwrite
        if( *entities->activeTimers[eid] <= 0.0f ) {
            //EntityId entityId = ++g_NumEntities;

            AnimatedSprite *playerAnimatedSprite = entities->animatedSprites[playerId];

            *entities->currentTiles[eid] = *entities->currentTiles[playerId];
            *entities->directions[eid] = *entities->directions[playerId];
            *entities->worldPositions[eid] = *entities->worldPositions[playerId];
            entities->animatedSprites[eid]->accumulator = playerAnimatedSprite->accumulator;
            entities->animatedSprites[eid]->current_anim_row = playerAnimatedSprite->current_anim_row;
            entities->animatedSprites[eid]->current_frame_col = playerAnimatedSprite->current_frame_col;
            entities->animatedSprites[eid]->default_texture_atlas_id = playerAnimatedSprite->default_texture_atlas_id;
            entities->animatedSprites[eid]->frame_interval = playerAnimatedSprite->frame_interval;
            entities->animatedSprites[eid]->num_frames_col = playerAnimatedSprite->num_frames_col;
            entities->animatedSprites[eid]->texture_atlas_id = playerAnimatedSprite->texture_atlas_id;

            entities->renderDatas[ eid ] = renderDataInit();
            entities->renderDatas[eid]->alphaMod = 150;

            *entities->mirrorEntityRefs[eid] = playerId;
            *entities->activeTimers[eid] = activeTime;

            return eid;
        }

        
    }
    // none was overwritten, so need to make a new one
    EntityId entityId = createTempMirrorPlayer( entities, playerId, activeTime );
    return entityId;
}

void overwriteSpeedBoostTimer(Entities *entities,EntityId playerId, float speed, float duration ) {
    if( entities->speedBoostTimers[playerId] == NULL ) {
        entities->speedBoostTimers[playerId] = (float *)malloc(sizeof(float));
    }
    *entities->speedBoostTimers[playerId] = duration;
}

void processSpeedBoostTimer( Entities *entities, float deltaTime ) {
    for( int eid = 0; eid < g_NumEntities; eid++) {
        if( entities->speedBoostTimers[ eid ] == NULL ) {
            continue;
        }
        if(*entities->speedBoostTimers[eid] >= 0.0f ) {
            *entities->speedMultipliers[eid] += 0.5f;
            *entities->speedBoostTimers[eid] -= deltaTime;
        }
    }
}


void processDeathTimers( Entities *entities, LevelConfig *levelConfig, float deltaTime ) {
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if(entities->deathTimers[eid] == NULL ) {
            continue;
        }
        if( *entities->deathTimers[eid] <= 0.0f ){
            continue;
        }

        entities->animatedSprites[eid]->current_anim_row = 2;
        *entities->deathTimers[eid] -= deltaTime;
        if( entities->renderDatas[eid]->scale > 0.0f ) {
            if( *entities->deathTimers[eid] < 1.5f ) {
                entities->renderDatas[eid]->scale -= deltaTime * 1.5;
            }
        }
        if( *entities->deathTimers[eid] <= 0.0f && *entities->isActive[eid] == SDL_TRUE ) {
            *entities->currentTiles[eid] = levelConfig->pacStartingTile;
            //entities->actors[eid]->current_tile.y -= 1;
            *entities->nextTiles[eid] = levelConfig->pacStartingTile;
            *entities->worldPositions[eid] = (Position_f){
                levelConfig->pacStartingTile.x * TILE_SIZE + (TILE_SIZE*0.5),
                (levelConfig->pacStartingTile.y ) * TILE_SIZE + (TILE_SIZE*0.5),
            };
            entities->sensors[eid]->worldTopSensor = (SDL_Point){
                entities->worldPositions[eid]->x,
                entities->worldPositions[eid]->y - (TILE_SIZE*0.5)
            };
            entities->sensors[eid]->worldBottomSensor = (SDL_Point){
                entities->worldPositions[eid]->x,
                entities->worldPositions[eid]->y + (TILE_SIZE*0.5)
            };
            entities->sensors[eid]->worldLeftSensor = (SDL_Point){
                entities->worldPositions[eid]->x - (TILE_SIZE*0.5),
                entities->worldPositions[eid]->y
            };
            entities->sensors[eid]->worldRightSensor = (SDL_Point){
                entities->worldPositions[eid]->x + (TILE_SIZE*0.5),
                entities->worldPositions[eid]->y
            };

            *entities->directions[eid] = DIR_NONE;

            //actor_set_current_tile( entities->actors[ eid ] );
            *entities->nextTiles[eid] = *entities->currentTiles[eid];
            *entities->invincibilityTimers[ eid ] = 5.0f;

            *entities->respawnTimers[eid] = 0.5f;

            Mix_PlayChannel(PAC_RESPAWN_CHANNEL, g_PacRespawnSound, 0 );
        }
    }
}


void processRespawnTimers( Entities *entities, float deltaTime ) {
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if(entities->respawnTimers[eid] == NULL ) {
            continue;
        }
        if( *entities->respawnTimers[eid] <= 0.0f ){
            continue;
        }

        *entities->respawnTimers[eid] -= deltaTime;
        if(entities->renderDatas[eid]->scale < 1.0f ) {
            entities->renderDatas[eid]->scale += deltaTime * 3;
        }
    }
}


void processTempMirrorPlayers( Entities *entities, float deltaTime ) {
    for( int eid = 0; eid < g_NumEntities; eid++ ) {
        if( entities->mirrorEntityRefs[ eid ] == NULL ) {
            continue;
        }

        if( *entities->activeTimers[eid] <= 0.0f ) {
            entities->renderDatas[ eid ]->alphaMod = 0;
            continue;
        }

        EntityId playerId = *entities->mirrorEntityRefs[ eid ];
        AnimatedSprite *playerAnimatedSprite = entities->animatedSprites[playerId];

        const int tileCols = TILE_COLS - 1; // accidentally gave map one more than it should have
        const int midTileX = tileCols*0.5;

        
        entities->currentTiles[eid]->y = entities->currentTiles[eid]->y;
        if( midTileX == entities->currentTiles[playerId]->x ) {
            entities->currentTiles[eid]->x = entities->currentTiles[playerId]->x;
        }
        else if( entities->currentTiles[playerId]->x < midTileX ) {
            int difference = midTileX - entities->currentTiles[playerId]->x;
            entities->currentTiles[eid]->x = midTileX + difference;
        }
        else if( entities->currentTiles[playerId]->x > midTileX ) {
            int difference = entities->currentTiles[playerId]->x - midTileX;
            entities->currentTiles[eid]->x = midTileX - difference;
        }

        const int midWorldPointX = midTileX*TILE_SIZE + TILE_SIZE*0.5;

        *entities->directions[eid] = *entities->directions[playerId];
        if( *entities->directions[eid] == DIR_LEFT || *entities->directions[eid] == DIR_RIGHT) {
            *entities->directions[eid] = opposite_directions[*entities->directions[eid] ];
        }

        entities->worldPositions[eid]->y = entities->worldPositions[eid]->y;

        if( midWorldPointX == entities->worldPositions[playerId]->x ) {
            entities->worldPositions[eid]->x = entities->worldPositions[playerId]->x ;
        }
        else if( entities->worldPositions[playerId]->x < midWorldPointX ) {
            int difference = midWorldPointX - entities->worldPositions[playerId]->x;
            entities->worldPositions[eid]->x = midWorldPointX + difference;
        }
        else if( entities->worldPositions[playerId]->x > midWorldPointX ) {
            int difference = entities->worldPositions[playerId]->x - midWorldPointX;
            entities->worldPositions[eid]->x = midWorldPointX - difference;
        }
        entities->worldPositions[eid]->x -= ACTOR_SIZE;


        entities->worldPositions[eid]->x = entities->worldPositions[eid]->x + ACTOR_SIZE/2;
        entities->worldPositions[eid]->y = entities->worldPositions[eid]->y + ACTOR_SIZE/2;

        entities->velocities[eid]->x = -entities->velocities[playerId]->x;
        entities->velocities[eid]->y = entities->velocities[playerId]->y;

        entities->renderDatas[eid]->alphaMod = 150;

        // pacman animation row
        if( entities->velocities[eid]->x > 0 && entities->velocities[eid]->y == 0 ) { // right
            entities->animatedSprites[ eid ]->current_anim_row = 0;
        }
        if( entities->velocities[eid]->x < 0 && entities->velocities[eid]->y == 0 ) { // left
            entities->animatedSprites[ eid ]->current_anim_row = 1;
        }
        if( entities->velocities[eid]->x == 0 && entities->velocities[eid]->y > 0 ) { // down
            entities->animatedSprites[ eid ]->current_anim_row = 2;
        }
        if( entities->velocities[eid]->x == 0 && entities->velocities[eid]->y < 0 ) { // up
            entities->animatedSprites[ eid ]->current_anim_row = 3;
        }
        if( entities->velocities[eid]->x > 0 && entities->velocities[eid]->y < 0 ) { //  up-right
            entities->animatedSprites[ eid ]->current_anim_row = 4;
        }
        if( entities->velocities[eid]->x < 0 && entities->velocities[eid]->y < 0 ) { // up-left
            entities->animatedSprites[ eid ]->current_anim_row = 5;
        }
        if( entities->velocities[eid]->x > 0 && entities->velocities[eid]->y > 0 ) { // down-right
            entities->animatedSprites[ eid ]->current_anim_row = 6;
        }
        if( entities->velocities[eid]->x < 0 && entities->velocities[eid]->y > 0 ) { // down-left
            entities->animatedSprites[ eid ]->current_anim_row = 7;
        }

        entities->animatedSprites[eid]->accumulator = playerAnimatedSprite->accumulator;
        //entities->animatedSprites[eid]->current_anim_row = playerAnimatedSprite->current_anim_row;
        entities->animatedSprites[eid]->current_frame_col = playerAnimatedSprite->current_frame_col;
        entities->animatedSprites[eid]->default_texture_atlas_id = playerAnimatedSprite->default_texture_atlas_id;
        entities->animatedSprites[eid]->frame_interval = playerAnimatedSprite->frame_interval;
        entities->animatedSprites[eid]->num_frames_col = playerAnimatedSprite->num_frames_col;
        entities->animatedSprites[eid]->texture_atlas_id = playerAnimatedSprite->texture_atlas_id;

        *entities->activeTimers[ eid ] -= deltaTime;        

    }
}

void stopGhostsForDuration(Entities *entities, float duration) {
    EntityId gid;
    for( int i = 0; i < gNumGhosts; i++ ) {
        gid = gGhostIds[ i ];
        *entities->stopTimers[gid] += duration;
    }
}


void stopPlayersForDuration(Entities *entities, EntityId *playerIds, unsigned int numPlayers, float duration ) {
    EntityId pid = 0;
    for( int i = 0; i < numPlayers; i++ ) {
        pid = playerIds[ i ];
        if( entities->stopTimers[pid] == NULL ) {
            continue;
        }
        *entities->stopTimers[pid] += duration;

    }
}

void processStopTimers(Entities *entities, float deltaTime ) {
    for( int eid = 0; eid < g_NumEntities; eid++ ) {
        if( entities->stopTimers[eid] == NULL ) {
            continue;
        }
        if( *entities->stopTimers[eid] <= 0.0f ) {
            continue;
        } 
        *entities->stopTimers[eid] -= deltaTime;
    }
}

void makePlayerInvincibleForDuration( Entities *entities, EntityId playerId, float duration) {
    *entities->invincibilityTimers[ playerId ] = duration;
}

void processInvincibilityTimers( Entities *entities, float deltaTime) {
    for( int eid = 0; eid < g_NumEntities ; eid++ ) {
        if( entities->invincibilityTimers[eid] == NULL ) {
            continue;
        }
        if( *entities->invincibilityTimers[eid] <= 0.0f) {
            continue;
        }
        *entities->invincibilityTimers[eid] -= deltaTime;
        
        // entities->renderDatas[eid]->alphaMod -= 100; // make player semi transparent to appear immune to hurt
    }
}

void processTemporaryPickup( Entities *entities, EntityId *playerIds, unsigned int numPlayers, LevelConfig *levelConfig, Score *score, TileMap *tilemap, unsigned int numDotsLeft, float deltaTime ) {
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->pickupTypes[ eid ] == NULL || entities->numDots[ eid ] == NULL || entities->activeTimers[ eid ] == NULL ) {
            continue;
        }

        if( *entities->activeTimers[ eid ] <= 0.0f ){
            entities->renderDatas[ eid ]->alphaMod = 0;
            // SDL_Texture *texture = g_texture_atlases[ entities->animatedSprites[ eid ]->texture_atlas_id ].texture;
            // SDL_SetTextureAlphaMod( texture, 0 );
            continue;
        }
        
        // pickup is active
        int numDotsEaten = g_StartingNumDots - numDotsLeft;
        if( *entities->numDots[ eid ]  <= numDotsEaten ) {
            *entities->activeTimers[ eid ] -= deltaTime;
            entities->renderDatas[ eid ]->alphaMod = 255;
            // SDL_Texture *texture = g_texture_atlases[ entities->animatedSprites[ eid ]->texture_atlas_id ].texture;
            // SDL_SetTextureAlphaMod( texture, 255 );

            // issue if more than one active
            if(*entities->activeTimers[eid] < 3.0f) {
                blinkProcess(&pickupBlink, deltaTime);
                entities->renderDatas[eid]->alphaMod = pickupBlink.values[pickupBlink.current_value_idx];
            }

            // move it
            if( points_equal( *entities->nextTiles[eid], *entities->currentTiles[eid] ) ) {
                set_random_direction_and_next_tile( entities, eid, tilemap );    
            }
            ghost_move( entities, eid, tilemap, deltaTime );
            entities->collisionRects[eid]->x = entities->worldPositions[eid]->x - ACTOR_SIZE*0.5;
            entities->collisionRects[eid]->y = entities->worldPositions[eid]->y - ACTOR_SIZE*0.5;
            entities->collisionRects[eid]->w = ACTOR_SIZE;
            entities->collisionRects[eid]->h = ACTOR_SIZE;

            EntityId playerId;
            for( int i = 0; i < numPlayers; i++ ) {
                playerId = playerIds[ i ];
                if( *entities->isActive[playerId] == SDL_FALSE ) {
                    continue;
                }
                // player picks up
                if( entitiesIntersecting(entities, playerId, eid) ) {
                    *entities->activeTimers[ eid ] = 0.0f;
                    score->score_number += *entities->scores[ eid ];


                    Mix_PlayChannel( PICKUP_EAT_CHANNEL, g_PickupEaten, 0 );

                    for( int i = 0; i < g_NumTimedMessages; i++ ) {
                        if( g_TimedMessages[ i ].remainingTime <= 0.0f ) {
                            g_TimedMessages[ i ].remainingTime = 0.85f;
                            g_TimedMessages[ i ].world_position = tile_grid_point_to_world_point( *entities->currentTiles[playerId] );
                            snprintf( g_TimedMessages[ i ].message, 8, "%d", *entities->scores[eid] );
                            g_TimedMessages[ i ].color.r = 255 ;
                            g_TimedMessages[ i ].color.g = 255;
                            g_TimedMessages[ i ].color.b = 255;
                            SDL_Surface *msgSurface = TTF_RenderText_Solid( g_TimedMessages[ i ].font,  g_TimedMessages[ i ].message, g_TimedMessages[ i ].color );
                            g_TimedMessages[ i ].messageTexture = SDL_CreateTextureFromSurface( gRenderer, msgSurface );
                            g_TimedMessages[ i ].render_dest_rect.x = world_point_to_screen_point(g_TimedMessages[ i ].world_position, tilemap->tm_screen_position).x;
                            g_TimedMessages[ i ].render_dest_rect.y = world_point_to_screen_point(g_TimedMessages[ i ].world_position, tilemap->tm_screen_position).y;
                            g_TimedMessages[ i ].render_dest_rect.w = msgSurface->w;
                            g_TimedMessages[ i ].render_dest_rect.h = msgSurface->h;
                            SDL_FreeSurface( msgSurface );
                            g_TimedMessages[ i ].l = lerpInit( g_TimedMessages[ i ].world_position.y - TILE_SIZE*1.25, g_TimedMessages[ i ].world_position.y, 0.33f );

                            break;

                        }
                    }

                    switch( *entities->pickupTypes[ eid ] ) {
                        case FRUIT_PICKUP:
                            break; // do nothing. Just a score
                        case MIRROR_PICKUP:
                            // look for available entity to overwrite
                            overwriteInactiveTempMirrorPlayer( entities, playerId, 8.0f);
                            break;
                        case POWER_PELLET_PICKUP:
                            allGhostsVulnerableStateEnter( entities, levelConfig );  
                            break;
                        case SPEED_BOOST_PICKUP:
                            overwriteSpeedBoostTimer( entities, playerId, gBaseSpeed * 1.2, 8.0f );
                            break;
                        case SHIELD_PICKUP:
                            makePlayerInvincibleForDuration( entities, playerId, 10.0f);
                            break;
                        case STOP_GHOSTS_PICKUP:
                            stopGhostsForDuration(entities, 7.0f);
                            break;
                        case NONE_PICKUP:
                            break;
                    }
                    break;

                }
            }

        }
        else {
            entities->renderDatas[ eid ]->alphaMod = 0;
        }

    }
}

SDL_bool entitiesIntersecting(Entities *entities, EntityId eid1, EntityId eid2 ){
    SDL_Rect resultRect;
    if ( SDL_IntersectRect(entities->collisionRects[eid1], entities->collisionRects[eid2], &resultRect) ){
        return SDL_TRUE;
    }
    return SDL_FALSE;
}