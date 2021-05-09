#ifndef TARGETING_H
#define TARGETING_H

#include <SDL2/SDL.h>
#include "actor.h"
#include "comparisons.h"
#include "tiles.h"

const SDL_Point shadow_home_tile = { 0, 0 };
const SDL_Point ambush_home_tile = { TILE_COLS - 1, 0 };
const SDL_Point moody_home_tile = { 0, TILE_ROWS - 1 };
const SDL_Point pokey_home_tile = { TILE_COLS - 1, TILE_ROWS - 1};


void set_direction_and_next_tile_shortest_to_target( Actor *actor, TileMap *tm ) {
    SDL_Point tile_above, tile_below, tile_left, tile_right;
    tile_above.x = actor->current_tile.x;
    tile_above.y = actor->current_tile.y - 1;
    tile_below.x = actor->current_tile.x;
    tile_below.y = actor->current_tile.y + 1;
    tile_left.x = actor->current_tile.x - 1;
    tile_left.y = actor->current_tile.y;
    tile_right.x = actor->current_tile.x + 1;
    tile_right.y = actor->current_tile.y;

    SDL_Point surrounding_tiles[ 4 ];
    surrounding_tiles[ DIR_UP ] = tile_above;
    surrounding_tiles[ DIR_DOWN ] = tile_below;
    surrounding_tiles[ DIR_LEFT ] = tile_left;
    surrounding_tiles[ DIR_RIGHT ] = tile_right;

    float above_to_target_dist = distance(tile_above.x, tile_above.y,  actor->target_tile.x, actor->target_tile.y ); 
    float below_to_target_dist = distance(tile_below.x, tile_below.y,  actor->target_tile.x, actor->target_tile.y ); 
    float left_to_target_dist  = distance(tile_left.x, tile_left.y,  actor->target_tile.x, actor->target_tile.y ); 
    float right_to_target_dist = distance(tile_right.x, tile_right.y,  actor->target_tile.x, actor->target_tile.y ); 


    float lengths[ 4 ];
    lengths[ DIR_UP ] = above_to_target_dist;
    lengths[ DIR_DOWN ] = below_to_target_dist;
    lengths[ DIR_LEFT ] = left_to_target_dist;
    lengths[ DIR_RIGHT ] = right_to_target_dist;

    Direction opposite_directions[ 4 ];
    opposite_directions[ DIR_UP ] = DIR_DOWN;
    opposite_directions[ DIR_DOWN ] = DIR_UP;
    opposite_directions[ DIR_LEFT ] = DIR_RIGHT;
    opposite_directions[ DIR_RIGHT ] = DIR_LEFT;

    int shortest_direction = opposite_directions[ actor->direction ]; // this will ensure, that if all options are run through and ghost hasnt found a tile NOT behind him, he/she will just turn around
    float shortest_length = 9999.0f; // just some high number


    for( int i = 0; i < 4; ++i ) {
        if( !two_dimensional_indexes_equal( tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ], EMPTY_TILE_TEXTURE_ATLAS_INDEX ) ) {
            continue;
        }

        if( i == opposite_directions[ actor->direction  ] ) continue;

        if ( lengths[ i ] < shortest_length ) {
            shortest_direction = i;
            shortest_length = lengths[ i ];
        }
    }

    actor->direction = (Direction) shortest_direction;
    actor->next_tile = surrounding_tiles[ shortest_direction ];
}



void set_scatter_target_tile( Actor **actors, uint8_t actor_id, TileMap *tm, SDL_Point home_tile ) {
    actors[ actor_id ]->target_tile = home_tile;
}

void set_all_scatter_target_tile( Actor **actors ) {
    actors[ 1 ]->target_tile = shadow_home_tile;
    actors[ 2 ]->target_tile = ambush_home_tile;
    actors[ 3 ]->target_tile = moody_home_tile;
    actors[ 4 ]->target_tile = pokey_home_tile;
}

void set_shadow_target_tile( Actor **actors, uint8_t actor_id, TileMap *tm ) {
    actors[ actor_id ]->target_tile = actors[ 0 ]->current_tile;
}


void set_ambush_target_tile( Actor **actors, TileMap *tm ) {

    switch( actors[ 0 ]->direction ) {
        case DIR_UP:
            actors[ 2 ]->target_tile.x = actors[ 0 ]->current_tile.x;
            actors[ 2 ]->target_tile.y = actors[ 0 ]->current_tile.y - 4;
            break;
        case DIR_DOWN:
            actors[ 2 ]->target_tile.x = actors[ 0 ]->current_tile.x;
            actors[ 2 ]->target_tile.y = actors[ 0 ]->current_tile.y + 4;
            break;
        case DIR_LEFT:
            actors[ 2 ]->target_tile.x = actors[ 0 ]->current_tile.x - 4;
            actors[ 2 ]->target_tile.y = actors[ 0 ]->current_tile.y;
            break;
        case DIR_RIGHT:
            actors[ 2 ]->target_tile.x = actors[ 0 ]->current_tile.x + 4;
            actors[ 2 ]->target_tile.y = actors[ 0 ]->current_tile.y;
            break;
        default :
            actors[ 2 ]->target_tile.x = actors[ 0 ]->current_tile.x;
            actors[ 2 ]->target_tile.y = actors[ 0 ]->current_tile.y;
            break;
    }   
}

void set_moody_target_tile(Actor **actors, TileMap *tm) {

    SDL_Point other_ghost_current_tile = actors[ 1 ]->current_tile;
    SDL_Point offset_tile;
    Line_Points line_from_other_ghost_to_offset;
    int tiles_x;
    int tiles_y;

    switch( actors[ 0 ]->direction ) {
        case DIR_UP:
            offset_tile.x = actors[ 0 ]->current_tile.x;
            offset_tile.y = actors[ 0 ]->current_tile.y - 2;
            
            line_from_other_ghost_to_offset.a.x = actors[ 1 ]->current_tile.x;
            line_from_other_ghost_to_offset.a.y = actors[ 1 ]->current_tile.y;
            line_from_other_ghost_to_offset.b.x = offset_tile.x;
            line_from_other_ghost_to_offset.b.y = offset_tile.x;

            tiles_x = offset_tile.x - other_ghost_current_tile.x;
            tiles_y = offset_tile.y - other_ghost_current_tile.y;

            actors[ 3 ]->target_tile.x = offset_tile.x + tiles_x;
            actors[ 3 ]->target_tile.y = offset_tile.y + tiles_y;

            break;
        case DIR_DOWN:
            offset_tile.x = actors[ 0 ]->current_tile.x;
            offset_tile.y = actors[ 0 ]->current_tile.y + 2;

             tiles_x = offset_tile.x - other_ghost_current_tile.x;
             tiles_y = offset_tile.y - other_ghost_current_tile.y;

            actors[ 3 ]->target_tile.x = offset_tile.x + tiles_x;
            actors[ 3 ]->target_tile.y = offset_tile.y + tiles_y;

            break;
        case DIR_LEFT:
            offset_tile.x = actors[ 0 ]->current_tile.x - 2;
            offset_tile.y = actors[ 0 ]->current_tile.y;

             tiles_x = offset_tile.x - other_ghost_current_tile.x;
             tiles_y = offset_tile.y - other_ghost_current_tile.y;

            actors[ 3 ]->target_tile.x = offset_tile.x + tiles_x;
            actors[ 3 ]->target_tile.y = offset_tile.y + tiles_y;

            break;
        case DIR_RIGHT:
            offset_tile.x = actors[ 0 ]->current_tile.x + 2;
            offset_tile.y = actors[ 0 ]->current_tile.y;

             tiles_x = offset_tile.x - other_ghost_current_tile.x;
             tiles_y = offset_tile.y - other_ghost_current_tile.y;

            actors[ 3 ]->target_tile.x = offset_tile.x + tiles_x;
            actors[ 3 ]->target_tile.y = offset_tile.y + tiles_y;

            break;
        default :
            offset_tile.x = actors[ 0 ]->current_tile.x;
            offset_tile.y = actors[ 0 ]->current_tile.y;

             tiles_x = offset_tile.x - other_ghost_current_tile.x;
             tiles_y = offset_tile.y - other_ghost_current_tile.y;

            actors[ 3 ]->target_tile.x = offset_tile.x + tiles_x;
            actors[ 3 ]->target_tile.y = offset_tile.y + tiles_y;

            break;
    }
    if ( actors[ 3 ]->target_tile.x < 0 ) actors[ 3 ]->target_tile.x = 0;
    if ( actors[ 3 ]->target_tile.y < 0 ) actors[ 3 ]->target_tile.y = 0;

    if ( actors[ 3 ]->target_tile.x > TILE_COLS - 1 ) actors[ 3 ]->target_tile.x = TILE_COLS - 1;
    if ( actors[ 3 ]->target_tile.y > TILE_ROWS - 1 ) actors[ 3 ]->target_tile.y = TILE_ROWS - 1;

}

void set_pokey_target_tile( Actor **actors, TileMap *tm ) {
    SDL_Point b_tile_above, b_tile_below, b_tile_left, b_tile_right;
    float shadow_range = 64;
    SDL_Point b_current_tile = actors[ 4 ]->current_tile;
    SDL_Point pac_current_tile = actors[ 0 ]->current_tile;

    float distance_to_pacman = distance( b_current_tile.x, b_current_tile.y, pac_current_tile.x, pac_current_tile.y );

    if( distance_to_pacman >= shadow_range ) {
        set_shadow_target_tile( actors, 4, tm );
    }
    else {
        set_scatter_target_tile( actors, 4, tm, pokey_home_tile );
    }
}

void set_pen_target_tile( Actor *actor, TileMap *tm ) {
    SDL_Point pen_grid_point = { 22, 13 };
    actor->target_tile = pen_grid_point;
}

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