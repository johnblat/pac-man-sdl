#ifndef PAC_H
#define PAC_H

#include <SDL2/SDL.h>

#include "jb_types.h"
#include "constants.h"
#include "tiles.h"

#define distance(x1, y1, x2, y2) ( (x2 - x1 ) * (x2 - x1) + (y2 - y1) * (y2 - y1) )


/**
 * Actor is responsible for interacting with other actors, objects, and the world itself via simulation
 */

#define ACTOR_SIZE TILE_SIZE

typedef enum Direction {
    DIR_DOWN,
    DIR_UP,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_NONE
} Direction;

extern Direction opposite_directions[ 4 ];


// typedef struct Position {
//     Position_f  world_position; // relative position inside the tilemapped world. 
//     SDL_Point   world_center_point; // 
//     SDL_Point   current_tile; // tm coord where the actor currently is

// } Position;

typedef struct Actor {
    Position_f  world_position; // relative position inside the tilemapped world. 
    SDL_Point   world_center_point; // 
    SDL_Point   current_tile; // tm coord where the actor currently is
    SDL_Point   next_tile;    // tm coord where the actor will move to next
    SDL_Point   target_tile;  // tm coord where the actor is trying to get to. unused by pac
    SDL_Point   world_top_sensor;   // relative position inside the tilemapped world.
    SDL_Point   world_bottom_sensor;// relative position inside the tilemapped world.
    SDL_Point   world_left_sensor;  // relative position inside the tilemapped world.
    SDL_Point   world_right_sensor; // relative position inside the tilemapped world.
    Direction   direction;    // direction the actor wants to move
    float       base_speed;   // base_speed. Might want to consider moving this elsewhere? It's going to be the same for all actors
    float       speed_multp;  // multiply base speed by multiplier to get velocity
    Vector_f    velocity;     // current velocity
} Actor;


/******************
 * END MODES********
 * *****************/


Actor *init_actor( SDL_Point initial_tile, SDL_Point tilemap_offset, float base_speed, float speed_multp ) ;

// set current_tile and then call this to reset everything as if its initialization
void actor_reset_data( Actor *actor, SDL_Point initial_tile );



void actor_align_world_data_based_on_world_position( Actor *actor );

#endif
