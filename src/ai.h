#ifndef AI_H
#define AI_H

#include <SDL2/SDL.h>
#include "actor.h"
#include "tiles.h"
/**
 * More specific Ghost behavior.
 * Ghosts should get a default behavior
 * and an active behavior
 */
typedef enum {
    SHADOW,    // blinky and clyde
    AMBUSH,    // pinky
    MOODY,     // inky
    TSUNDERE,     // clyde. Will either make current behavior shadow or scatter.
    SCATTER,   // all and clyde
    VULNERABLE // all
} GhostBehavior;

/**
 * Will be global for all ghosts.
 * Will determine their more specific
 * behavior
 */
typedef enum {
    MODE_CHASE,     // use default behavior
    MODE_SCATTER,   // use scatter behavior
    MODE_VULNERABLE // use vulnerable behavior
} GhostMode;

void set_shadow_next_tile( Actor **actors, TileMap *tm ) {
    SDL_Point b_tile_above, b_tile_below, b_tile_left, b_tile_right;

    SDL_Point b_current_tile = actors[ 1 ]->current_tile;
    SDL_Point pac_current_tile = actors[ 0 ]->current_tile;
    SDL_Point b_target_tile = pac_current_tile;
    actors[ 1 ]->target_tile = b_target_tile;

    b_tile_above.x = b_current_tile.x;
    b_tile_above.y = b_current_tile.y - 1;
    b_tile_below.x = b_current_tile.x;
    b_tile_below.y = b_current_tile.y + 1;
    b_tile_left.x = b_current_tile.x - 1;
    b_tile_left.y = b_current_tile.y;
    b_tile_right.x = b_current_tile.x + 1;
    b_tile_right.y = b_current_tile.y;

    SDL_Point surrounding_tiles[ 4 ];
    surrounding_tiles[ DIR_UP ] = b_tile_above;
    surrounding_tiles[ DIR_DOWN ] = b_tile_below;
    surrounding_tiles[ DIR_LEFT ] = b_tile_left;
    surrounding_tiles[ DIR_RIGHT ] = b_tile_right;

    float above_to_pac_dist = ( b_tile_above.x - b_target_tile.x ) * ( b_tile_above.x - b_target_tile.x )  
        + ( b_tile_above.y - b_target_tile.y ) * ( b_tile_above.y - b_target_tile.y );
    float below_to_pac_dist = ( b_tile_below.x - b_target_tile.x ) * ( b_tile_below.x - b_target_tile.x ) 
        + ( b_tile_below.y - b_target_tile.y ) * ( b_tile_below.y - b_target_tile.y );
    float left_to_pac_dist = ( b_tile_left.x - b_target_tile.x ) * ( b_tile_left.x - b_target_tile.x )  
        + ( b_tile_left.y - b_target_tile.y ) * ( b_tile_left.y - b_target_tile.y );
    float right_to_pac_dist = ( b_tile_right.x - b_target_tile.x ) * ( b_tile_right.x - b_target_tile.x ) 
        + ( b_tile_right.y - b_target_tile.y ) * ( b_tile_right.y - b_target_tile.y );

    float lengths[ 4 ];
    lengths[ DIR_UP ] = above_to_pac_dist;
    lengths[ DIR_DOWN ] = below_to_pac_dist;
    lengths[ DIR_LEFT ] = left_to_pac_dist;
    lengths[ DIR_RIGHT ] = right_to_pac_dist;

    Direction opposite_directions[ 4 ];
    opposite_directions[ DIR_UP ] = DIR_DOWN;
    opposite_directions[ DIR_DOWN ] = DIR_UP;
    opposite_directions[ DIR_LEFT ] = DIR_RIGHT;
    opposite_directions[ DIR_RIGHT ] = DIR_LEFT;

    Uint8 shortest_direction = 0;
    float shortest_length = 9999.0f; // just some high number

    for( int i = 0; i < 4; ++i ) {
        if( tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
            && tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                continue;
        }

        if( i == opposite_directions[ actors[ 1 ]->direction  ] ) continue;

        if ( lengths[ i ] < shortest_length ) {
            shortest_direction = i;
            shortest_length = lengths[ i ];
        }
    }

    actors[ 1 ]->direction = (Direction ) shortest_direction;
    actors[ 1 ]->next_tile = surrounding_tiles[ shortest_direction ];

    
}

#endif