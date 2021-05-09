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

Direction opposite_directions[ 4 ] = { DIR_DOWN, DIR_UP, DIR_RIGHT, DIR_LEFT };


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
} Actor;

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


void actor_set_current_tile( Actor *actor) ;

Actor *init_actor( Position_f initial_position ) ;

void set_direction_and_next_tile_shortest_to_target( Actor *actor, TileMap *tm ) ;

void set_scatter_target_tile( Actor **actors, uint8_t actor_id, TileMap *tm ) ;

void set_all_scatter_target_tile( Actor **actors );

void set_shadow_target_tile( Actor **actors, uint8_t actor_id, TileMap *tm );

void set_ambush_target_tile( Actor **actors, TileMap *tm ) ;

void set_moody_target_tile(Actor **actors, TileMap *tm) ;

void set_pokey_target_tile( Actor **actors, TileMap *tm ) ;

void pac_collect_dot( Actor *pacmonster, char dots[ TILE_ROWS ][ TILE_COLS ], Score *score, SDL_Renderer *renderer );

void pac_try_set_direction( Actor *pacmonster, const Uint8 *current_key_states, TileMap *tm );

void move( Actor *actor, Vector_f velocity ) ;

void ghost_move( Actor **actors, TileMap *tm, float delta_time ) ;

void pac_try_move( Actor *pacmonster,  TileMap *tm, float delta_time );

#endif
