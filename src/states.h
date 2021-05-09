#ifndef STATES_H
#define STATES_H

#include "targeting.h"
#include "actor.h"
#include "render.h"
#include "comparisons.h"

/***************************************
 * ***** STATE SHIT *********************
 * *********************************/

/**
 * Will be global for all ghosts.
 * Will determine their more specific
 * behavior
 */

// the length of time the ghosts should be in scatter or chase
// starts with scatter, then alternates.
// after last element is used, then chase is indefinite
// even periods are scatter
// odd periods are chase

SDL_Point ghost_pen_tile = {22, 11};

typedef enum {
    STATE_NORMAL,
    STATE_VULNERABLE,
    STATE_GO_TO_PEN
} GhostState;

/**
 * Will be global for all ghosts.
 * Will determine their more specific
 * behavior
 */
/*****************
 * MODES *********
 * *************/
const uint8_t NUM_SCATTER_CHASE_PERIODS = 9;
uint8_t g_current_scatter_chase_period = 0;
uint8_t g_scatter_chase_period_seconds[ NUM_SCATTER_CHASE_PERIODS ] = { 7, 20, 7, 20, 5, 20, 5, 0 };
//uint8_t g_scatter_chase_period_seconds[ NUM_SCATTER_CHASE_PERIODS ] = { 2, 2, 2, 2, 2, 2, 2, 0 };
typedef enum {
    MODE_CHASE,     // use default behavior
    MODE_SCATTER,   // use scatter behavior
} GhostMode;

GhostMode g_current_ghost_mode = MODE_SCATTER;

/******************
 * END MODES********
 * *****************/

void vulnerable_enter_all( Actor **ghosts, RenderTexture **render_textures ) {
    uint8_t vulnerable_texture_atlas_id = 5;
    for( int i = 1; i < 5; ++i ) {
        render_textures[ i ]->texture_atlas_id = vulnerable_texture_atlas_id;
        ghosts[ i ]->direction = opposite_directions[ ghosts[ i ]->direction ];
        ghosts[ i ]->next_tile = ghosts[ i ]->current_tile;
    } 
}

void vulnerable_enter( Actor *ghost, RenderTexture *render_texture ) {
    uint8_t vulnerable_texture_atlas_id = 5;

    render_texture->texture_atlas_id = vulnerable_texture_atlas_id;
    ghost->direction = opposite_directions[ ghost->direction ];
    ghost->next_tile = ghost->current_tile;
    ghost->speed = 90;

}


void set_vulnerable_direction_and_next_tile( Actor *ghost, TileMap *tm ) {
    SDL_Point tile_above, tile_below, tile_left, tile_right;
    tile_above.x = ghost->current_tile.x;
    tile_above.y = ghost->current_tile.y - 1;
    tile_below.x = ghost->current_tile.x;
    tile_below.y = ghost->current_tile.y + 1;
    tile_left.x = ghost->current_tile.x - 1;
    tile_left.y = ghost->current_tile.y;
    tile_right.x = ghost->current_tile.x + 1;
    tile_right.y = ghost->current_tile.y;

    SDL_Point surrounding_tiles[ 4 ];
    surrounding_tiles[ DIR_UP ] = tile_above;
    surrounding_tiles[ DIR_DOWN ] = tile_below;
    surrounding_tiles[ DIR_LEFT ] = tile_left;
    surrounding_tiles[ DIR_RIGHT ] = tile_right;

    // just choosing first open tile ghost sees
     for( int i = 0; i < 4; ++i ) {
        if( !two_dimensional_indexes_equal(tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ], EMPTY_TILE_TEXTURE_ATLAS_INDEX ) ) { 
            continue;
        }

        if( i == opposite_directions[ ghost->direction  ] ) continue;

        ghost->next_tile.x = surrounding_tiles[ i ].x;
        ghost->next_tile.y = surrounding_tiles[ i ].y;

        ghost->direction = (Direction) i;
        break;

    }
}

void vulnerable_process( Actor *actor, TileMap *tm ) {
    if( points_equal( actor->next_tile, actor->current_tile ) ) {
        set_vulnerable_direction_and_next_tile( actor, tm );    
    }
}

void normal_enter( Actor *ghost, RenderTexture *render_texture, uint8_t texture_atlas_id ) {
    // set texture atlas id to the id
    render_texture->texture_atlas_id = texture_atlas_id;
    ghost->next_tile = ghost->current_tile;
    ghost->speed = 120;

}

void normal_process( Actor **actors, uint8_t ghost_id, TileMap *tm ) {
    SDL_Point home_tiles[ 5 ] = { shadow_home_tile, shadow_home_tile, ambush_home_tile, moody_home_tile, pokey_home_tile };

    switch( g_current_ghost_mode ) {
        case MODE_CHASE:
            switch (ghost_id) {
                case 1:
                    if(
                         points_equal(  actors[ ghost_id ]->next_tile, actors[ ghost_id ]->current_tile )) {
                        set_shadow_target_tile( actors, ghost_id, tm );
                        set_direction_and_next_tile_shortest_to_target( actors[ ghost_id ], tm );
                    }
                    break;
                case 2:
                    if( points_equal( actors[ ghost_id ]->next_tile, actors[ ghost_id ]->current_tile ) ) {
                        set_ambush_target_tile( actors, tm );
                        set_direction_and_next_tile_shortest_to_target( actors[ ghost_id ], tm );
                    }
                    break;
                case 3:
                    if(points_equal( actors[ ghost_id ]->next_tile, actors[ ghost_id ]->current_tile ) ) {
                        set_moody_target_tile( actors, tm );
                        set_direction_and_next_tile_shortest_to_target( actors[ ghost_id ], tm );
                    }
                    break;
                case 4:
                    if( points_equal( actors[ ghost_id ]->next_tile, actors[ ghost_id ]->current_tile )) {
                        set_pokey_target_tile( actors, tm );
                        set_direction_and_next_tile_shortest_to_target( actors[ ghost_id ], tm );
                    }
                    break;

                default:
                    break;
            }
            break;
        case MODE_SCATTER:
            set_scatter_target_tile( actors, ghost_id, tm, home_tiles[ ghost_id ] );
            if( points_equal( actors[ ghost_id ]->next_tile, actors[ ghost_id ]->current_tile ) ) {
                set_direction_and_next_tile_shortest_to_target( actors[ ghost_id ], tm );
            }
            break;

    }

    if ( actors[ 0 ]->current_tile.x == actors[ ghost_id ]->current_tile.x 
    && actors[ 0 ]->current_tile.y == actors[ ghost_id ]->current_tile.y  ) {
        actors[ 0 ]->position.x = TILE_SIZE * 22;
        actors[ 0 ]->position.y =  TILE_SIZE * 15;
        actors[ 0 ]->center_point.x = ( int ) actors[ 0 ]->position.x + ( ACTOR_SIZE / 2 );
        actors[ 0 ]->center_point.y = ( int ) actors[ 0 ]->position.y + ( ACTOR_SIZE / 2 );

        actors[ 0 ]->top_sensor.x = actors[ 0 ]->position.x + ( ACTOR_SIZE / 2 );
        actors[ 0 ]->top_sensor.y = actors[ 0 ]->position.y;

        actors[ 0 ]->bottom_sensor.x = actors[ 0 ]->position.x + ( ACTOR_SIZE / 2 );
        actors[ 0 ]->bottom_sensor.y = actors[ 0 ]->position.y + ACTOR_SIZE;

        actors[ 0 ]->left_sensor.x = actors[ 0 ]->position.x;
        actors[ 0 ]->left_sensor.y = actors[ 0 ]->position.y + ( ACTOR_SIZE / 2 );

        actors[ 0 ]->right_sensor.x = actors[ 0 ]->position.x + ACTOR_SIZE;
        actors[ 0 ]->right_sensor.y = actors[ 0 ]->position.y + ( ACTOR_SIZE / 2 );    

        actors[ 0 ]->direction = DIR_NONE;

        actor_set_current_tile( actors[ 0 ] );
        actors[ 0 ]->next_tile = actors[ 0 ]->current_tile;
        actors[ 0 ]->next_tile = actors[ 0 ]->current_tile;
    }
    
}

void go_to_pen_enter( Actor *actor, RenderTexture *render_texture, uint8_t id ) {
    uint8_t texture_atlas_id = 6;

    render_texture->texture_atlas_id = texture_atlas_id;
    actor->next_tile = actor->current_tile;
    actor->target_tile = ghost_pen_tile;
    actor->speed = 220;
}


void go_to_pen_process( Actor *actor, TileMap *tm ) {
    //set_pen_target_tile( actor, tm );
    if( points_equal(actor->next_tile, actor->current_tile ) && !points_equal(actor->current_tile, ghost_pen_tile ) ) {
        set_direction_and_next_tile_shortest_to_target( actor, tm );
    }
}

void states_machine_process( Actor **actors, GhostState *ghost_states, TileMap *tm ) {
    
    // process
    for( int i = 1; i < 5; ++i ){
        switch( ghost_states[ i ] ) {
            case STATE_NORMAL:
                normal_process( actors, i, tm );
                break;
            case STATE_VULNERABLE:
                vulnerable_process(actors[ i ], tm );
                break;
            case STATE_GO_TO_PEN:
                go_to_pen_process( actors[ i ], tm );
                break;
        }
    }
}

/****************
 * END STATE SHIT *
 * ****************/
#endif