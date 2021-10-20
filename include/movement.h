#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "tiles.h"
#include "entity.h"
#include "userTypeDefinitions.h"
#include <inttypes.h>


static void alignWorldDataBasedOnWorldPosition( Entities *entities, EntityId eid );


void inputMovementSystem( Entities *entities, TileMap *tilemap, float deltaTime );

void dashTimersSystem(ecs_iter_t *it );

void moveActor( Position *position, Velocity velocity, CurrentTile *currentTile, CollisionRect *collisionRect, Sensor *sensor);

void nonPlayerInputEntityMovementSystem( ecs_iter_t *it) ;


#endif