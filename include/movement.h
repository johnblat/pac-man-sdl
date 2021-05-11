#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "actor.h"
#include "tiles.h"
#include <inttypes.h>

void set_direction_and_next_tile_shortest_to_target( Actor *actor, TileMap *tm ) ;

void actor_set_current_tile( Actor *actor);

void pac_try_set_direction( Actor *pacmonster, const uint8_t *current_key_states, TileMap *tm );

void move( Actor *actor, Vector_f velocity ) ;

void ghost_move( Actor **actors, TileMap *tm, float delta_time ) ;

void pac_try_move( Actor *pacmonster,  TileMap *tm, float delta_time );

#endif