#ifndef PAC_H
#define PAC_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include "animation.h"
#include "jb_types.h"
#include "constants.h"
#include "tiles.h"

#define distance(x1, y1, x2, y2) ( (x2 - x1 ) * (x2 - x1) + (y2 - y1) * (y2 - y1) )

SDL_Point shadow_home_tile = { 0, 0 };
SDL_Point ambush_home_tile = { TILE_COLS - 1, 0 };
SDL_Point moody_home_tile = { 0, TILE_ROWS - 1 };
SDL_Point pokey_home_tile = { TILE_COLS - 1, TILE_ROWS - 1};

/**
 * Actor is responsible for interacting with other actors, objects, and the world itself via simulation
 */

const int ACTOR_SIZE = TILE_SIZE;

typedef enum Direction {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_NONE
} Direction;

/**
 * More specific Ghost behavior.
 * Ghosts should get a default behavior
 * and an active behavior
 */
typedef enum {
    SHADOW,    // blinky and clyde
    AMBUSH,    // pinky
    MOODY,     // inky
    POKEY,     // clyde. Will either make current behavior shadow or scatter.
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

//int pac_src_sprite_size = 64; 

typedef struct {
    SDL_Point next_tile;
    Direction current_direction;
    Vector_f velocity;
} Movement;

typedef struct {
    int default_behavior;
    int current_behavior;
    SDL_Point target_tile;
} AiDecisions;

typedef struct {

} Sensors;

typedef struct Actor {
    Position_f  position;     // screen position
    SDL_Point   center_point; // screen position
    SDL_Point   current_tile; // grid coord where the actor currently is
    SDL_Point   next_tile;    // grid coord where the actor will move to next
    SDL_Point   target_tile;  // grid coord where the actor is trying to get to. unused by pac
    SDL_Point   top_sensor;   // screen position
    SDL_Point   bottom_sensor;// screen position
    SDL_Point   left_sensor;  // screen position
    SDL_Point   right_sensor; // screen position
    Direction   direction;    // direction the actor wants to move
    Vector_f    velocity;     // current velocity
    GhostBehavior chase_behavior; // the normal chase behavior 
    GhostBehavior current_behavior; // may or may not be chase behavior depending on their chase behavior or the current mode. For instance, pokey chase behavior will alternate between a current behavior of shadow or scatter. 
} Actor;

void actor_set_current_tile( Actor *actor ) {
    actor->current_tile.x = ( ( actor->position.x + TILE_SIZE / 2 ) / TILE_SIZE ) ;
    actor->current_tile.y = ( ( ( actor->position.y + TILE_SIZE / 2 ) - (TILE_SIZE * 2 ) ) / TILE_SIZE ) ;
}

Actor *init_actor( Position_f initial_position ) {
    Actor *actor;
    actor = ( Actor *) malloc( sizeof( Actor ) );

    actor->position.x = initial_position.x;
    actor->position.y = initial_position.y;

    actor->center_point.x = ( int ) actor->position.x + ( ACTOR_SIZE / 2 );
    actor->center_point.y = ( int ) actor->position.y + ( ACTOR_SIZE / 2 );

    actor->top_sensor.x = actor->position.x + ( ACTOR_SIZE / 2 );
    actor->top_sensor.y = actor->position.y;

    actor->bottom_sensor.x = actor->position.x + ( ACTOR_SIZE / 2 );
    actor->bottom_sensor.y = actor->position.y + ACTOR_SIZE;

    actor->left_sensor.x = actor->position.x;
    actor->left_sensor.y = actor->position.y + ( ACTOR_SIZE / 2 );

    actor->right_sensor.x = actor->position.x + ACTOR_SIZE;
    actor->right_sensor.y = actor->position.y + ( ACTOR_SIZE / 2 );    

    actor->direction = DIR_NONE;

    actor_set_current_tile( actor );
    actor->next_tile = actor->current_tile;
    actor->next_tile = actor->current_tile;

    return actor;
}

void set_scatter_next_tile( Actor **actors, uint8_t actor_id, TileMap *tm ) {
    actors[ actor_id ]->target_tile = pokey_home_tile;

    SDL_Point b_tile_above, b_tile_below, b_tile_left, b_tile_right;

    SDL_Point b_current_tile = actors[ actor_id ]->current_tile;


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

    float above_to_pac_dist = ( b_tile_above.x - actors[ actor_id ]->target_tile.x ) * ( b_tile_above.x - actors[ actor_id ]->target_tile.x )  
        + ( b_tile_above.y - actors[ actor_id ]->target_tile.y ) * ( b_tile_above.y - actors[ actor_id ]->target_tile.y );
    float below_to_pac_dist = ( b_tile_below.x - actors[ actor_id ]->target_tile.x ) * ( b_tile_below.x - actors[ actor_id ]->target_tile.x ) 
        + ( b_tile_below.y - actors[ actor_id ]->target_tile.y ) * ( b_tile_below.y - actors[ actor_id ]->target_tile.y );
    float left_to_pac_dist = ( b_tile_left.x - actors[ actor_id ]->target_tile.x ) * ( b_tile_left.x - actors[ actor_id ]->target_tile.x )  
        + ( b_tile_left.y - actors[ actor_id ]->target_tile.y ) * ( b_tile_left.y - actors[ actor_id ]->target_tile.y );
    float right_to_pac_dist = ( b_tile_right.x - actors[ actor_id ]->target_tile.x ) * ( b_tile_right.x - actors[ actor_id ]->target_tile.x ) 
        + ( b_tile_right.y - actors[ actor_id ]->target_tile.y ) * ( b_tile_right.y - actors[ actor_id ]->target_tile.y );

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

    int shortest_direction = 0;
    float shortest_length = 9999.0f; // just some high number

    for( int i = 0; i < 4; ++i ) {
        if( tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
            && tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                continue;
        }

        if( i == opposite_directions[ actors[ actor_id  ]->direction  ] ) continue;

        if ( lengths[ i ] < shortest_length ) {
            shortest_direction = i;
            shortest_length = lengths[ i ];
        }
    }

    actors[ actor_id ]->direction = (Direction) shortest_direction;
    actors[ actor_id ]->next_tile = surrounding_tiles[ shortest_direction ];

}

void set_shadow_next_tile( Actor **actors, uint8_t actor_id, TileMap *tm ) {
    SDL_Point b_tile_above, b_tile_below, b_tile_left, b_tile_right;

    SDL_Point b_current_tile = actors[ actor_id ]->current_tile;
    SDL_Point pac_current_tile = actors[ 0 ]->current_tile;
    SDL_Point b_target_tile = pac_current_tile;
    actors[ actor_id ]->target_tile = b_target_tile;

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

    int shortest_direction = 0;
    float shortest_length = 99990.9f; // just some high number

    for( int i = 0; i < 4; ++i ) {
        if( tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
            && tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                continue;
        }

        if( i == opposite_directions[ actors[ actor_id ]->direction  ] ) continue;

        if ( lengths[ i ] < shortest_length ) {
            shortest_direction = i;
            shortest_length = lengths[ i ];
        }
    }

    actors[ actor_id ]->direction = (Direction) shortest_direction;
    actors[ actor_id ]->next_tile = surrounding_tiles[ shortest_direction ];

    
}



void set_ambush_next_tile( Actor **actors, TileMap *tm ) {
    SDL_Point b_tile_above, b_tile_below, b_tile_left, b_tile_right;

    SDL_Point b_current_tile = actors[ 2 ]->current_tile;
    SDL_Point pac_current_tile = actors[ 0 ]->current_tile;
    SDL_Point b_target_tile;
    switch( actors[ 0 ]->direction ) {
        case DIR_UP:
            b_target_tile.x = pac_current_tile.x;
            b_target_tile.y = pac_current_tile.y - 4;
            break;
        case DIR_DOWN:
            b_target_tile.x = pac_current_tile.x;
            b_target_tile.y = pac_current_tile.y + 4;
            break;
        case DIR_LEFT:
            b_target_tile.x = pac_current_tile.x - 4;
            b_target_tile.y = pac_current_tile.y;
            break;
        case DIR_RIGHT:
            b_target_tile.x = pac_current_tile.x + 4;
            b_target_tile.y = pac_current_tile.y;
            break;
        default :
            b_target_tile.x = pac_current_tile.x;
            b_target_tile.y = pac_current_tile.y;
            break;
    }
    actors[ 2 ]->target_tile = b_target_tile;

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

    int shortest_direction = 0;
    float shortest_length = 9999.0f; // just some high number

    for( int i = 0; i < 4; ++i ) {
        if( tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
            && tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                continue;
        }

        if( i == opposite_directions[ actors[ 2 ]->direction  ] ) continue;

        if ( lengths[ i ] < shortest_length ) {
            shortest_direction = i;
            shortest_length = lengths[ i ];
        }
    }

    actors[ 2 ]->direction = (Direction) shortest_direction;
    actors[ 2 ]->next_tile = surrounding_tiles[ shortest_direction ];

    
}

void set_moody_next_tile(Actor **actors, TileMap *tm) {
    SDL_Point tile_above, tile_below, tile_left, tile_right;

    SDL_Point pac_current_tile = actors[ 0 ]->current_tile;
    SDL_Point other_ghost_current_tile = actors[ 1 ]->current_tile;
    SDL_Point b_target_tile;
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

            b_target_tile.x = offset_tile.x + tiles_x;
            b_target_tile.y = offset_tile.y + tiles_y;

            break;
        case DIR_DOWN:
            offset_tile.x = actors[ 0 ]->current_tile.x;
            offset_tile.y = actors[ 0 ]->current_tile.y + 2;

             tiles_x = offset_tile.x - other_ghost_current_tile.x;
             tiles_y = offset_tile.y - other_ghost_current_tile.y;

            b_target_tile.x = offset_tile.x + tiles_x;
            b_target_tile.y = offset_tile.y + tiles_y;

            break;
        case DIR_LEFT:
            offset_tile.x = actors[ 0 ]->current_tile.x - 2;
            offset_tile.y = actors[ 0 ]->current_tile.y;

             tiles_x = offset_tile.x - other_ghost_current_tile.x;
             tiles_y = offset_tile.y - other_ghost_current_tile.y;

            b_target_tile.x = offset_tile.x + tiles_x;
            b_target_tile.y = offset_tile.y + tiles_y;

            break;
        case DIR_RIGHT:
            offset_tile.x = actors[ 0 ]->current_tile.x + 2;
            offset_tile.y = actors[ 0 ]->current_tile.y;

             tiles_x = offset_tile.x - other_ghost_current_tile.x;
             tiles_y = offset_tile.y - other_ghost_current_tile.y;

            b_target_tile.x = offset_tile.x + tiles_x;
            b_target_tile.y = offset_tile.y + tiles_y;

            break;
        default :
            offset_tile.x = actors[ 0 ]->current_tile.x;
            offset_tile.y = actors[ 0 ]->current_tile.y;

             tiles_x = offset_tile.x - other_ghost_current_tile.x;
             tiles_y = offset_tile.y - other_ghost_current_tile.y;

            b_target_tile.x = offset_tile.x + tiles_x;
            b_target_tile.y = offset_tile.y + tiles_y;

            break;
    }
    actors[ 3 ]->target_tile = b_target_tile;
    if ( actors[ 3 ]->target_tile.x < 0 ) actors[ 3 ]->target_tile.x = 0;
    if ( actors[ 3 ]->target_tile.y < 0 ) actors[ 3 ]->target_tile.y = 0;

    if ( actors[ 3 ]->target_tile.x > TILE_COLS - 1 ) actors[ 3 ]->target_tile.x = TILE_COLS - 1;
    if ( actors[ 3 ]->target_tile.y > TILE_ROWS - 1 ) actors[ 3 ]->target_tile.y = TILE_ROWS - 1;


    tile_above.x = actors[ 3 ]->current_tile.x;
    tile_above.y = actors[ 3 ]->current_tile.y - 1;
    tile_below.x = actors[ 3 ]->current_tile.x;
    tile_below.y = actors[ 3 ]->current_tile.y + 1;
    tile_left.x = actors[ 3 ]->current_tile.x - 1;
    tile_left.y = actors[ 3 ]->current_tile.y;
    tile_right.x = actors[ 3 ]->current_tile.x + 1;
    tile_right.y = actors[ 3 ]->current_tile.y;

    SDL_Point surrounding_tiles[ 4 ];
    surrounding_tiles[ DIR_UP ] = tile_above;
    surrounding_tiles[ DIR_DOWN ] = tile_below;
    surrounding_tiles[ DIR_LEFT ] = tile_left;
    surrounding_tiles[ DIR_RIGHT ] = tile_right;

    float above_to_pac_dist = ( tile_above.x - b_target_tile.x ) * ( tile_above.x - b_target_tile.x )  
        + ( tile_above.y - b_target_tile.y ) * ( tile_above.y - b_target_tile.y );
    float below_to_pac_dist = ( tile_below.x - b_target_tile.x ) * ( tile_below.x - b_target_tile.x ) 
        + ( tile_below.y - b_target_tile.y ) * ( tile_below.y - b_target_tile.y );
    float left_to_pac_dist = ( tile_left.x - b_target_tile.x ) * ( tile_left.x - b_target_tile.x )  
        + ( tile_left.y - b_target_tile.y ) * ( tile_left.y - b_target_tile.y );
    float right_to_pac_dist = ( tile_right.x - b_target_tile.x ) * ( tile_right.x - b_target_tile.x ) 
        + ( tile_right.y - b_target_tile.y ) * ( tile_right.y - b_target_tile.y );

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

    int shortest_direction = 0;
    float shortest_length = 9999.0f; // just some high number

    for( int i = 0; i < 4; ++i ) {
        if( tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
            && tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                continue;
        }

        if( i == opposite_directions[ actors[ 3 ]->direction  ] ) continue;

        if ( lengths[ i ] < shortest_length ) {
            shortest_direction = i;
            shortest_length = lengths[ i ];
        }
    }

    actors[ 3 ]->direction = (Direction) shortest_direction;
    actors[ 3 ]->next_tile = surrounding_tiles[ shortest_direction ];
}

void set_pokey_next_tile( Actor **actors, TileMap *tm ) {
    SDL_Point b_tile_above, b_tile_below, b_tile_left, b_tile_right;
    float shadow_range = 64;
    SDL_Point b_current_tile = actors[ 4 ]->current_tile;
    SDL_Point pac_current_tile = actors[ 0 ]->current_tile;

    float distance_to_pacman = distance( b_current_tile.x, b_current_tile.y, pac_current_tile.x, pac_current_tile.y );

    if( distance_to_pacman >= shadow_range ) {
        set_shadow_next_tile( actors, 4, tm );
    }
    else {
        set_scatter_next_tile( actors, 4, tm );
    }
}


void pac_collect_dot( Actor *pacmonster, char dots[ TILE_ROWS ][ TILE_COLS ], Score *score, SDL_Renderer *renderer ) {
    if( dots[ pacmonster->current_tile.y ][ pacmonster->current_tile.x ] == 'x') {
        // get rid of dot marker
        dots[ pacmonster->current_tile.y ][ pacmonster->current_tile.x ] = ' ';
        
        score->score_number += 10;
        
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


void pac_try_set_direction( Actor *pacmonster, const Uint8 *current_key_states, TileMap *tm ) {

    // don't allow changing direciton if pacman is more than half of the tile
    if( current_key_states[ SDL_SCANCODE_UP ] ) {
        SDL_Point tile_above = { pacmonster->current_tile.x, pacmonster->current_tile.y - 1 };
        SDL_Rect tile_above_rect = {tile_grid_point_to_screen_point( tile_above, tm->tm_screen_position ).x, tile_grid_point_to_screen_point( tile_above, tm->tm_screen_position ).y, TILE_SIZE, TILE_SIZE };

        if( pacmonster->center_point.x > tile_above_rect.x + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_RIGHT ) {
            return;
        }

        if( pacmonster->center_point.x < tile_above_rect.x + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_LEFT ) {
            return;
        }

        if(tm->tm_texture_atlas_indexes[ tile_above.y ][ tile_above.x ].r == EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
            && tm->tm_texture_atlas_indexes[ tile_above.y ][ tile_above.x ].c == EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) 
        {
            pacmonster->direction = DIR_UP;
        }
        else {
            return;
        }
    }

    if( current_key_states[ SDL_SCANCODE_DOWN ] ) {
        SDL_Point tile_below = { pacmonster->current_tile.x, pacmonster->current_tile.y + 1 };
        SDL_Rect tile_below_rect = {tile_grid_point_to_screen_point( tile_below, tm->tm_screen_position ).x, tile_grid_point_to_screen_point( tile_below, tm->tm_screen_position ).y, TILE_SIZE, TILE_SIZE };

        if( pacmonster->center_point.x > tile_below_rect.x + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_RIGHT ) {
            return;
        }
        if( pacmonster->center_point.x < tile_below_rect.x + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_LEFT ) {
            return;
        }

        if(tm->tm_texture_atlas_indexes[ tile_below.y ][ tile_below.x ].r == EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
            && tm->tm_texture_atlas_indexes[ tile_below.y ][ tile_below.x ].c == EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) 
        {
            pacmonster->direction = DIR_DOWN;
        }
        else {
            return;
        }
    }

    if( current_key_states[ SDL_SCANCODE_LEFT ] ) {
        SDL_Point tile_to_left = { pacmonster->current_tile.x - 1, pacmonster->current_tile.y  };
        SDL_Rect tile_to_left_rect = {tile_grid_point_to_screen_point( tile_to_left, tm->tm_screen_position ).x, tile_grid_point_to_screen_point( tile_to_left, tm->tm_screen_position ).y, TILE_SIZE, TILE_SIZE };

        if( pacmonster->center_point.y > tile_to_left_rect.y + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_DOWN ) {
            return;
        }
        if( pacmonster->center_point.y < tile_to_left_rect.y + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_UP ) {
            return;
        }

        if(tm->tm_texture_atlas_indexes[ tile_to_left.y ][ tile_to_left.x ].r == EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
            && tm->tm_texture_atlas_indexes[ tile_to_left.y ][ tile_to_left.x ].c == EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) 
        {
            pacmonster->direction = DIR_LEFT;
        }
        else {
            return;
        }
    }

    if( current_key_states[ SDL_SCANCODE_RIGHT ] ) {
        SDL_Point tile_to_right = { pacmonster->current_tile.x + 1, pacmonster->current_tile.y };
        SDL_Rect tile_to_right_rect = {tile_grid_point_to_screen_point( tile_to_right, tm->tm_screen_position ).x, tile_grid_point_to_screen_point( tile_to_right, tm->tm_screen_position ).y, TILE_SIZE, TILE_SIZE };

        if( pacmonster->center_point.y > tile_to_right_rect.y + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_DOWN ) {
            return;
        }
        if( pacmonster->center_point.y < tile_to_right_rect.y + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_UP ) {
            return;
        }

        if(tm->tm_texture_atlas_indexes[ tile_to_right.y ][ tile_to_right.x ].r == EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
            && tm->tm_texture_atlas_indexes[ tile_to_right.y ][ tile_to_right.x ].c == EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) 
        {
            pacmonster->direction = DIR_RIGHT;
        }
        else {
            return;
        }
    }
}


void move( Actor *actor, Vector_f velocity ) {
    actor->position.x += velocity.x;
    actor->position.y += velocity.y;

    actor->center_point.x = ( int ) actor->position.x + ( TILE_SIZE / 2 );
    actor->center_point.y = ( int ) actor->position.y + ( TILE_SIZE / 2 );

    actor->current_tile.x = ( ( actor->position.x + TILE_SIZE / 2 ) / TILE_SIZE ) ;
    actor->current_tile.y = ( ( ( actor->position.y + TILE_SIZE / 2 ) - (TILE_SIZE * 2 ) ) / TILE_SIZE ) ;

    actor->top_sensor.x = actor->position.x + ( TILE_SIZE / 2 );
    actor->top_sensor.y = actor->position.y;

    actor->bottom_sensor.x = actor->position.x + ( TILE_SIZE / 2 );
    actor->bottom_sensor.y = actor->position.y + TILE_SIZE;

    actor->left_sensor.x = actor->position.x;
    actor->left_sensor.y = actor->position.y + ( TILE_SIZE / 2 );

    actor->right_sensor.x = actor->position.x + TILE_SIZE;
    actor->right_sensor.y = actor->position.y + ( TILE_SIZE / 2 );
}

void ghost_move( Actor **actors, TileMap *tm, float delta_time ) {
    int speed = 160;
    

    if( actors[ 1 ]->next_tile.x == actors[ 1 ]->current_tile.x
        && actors[ 1 ]->next_tile.y == actors[ 1 ]->current_tile.y ) {

        set_shadow_next_tile( actors, 1, tm );
    }

     if( actors[ 2 ]->next_tile.x == actors[ 2 ]->current_tile.x
        && actors[ 2 ]->next_tile.y == actors[ 2 ]->current_tile.y ) {

        set_ambush_next_tile( actors, tm );
    }
    if( actors[ 3 ]->next_tile.x == actors[ 3 ]->current_tile.x
        && actors[ 3 ]->next_tile.y == actors[ 3 ]->current_tile.y ) {

        set_moody_next_tile( actors, tm );
    }
    if( actors[ 4 ]->next_tile.x == actors[ 4 ]->current_tile.x
        && actors[ 4 ]->next_tile.y == actors[ 4 ]->current_tile.y ) {

        set_pokey_next_tile( actors, tm );
    }

    for( int i = 1; i < 5; ++i ) {
        Vector_f velocity = { 0, 0 };
        if( actors[ i ]->direction == DIR_UP ) {
            // set velocity
            if ( actors[ i ]->center_point.x == tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = -1;
                velocity.x *= speed * delta_time;
                velocity.y *= speed * delta_time;
            } 
            else if( actors[ i ]->center_point.x < tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 1;
                velocity.y = 0;
                velocity.x *= speed * delta_time;
                velocity.y *= speed * delta_time;
            }
            else if( actors[ i ]->center_point.x > tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 )){
                velocity.x = -1;
                velocity.y = 0;
                velocity.x *= speed * delta_time;
                velocity.y *= speed * delta_time;
            }
        }
        else if( actors[ i ]->direction == DIR_DOWN ){
            // set velocity
            if ( actors[ i ]->center_point.x == tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = 1;
                velocity.x *= speed * delta_time;
                velocity.y *= speed * delta_time;
            } 
            else if( actors[ i ]->center_point.x < tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 1;
                velocity.y = 0;
                velocity.x *= speed * delta_time;
                velocity.y *= speed * delta_time;
            }
            else if( actors[ i ]->center_point.x >tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 )){
                velocity.x = -1;
                velocity.y = 0;
                velocity.x *= speed * delta_time;
                velocity.y *= speed * delta_time;
            }
        } 
        else if( actors[ i ]->direction == DIR_LEFT ) {
            // set velocity
            if ( actors[ i ]->center_point.y == tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = -1;
                velocity.y = 0;
                velocity.x *= speed * delta_time;
                velocity.y *= speed * delta_time;
            } 
            else if( actors[ i ]->center_point.y < tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = 1;
                velocity.x *= speed * delta_time;
                velocity.y *= speed * delta_time;
            }
            else if( actors[ i ]->center_point.y >tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 )){
                velocity.x = 0;
                velocity.y = -1;
                velocity.x *= speed * delta_time;
                velocity.y *= speed * delta_time;
            }
        }
        else if(actors[ i ]->direction == DIR_RIGHT ) {
            // set velocity
            if ( actors[ i ]->center_point.y == tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 1;
                velocity.y = 0;
                velocity.x *= speed * delta_time;
                velocity.y *= speed * delta_time;
            } 
            else if( actors[ i ]->center_point.y < tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = 1;
                velocity.x *= speed * delta_time;
                velocity.y *= speed * delta_time;
            }
            else if( actors[ i ]->center_point.y >tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 )){
                velocity.x = 0;
                velocity.y = -1;
                velocity.x *= speed * delta_time;
                velocity.y *= speed * delta_time;
            }
        }
        move(actors[ i ], velocity );
    }
    
}

void pac_try_move( Actor *pacmonster,  TileMap *tm, float delta_time ) {
    
    int pac_speed = 170;
    Vector_f velocity = { 0, 0 };

    if( pacmonster->direction == DIR_UP ) {
        pacmonster->next_tile.x = pacmonster->current_tile.x;
        pacmonster->next_tile.y = pacmonster->current_tile.y - 1;

        // set velocity
        if ( pacmonster->center_point.x == tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0;
            velocity.y = -1;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        } 
        else if( pacmonster->center_point.x < tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0.7071068;
            velocity.y = -0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }
        else if( pacmonster->center_point.x >tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 )){
            velocity.x = -0.7071068;
            velocity.y = -0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }

        // move
        move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_screen_point( pacmonster->next_tile, tm->tm_screen_position );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // top sensor is inside target tile
        if( pacmonster->top_sensor.y < target_tile_rect.y + TILE_SIZE 
        && pacmonster->top_sensor.x > target_tile_rect.x 
        && pacmonster->top_sensor.x < target_tile_rect.x + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
            && tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                move(pacmonster, reversed_velocity );
            }
        }
    }

    if( pacmonster->direction == DIR_DOWN ) {
        pacmonster->next_tile.x = pacmonster->current_tile.x;
        pacmonster->next_tile.y = pacmonster->current_tile.y + 1;

        // set velocity
        if ( pacmonster->center_point.x == tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0;
            velocity.y = 1;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        } 
        else if( pacmonster->center_point.x < tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0.7071068;
            velocity.y = 0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }
        else if( pacmonster->center_point.x >tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 )){
            velocity.x = -0.7071068;
            velocity.y = 0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }

        // move
        move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_screen_point( pacmonster->next_tile, tm->tm_screen_position );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->bottom_sensor.y > target_tile_rect.y 
        && pacmonster->bottom_sensor.x > target_tile_rect.x 
        && pacmonster->bottom_sensor.x < target_tile_rect.x + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
            && tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                move(pacmonster, reversed_velocity );
            }
        }
    }

    if( pacmonster->direction == DIR_LEFT ) {
        pacmonster->next_tile.x = pacmonster->current_tile.x - 1;
        pacmonster->next_tile.y = pacmonster->current_tile.y;

        // set velocity
        if ( pacmonster->center_point.y == tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = -1;
            velocity.y = 0;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        } 
        else if( pacmonster->center_point.y < tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = -0.7071068;
            velocity.y = 0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }
        else if( pacmonster->center_point.y >tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 )){
            velocity.x = -0.7071068;
            velocity.y = -0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }

        // move
        move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_screen_point( pacmonster->next_tile, tm->tm_screen_position );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->left_sensor.x < target_tile_rect.x + TILE_SIZE
        && pacmonster->left_sensor.y > target_tile_rect.y 
        && pacmonster->left_sensor.y < target_tile_rect.y + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
            && tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                move(pacmonster, reversed_velocity );
            }
        }
    }

    if( pacmonster->direction == DIR_RIGHT ) {
        pacmonster->next_tile.x = pacmonster->current_tile.x + 1;
        pacmonster->next_tile.y = pacmonster->current_tile.y;

        // set velocity
        if ( pacmonster->center_point.y == tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 1;
            velocity.y = 0;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        } 
        else if( pacmonster->center_point.y < tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0.7071068;
            velocity.y = 0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }
        else if( pacmonster->center_point.y >tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 )){
            velocity.x = 0.7071068;
            velocity.y = -0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }

        // move
        move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_screen_point( pacmonster->next_tile, tm->tm_screen_position );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->right_sensor.x > target_tile_rect.x 
        && pacmonster->right_sensor.y > target_tile_rect.y 
        && pacmonster->right_sensor.y < target_tile_rect.y + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
            && tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                move(pacmonster, reversed_velocity );
            }
        }
    }
}

#endif
