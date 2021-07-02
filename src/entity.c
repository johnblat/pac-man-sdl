#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "sounds.h"
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


unsigned int g_NumEntities = 0;

EntityId createPlayer( Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite ) {
    EntityId entityId = g_NumEntities;
    g_NumEntities++;

    // allocate
    entities->positions      [ entityId ] = (Position *)                malloc(sizeof(Position));
    entities->actors         [ entityId ] = ( Actor * )                 malloc( sizeof( Actor ) );
    entities->animatedSprites[ entityId ] = (AnimatedSprite * )         malloc( sizeof( Actor ) );
    entities->renderDatas   [ entityId ] = (RenderData *)malloc(sizeof(RenderData ) );
    entities->chargeTimers   [ entityId ] = (float * )                  malloc(sizeof(float));
    entities->dashTimers     [ entityId ] = (float * )                  malloc(sizeof(float ) );
    entities->slowTimers     [ entityId ] = ( float * )malloc(sizeof(float ) );
    entities->inputMasks     [ entityId ] = (uint8_t * ) malloc(sizeof( uint8_t ) );

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

    return entityId;
}

EntityId createGhost(  Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite, TargetingBehavior targetingBehavior ) {
    EntityId entityId = g_NumEntities;
    g_NumEntities++;

    // allocate
    entities->positions      [ entityId ] = (Position *)                malloc(sizeof(Position));
    entities->actors         [ entityId ] = ( Actor * )                 malloc( sizeof( Actor ) );
    entities->animatedSprites[ entityId ] = (AnimatedSprite * )         malloc( sizeof( Actor ) );
    entities->renderDatas   [ entityId ] = (RenderData *)malloc(sizeof(RenderData ) );
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

void ghostsProcess( Entities *entities, EntityId *playerIds, unsigned int numPlayers, TileMap *tilemap, float deltaTime ) {
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
                normal_process( entities, eid, playerIds, numPlayers, tilemap );
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
    entities->animatedSprites[ entityId ] = (AnimatedSprite * )         malloc( sizeof( Actor ) );
    entities->renderDatas   [ entityId ] = (RenderData *)malloc(sizeof(RenderData ) );
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
            
            snprintf( score->score_text, 32, "Score : %d", score->score_number );
            SDL_Surface *score_surface = TTF_RenderText_Solid( score->font, score->score_text, score->score_color );

            SDL_DestroyTexture( score->score_texture );
            score->score_texture = SDL_CreateTextureFromSurface( renderer, score_surface );
            score->score_render_dst_rect.x = 10;
            score->score_render_dst_rect.y = 10;
            score->score_render_dst_rect.w = score_surface->w;
            score->score_render_dst_rect.h = score_surface->h;

            SDL_FreeSurface( score_surface );

        }
    }
        

}