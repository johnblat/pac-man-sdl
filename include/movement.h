#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "actor.h"
#include "tiles.h"
#include "entity.h"
#include <inttypes.h>


void actor_set_current_tile( Actor *actor);

void inputToTryMoveProcess( Entities *entities, TileMap *tilemap, float deltaTime );

void dashTimersProcess( Entities *entities, float deltaTime );

void moveActors( Entities *entities ) ;

void ghost_move( Actor **actors, TileMap *tm, float delta_time ) ;

void pac_try_move( Actor *pacmonster,  TileMap *tm, float delta_time );

#endif