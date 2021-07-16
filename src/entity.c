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


unsigned int g_NumEntities = 0;

EntityId createPlayer( Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite ) {
    EntityId entityId = g_NumEntities;
    g_NumEntities++;

    // allocate
    entities->positions      [ entityId ] = (Position *)                malloc(sizeof(Position));
    entities->actors         [ entityId ] = ( Actor * )                 malloc( sizeof( Actor ) );
    entities->chargeTimers   [ entityId ] = (float * )                  malloc(sizeof(float));
    entities->dashTimers     [ entityId ] = (float * )                  malloc(sizeof(float ) );
    entities->slowTimers     [ entityId ] = ( float * )malloc(sizeof(float ) );
    entities->inputMasks     [ entityId ] = (uint8_t * ) malloc(sizeof( uint8_t ) );
    entities->dashCooldownStocks[ entityId ] = (CooldownStock *)malloc( sizeof( CooldownStock ) );

    //initialize
    // position
    entities->positions[ entityId ]->current_tile = levelConfig->pacStartingTile;
    entities->positions[ entityId ]->world_position.x = levelConfig->pacStartingTile.x * TILE_SIZE;
    entities->positions[ entityId ]->world_position.y = levelConfig->pacStartingTile.y * TILE_SIZE;

    entities->positions[ entityId ]->world_center_point.x = ( int ) entities->positions[ entityId ]->world_position.x + ( ACTOR_SIZE / 2 );
    entities->positions[ entityId ]->world_center_point.y = ( int ) entities->positions[ entityId ]->world_position.y + ( ACTOR_SIZE / 2 );
    //actor
    entities->actors[ entityId ]->current_tile = levelConfig->pacStartingTile;
    entities->actors[ entityId ]->world_position.x = levelConfig->pacStartingTile.x * TILE_SIZE;
    entities->actors[ entityId ]->world_position.y = levelConfig->pacStartingTile.y * TILE_SIZE;

    entities->actors[ entityId ]->world_center_point.x = ( int ) entities->actors[ entityId ]->world_position.x + ( ACTOR_SIZE / 2 );
    entities->actors[ entityId ]->world_center_point.y = ( int ) entities->actors[ entityId ]->world_position.y + ( ACTOR_SIZE / 2 );

    entities->actors[ entityId ]->world_top_sensor.x = entities->positions[ entityId ]->world_position.x + ( ACTOR_SIZE / 2 );
    entities->actors[ entityId ]->world_top_sensor.y = entities->positions[ entityId ]->world_position.y;

    entities->actors[ entityId ]->world_bottom_sensor.x = entities->positions[ entityId ]->world_position.x + ( ACTOR_SIZE / 2 );
    entities->actors[ entityId ]->world_bottom_sensor.y = entities->positions[ entityId ]->world_position.y + ACTOR_SIZE;

    entities->actors[ entityId ]->world_left_sensor.x = entities->positions[ entityId ]->world_position.x;
    entities->actors[ entityId ]->world_left_sensor.y = entities->positions[ entityId ]->world_position.y + ( ACTOR_SIZE / 2 );

    entities->actors[ entityId ]->world_right_sensor.x = entities->positions[ entityId ]->world_position.x + ACTOR_SIZE;
    entities->actors[ entityId ]->world_right_sensor.y = entities->positions[ entityId ]->world_position.y + ( ACTOR_SIZE / 2 );    
    
    entities->actors[ entityId ]->velocity.x = 0.0f;
    entities->actors[ entityId ]->velocity.y = 0.0f;

    entities->actors[ entityId ]->direction = DIR_NONE;

    entities->actors[ entityId ]->next_tile = entities->positions[ entityId ]->current_tile;
    entities->actors[ entityId ]->next_tile = entities->positions[ entityId ]->current_tile;

    entities->actors[ entityId ]->base_speed = levelConfig->baseSpeed;
    entities->actors[ entityId ]->speed_multp = 1.0f;

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
    

    return entityId;
}

EntityId createGhost(  Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite, TargetingBehavior targetingBehavior ) {
    EntityId entityId = g_NumEntities;
    g_NumEntities++;

    // allocate
    entities->positions      [ entityId ] = (Position *)                malloc(sizeof(Position));
    entities->actors         [ entityId ] = ( Actor * )                 malloc( sizeof( Actor ) );
    entities->targetingBehaviors     [ entityId ] = (TargetingBehavior *)              malloc(sizeof(TargetingBehavior));
    entities->ghostStates[ entityId] = (GhostState *)malloc(sizeof(GhostState));

    //initialize
    // position
    entities->positions[ entityId ]->current_tile = levelConfig->ghostPenTile;
    entities->positions[ entityId ]->world_position.x = levelConfig->ghostPenTile.x * TILE_SIZE;
    entities->positions[ entityId ]->world_position.y = levelConfig->ghostPenTile.y * TILE_SIZE;

    entities->positions[ entityId ]->world_center_point.x = ( int ) entities->positions[ entityId ]->world_position.x + ( ACTOR_SIZE / 2 );
    entities->positions[ entityId ]->world_center_point.y = ( int ) entities->positions[ entityId ]->world_position.y + ( ACTOR_SIZE / 2 );
    //actor
    entities->actors[ entityId ]->current_tile = levelConfig->ghostPenTile;
    entities->actors[ entityId ]->world_position.x = levelConfig->ghostPenTile.x * TILE_SIZE;
    entities->actors[ entityId ]->world_position.y = levelConfig->ghostPenTile.y * TILE_SIZE;

    entities->actors[ entityId ]->world_center_point.x = ( int ) entities->actors[ entityId ]->world_position.x + ( ACTOR_SIZE / 2 );
    entities->actors[ entityId ]->world_center_point.y = ( int ) entities->actors[ entityId ]->world_position.y + ( ACTOR_SIZE / 2 );

    entities->actors[ entityId ]->velocity.x = 0.0f;
    entities->actors[ entityId ]->velocity.y = 0.0f;

    entities->actors[ entityId ]->direction = DIR_NONE;

    entities->actors[ entityId ]->next_tile = entities->positions[ entityId ]->current_tile;
    entities->actors[ entityId ]->next_tile = entities->positions[ entityId ]->current_tile;

    entities->actors[ entityId ]->base_speed = levelConfig->baseSpeed;
    entities->actors[ entityId ]->speed_multp = 0.85f;

    entities->animatedSprites[ entityId ] = animatedSprite;
    entities->renderDatas[ entityId ] = renderDataInit( );

    *entities->ghostStates[ entityId ] = STATE_LEAVE_PEN;
    leave_pen_enter( entities, entityId );
    *entities->targetingBehaviors[ entityId ] = targetingBehavior;

    return entityId;
}

// EntityId createPowerPellet(Entities *entities, AnimatedSprite *animatedSprite, SDL_Point tile ) {

// }

EntityId createInitialTemporaryPickup( Entities *entities, LevelConfig *levelConfig ) {
    EntityId entityId = g_NumEntities;
    g_NumEntities++;

    entities->positions[ entityId ] = (Position * ) malloc( sizeof( Position ) );
    entities->actors[ entityId ] = (Actor * ) malloc( sizeof( Actor ) );
    entities->activeTimer[ entityId ] = (float *) malloc(sizeof(float));
    entities->pickupTypes[ entityId] = (PickupType *)malloc(sizeof( PickupType )) ;
    entities->numDots[ entityId ] = (unsigned int *) malloc( sizeof( unsigned int ) );

    entities->positions[ entityId ]->current_tile = levelConfig->pacStartingTile;
    entities->positions[ entityId ]->world_position.x = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).x;
    entities->positions[ entityId ]->world_position.y = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).y;
    entities->positions[ entityId ]->world_center_point.x = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).x + ACTOR_SIZE/2;
    entities->positions[ entityId ]->world_center_point.y = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).y + ACTOR_SIZE/2;

    entities->actors[ entityId ]->current_tile = levelConfig->pacStartingTile;
    entities->actors[ entityId ]->world_position.x = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).x;
    entities->actors[ entityId ]->world_position.y = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).y;
    entities->actors[ entityId ]->world_center_point.x = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).x + ACTOR_SIZE/2;
    entities->actors[ entityId ]->world_center_point.y = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).y + ACTOR_SIZE/2;

    entities->animatedSprites[ entityId ] = init_animation( 0, 1, 1, 1 ); // need to set texture atlas later. This is an initial invalid texture atlas id.

    *entities->activeTimer[ entityId ] = 0.0f;
    *entities->pickupTypes[ entityId ] = NONE_PICKUP; // this will be skipped over. Its a NULL-like value for the pickup type
    *entities->numDots[ entityId ] = 0;

    entities->renderDatas[ entityId ] = renderDataInit();

    entities->score[ entityId ] = (unsigned int *)malloc( sizeof(unsigned int));
    *entities->score[ entityId ] = 0;

    return entityId;


}

EntityId createFruit( Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite, unsigned int numDots  ) {
    EntityId entityId = g_NumEntities;
    g_NumEntities++;

    entities->positions[ entityId ] = (Position * ) malloc( sizeof( Position ) );
    entities->actors[ entityId ] = (Actor * ) malloc( sizeof( Actor ) );
    entities->activeTimer[ entityId ] = (float *) malloc(sizeof(float));
    entities->pickupTypes[ entityId] = (PickupType *)malloc(sizeof( PickupType )) ;
    entities->numDots[ entityId ] = (unsigned int *) malloc( sizeof( unsigned int ) );


    entities->positions[ entityId ]->current_tile = levelConfig->pacStartingTile;
    entities->positions[ entityId ]->world_position.x = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).x;
    entities->positions[ entityId ]->world_position.y = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).y;
    entities->positions[ entityId ]->world_center_point.x = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).x + ACTOR_SIZE/2;
    entities->positions[ entityId ]->world_center_point.y = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).y + ACTOR_SIZE/2;

    entities->actors[ entityId ]->current_tile = levelConfig->pacStartingTile;
    entities->actors[ entityId ]->world_position.x = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).x;
    entities->actors[ entityId ]->world_position.y = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).y;
    entities->actors[ entityId ]->world_center_point.x = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).x + ACTOR_SIZE/2;
    entities->actors[ entityId ]->world_center_point.y = tile_grid_point_to_world_point( levelConfig->pacStartingTile ).y + ACTOR_SIZE/2;

    entities->animatedSprites[ entityId ] = animatedSprite;

    *entities->activeTimer[ entityId ] = 10.0f;
    *entities->pickupTypes[ entityId ] = FRUIT_PICKUP;
    *entities->numDots[ entityId ] = numDots;

    entities->renderDatas[ entityId ] = renderDataInit();

    return entityId;
}

void ghostsProcess( Entities *entities, EntityId *playerIds, unsigned int numPlayers, TileMap *tilemap, float deltaTime, LevelConfig *levelConfig ) {
    Actor **actors = entities->actors;
    TargetingBehavior **targetingBehaviors = entities->targetingBehaviors;
    GhostState **ghostStates = entities->ghostStates;


    for( int eid = 0; eid < g_NumEntities; eid++ ) {
        // skip non ghosts
        if( actors[ eid ] == NULL || targetingBehaviors[ eid ] == NULL || ghostStates[ eid ] == NULL ) {
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
                go_to_pen_process(entities, eid, tilemap );
                break;
            case STATE_LEAVE_PEN:
                leave_pen_process( entities, eid, tilemap );
                break;
            default:
                // something went wrong
                fprintf(stderr, "Entity %d not set to valid state. State: %d\n", eid, *ghostStates[ eid ] );
                break;
        }
        
        set_animation_row( entities->animatedSprites[ eid ], entities->actors[ eid ] );

        

    }
}

EntityId createPowerPellet(Entities *entities, AnimatedSprite *animatedSprite, SDL_Point tile ) {
    EntityId entityId = g_NumEntities;
    g_NumEntities++;

    entities->positions      [ entityId ] = (Position *)                malloc(sizeof(Position));
    entities->actors        [ entityId ] = (Actor * ) malloc(sizeof(Actor)); // should be deprecated
    entities->pickupTypes   [ entityId ] = ( PickupType *) malloc( sizeof( PickupType ) );

    entities->positions[ entityId ]->current_tile = tile;
    entities->positions[ entityId ]->world_center_point.x = tile_grid_point_to_world_point( tile ).x + TILE_SIZE/2;
    entities->positions[ entityId ]->world_center_point.y = tile_grid_point_to_world_point( tile ).y + TILE_SIZE/2;
    entities->positions[ entityId ]->world_position.x = tile_grid_point_to_world_point( tile ).x;
    entities->positions[ entityId ]->world_position.y = tile_grid_point_to_world_point( tile ).y;

    entities->actors[ entityId ]->current_tile = tile;
    entities->actors[ entityId ]->world_center_point.x = tile_grid_point_to_world_point( tile ).x + TILE_SIZE/2;
    entities->actors[ entityId ]->world_center_point.y = tile_grid_point_to_world_point( tile ).y + TILE_SIZE/2;
    entities->actors[ entityId ]->world_position.x = tile_grid_point_to_world_point( tile ).x;
    entities->actors[ entityId ]->world_position.y = tile_grid_point_to_world_point( tile ).y;

    entities->animatedSprites[ entityId ] = animatedSprite;

    entities->renderDatas[ entityId ] = renderDataInit( );

    *entities->pickupTypes [ entityId ] = POWER_PELLET_PICKUP;

    return entityId;

}


void collectDotProcess( Entities *entities, char dots[ TILE_ROWS ][ TILE_COLS ], unsigned int *num_dots, Score *score, SDL_Renderer *renderer ) {
    for( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
        if( entities->inputMasks[ eid ] == NULL ) {
            continue; // only entities with input ability should collect dots
        }
        if( dots[ entities->actors[ eid ]->current_tile.y ][ entities->actors[ eid ]->current_tile.x ] == 'x') {
     
            Mix_PlayChannel( -1, g_PacChompSound, 0 );

            // get rid of dot marker
            dots[ entities->actors[ eid ]->current_tile.y ][ entities->actors[ eid ]->current_tile.x ] = ' ';

            unsigned int n = *num_dots - 1;
            *num_dots = n;
            
            score->score_number += 20;

        }
    }
        

}

void tempMirrorPlayerCollectDotProcess( Entities *entities, char dots[ TILE_ROWS ][ TILE_COLS ], Score *score ) {
    for( int eid = 0; eid < g_NumEntities; eid++ ) {
        if( entities->mirrorEntityRef[ eid ] == NULL ) {
            continue;
        }
        // dont process if inactive
        if( *entities->activeTimer[eid] <= 0.0f ) {
            continue;
        }
        if( dots[ entities->actors[ eid ]->current_tile.y ][ entities->actors[ eid ]->current_tile.x ] == 'x') {
     
            Mix_PlayChannel( -1, g_PacChompSound, 0 );

            // get rid of dot marker
            dots[ entities->actors[ eid ]->current_tile.y ][ entities->actors[ eid ]->current_tile.x ] = ' ';

            g_NumDots -= 1;
            
            score->score_number += 20;

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

    Actor *playerActor = entities->actors[playerId];
    AnimatedSprite *playerAnimatedSprite = entities->animatedSprites[playerId];

    entities->positions[entityId] = (Position *)malloc(sizeof(Position));
    entities->actors[ entityId ] = (Actor *)malloc(sizeof(Actor));
    entities->animatedSprites[ entityId ] = (AnimatedSprite *)malloc(sizeof(AnimatedSprite));
    entities->mirrorEntityRef[ entityId ] = (EntityId *)malloc(sizeof(EntityId));
    entities->activeTimer[entityId]=(float *)malloc(sizeof(float));

    entities->actors[entityId]->current_tile = playerActor->current_tile;
    entities->actors[entityId]->direction = playerActor->direction;
    entities->actors[entityId]->world_position = playerActor->world_position;
    entities->actors[entityId]->world_center_point = playerActor->world_center_point;

    entities->animatedSprites[entityId]->accumulator = playerAnimatedSprite->accumulator;
    entities->animatedSprites[entityId]->current_anim_row = playerAnimatedSprite->current_anim_row;
    entities->animatedSprites[entityId]->current_frame_col = playerAnimatedSprite->current_frame_col;
    entities->animatedSprites[entityId]->default_texture_atlas_id = playerAnimatedSprite->default_texture_atlas_id;
    entities->animatedSprites[entityId]->frame_interval = playerAnimatedSprite->frame_interval;
    entities->animatedSprites[entityId]->num_frames_col = playerAnimatedSprite->num_frames_col;
    entities->animatedSprites[entityId]->texture_atlas_id = playerAnimatedSprite->texture_atlas_id;

    entities->renderDatas[ entityId ] = renderDataInit( );
    entities->renderDatas[entityId]->alphaMod = 150;

    *entities->mirrorEntityRef[entityId] = playerId;
    *entities->activeTimer[entityId] = activeTime;

    return entityId;
}

/**
 * Overwrite the first inactive entity. creates if none to overwrite
*/
EntityId overwriteInactiveTempMirrorPlayer( Entities *entities, EntityId playerId, float activeTime ) {
    for( int eid = 0 ; eid < g_NumEntities; eid++ ) {
        if( entities->mirrorEntityRef[eid] == NULL ) {
            continue;
        }
        // can overwrite
        if( *entities->activeTimer[eid] <= 0.0f ) {
            //EntityId entityId = ++g_NumEntities;

            Actor *playerActor = entities->actors[playerId];
            AnimatedSprite *playerAnimatedSprite = entities->animatedSprites[playerId];

            entities->actors[ eid ] = (Actor *)malloc(sizeof(Actor));
            entities->animatedSprites[ eid ] = (AnimatedSprite *)malloc(sizeof(AnimatedSprite));
            entities->mirrorEntityRef[ eid ] = (EntityId *)malloc(sizeof(EntityId));
            entities->activeTimer[eid]=(float *)malloc(sizeof(float));

            entities->actors[eid]->current_tile = playerActor->current_tile;
            entities->actors[eid]->direction = playerActor->direction;
            entities->actors[eid]->world_position = playerActor->world_position;
            entities->actors[eid]->world_center_point = playerActor->world_center_point;

            entities->animatedSprites[eid]->accumulator = playerAnimatedSprite->accumulator;
            entities->animatedSprites[eid]->current_anim_row = playerAnimatedSprite->current_anim_row;
            entities->animatedSprites[eid]->current_frame_col = playerAnimatedSprite->current_frame_col;
            entities->animatedSprites[eid]->default_texture_atlas_id = playerAnimatedSprite->default_texture_atlas_id;
            entities->animatedSprites[eid]->frame_interval = playerAnimatedSprite->frame_interval;
            entities->animatedSprites[eid]->num_frames_col = playerAnimatedSprite->num_frames_col;
            entities->animatedSprites[eid]->texture_atlas_id = playerAnimatedSprite->texture_atlas_id;

            entities->renderDatas[ eid ] = renderDataInit( );
            entities->renderDatas[eid]->alphaMod = 150;

            *entities->mirrorEntityRef[eid] = playerId;
            *entities->activeTimer[eid] = activeTime;

            return eid;
        }

        
    }
    // none was overwritten, so need to make a new one
    EntityId entityId = createTempMirrorPlayer( entities, playerId, activeTime );
    return entityId;
}

void overwriteSpeedBoostTimer(Entities *entities,EntityId playerId, float speed, float duration ) {
    if( entities->baseSpeedBoostTimer[playerId] == NULL ) {
        entities->baseSpeedBoostTimer[playerId] = (float *)malloc(sizeof(float));
    }
    *entities->baseSpeedBoostTimer[playerId] = duration;
}

void processSpeedBoostTimer( Entities *entities, float deltaTime ) {
    for( int eid = 0; eid < g_NumEntities; eid++) {
        if( entities->baseSpeedBoostTimer[ eid ] == NULL ) {
            continue;
        }
        if(*entities->baseSpeedBoostTimer[eid] >= 0.0f ) {
            entities->actors[ eid ]->speed_multp += 1.5;
            *entities->baseSpeedBoostTimer[eid] -= deltaTime;
        }
    }
}

void processTempMirrorPlayers( Entities *entities, float deltaTime ) {
    for( int eid = 0; eid < g_NumEntities; eid++ ) {
        if( entities->mirrorEntityRef[ eid ] == NULL ) {
            continue;
        }

        if( *entities->activeTimer[eid] <= 0.0f ) {
            entities->renderDatas[ eid ]->alphaMod = 0;
            continue;
        }

        EntityId playerId = *entities->mirrorEntityRef[ eid ];
        Actor *playerActor = entities->actors[playerId];
        AnimatedSprite *playerAnimatedSprite = entities->animatedSprites[playerId];

        const int tileCols = TILE_COLS - 1; // accidentally gave map one more than it should have
        const int midTileX = tileCols*0.5;

        entities->actors[eid]->current_tile.y = playerActor->current_tile.y;
        if( midTileX == playerActor->current_tile.x ) {
            entities->actors[eid]->current_tile.x = playerActor->current_tile.x ;
        }
        else if( playerActor->current_tile.x < midTileX ) {
            int difference = midTileX - playerActor->current_tile.x;
            entities->actors[eid]->current_tile.x = midTileX + difference;
        }
        else if( playerActor->current_tile.x > midTileX ) {
            int difference = playerActor->current_tile.x - midTileX;
            entities->actors[eid]->current_tile.x = midTileX - difference;
        }

        const int midWorldPointX = midTileX*TILE_SIZE + TILE_SIZE*0.5;

        entities->actors[eid]->direction = playerActor->direction;
        if( entities->actors[eid]->direction == DIR_LEFT || entities->actors[eid]->direction == DIR_RIGHT) {
            entities->actors[eid]->direction = opposite_directions[entities->actors[eid]->direction ];
        }

        entities->actors[eid]->world_position.y = playerActor->world_position.y;

        if( midWorldPointX == playerActor->world_position.x ) {
            entities->actors[eid]->world_position.x = playerActor->world_position.x ;
        }
        else if( playerActor->world_position.x < midWorldPointX ) {
            int difference = midWorldPointX - playerActor->world_position.x;
            entities->actors[eid]->world_position.x = midWorldPointX + difference;
        }
        else if( playerActor->world_position.x > midWorldPointX ) {
            int difference = playerActor->world_position.x - midWorldPointX;
            entities->actors[eid]->world_position.x = midWorldPointX - difference;
        }
        entities->actors[eid]->world_position.x -= ACTOR_SIZE;


        entities->actors[eid]->world_center_point = playerActor->world_center_point;

        entities->actors[eid]->velocity.x = -playerActor->velocity.x;
        entities->actors[eid]->velocity.y = playerActor->velocity.y;

        // pacman animation row
        if( entities->actors[ eid ]->velocity.x > 0 && entities->actors[ eid ]->velocity.y == 0 ) { // right
            entities->animatedSprites[ eid ]->current_anim_row = 0;
        }
        if( entities->actors[ eid ]->velocity.x < 0 && entities->actors[ eid ]->velocity.y == 0 ) { // left
            entities->animatedSprites[ eid ]->current_anim_row = 1;
        }
        if( entities->actors[ eid ]->velocity.x == 0 && entities->actors[ eid ]->velocity.y > 0 ) { // down
            entities->animatedSprites[ eid ]->current_anim_row = 2;
        }
        if( entities->actors[ eid ]->velocity.x == 0 && entities->actors[ eid ]->velocity.y < 0 ) { // up
            entities->animatedSprites[ eid ]->current_anim_row = 3;
        }
        if( entities->actors[ eid ]->velocity.x > 0 && entities->actors[ eid ]->velocity.y < 0 ) { //  up-right
            entities->animatedSprites[ eid ]->current_anim_row = 4;
        }
        if( entities->actors[ eid ]->velocity.x < 0 && entities->actors[ eid ]->velocity.y < 0 ) { // up-left
            entities->animatedSprites[ eid ]->current_anim_row = 5;
        }
        if( entities->actors[ eid ]->velocity.x > 0 && entities->actors[ eid ]->velocity.y > 0 ) { // down-right
            entities->animatedSprites[ eid ]->current_anim_row = 6;
        }
        if( entities->actors[ eid ]->velocity.x < 0 && entities->actors[ 0 ]->velocity.y > 0 ) { // down-left
            entities->animatedSprites[ eid ]->current_anim_row = 7;
        }

        entities->animatedSprites[eid]->accumulator = playerAnimatedSprite->accumulator;
        //entities->animatedSprites[eid]->current_anim_row = playerAnimatedSprite->current_anim_row;
        entities->animatedSprites[eid]->current_frame_col = playerAnimatedSprite->current_frame_col;
        entities->animatedSprites[eid]->default_texture_atlas_id = playerAnimatedSprite->default_texture_atlas_id;
        entities->animatedSprites[eid]->frame_interval = playerAnimatedSprite->frame_interval;
        entities->animatedSprites[eid]->num_frames_col = playerAnimatedSprite->num_frames_col;
        entities->animatedSprites[eid]->texture_atlas_id = playerAnimatedSprite->texture_atlas_id;

        *entities->activeTimer[ eid ] -= deltaTime;        

    }
}

void processTemporaryPickup( Entities *entities, EntityId *playerIds, unsigned int numPlayers, Score *score, TileMap *tilemap, unsigned int numDotsLeft, float deltaTime ) {
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->pickupTypes[ eid ] == NULL || entities->numDots[ eid ] == NULL || entities->activeTimer[ eid ] == NULL ) {
            continue;
        }

        if( *entities->activeTimer[ eid ] <= 0.0f ){
            entities->renderDatas[ eid ]->alphaMod = 0;
            // SDL_Texture *texture = g_texture_atlases[ entities->animatedSprites[ eid ]->texture_atlas_id ].texture;
            // SDL_SetTextureAlphaMod( texture, 0 );
            continue;
        }
        
        // pickup is active
        int numDotsEaten = g_StartingNumDots - numDotsLeft;
        if( *entities->numDots[ eid ]  <= numDotsEaten ) {
            *entities->activeTimer[ eid ] -= deltaTime;
            entities->renderDatas[ eid ]->alphaMod = 255;
            // SDL_Texture *texture = g_texture_atlases[ entities->animatedSprites[ eid ]->texture_atlas_id ].texture;
            // SDL_SetTextureAlphaMod( texture, 255 );

            EntityId playerId;
            for( int i = 0; i < numPlayers; i++ ) {
                playerId = playerIds[ i ];
                // player picks up
                if( points_equal( entities->actors[ playerId ]->current_tile, entities->actors[ eid ]->current_tile ) ) {
                    *entities->activeTimer[ eid ] = 0.0f;
                    score->score_number += *entities->score[ eid ];

                     for( int i = 0; i < g_NumTimedMessages; i++ ) {
                        if( g_TimedMessages[ i ].remainingTime <= 0.0f ) {
                            g_TimedMessages[ i ].remainingTime = 0.85f;
                            g_TimedMessages[ i ].world_position = tile_grid_point_to_world_point( entities->actors[ playerId ]->current_tile );
                            snprintf( g_TimedMessages[ i ].message, 8, "%d", 500 );
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
                            break;
                        case SPEED_BOOST_PICKUP:
                            overwriteSpeedBoostTimer( entities, playerId, gBaseSpeed * 1.5, 5.0f );
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