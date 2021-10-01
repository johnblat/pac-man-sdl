#ifndef PAC_H
#define PAC_H

#include <SDL2/SDL.h>

#include "jb_types.h"
#include "constants.h"
#include "tiles.h"
#include "entity.h"



// typedef struct Position {
//     Position_f  world_position; // relative position inside the tilemapped world. 
//     SDL_Point   world_center_point; // 
//     SDL_Point   current_tile; // tm coord where the actor currently is

// } Position;



/******************
 * END MODES********
 * *****************/


// Actor *init_actor( SDL_Point initial_tile, SDL_Point tilemap_offset, float base_speed, float speed_multp ) ;

// set current_tile and then call this to reset everything as if its initialization
void actor_reset_data( Entities *entities, EntityId eid, SDL_Point initial_tile );


void actor_align_world_data_based_on_world_position( Entities *entities, EntityId eid );

#endif
