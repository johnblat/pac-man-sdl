#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "tiles.h"
#include "entity.h"
#include <inttypes.h>


static void alignWorldDataBasedOnWorldPosition( Entities *entities, EntityId eid );


void inputMovementSystem( Entities *entities, TileMap *tilemap, float deltaTime );

void dashTimersSystem( Entities *entities, float deltaTime );

static void moveActor( Entities *entities, EntityId eid, Vector_f vel );

void nonPlayerInputEntityMovementSystem( Entities *entities, EntityId ghostId, TileMap *tm, float delta_time ) ;


#endif