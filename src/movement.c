#include <SDL2/SDL.h>
#include "entity.h"
#include "movement.h"
#include "tiles.h"
#include "comparisons.h"
#include "jb_types.h"
#include "input.h"
#include "globalData.h"
#include "userTypeDefinitions.h"

static void tile_wrap( SDL_Point *tile ) {
    if( tile->y >= TILE_ROWS ) 
        tile->y = 0;
    if( tile->y < 0 )
        tile->y = TILE_ROWS - 1;
    if( tile->x >= TILE_COLS )
        tile->x = 0;
    if( tile->x < 0 )
        tile->x = TILE_COLS - 1;
}


void alignTileDataToPosition(Position position, CurrentTile *currentTile, 
                             SDL_Rect *collisionRect ){
    currentTile->x = position.x / TILE_SIZE;
    currentTile->y = position.y / TILE_SIZE;
    
    collisionRect->w = TILE_SIZE*0.5;
    collisionRect->h = TILE_SIZE*0.5;
    collisionRect->x = position.x - collisionRect->w*0.5;
    collisionRect->y = position.y - collisionRect->h*0.5;
}

void alignSensorDataToPosition(Position position, Sensor *sensor){
    sensor->worldTopSensor = (SDL_Point){
        position.x,
        position.y - (TILE_SIZE*0.5)
    };
    sensor->worldBottomSensor = (SDL_Point){
        position.x,
        position.y + (TILE_SIZE*0.5)
    };
    sensor->worldLeftSensor = (SDL_Point){
        position.x - (TILE_SIZE*0.5),
        position.y 
    };
    sensor->worldRightSensor = (SDL_Point){
        position.x + (TILE_SIZE*0.5),
        position.y
    };
}



// static void alignWorldDataBasedOnWorldPosition( Entities *entities, EntityId eid ) {

//     entities->currentTiles[eid]->x = entities->worldPositions[eid]->x / TILE_SIZE;
//     entities->currentTiles[eid]->y = entities->worldPositions[eid]->y / TILE_SIZE;

//     if(entities->sensors[eid] != NULL){
//         entities->sensors[eid]->worldTopSensor = (SDL_Point){
//             entities->worldPositions[eid]->x,
//             entities->worldPositions[eid]->y - (TILE_SIZE*0.5)
//         };

//         entities->sensors[eid]->worldBottomSensor = (SDL_Point){
//             entities->worldPositions[eid]->x,
//             entities->worldPositions[eid]->y + (TILE_SIZE*0.5)
//         };

//         entities->sensors[eid]->worldLeftSensor = (SDL_Point){
//             entities->worldPositions[eid]->x - (TILE_SIZE*0.5),
//             entities->worldPositions[eid]->y 
//         };

//         entities->sensors[eid]->worldRightSensor = (SDL_Point){
//             entities->worldPositions[eid]->x + (TILE_SIZE*0.5),
//             entities->worldPositions[eid]->y
//         };
//     }
    

//     entities->collisionRects[eid]->w = TILE_SIZE*0.5;
//     entities->collisionRects[eid]->h = TILE_SIZE*0.5;
//     entities->collisionRects[eid]->x = entities->worldPositions[eid]->x - entities->collisionRects[eid]->w*0.5;
//     entities->collisionRects[eid]->y = entities->worldPositions[eid]->y - entities->collisionRects[eid]->h*0.5;
// }


void trySetDirectionForInputMaskSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, InputMask);
    ECS_COMPONENT(gEcsWorld, Direction);
    ECS_COMPONENT(gEcsWorld, CurrentTile);

    InputMask *inputMasks = ecs_term(gEcsWorld, InputMask, 1);
    Direction *directions = ecs_term(gEcsWorld, Direction, 2);
    CurrentTile *currentTiles = ecs_term(gEcsWorld, CurrentTile, 3);


    TileMap *tilemap = it->ctx;

    for(int i = 0; i < it->count; i++){
        if( inputMasks[ i ] & g_INPUT_UP ) {
            SDL_Point tile_above = {currentTiles[i].x,currentTiles[i].y - 1 };
            tile_wrap( &tile_above );
            
            if( tilemap->tm_walls[ tile_above.y ][ tile_above.x ] != 'x' ) 
            {
                directions[i] = DIR_UP;
            }
            else {
                return;
            }
        }
        else if( inputMasks[ i ] & g_INPUT_DOWN  ) {
            SDL_Point tile_below = {currentTiles[i].x,currentTiles[i].y + 1 };
            tile_wrap( &tile_below );

            if( points_equal( tile_below, tilemap->one_way_tile) ) {
                return;
            }
            if(tilemap->tm_walls[ tile_below.y ][ tile_below.x ] != 'x' ) 
            {
                directions[i] = DIR_DOWN;
            }
            else {
                return;
            }
        }
        else if( inputMasks[ i ] & g_INPUT_LEFT  ) {
            SDL_Point tile_to_left = {currentTiles[i].x - 1,currentTiles[i].y  };
            tile_wrap( &tile_to_left );

            if( points_equal( tile_to_left, tilemap->one_way_tile) ) {
                return;
            }
            if(tilemap->tm_walls[ tile_to_left.y ][ tile_to_left.x ] != 'x'  ) 
            {
                directions[i] = DIR_LEFT;
            }
            else {
                return;
            }
        }
        else if( inputMasks[ i ] & g_INPUT_RIGHT ) {
            SDL_Point tile_to_right = {currentTiles[i].x + 1,currentTiles[i].y };
            tile_wrap( &tile_to_right );

            if( points_equal( tile_to_right, tilemap->one_way_tile) ) {
                return;
            }
            if(tilemap->tm_walls[ tile_to_right.y ][ tile_to_right.x ] != 'x'  ) 
            {
                directions[i] = DIR_RIGHT;
            }
            else {
                return;
            }
        }
    }
}

static void trySetDirection( Entities *entities, EntityId entityId, TileMap *tilemap ) {
    uint8_t **inputMasks = entities->inputMasks;

    if( *inputMasks[ entityId ] & g_INPUT_UP ) {
        SDL_Point tile_above = {entities->currentTiles[entityId]->x,entities->currentTiles[entityId]->y - 1 };

        tile_wrap( &tile_above );
        

        if( tilemap->tm_walls[ tile_above.y ][ tile_above.x ] != 'x' ) 
        {
            *entities->directions[entityId] = DIR_UP;
        }
        
        else {
            return;
        }
    }

    if( *inputMasks[ entityId ] & g_INPUT_DOWN  ) {
        SDL_Point tile_below = {entities->currentTiles[entityId]->x,entities->currentTiles[entityId]->y + 1 };

        tile_wrap( &tile_below );


        if( points_equal( tile_below, tilemap->one_way_tile) ) {
            return;
        }

        if(tilemap->tm_walls[ tile_below.y ][ tile_below.x ] != 'x' ) 
        {
            *entities->directions[entityId] = DIR_DOWN;
        }
        else {
            return;
        }
    }

    if( *inputMasks[ entityId ] & g_INPUT_LEFT  ) {
        SDL_Point tile_to_left = {entities->currentTiles[entityId]->x - 1,entities->currentTiles[entityId]->y  };

        tile_wrap( &tile_to_left );

        if( points_equal( tile_to_left, tilemap->one_way_tile) ) {
            return;
        }

        if(tilemap->tm_walls[ tile_to_left.y ][ tile_to_left.x ] != 'x'  ) 
        {
            *entities->directions[entityId] = DIR_LEFT;
        }
        else {
            return;
        }
    }

    if( *inputMasks[ entityId ] & g_INPUT_RIGHT ) {
        SDL_Point tile_to_right = {entities->currentTiles[entityId]->x + 1,entities->currentTiles[entityId]->y };

        tile_wrap( &tile_to_right );

        if( points_equal( tile_to_right, tilemap->one_way_tile) ) {
            return;
        }

        if(tilemap->tm_walls[ tile_to_right.y ][ tile_to_right.x ] != 'x'  ) 
        {
            *entities->directions[entityId] = DIR_RIGHT;
        }
        else {
            return;
        }
    }
}

void handlePlayerCollisionNEW(ecs_entity_t eid){
    ECS_COMPONENT(gEcsWorld, IsActive);
    ECS_COMPONENT(gEcsWorld, NextTile);
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    ECS_COMPONENT(gEcsWorld, CollisionRect);
    ECS_COMPONENT(gEcsWorld, Velocity);
    ECS_COMPONENT(gEcsWorld, Position);
    ECS_COMPONENT(gEcsWorld, Sensor);


    IsActive *playerIsActive;
    NextTile *nextTile;
    CurrentTile *playerCurrentTile;
    CurrentTile *currentTile;
    Velocity *velocity;
    Position *position;
    CollisionRect *collisionRect;
    Sensor *sensor;

    ecs_entity_t pid;
    for(int pidx = 0; pidx < gNumPlayers; pidx++){
        pid = gPlayerIds[pidx];
        if(pid == eid) continue;
        playerIsActive = ecs_get(gEcsWorld, pid, IsActive);
        if(!*playerIsActive) continue;
        nextTile = ecs_get(gEcsWorld, eid, NextTile);
        playerCurrentTile = ecs_get(gEcsWorld, pid, CurrentTile);
        if(points_equal(*nextTile, *playerCurrentTile)){
            velocity = ecs_get(gEcsWorld, eid, Velocity);
            Velocity reversedVelocity = {
                -velocity->x,
                -velocity->y
            };
            // move player
            position = ecs_get(gEcsWorld, eid, Position);
            collisionRect = ecs_get(gEcsWorld, eid, CollisionRect);
            sensor = ecs_get(gEcsWorld, eid, Sensor);
            currentTile = ecs_get(gEcsWorld, eid, CurrentTile);
            moveActor(position, *velocity, currentTile, collisionRect, sensor);
        }
    }
}

// static void handlePlayerCollision(Entities *entities, EntityId eid ) {
//     EntityId pid = 0;
//     for( int pidx = 0; pidx < gNumPlayers; pidx++ ) {
//         pid = gPlayerIds[pidx];
//         if( pid == eid ) {
//             continue;
//         }
//         // dont bump into inactive players
//         if(entities->isActive[pid] != NULL && *entities->isActive[pid] == SDL_FALSE) {
//             continue;
//         }
//         if( points_equal( *entities->nextTiles[pid], *entities->currentTiles[pid] ) ){
//             Velocity reversed_velocity = { -entities->velocities[pid]->x, -entities->velocities[pid]->y };
//             moveActor(entities, pid, reversed_velocity );
//         }
//     }
// }

void alignPositionToValueIfPassed(float vel, float *src, float target, Position *p, CurrentTile *ct, CollisionRect *cr, Sensor *sensor ){
    if( vel > 0 ) {
        if( *src > target ) {
            *src = target;
            alignTileDataToPosition(*p, ct, cr);
            alignSensorDataToPosition(*p, sensor);
        }
    }
    else if( vel < 0 ) {
        if( *src < target ) {
            *src = target;
            alignTileDataToPosition(*p, ct, cr);
            alignSensorDataToPosition(*p, sensor);
        }
    }
}


// static void alignPositionToValueIfPassed(Entities *entities, EntityId eid, float vel, float *src, float target ){
//     if( vel > 0 ) {
//         if( *src > target ) {
//             *src = target;
//             alignWorldDataBasedOnWorldPosition(entities, eid);
//         }
//     }
//     else if( vel < 0 ) {
//         if( *src < target ) {
//             *src = target;
//             alignWorldDataBasedOnWorldPosition(entities, eid);
//         }
//     }
// }

void alignPositionToCurrentTileIfSensorColliding(Position *position, CurrentTile *ct, NextTile *nt, CollisionRect *cr, Velocity *v, Sensor *s, SDL_Point *individualSensor, TileMap *tilemap, ecs_entity_t eid){
    SDL_Point nextTileWorldPosition = tile_grid_point_to_world_point( *nt);
    SDL_Rect nextTileRect = { nextTileWorldPosition.x, nextTileWorldPosition.y, TILE_SIZE, TILE_SIZE  };

    if( individualSensor->y < nextTileRect.y + TILE_SIZE 
    && individualSensor->y > nextTileRect.y
    && individualSensor->x > nextTileRect.x 
    && individualSensor->x < nextTileRect.x + TILE_SIZE ) {
        // target tile is a wall
        if ( tilemap->tm_walls[ nt->y ][ nt->x ] == 'x' ) {
            Velocity reversed_velocity = { -v->x, -v->y };
            moveActor(position, *v, ct, cr, s);
        }

        // other pac-men?
        handlePlayerCollisionNEW(eid);
        alignTileDataToPosition(*position,ct, cr);
        alignSensorDataToPosition(*position, s);
    }
}

// static void alignPositionToCurrentTileIfSensorColliding(Entities *entities, EntityId eid, SDL_Point sensor, TileMap *tilemap){
//     SDL_Point nextTileWorldPosition = tile_grid_point_to_world_point( *entities->nextTiles[eid]);
//     SDL_Rect nextTileRect = { nextTileWorldPosition.x, nextTileWorldPosition.y, TILE_SIZE, TILE_SIZE  };

//     if( sensor.y < nextTileRect.y + TILE_SIZE 
//     && sensor.y > nextTileRect.y
//     && sensor.x > nextTileRect.x 
//     && sensor.x < nextTileRect.x + TILE_SIZE ) {
//         // target tile is a wall
//         if ( tilemap->tm_walls[ entities->nextTiles[eid]->y ][ entities->nextTiles[eid]->x ] == 'x' ) {
//             Velocity reversed_velocity = { -entities->velocities[eid]->x, -entities->velocities[eid]->y };
//             moveActor(entities,eid, reversed_velocity );
//         }

//         // other pac-men?
//         handlePlayerCollision(entities, eid );
//         alignWorldDataBasedOnWorldPosition(entities, eid);
//     }
// }


void movementSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, Position);

}


void inputMaskMovementSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, Position);
    ECS_COMPONENT(gEcsWorld, Direction);
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    ECS_COMPONENT(gEcsWorld, NextTile);
    ECS_COMPONENT(gEcsWorld, Velocity);
    ECS_COMPONENT(gEcsWorld, InputMask);
    ECS_COMPONENT(gEcsWorld, StopTimer);
    ECS_COMPONENT(gEcsWorld, DeathTimer);
    ECS_COMPONENT(gEcsWorld, BaseSpeed);
    ECS_COMPONENT(gEcsWorld, SpeedMultiplier);
    ECS_COMPONENT(gEcsWorld, Sensor);
    ECS_COMPONENT(gEcsWorld, CollisionRect); // really just used so that it can be aligned 


    Position *positions = ecs_term(it, Position, 1);
    Direction *directions = ecs_term(it, Direction, 2);
    CurrentTile *currentTiles = ecs_term(it, CurrentTile, 3);
    NextTile *nextTiles = ecs_term(it, NextTile, 4);
    Velocity *velocities = ecs_term(it, Velocity, 5);
    InputMask *inputMasks = ecs_term(it, InputMask, 6);
    StopTimer *stopTimers = ecs_term(it, StopTimer, 7);
    DeathTimer *deathTimers = ecs_term(it, DeathTimer, 8);
    BaseSpeed *baseSpeeds = ecs_term(it, BaseSpeed, 9);
    SpeedMultiplier *speedMultipliers = ecs_term(it, SpeedMultiplier, 10);
    Sensor *sensors = ecs_term(it, Sensor, 11);
    CollisionRect *collisionRects = ecs_term(it, CollisionRect, 12);


    TileMap *tilemap = it->ctx;


    for(int i = 0; i < it->count; i++){
        if(stopTimers[i] > 0.0f) continue;
        if(deathTimers[i] > 0.0f) continue;

        if( directions[i] == DIR_UP ) {
            nextTiles[i].x = currentTiles[i].x;
            nextTiles[i].y = currentTiles[i].y - 1;
            if( nextTiles[i].y < 0 ){
                nextTiles[i].y = TILE_ROWS-1;
            } 

            // set velocity
            if ( positions[i].x == tile_grid_point_to_world_point( currentTiles[i] ).x  + ( TILE_SIZE / 2 ) ) {
                velocities[i].x = 0;
                velocities[i].y = -1;
            } 
            else if( positions[i].x < tile_grid_point_to_world_point( currentTiles[i] ).x  + ( TILE_SIZE / 2 ) ) {
                velocities[i].x = 0.7071068;
                velocities[i].y = -0.7071068;
            }
            else if( positions[i].x >tile_grid_point_to_world_point( currentTiles[i]).x  + ( TILE_SIZE / 2 )){
            velocities[i].x = -0.7071068;
                velocities[i].y = -0.7071068;
                
            }

            velocities[i].x *= baseSpeeds[i] * speedMultipliers[i] * it->delta_time;
            velocities[i].y *= baseSpeeds[i] * speedMultipliers[i]  * it->delta_time;
            // move
            moveActor(&positions[i], velocities[i], &currentTiles[i], &collisionRects[i], &sensors[i]);
            
            // make sure pacman doesn't pass centerpt
            // alignTileDataToPosition(positions[i], &currentTiles[i], &collisionRects[i]);
            // alignSensorDataToPosition(positions[i], &sensors[i]);
            alignPositionToValueIfPassed(
                velocities[i].x, 
                &positions[i].x, 
                tile_grid_point_to_world_point(currentTiles[i]).x + (TILE_SIZE * 0.5 ), 
                &positions[i],
                &currentTiles[i],
                &collisionRects[i],
                &sensors[i] 
            );

            // collision
            // top sensor is inside target tile
            alignPositionToCurrentTileIfSensorColliding(
                &positions[i],
                &currentTiles[i],
                &nextTiles[i],
                &collisionRects[i],
                &velocities[i],
                &sensors[i],
                &sensors[i].worldTopSensor,
                tilemap,
                it->entities[i]
            );
        }

        if( directions[i] == DIR_DOWN ) {
            nextTiles[i].x = currentTiles[i].x;
            nextTiles[i].y = currentTiles[i].y + 1;
            if( nextTiles[i].y >= TILE_ROWS-1 ){
                nextTiles[i].y = 0;
            } 

            // set velocity
            if ( positions[i].x == tile_grid_point_to_world_point( currentTiles[i]).x  + ( TILE_SIZE / 2 ) ) {
                velocities[i].x = 0;
                velocities[i].y = 1;
            } 
            else if( positions[i].x < tile_grid_point_to_world_point( currentTiles[i] ).x  + ( TILE_SIZE / 2 ) ) {
                velocities[i].x = 0.7071068;
                velocities[i].y = 0.7071068;
            }
            else if( positions[i].x >tile_grid_point_to_world_point( currentTiles[i]).x  + ( TILE_SIZE / 2 )){
                velocities[i].x = -0.7071068;
                velocities[i].y = 0.7071068;
            }
            velocities[i].x *= baseSpeeds[i] * speedMultipliers[i] * it->delta_time;
            velocities[i].y *= baseSpeeds[i] * speedMultipliers[i] * it->delta_time;
            // move
            moveActor(&positions[i], velocities[i], &currentTiles[i], &collisionRects[i], &sensors[i]);
            
            alignPositionToValueIfPassed(
                velocities[i].x, 
                &positions[i].x, 
                tile_grid_point_to_world_point(currentTiles[i]).x + (TILE_SIZE * 0.5 ), 
                &positions[i],
                &currentTiles[i],
                &collisionRects[i],
                &sensors[i] 
            );

            // collision
            alignPositionToCurrentTileIfSensorColliding(
                &positions[i],
                &currentTiles[i],
                &nextTiles[i],
                &collisionRects[i],
                &velocities[i],
                &sensors[i],
                &sensors[i].worldBottomSensor,
                tilemap,
                it->entities[i]
            );
            // make sure pacman doesn't pass centerpt
            // alignPositionToValueIfPassed(entities, i, velocities[i].x, &positions[i].x, tile_grid_point_to_world_point(currentTiles[i]).x + (TILE_SIZE * 0.5 ) );


            // // collision
            // alignPositionToCurrentTileIfSensorColliding(entities, i, sensors[i].worldBottomSensor, tilemap );

            SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( nextTiles[i]);
            SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

            if( sensors[i].worldBottomSensor.y > target_tile_rect.y 
            && sensors[i].worldBottomSensor.x > target_tile_rect.x 
            && sensors[i].worldBottomSensor.x < target_tile_rect.x + TILE_SIZE ) {
                // target tile is a one_way_tile
                if( points_equal(tilemap->one_way_tile, nextTiles[i])) {
                    Velocity reversed_velocity = { -velocities[i].x, -velocities[i].y };
                    moveActor(&positions[i], reversed_velocity, &currentTiles[i], &collisionRects[i], &sensors[i]);
                }
            }
        }

        if( directions[i] == DIR_LEFT ) {
            nextTiles[i].x = currentTiles[i].x - 1;
            nextTiles[i].y = currentTiles[i].y;
            if( nextTiles[i].x < 0 ){
                nextTiles[i].x = 0;
            } 

            // set velocity
            if ( positions[i].y == tile_grid_point_to_world_point( currentTiles[i] ).y  + ( TILE_SIZE / 2 ) ) {
                velocities[i].x = -1;
                velocities[i].y = 0;
            } 
            else if( positions[i].y < tile_grid_point_to_world_point( currentTiles[i] ).y  + ( TILE_SIZE / 2 ) ) {
                velocities[i].x = -0.7071068;
                velocities[i].y = 0.7071068;
            }
            else if( positions[i].y >tile_grid_point_to_world_point( currentTiles[i]).y  + ( TILE_SIZE / 2 )){
                velocities[i].x = -0.7071068;
                velocities[i].y = -0.7071068;
            }

            velocities[i].x *= baseSpeeds[i] * speedMultipliers[i] * it->delta_time;
            velocities[i].y *= baseSpeeds[i] * speedMultipliers[i] * it->delta_time;
            // move
            moveActor(&positions[i], velocities[i], &currentTiles[i], &collisionRects[i], &sensors[i]);

            alignPositionToValueIfPassed(
                velocities[i].y, 
                &positions[i].y, 
                tile_grid_point_to_world_point(currentTiles[i]).y + (TILE_SIZE * 0.5 ), 
                &positions[i],
                &currentTiles[i],
                &collisionRects[i],
                &sensors[i] 
            );

            // collision
            alignPositionToCurrentTileIfSensorColliding(
                &positions[i],
                &currentTiles[i],
                &nextTiles[i],
                &collisionRects[i],
                &velocities[i],
                &sensors[i],
                &sensors[i].worldLeftSensor,
                tilemap,
                it->entities[i]
            );

            // make sure pacman doesn't pass centerpt
            // alignPositionToValueIfPassed(entities, i, velocities[i].y, &positions[i].y, tile_grid_point_to_world_point(currentTiles[i]).y + (TILE_SIZE * 0.5 ) );

            // // collision

            // alignPositionToCurrentTileIfSensorColliding(entities, i, sensors[i].worldLeftSensor, tilemap );

        }

        if( directions[i] == DIR_RIGHT ) {
            nextTiles[i].x = currentTiles[i].x + 1;
            nextTiles[i].y = currentTiles[i].y;
            if( nextTiles[i].x >= TILE_COLS ){
                nextTiles[i].x = 0;
            } 
            

            // set velocity
            if ( positions[i].y == tile_grid_point_to_world_point( currentTiles[i]).y  + ( TILE_SIZE / 2 ) ) {
                velocities[i].x = 1;
                velocities[i].y = 0;
            } 
            else if( positions[i].y < tile_grid_point_to_world_point( currentTiles[i] ).y  + ( TILE_SIZE / 2 ) ) {
                velocities[i].x = 0.7071068;
                velocities[i].y = 0.7071068;
            }
            else if( positions[i].y >tile_grid_point_to_world_point( currentTiles[i] ).y  + ( TILE_SIZE / 2 )){
                velocities[i].x = 0.7071068;
                velocities[i].y = -0.7071068;
            }

            velocities[i].x *= baseSpeeds[i] * speedMultipliers[i] * it->delta_time;
            velocities[i].y *= baseSpeeds[i] * speedMultipliers[i] * it->delta_time;
            // move
            moveActor(&positions[i], velocities[i], &currentTiles[i], &collisionRects[i], &sensors[i]);

            alignPositionToValueIfPassed(
                velocities[i].x, 
                &positions[i].x, 
                tile_grid_point_to_world_point(currentTiles[i]).x + (TILE_SIZE * 0.5 ), 
                &positions[i],
                &currentTiles[i],
                &collisionRects[i],
                &sensors[i] 
            );

            // collision
            alignPositionToCurrentTileIfSensorColliding(
                &positions[i],
                &currentTiles[i],
                &nextTiles[i],
                &collisionRects[i],
                &velocities[i],
                &sensors[i],
                &sensors[i].worldRightSensor,
                tilemap,
                it->entities[i]
            );

            // make sure pacman doesn't pass centerpt
            // alignPositionToValueIfPassed(entities, i, velocities[i].y, &positions[i].y, tile_grid_point_to_world_point(currentTiles[i]).y + (TILE_SIZE * 0.5 ) );

            // // collision
            // alignPositionToCurrentTileIfSensorColliding(entities, i, sensors[i].worldRightSensor, tilemap );

        }

    }
}


void setAnimationRowBasedOnVelocitySystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, Velocity);
    ECS_COMPONENT(gEcsWorld, AnimatedSprite);

    Velocity *velocities = ecs_term(it, Velocity, 1);
    AnimatedSprite *animatedSprites = ecs_term(it, AnimatedSprite, 2);

    for(int i = 0; i < it->count; i++){
        if( velocities[i].x > 0 && velocities[i].y == 0 ) { // right
            animatedSprites[i].current_anim_row = 0;
        }
        if( velocities[i].x < 0 && velocities[i].y == 0 ) { // left
            animatedSprites[i].current_anim_row = 1;
        }
        if( velocities[i].x == 0 && velocities[i].y > 0 ) { // down
            animatedSprites[i].current_anim_row = 2;
        }
        if( velocities[i].x == 0 && velocities[i].y < 0 ) { // up
            animatedSprites[i].current_anim_row = 3;
        }
        if( velocities[i].x > 0 && velocities[i].y < 0 ) { //  up-right
            animatedSprites[i].current_anim_row = 4;
        }
        if( velocities[i].x < 0 && velocities[i].y < 0 ) { // up-left
            animatedSprites[i].current_anim_row = 5;
        }
        if( velocities[i].x > 0 && velocities[i].y > 0 ) { // down-right
            animatedSprites[i].current_anim_row = 6;
        }
        if( velocities[i].x < 0 && velocities[i].y > 0 ) { // down-left
            animatedSprites[i].current_anim_row = 7;
        }
    }
}

// void inputMovementSystem( Entities *entities, TileMap *tilemap, float deltaTime ) {
//     uint8_t **inputMasks  = entities->inputMasks;

//     for( int i = 0; i < g_NumEntities; ++i  ) {
//         // check if should not process
//         if( inputMasks[ i ] == NULL || entities->worldPositions[i] == NULL ) {
//             continue;
//         }
//         if( entities->stopTimers[ i ] != NULL && *entities->stopTimers[i] > 0.0f ) {
//             continue;
//         }
//         if( entities->deathTimers[ i ] != NULL && *entities->deathTimers[i] > 0.0f ) {
//             continue;
//         }
//         // should process
//         // don't allow changing direciton if pacman is more than half of the tile
//         trySetDirection( entities, i, tilemap );

//         // try moving

//         if( *entities->directions[i] == DIR_UP ) {
//             entities->nextTiles[i]->x = entities->currentTiles[i]->x;
//             entities->nextTiles[i]->y = entities->currentTiles[i]->y - 1;
//             if( entities->nextTiles[i]->y < 0 ){
//                 entities->nextTiles[i]->y = TILE_ROWS-1;
//             } 

//             // set velocity
//             if ( entities->worldPositions[i]->x == tile_grid_point_to_world_point( *entities->currentTiles[i] ).x  + ( TILE_SIZE / 2 ) ) {
//                 entities->velocities[i]->x = 0;
//                 entities->velocities[i]->y = -1;
//             } 
//             else if( entities->worldPositions[i]->x < tile_grid_point_to_world_point( *entities->currentTiles[i] ).x  + ( TILE_SIZE / 2 ) ) {
//                 entities->velocities[i]->x = 0.7071068;
//                 entities->velocities[i]->y = -0.7071068;
//             }
//             else if( entities->worldPositions[i]->x >tile_grid_point_to_world_point( *entities->currentTiles[i]).x  + ( TILE_SIZE / 2 )){
//             entities->velocities[i]->x = -0.7071068;
//                 entities->velocities[i]->y = -0.7071068;
                
//             }

//             entities->velocities[i]->x *= *entities->baseSpeeds[i] * *entities->speedMultipliers[i] * deltaTime;
//             entities->velocities[i]->y *= *entities->baseSpeeds[i] * *entities->speedMultipliers[i]  * deltaTime;
//             // move
//             moveActor(entities, i, *entities->velocities[i]);
            
//             // make sure pacman doesn't pass centerpt
//             alignPositionToValueIfPassed(entities, i, entities->velocities[i]->x, &entities->worldPositions[i]->x, tile_grid_point_to_world_point(*entities->currentTiles[i]).x + (TILE_SIZE * 0.5 ) );

//             // collision
//             // top sensor is inside target tile
//             alignPositionToCurrentTileIfSensorColliding(entities, i, entities->sensors[i]->worldTopSensor, tilemap );

//         }

//         if( *entities->directions[i] == DIR_DOWN ) {
//             entities->nextTiles[i]->x = entities->currentTiles[i]->x;
//             entities->nextTiles[i]->y = entities->currentTiles[i]->y + 1;
//             if( entities->nextTiles[i]->y >= TILE_ROWS-1 ){
//                 entities->nextTiles[i]->y = 0;
//             } 

//             // set velocity
//             if ( entities->worldPositions[i]->x == tile_grid_point_to_world_point( *entities->currentTiles[i]).x  + ( TILE_SIZE / 2 ) ) {
//                 entities->velocities[i]->x = 0;
//                 entities->velocities[i]->y = 1;
//             } 
//             else if( entities->worldPositions[i]->x < tile_grid_point_to_world_point( *entities->currentTiles[i] ).x  + ( TILE_SIZE / 2 ) ) {
//                 entities->velocities[i]->x = 0.7071068;
//                 entities->velocities[i]->y = 0.7071068;
//             }
//             else if( entities->worldPositions[i]->x >tile_grid_point_to_world_point( *entities->currentTiles[i]).x  + ( TILE_SIZE / 2 )){
//                 entities->velocities[i]->x = -0.7071068;
//                 entities->velocities[i]->y = 0.7071068;
//             }
//             entities->velocities[i]->x *= *entities->baseSpeeds[i] * *entities->speedMultipliers[i] * deltaTime;
//             entities->velocities[i]->y *= *entities->baseSpeeds[i] * *entities->speedMultipliers[i] * deltaTime;
//             // move
//             moveActor(entities, i, *entities->velocities[i] );

//             // make sure pacman doesn't pass centerpt
//             alignPositionToValueIfPassed(entities, i, entities->velocities[i]->x, &entities->worldPositions[i]->x, tile_grid_point_to_world_point(*entities->currentTiles[i]).x + (TILE_SIZE * 0.5 ) );


//             // collision
//             alignPositionToCurrentTileIfSensorColliding(entities, i, entities->sensors[i]->worldBottomSensor, tilemap );

//             SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( *entities->nextTiles[i]);
//             SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

//             if( entities->sensors[i]->worldBottomSensor.y > target_tile_rect.y 
//             && entities->sensors[i]->worldBottomSensor.x > target_tile_rect.x 
//             && entities->sensors[i]->worldBottomSensor.x < target_tile_rect.x + TILE_SIZE ) {
//                 // target tile is a one_way_tile
//                 if( points_equal(tilemap->one_way_tile, *entities->nextTiles[i])) {
//                     Velocity reversed_velocity = { -entities->velocities[i]->x, -entities->velocities[i]->y };
//                     moveActor(entities,i, reversed_velocity );
//                 }
//             }
//         }

//         if( *entities->directions[i] == DIR_LEFT ) {
//             entities->nextTiles[i]->x = entities->currentTiles[i]->x - 1;
//             entities->nextTiles[i]->y = entities->currentTiles[i]->y;
//             if( entities->nextTiles[i]->x < 0 ){
//                 entities->nextTiles[i]->x = 0;
//             } 

//             // set velocity
//             if ( entities->worldPositions[i]->y == tile_grid_point_to_world_point( *entities->currentTiles[i] ).y  + ( TILE_SIZE / 2 ) ) {
//                 entities->velocities[i]->x = -1;
//                 entities->velocities[i]->y = 0;
//             } 
//             else if( entities->worldPositions[i]->y < tile_grid_point_to_world_point( *entities->currentTiles[i] ).y  + ( TILE_SIZE / 2 ) ) {
//                 entities->velocities[i]->x = -0.7071068;
//                 entities->velocities[i]->y = 0.7071068;
//             }
//             else if( entities->worldPositions[i]->y >tile_grid_point_to_world_point( *entities->currentTiles[i]).y  + ( TILE_SIZE / 2 )){
//                 entities->velocities[i]->x = -0.7071068;
//                 entities->velocities[i]->y = -0.7071068;
//             }

//             entities->velocities[i]->x *= *entities->baseSpeeds[i] * *entities->speedMultipliers[i] * deltaTime;
//             entities->velocities[i]->y *= *entities->baseSpeeds[i] * *entities->speedMultipliers[i] * deltaTime;
//             // move
//             moveActor(entities, i, *entities->velocities[i] );

//             // make sure pacman doesn't pass centerpt
//             alignPositionToValueIfPassed(entities, i, entities->velocities[i]->y, &entities->worldPositions[i]->y, tile_grid_point_to_world_point(*entities->currentTiles[i]).y + (TILE_SIZE * 0.5 ) );

//             // collision

//             alignPositionToCurrentTileIfSensorColliding(entities, i, entities->sensors[i]->worldLeftSensor, tilemap );

//         }

//         if( *entities->directions[i] == DIR_RIGHT ) {
//             entities->nextTiles[i]->x = entities->currentTiles[i]->x + 1;
//             entities->nextTiles[i]->y = entities->currentTiles[i]->y;
//             if( entities->nextTiles[i]->x >= TILE_COLS ){
//                 entities->nextTiles[i]->x = 0;
//             } 
            

//             // set velocity
//             if ( entities->worldPositions[i]->y == tile_grid_point_to_world_point( *entities->currentTiles[i]).y  + ( TILE_SIZE / 2 ) ) {
//                 entities->velocities[i]->x = 1;
//                 entities->velocities[i]->y = 0;
//             } 
//             else if( entities->worldPositions[i]->y < tile_grid_point_to_world_point( *entities->currentTiles[i] ).y  + ( TILE_SIZE / 2 ) ) {
//                 entities->velocities[i]->x = 0.7071068;
//                 entities->velocities[i]->y = 0.7071068;
//             }
//             else if( entities->worldPositions[i]->y >tile_grid_point_to_world_point( *entities->currentTiles[i] ).y  + ( TILE_SIZE / 2 )){
//                 entities->velocities[i]->x = 0.7071068;
//                 entities->velocities[i]->y = -0.7071068;
//             }

//             entities->velocities[i]->x *= *entities->baseSpeeds[i] * *entities->speedMultipliers[i] * deltaTime;
//             entities->velocities[i]->y *= *entities->baseSpeeds[i] * *entities->speedMultipliers[i] * deltaTime;
//             // move
//             moveActor(entities, i, *entities->velocities[i] );

//             // make sure pacman doesn't pass centerpt
//             alignPositionToValueIfPassed(entities, i, entities->velocities[i]->y, &entities->worldPositions[i]->y, tile_grid_point_to_world_point(*entities->currentTiles[i]).y + (TILE_SIZE * 0.5 ) );

//             // collision
//             alignPositionToCurrentTileIfSensorColliding(entities, i, entities->sensors[i]->worldRightSensor, tilemap );

//         }

//         // pacman animation row
//         if( entities->velocities[i]->x > 0 && entities->velocities[i]->y == 0 ) { // right
//             entities->animatedSprites[ i ]->current_anim_row = 0;
//         }
//         if( entities->velocities[i]->x < 0 && entities->velocities[i]->y == 0 ) { // left
//             entities->animatedSprites[ i ]->current_anim_row = 1;
//         }
//         if( entities->velocities[i]->x == 0 && entities->velocities[i]->y > 0 ) { // down
//             entities->animatedSprites[ i ]->current_anim_row = 2;
//         }
//         if( entities->velocities[i]->x == 0 && entities->velocities[i]->y < 0 ) { // up
//             entities->animatedSprites[ i ]->current_anim_row = 3;
//         }
//         if( entities->velocities[i]->x > 0 && entities->velocities[i]->y < 0 ) { //  up-right
//             entities->animatedSprites[ i ]->current_anim_row = 4;
//         }
//         if( entities->velocities[i]->x < 0 && entities->velocities[i]->y < 0 ) { // up-left
//             entities->animatedSprites[ i ]->current_anim_row = 5;
//         }
//         if( entities->velocities[i]->x > 0 && entities->velocities[i]->y > 0 ) { // down-right
//             entities->animatedSprites[ i ]->current_anim_row = 6;
//         }
//         if( entities->velocities[i]->x < 0 && entities->velocities[i]->y > 0 ) { // down-left
//             entities->animatedSprites[ i ]->current_anim_row = 7;
//         }
//     }
    
// }


void moveActor(Position *position, Velocity velocity, CurrentTile *currentTile, CollisionRect *collisionRect, Sensor *sensor){
    position->x += velocity.x;
    position->y += velocity.y;

    // MOVE TO OTHER SIDE OF SCREEN IF OFF EDGE
    if( position->x > SCREEN_WIDTH ) {
        position->x = -TILE_SIZE ;
    }
    if( position->y > TILE_ROWS * TILE_SIZE  ) {
        position->y = -TILE_SIZE;
    }
    if( position->x < -TILE_SIZE ) {
        position->x = SCREEN_WIDTH;
    }
    if( position->y < -TILE_SIZE ) {        
        position->y = TILE_ROWS * TILE_SIZE;
    }

    alignTileDataToPosition(*position, currentTile, collisionRect);
    alignSensorDataToPosition(*position, sensor);
}

void moveActor2(Position *position, Velocity velocity, CurrentTile *currentTile, CollisionRect *collisionRect){
    position->x += velocity.x;
    position->y += velocity.y;

    // MOVE TO OTHER SIDE OF SCREEN IF OFF EDGE
    if( position->x > SCREEN_WIDTH ) {
        position->x = -TILE_SIZE ;
    }
    if( position->y > TILE_ROWS * TILE_SIZE  ) {
        position->y = -TILE_SIZE;
    }
    if( position->x < -TILE_SIZE ) {
        position->x = SCREEN_WIDTH;
    }
    if( position->y < -TILE_SIZE ) {        
        position->y = TILE_ROWS * TILE_SIZE;
    }

    alignTileDataToPosition(*position, currentTile, collisionRect);
}

// static void moveActor( Entities *entities, EntityId eid, Velocity velocity ) {
//     // skip if null actor
//     if( entities->worldPositions[eid] == NULL ) {
//         return;
//     }
//     // process actor
//     entities->worldPositions[eid]->x += velocity.x;
//     entities->worldPositions[eid]->y += velocity.y;

//     // MOVE TO OTHER SIDE OF SCREEN IF OFF EDGE
//     if( entities->worldPositions[eid]->x > SCREEN_WIDTH ) {
//         entities->worldPositions[eid]->x = -TILE_SIZE ;
//     }
//     if( entities->worldPositions[eid]->y > TILE_ROWS * TILE_SIZE  ) {
//         entities->worldPositions[eid]->y = -TILE_SIZE;
//     }
//     if( entities->worldPositions[eid]->x < -TILE_SIZE ) {
//         entities->worldPositions[eid]->x = SCREEN_WIDTH;
//     }
//     if( entities->worldPositions[eid]->y < -TILE_SIZE ) {        
//         entities->worldPositions[eid]->y = TILE_ROWS * TILE_SIZE;
//     }

//     alignWorldDataBasedOnWorldPosition( entities, eid );

    
// }

float g_PAC_DASH_SPEED_MULTR = 2.5f;
float g_PAC_DASH_TIME_MAX = 1.0f;

void dashTimersSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, InputMask);
    ECS_COMPONENT(gEcsWorld, ChargeTimer);
    ECS_COMPONENT(gEcsWorld, SlowTimer);
    ECS_COMPONENT(gEcsWorld, DashTimer);
    ECS_COMPONENT(gEcsWorld, CooldownStock);
    ECS_COMPONENT(gEcsWorld, AnimatedSprite);
    ECS_COMPONENT(gEcsWorld, SpeedMultiplier);

    InputMask *inputMasks = ecs_term(it, InputMask, 1);
    ChargeTimer *chargeTimers = ecs_term(it, ChargeTimer, 2);
    SlowTimer *slowTimers = ecs_term(it, SlowTimer, 3);
    DashTimer *dashTimers = ecs_term(it, DashTimer, 4);
    CooldownStock *cooldownStocks = ecs_term(it, CooldownStock, 5);
    AnimatedSprite *animatedSprites = ecs_term(it, AnimatedSprite, 6);
    SpeedMultiplier *speedMultipliers = ecs_term(it, SpeedMultiplier, 7);

    for(int i = 0; i < it->count; i++){
        // charge or dash
        if( inputMasks[i] & g_INPUT_ACTION ) {
            chargeTimers[i] += it->delta_time;
        }
        else if( inputMasks[i] ^ g_INPUT_ACTION && chargeTimers[i] > 0.0f ) {

            if( cooldownStocks[i].currentNumStock > 0 ) {
                dashTimers[i] = chargeTimers[i] > g_PAC_DASH_TIME_MAX ? g_PAC_DASH_TIME_MAX : chargeTimers[i];
                chargeTimers[i] = 0.0f;

                // remove a stock
                cooldownStocks[i].currentNumStock--;
                cooldownStocks[i].cooldownTimer = cooldownStocks[i].cooldownDuration;
            }
            else {
                chargeTimers[i] = 0.0f;
            }
        }
        
        speedMultipliers[i] = 1.0f;
        if( slowTimers[i] > 0 ) {
            speedMultipliers[i] -= 0.4f;
            slowTimers[i] -= it->delta_time;
        }
        if( dashTimers[i] > 0 ) {
            speedMultipliers[i] = g_PAC_DASH_SPEED_MULTR;
            dashTimers[i] -= it->delta_time;
            SDL_SetTextureAlphaMod( g_texture_atlases[ animatedSprites[i].texture_atlas_id ].texture, 150 );
        }
        else {
            SDL_SetTextureAlphaMod( g_texture_atlases[ animatedSprites[i].texture_atlas_id  ].texture, 255 );
        }
        if( chargeTimers[i] > 0 ) {
            speedMultipliers[i] -= 0.25f;
        }
    }
}


// void dashTimersSystem( Entities *entities, float deltaTime ) {
//     for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++) {
//         if( entities->dashTimers[ eid ] == NULL || entities->chargeTimers[ eid ] == NULL || entities->slowTimers[ eid ] == NULL || entities->inputMasks[ eid ] == NULL ) { //skip if doesn't have dash timers
//             continue;
//         }

//         // charge or dash
        
//         if( *entities->inputMasks[ eid ] & g_INPUT_ACTION ) {
//             *entities->chargeTimers[ eid ] += deltaTime;
//         }
//         else if( *entities->inputMasks[ eid ] ^ g_INPUT_ACTION && *entities->chargeTimers[ eid ] > 0.0f ) {

//             if( entities->dashCooldownStocks[ eid ]->currentNumStock > 0 ) {
//                 *entities->dashTimers[ eid ] = *entities->chargeTimers[ eid ] > g_PAC_DASH_TIME_MAX ? g_PAC_DASH_TIME_MAX : *entities->chargeTimers[ eid ];
//                 *entities->chargeTimers[ eid ] = 0.0f;

//                 // remove a stock
//                 entities->dashCooldownStocks[ eid ]->currentNumStock--;
//                 entities->dashCooldownStocks[ eid ]->cooldownTimer = entities->dashCooldownStocks[ eid ]->cooldownDuration;
//             }
//             else {
//                 *entities->chargeTimers[ eid ] = 0.0f;
//             }
            

                
//         }
        
//         *entities->speedMultipliers[eid] = 1.0f;
//         if( *entities->slowTimers[ eid ] > 0 ) {
//             *entities->speedMultipliers[eid] -= 0.4f;
//             *entities->slowTimers[ eid ] -= deltaTime;
//         }
//         if( *entities->dashTimers[eid] > 0 ) {
//             *entities->speedMultipliers[eid] = g_PAC_DASH_SPEED_MULTR;
//             *entities->dashTimers[ eid ] -= deltaTime;
//             SDL_SetTextureAlphaMod( g_texture_atlases[ entities->animatedSprites[ eid ]->texture_atlas_id ].texture, 150 );
//         }
//         else {
//             SDL_SetTextureAlphaMod( g_texture_atlases[ entities->animatedSprites[ eid ]->texture_atlas_id  ].texture, 255 );
//         }
//         if( *entities->chargeTimers[ eid ] > 0 ) {
//             *entities->speedMultipliers[eid] -= 0.25f;
//         }
//     }
        
// }


void nonPlayerInputEntityMovementSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, Position);
    ECS_COMPONENT(gEcsWorld, Direction);
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    ECS_COMPONENT(gEcsWorld, Velocity);
    ECS_COMPONENT(gEcsWorld, CollisionRect);
    ECS_COMPONENT(gEcsWorld, Velocity);
    ECS_COMPONENT(gEcsWorld, BaseSpeed);
    ECS_COMPONENT(gEcsWorld, SpeedMultiplier);


    Position *positions = ecs_term(it, Position, 1);
    Direction *directions = ecs_term(it, Direction, 2);
    CurrentTile *currentTiles = ecs_term(it, CurrentTile, 3);
    Velocity *velocities = ecs_term(it, Velocity, 4);
    CollisionRect *collisionRects = ecs_term(it, CollisionRect, 5);
    BaseSpeed *baseSpeeds = ecs_term(it, BaseSpeed, 6);
    SpeedMultiplier *speedMultipliers = ecs_term(it, SpeedMultiplier, 7);


    for(int i = 0; i < it->count; i++){
        Velocity velocity = { 0, 0 };
        if( directions[i] == DIR_UP ) {
            // set velocity
            if( positions[i].x >= tile_grid_point_to_world_point( currentTiles[i] ).x  + ( TILE_SIZE / 2 ) - 2 
            && ( positions[i].x <= tile_grid_point_to_world_point( currentTiles[i] ).x  + ( TILE_SIZE / 2 ) + 2) ) {
                positions[i].x = tile_grid_point_to_world_point( currentTiles[i] ).x  + ( TILE_SIZE / 2 );
            }
            if ( positions[i].x == tile_grid_point_to_world_point( currentTiles[i] ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = -1;
            } 
            else if( positions[i].x < tile_grid_point_to_world_point( currentTiles[i] ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 1;
                velocity.y = 0;
            }
            else if( positions[i].x > tile_grid_point_to_world_point( currentTiles[i] ).x  + ( TILE_SIZE / 2 )){
                velocity.x = -1;
                velocity.y = 0;
            }
        }
        else if( directions[i] == DIR_DOWN ){
            // set velocity
            if ( positions[i].x == tile_grid_point_to_world_point( currentTiles[i] ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = 1;
                
            } 
            else if( positions[i].x < tile_grid_point_to_world_point( currentTiles[i] ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 1;
                velocity.y = 0;

            }
            else if( positions[i].x >tile_grid_point_to_world_point( currentTiles[i] ).x  + ( TILE_SIZE / 2 )){
                velocity.x = -1;
                velocity.y = 0;

            }
        } 
        else if( directions[i] == DIR_LEFT ) {
            // set velocity
            if ( positions[i].y == tile_grid_point_to_world_point( currentTiles[i] ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = -1;
                velocity.y = 0;

            } 
            else if( positions[i].y < tile_grid_point_to_world_point( currentTiles[i] ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = 1;
                
            }
            else if( positions[i].y >tile_grid_point_to_world_point( currentTiles[i] ).y  + ( TILE_SIZE / 2 )){
                velocity.x = 0;
                velocity.y = -1;

            }
        }
        else if(directions[i] == DIR_RIGHT ) {
            // set velocity
            if ( positions[i].y == tile_grid_point_to_world_point( currentTiles[i] ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 1;
                velocity.y = 0;

            } 
            else if( positions[i].y < tile_grid_point_to_world_point( currentTiles[i] ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = 1;

            }
            else if( positions[i].y >tile_grid_point_to_world_point( currentTiles[i] ).y  + ( TILE_SIZE / 2 )){
                velocity.x = 0;
                velocity.y = -1;

            }
        }
        velocity.x *= baseSpeeds[i] * speedMultipliers[i] * it->delta_time;
        velocity.y *= baseSpeeds[i] * speedMultipliers[i] * it->delta_time;

        moveActor2(&positions[i], velocity, &currentTiles[i], &collisionRects[i]);

        // account for overshooting
        // note velocity will never be in x and y direction.
        if( velocity.x > 0 && !( directions[i] == DIR_RIGHT )) { // right
            if( positions[i].x > tile_grid_point_to_world_point( currentTiles[i] ).x + ( TILE_SIZE / 2 ) ) {
                positions[i].x = ( tile_grid_point_to_world_point( currentTiles[i] ).x + ( TILE_SIZE / 2 ) ) ;
                alignTileDataToPosition(positions[i], &currentTiles[i], &collisionRects[i]);

            }
        }
        else if( velocity.x < 0 && !( directions[i] == DIR_LEFT )) { // left
            if( positions[i].x < tile_grid_point_to_world_point( currentTiles[i] ).x + ( TILE_SIZE / 2 ) ) {
                positions[i].x = ( tile_grid_point_to_world_point( currentTiles[i] ).x + ( TILE_SIZE / 2 ) ) ;
                alignTileDataToPosition(positions[i], &currentTiles[i], &collisionRects[i]);

            }
        }
        else if( velocity.y > 0 && !( directions[i] == DIR_DOWN )) { // down
            if( positions[i].y > tile_grid_point_to_world_point( currentTiles[i] ).y + ( TILE_SIZE / 2 ) ) {
                positions[i].y = ( tile_grid_point_to_world_point( currentTiles[i] ).y + ( TILE_SIZE / 2 ) ) ;
                alignTileDataToPosition(positions[i], &currentTiles[i], &collisionRects[i]);

            }
        }
        else if( velocity.y < 0 && !( directions[i] == DIR_UP )) { // up
            if( positions[i].y < tile_grid_point_to_world_point( currentTiles[i] ).y + ( TILE_SIZE / 2 ) ) {
                positions[i].y = ( tile_grid_point_to_world_point( currentTiles[i] ).y + ( TILE_SIZE / 2 ) ) ;
                alignTileDataToPosition(positions[i], &currentTiles[i], &collisionRects[i]);

            }
        }
    }
    

}


// void nonPlayerInputEntityMovementSystem( Entities *entities, EntityId eid, TileMap *tm, float delta_time ) {
//         Velocity velocity = { 0, 0 };
//         if( *entities->directions[eid] == DIR_UP ) {
//             // set velocity
//             if( entities->worldPositions[eid]->x >= tile_grid_point_to_world_point( *entities->currentTiles[eid] ).x  + ( TILE_SIZE / 2 ) - 2 
//             && ( entities->worldPositions[eid]->x <= tile_grid_point_to_world_point( *entities->currentTiles[eid] ).x  + ( TILE_SIZE / 2 ) + 2) ) {
//                 entities->worldPositions[eid]->x = tile_grid_point_to_world_point( *entities->currentTiles[eid] ).x  + ( TILE_SIZE / 2 );
//             }
//             if ( entities->worldPositions[eid]->x == tile_grid_point_to_world_point( *entities->currentTiles[eid] ).x  + ( TILE_SIZE / 2 ) ) {
//                 velocity.x = 0;
//                 velocity.y = -1;
//             } 
//             else if( entities->worldPositions[eid]->x < tile_grid_point_to_world_point( *entities->currentTiles[eid] ).x  + ( TILE_SIZE / 2 ) ) {
//                 velocity.x = 1;
//                 velocity.y = 0;
//             }
//             else if( entities->worldPositions[eid]->x > tile_grid_point_to_world_point( *entities->currentTiles[eid] ).x  + ( TILE_SIZE / 2 )){
//                 velocity.x = -1;
//                 velocity.y = 0;
//             }
//         }
//         else if( *entities->directions[eid] == DIR_DOWN ){
//             // set velocity
//             if ( entities->worldPositions[eid]->x == tile_grid_point_to_world_point( *entities->currentTiles[eid] ).x  + ( TILE_SIZE / 2 ) ) {
//                 velocity.x = 0;
//                 velocity.y = 1;
                
//             } 
//             else if( entities->worldPositions[eid]->x < tile_grid_point_to_world_point( *entities->currentTiles[eid] ).x  + ( TILE_SIZE / 2 ) ) {
//                 velocity.x = 1;
//                 velocity.y = 0;

//             }
//             else if( entities->worldPositions[eid]->x >tile_grid_point_to_world_point( *entities->currentTiles[eid] ).x  + ( TILE_SIZE / 2 )){
//                 velocity.x = -1;
//                 velocity.y = 0;

//             }
//         } 
//         else if( *entities->directions[eid] == DIR_LEFT ) {
//             // set velocity
//             if ( entities->worldPositions[eid]->y == tile_grid_point_to_world_point( *entities->currentTiles[eid] ).y  + ( TILE_SIZE / 2 ) ) {
//                 velocity.x = -1;
//                 velocity.y = 0;

//             } 
//             else if( entities->worldPositions[eid]->y < tile_grid_point_to_world_point( *entities->currentTiles[eid] ).y  + ( TILE_SIZE / 2 ) ) {
//                 velocity.x = 0;
//                 velocity.y = 1;
                
//             }
//             else if( entities->worldPositions[eid]->y >tile_grid_point_to_world_point( *entities->currentTiles[eid] ).y  + ( TILE_SIZE / 2 )){
//                 velocity.x = 0;
//                 velocity.y = -1;

//             }
//         }
//         else if(*entities->directions[eid] == DIR_RIGHT ) {
//             // set velocity
//             if ( entities->worldPositions[eid]->y == tile_grid_point_to_world_point( *entities->currentTiles[eid] ).y  + ( TILE_SIZE / 2 ) ) {
//                 velocity.x = 1;
//                 velocity.y = 0;

//             } 
//             else if( entities->worldPositions[eid]->y < tile_grid_point_to_world_point( *entities->currentTiles[eid] ).y  + ( TILE_SIZE / 2 ) ) {
//                 velocity.x = 0;
//                 velocity.y = 1;

//             }
//             else if( entities->worldPositions[eid]->y >tile_grid_point_to_world_point( *entities->currentTiles[eid] ).y  + ( TILE_SIZE / 2 )){
//                 velocity.x = 0;
//                 velocity.y = -1;

//             }
//         }
//         velocity.x *= *entities->baseSpeeds[eid] * *entities->speedMultipliers[eid] * delta_time;
//         velocity.y *= *entities->baseSpeeds[eid] * *entities->speedMultipliers[eid] * delta_time;

//         moveActor(entities, eid, velocity );

//         // account for overshooting
//         // note velocity will never be in x and y direction.
//         if( velocity.x > 0 && !( *entities->directions[eid] == DIR_RIGHT )) { // right
//             if( entities->worldPositions[eid]->x > tile_grid_point_to_world_point( *entities->currentTiles[eid] ).x + ( TILE_SIZE / 2 ) ) {
//                 entities->worldPositions[eid]->x = ( tile_grid_point_to_world_point( *entities->currentTiles[eid] ).x + ( TILE_SIZE / 2 ) ) ;
//                 alignWorldDataBasedOnWorldPosition(entities, eid );

//             }
//         }
//         else if( velocity.x < 0 && !( *entities->directions[eid] == DIR_LEFT )) { // left
//             if( entities->worldPositions[eid]->x < tile_grid_point_to_world_point( *entities->currentTiles[eid] ).x + ( TILE_SIZE / 2 ) ) {
//                 entities->worldPositions[eid]->x = ( tile_grid_point_to_world_point( *entities->currentTiles[eid] ).x + ( TILE_SIZE / 2 ) ) ;
//                 alignWorldDataBasedOnWorldPosition(entities, eid );

//             }
//         }
//         else if( velocity.y > 0 && !( *entities->directions[eid] == DIR_DOWN )) { // down
//             if( entities->worldPositions[eid]->y > tile_grid_point_to_world_point( *entities->currentTiles[eid] ).y + ( TILE_SIZE / 2 ) ) {
//                 entities->worldPositions[eid]->y = ( tile_grid_point_to_world_point( *entities->currentTiles[eid] ).y + ( TILE_SIZE / 2 ) ) ;
//                 alignWorldDataBasedOnWorldPosition(entities, eid );

//             }
//         }
//         else if( velocity.y < 0 && !( *entities->directions[eid] == DIR_UP )) { // up
//             if( entities->worldPositions[eid]->y < tile_grid_point_to_world_point( *entities->currentTiles[eid] ).y + ( TILE_SIZE / 2 ) ) {
//                 entities->worldPositions[eid]->y = ( tile_grid_point_to_world_point( *entities->currentTiles[eid] ).y + ( TILE_SIZE / 2 ) ) ;
//                 alignWorldDataBasedOnWorldPosition(entities, eid );

//             }
//         }
    
    
// }

