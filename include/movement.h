#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "actor.h"
#include "tiles.h"
#include <inttypes.h>


void actor_set_current_tile( Actor *actor);

void pac_try_set_direction( Actor *pacmonster, unsigned int gameControllerMask, TileMap *tm );

void move( Actor *actor, Vector_f velocity ) ;

void ghost_move( Actor **actors, TileMap *tm, float delta_time ) ;

void pac_try_move( Actor *pacmonster,  TileMap *tm, float delta_time );

#endif