#include <SDL2/SDL.h>
#include "actor.h"
#include "entity.h"
#include "tiles.h"
#include "comparisons.h"
#include "jb_types.h"
#include "input.h"

void tile_wrap( SDL_Point *tile ) {
    if( tile->y >= TILE_ROWS ) 
        tile->y = 0;
    if( tile->y < 0 )
        tile->y = TILE_ROWS - 1;
    if( tile->x >= TILE_COLS )
        tile->x = 0;
    if( tile->x < 0 )
        tile->x = TILE_COLS - 1;
}


void inputToTryMoveProcess( Entities *entities, TileMap *tilemap, float deltaTime ) {
    uint8_t *inputMasks [ MAX_NUM_ENTITIES ] = entities->inputMasks;
    Actor *actors [ MAX_NUM_ENTITIES ] = entities->actors;

    for( int i = 0; i < g_NumEntities; ++i  ) {
        // check if should not process
        if( inputMasks[ i ] == NULL || actors[ MAX_NUM_ENTITIES ] == NULL ) {
            continue;
        }
        // should process
        // don't allow changing direciton if pacman is more than half of the tile
        if( *inputMasks[ i ] & g_INPUT_UP ) {
            SDL_Point tile_above = { actors[ i ]->current_tile.x, actors[ i ]->current_tile.y - 1 };

            tile_wrap( &tile_above );
            
            SDL_Rect tile_above_rect = {tile_grid_point_to_world_point( tile_above ).x, tile_grid_point_to_world_point( tile_above ).y, TILE_SIZE, TILE_SIZE };

            if( actors[ i ]->world_center_point.x > tile_above_rect.x + ( TILE_SIZE / 2 ) && actors[ i ]->direction == DIR_RIGHT ) {
                return;
            }

            if( actors[ i ]->world_center_point.x < tile_above_rect.x + ( TILE_SIZE / 2 ) && actors[ i ]->direction == DIR_LEFT ) {
                return;
            }

            if(tilemap->tm_walls[ tile_above.y ][ tile_above.x ] != 'x' ) 
            {
                actors[ i ]->direction = DIR_UP;
            }
            else {
                return;
            }
        }

        if( *inputMasks[ i ] & g_INPUT_DOWN  ) {
            SDL_Point tile_below = { actors[ i ]->current_tile.x, actors[ i ]->current_tile.y + 1 };

            tile_wrap( &tile_below );

            SDL_Rect tile_below_rect = {tile_grid_point_to_world_point( tile_below ).x, tile_grid_point_to_world_point( tile_below ).y, TILE_SIZE, TILE_SIZE };

            if( actors[ i ]->world_center_point.x > tile_below_rect.x + ( TILE_SIZE / 2 ) && actors[ i ]->direction == DIR_RIGHT ) {
                return;
            }
            if( actors[ i ]->world_center_point.x < tile_below_rect.x + ( TILE_SIZE / 2 ) && actors[ i ]->direction == DIR_LEFT ) {
                return;
            }

            if( points_equal( tile_below, tilemap->one_way_tile) ) {
                return;
            }

            if(tilemap->tm_walls[ tile_below.y ][ tile_below.x ] != 'x' ) 
            {
                actors[ i ]->direction = DIR_DOWN;
            }
            else {
                return;
            }
        }

        if( *inputMasks[ i ] & g_INPUT_LEFT  ) {
            SDL_Point tile_to_left = { actors[ i ]->current_tile.x - 1, actors[ i ]->current_tile.y  };

            tile_wrap( &tile_to_left );

            SDL_Rect tile_to_left_rect = {tile_grid_point_to_world_point( tile_to_left ).x, tile_grid_point_to_world_point( tile_to_left ).y, TILE_SIZE, TILE_SIZE };

            if( actors[ i ]->world_center_point.y > tile_to_left_rect.y + ( TILE_SIZE / 2 ) && actors[ i ]->direction == DIR_DOWN ) {
                return;
            }
            if( actors[ i ]->world_center_point.y < tile_to_left_rect.y + ( TILE_SIZE / 2 ) && actors[ i ]->direction == DIR_UP ) {
                return;
            }

            if( points_equal( tile_to_left, tilemap->one_way_tile) ) {
                return;
            }

            if(tilemap->tm_walls[ tile_to_left.y ][ tile_to_left.x ] != 'x'  ) 
            {
                actors[ i ]->direction = DIR_LEFT;
            }
            else {
                return;
            }
        }

        if( *inputMasks[ i ] & g_INPUT_RIGHT ) {
            SDL_Point tile_to_right = { actors[ i ]->current_tile.x + 1, actors[ i ]->current_tile.y };

            tile_wrap( &tile_to_right );

            SDL_Rect tile_to_right_rect = {tile_grid_point_to_world_point( tile_to_right ).x, tile_grid_point_to_world_point( tile_to_right ).y, TILE_SIZE, TILE_SIZE };

            if( actors[ i ]->world_center_point.y > tile_to_right_rect.y + ( TILE_SIZE / 2 ) && actors[ i ]->direction == DIR_DOWN ) {
                return;
            }
            if( actors[ i ]->world_center_point.y < tile_to_right_rect.y + ( TILE_SIZE / 2 ) && actors[ i ]->direction == DIR_UP ) {
                return;
            }

            if( points_equal( tile_to_right, tilemap->one_way_tile) ) {
                return;
            }

            if(tilemap->tm_walls[ tile_to_right.y ][ tile_to_right.x ] != 'x'  ) 
            {
                actors[ i ]->direction = DIR_RIGHT;
            }
            else {
                return;
            }
        }

        // try moving

        if( actors[ i ]->direction == DIR_UP ) {
            actors[ i ]->next_tile.x = actors[ i ]->current_tile.x;
            actors[ i ]->next_tile.y = actors[ i ]->current_tile.y - 1;
            if( actors[ i ]->next_tile.y < 0 ){
                actors[ i ]->next_tile.y = 22;
            } 

            // set velocity
            if ( actors[ i ]->world_center_point.x == tile_grid_point_to_world_point( actors[ i ]->current_tile ).x  + ( TILE_SIZE / 2 ) ) {
                actors[ i ]->velocity.x = 0;
                actors[ i ]->velocity.y = -1;
            } 
            else if( actors[ i ]->world_center_point.x < tile_grid_point_to_world_point( actors[ i ]->current_tile ).x  + ( TILE_SIZE / 2 ) ) {
                actors[ i ]->velocity.x = 0.7071068;
                actors[ i ]->velocity.y = -0.7071068;
            }
            else if( actors[ i ]->world_center_point.x >tile_grid_point_to_world_point( actors[ i ]->current_tile).x  + ( TILE_SIZE / 2 )){
            actors[ i ]->velocity.x = -0.7071068;
                actors[ i ]->velocity.y = -0.7071068;
                
            }

            actors[ i ]->velocity.x *= actors[ i ]->base_speed * actors[ i ]->speed_multp * deltaTime;
            actors[ i ]->velocity.y *= actors[ i ]->base_speed * actors[ i ]->speed_multp  * deltaTime;
            // move
            moveActor(actors[ i ], actors[ i ]->velocity );
            
            // make sure pacman doesn't pass centerpt
            if( actors[ i ]->velocity.x > 0 ) {
                if( actors[ i ]->world_center_point.x > tile_grid_point_to_world_point(actors[ i ]->current_tile).x + (TILE_SIZE * 0.5 ) ) {
                    actors[ i ]->world_center_point.x = tile_grid_point_to_world_point(actors[ i ]->current_tile).x ;
                    actor_align_world_data_based_on_world_position( actors[ i ] );
                }
            }
            else if( actors[ i ]->velocity.x < 0 ) {
                if( actors[ i ]->world_center_point.x < tile_grid_point_to_world_point(actors[ i ]->current_tile).x + (TILE_SIZE * 0.5 ) ) {
                    actors[ i ]->world_position.x = tile_grid_point_to_world_point(actors[ i ]->current_tile).x;
                    actor_align_world_data_based_on_world_position( actors[ i ] );
                }
            }

            // collision

            SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( actors[ i ]->next_tile );
            SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

            // top sensor is inside target tile
            if( actors[ i ]->world_top_sensor.y < target_tile_rect.y + TILE_SIZE 
            && actors[ i ]->world_top_sensor.x > target_tile_rect.x 
            && actors[ i ]->world_top_sensor.x < target_tile_rect.x + TILE_SIZE ) {
                // target tile is a wall
                if ( tilemap->tm_walls[ actors[ i ]->next_tile.y ][ actors[ i ]->next_tile.x ] == 'x' ) {
                    Vector_f reversed_velocity = { -actors[ i ]->velocity.x, -actors[ i ]->velocity.y };
                    moveActor(actors[ i ], reversed_velocity );
                }
            }
        }

        if( actors[ i ]->direction == DIR_DOWN ) {
            actors[ i ]->next_tile.x = actors[ i ]->current_tile.x;
            actors[ i ]->next_tile.y = actors[ i ]->current_tile.y + 1;
            if( actors[ i ]->next_tile.y >= 23 ){
                actors[ i ]->next_tile.y = 0;
            } 

            // set velocity
            if ( actors[ i ]->world_center_point.x == tile_grid_point_to_world_point( actors[ i ]->current_tile).x  + ( TILE_SIZE / 2 ) ) {
                actors[ i ]->velocity.x = 0;
                actors[ i ]->velocity.y = 1;
            } 
            else if( actors[ i ]->world_center_point.x < tile_grid_point_to_world_point( actors[ i ]->current_tile ).x  + ( TILE_SIZE / 2 ) ) {
                actors[ i ]->velocity.x = 0.7071068;
                actors[ i ]->velocity.y = 0.7071068;
            }
            else if( actors[ i ]->world_center_point.x >tile_grid_point_to_world_point( actors[ i ]->current_tile).x  + ( TILE_SIZE / 2 )){
                actors[ i ]->velocity.x = -0.7071068;
                actors[ i ]->velocity.y = 0.7071068;
            }
            actors[ i ]->velocity.x *= actors[ i ]->base_speed * actors[ i ]->speed_multp * deltaTime;
            actors[ i ]->velocity.y *= actors[ i ]->base_speed * actors[ i ]->speed_multp * deltaTime;
            // move
            moveActor(actors[ i ], actors[ i ]->velocity );

            // make sure pacman doesn't pass centerpt
            if( actors[ i ]->velocity.x > 0 ) {
                if( actors[ i ]->world_center_point.x > tile_grid_point_to_world_point(actors[ i ]->current_tile).x + (TILE_SIZE * 0.5 ) ) {
                    actors[ i ]->world_position.x = tile_grid_point_to_world_point(actors[ i ]->current_tile).x ;
                    actor_align_world_data_based_on_world_position( actors[ i ] );
                }
            }
            else if( actors[ i ]->velocity.x < 0 ) {
                if( actors[ i ]->world_center_point.x < tile_grid_point_to_world_point(actors[ i ]->current_tile).x + (TILE_SIZE * 0.5 ) ) {
                    actors[ i ]->world_position.x = tile_grid_point_to_world_point(actors[ i ]->current_tile).x;
                    actor_align_world_data_based_on_world_position( actors[ i ] );
                }
            }

            // collision

            SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( actors[ i ]->next_tile );
            SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

            // sensor is inside target tile
            if( actors[ i ]->world_bottom_sensor.y > target_tile_rect.y 
            && actors[ i ]->world_bottom_sensor.x > target_tile_rect.x 
            && actors[ i ]->world_bottom_sensor.x < target_tile_rect.x + TILE_SIZE ) {
                // target tile is a wall
                if ( tilemap->tm_walls[ actors[ i ]->next_tile.y ][ actors[ i ]->next_tile.x ] == 'x') {
                    Vector_f reversed_velocity = { -actors[ i ]->velocity.x, -actors[ i ]->velocity.y };
                    moveActor(actors[ i ], reversed_velocity );
                }
            }
        }

        if( actors[ i ]->direction == DIR_LEFT ) {
            actors[ i ]->next_tile.x = actors[ i ]->current_tile.x - 1;
            actors[ i ]->next_tile.y = actors[ i ]->current_tile.y;
            if( actors[ i ]->next_tile.x < 0 ){
                actors[ i ]->next_tile.x = 0;
            } 

            // set velocity
            if ( actors[ i ]->world_center_point.y == tile_grid_point_to_world_point( actors[ i ]->current_tile ).y  + ( TILE_SIZE / 2 ) ) {
                actors[ i ]->velocity.x = -1;
                actors[ i ]->velocity.y = 0;
            } 
            else if( actors[ i ]->world_center_point.y < tile_grid_point_to_world_point( actors[ i ]->current_tile ).y  + ( TILE_SIZE / 2 ) ) {
                actors[ i ]->velocity.x = -0.7071068;
                actors[ i ]->velocity.y = 0.7071068;
            }
            else if( actors[ i ]->world_center_point.y >tile_grid_point_to_world_point( actors[ i ]->current_tile).y  + ( TILE_SIZE / 2 )){
                actors[ i ]->velocity.x = -0.7071068;
                actors[ i ]->velocity.y = -0.7071068;
            }

            actors[ i ]->velocity.x *= actors[ i ]->base_speed * actors[ i ]->speed_multp * deltaTime;
            actors[ i ]->velocity.y *= actors[ i ]->base_speed * actors[ i ]->speed_multp * deltaTime;
            // move
            moveActor(actors[ i ], actors[ i ]->velocity );

            // make sure pacman doesn't pass centerpt
            if( actors[ i ]->velocity.y > 0 ) {
                if( actors[ i ]->world_center_point.y > tile_grid_point_to_world_point(actors[ i ]->current_tile).y + (TILE_SIZE * 0.5 ) ) {
                    actors[ i ]->world_position.y = tile_grid_point_to_world_point(actors[ i ]->current_tile).y ;
                    actor_align_world_data_based_on_world_position( actors[ i ] );
                }
            }
            else if( actors[ i ]->velocity.y < 0 ) {
                if( actors[ i ]->world_center_point.y < tile_grid_point_to_world_point(actors[ i ]->current_tile).y + (TILE_SIZE * 0.5 ) ) {
                    actors[ i ]->world_position.y = tile_grid_point_to_world_point(actors[ i ]->current_tile).y ;
                    actor_align_world_data_based_on_world_position( actors[ i ] );
                }
            }

            // collision

            SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( actors[ i ]->next_tile);
            SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

            // sensor is inside target tile
            if( actors[ i ]->world_left_sensor.x < target_tile_rect.x + TILE_SIZE
            && actors[ i ]->world_left_sensor.y > target_tile_rect.y 
            && actors[ i ]->world_left_sensor.y < target_tile_rect.y + TILE_SIZE ) {
                // target tile is a wall
                if ( tilemap->tm_walls[ actors[ i ]->next_tile.y ][ actors[ i ]->next_tile.x ] == 'x' ) {
                    Vector_f reversed_velocity = { -actors[ i ]->velocity.x, -actors[ i ]->velocity.y };
                    moveActor(actors[ i ], reversed_velocity );
                }
            }
        }

        if( actors[ i ]->direction == DIR_RIGHT ) {
            actors[ i ]->next_tile.x = actors[ i ]->current_tile.x + 1;
            actors[ i ]->next_tile.y = actors[ i ]->current_tile.y;
            if( actors[ i ]->next_tile.x >= TILE_COLS ){
                actors[ i ]->next_tile.x = 0;
            } 
            

            // set velocity
            if ( actors[ i ]->world_center_point.y == tile_grid_point_to_world_point( actors[ i ]->current_tile).y  + ( TILE_SIZE / 2 ) ) {
                actors[ i ]->velocity.x = 1;
                actors[ i ]->velocity.y = 0;
            } 
            else if( actors[ i ]->world_center_point.y < tile_grid_point_to_world_point( actors[ i ]->current_tile ).y  + ( TILE_SIZE / 2 ) ) {
                actors[ i ]->velocity.x = 0.7071068;
                actors[ i ]->velocity.y = 0.7071068;
            }
            else if( actors[ i ]->world_center_point.y >tile_grid_point_to_world_point( actors[ i ]->current_tile ).y  + ( TILE_SIZE / 2 )){
                actors[ i ]->velocity.x = 0.7071068;
                actors[ i ]->velocity.y = -0.7071068;
            }

            actors[ i ]->velocity.x *= actors[ i ]->base_speed * actors[ i ]->speed_multp * deltaTime;
            actors[ i ]->velocity.y *= actors[ i ]->base_speed * actors[ i ]->speed_multp * deltaTime;
            // move
            moveActor(actors[ i ], actors[ i ]->velocity );

            // make sure pacman doesn't pass centerpt
            if( actors[ i ]->velocity.y > 0 ) {
                if( actors[ i ]->world_center_point.y > tile_grid_point_to_world_point(actors[ i ]->current_tile).y + (TILE_SIZE * 0.5 ) ) {
                    actors[ i ]->world_position.y = tile_grid_point_to_world_point(actors[ i ]->current_tile).y;
                    actor_align_world_data_based_on_world_position( actors[ i ] );
                }
            }
            else if( actors[ i ]->velocity.y < 0 ) {
                if( actors[ i ]->world_center_point.y < tile_grid_point_to_world_point(actors[ i ]->current_tile).y + (TILE_SIZE * 0.5 ) ) {
                    actors[ i ]->world_position.y = tile_grid_point_to_world_point(actors[ i ]->current_tile).y;
                    actor_align_world_data_based_on_world_position( actors[ i ] );
                }
            }

            // collision

            SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( actors[ i ]->next_tile );
            SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

            // sensor is inside target tile
            if( actors[ i ]->world_right_sensor.x > target_tile_rect.x 
            && actors[ i ]->world_right_sensor.y > target_tile_rect.y 
            && actors[ i ]->world_right_sensor.y < target_tile_rect.y + TILE_SIZE ) {
                // target tile is a wall
                if ( tilemap->tm_walls[ actors[ i ]->next_tile.y ][ actors[ i ]->next_tile.x ] == 'x') {
                    Vector_f reversed_velocity = { -actors[ i ]->velocity.x, -actors[ i ]->velocity.y };
                    moveActor(actors[ i ], reversed_velocity );
                }
            }
        }

        // pacman animation row
        if( entities->actors[ i ]->velocity.x > 0 && actors[ i ]->velocity.y == 0 ) { // right
            entities->animatedSprites[ i ]->current_anim_row = 0;
        }
        if( actors[ i ]->velocity.x < 0 && actors[ i ]->velocity.y == 0 ) { // left
            entities->animatedSprites[ i ]->current_anim_row = 1;
        }
        if( actors[ i ]->velocity.x == 0 && actors[ i ]->velocity.y > 0 ) { // down
            entities->animatedSprites[ i ]->current_anim_row = 2;
        }
        if( actors[ i ]->velocity.x == 0 && actors[ i ]->velocity.y < 0 ) { // up
            entities->animatedSprites[ i ]->current_anim_row = 3;
        }
        if( actors[ i ]->velocity.x > 0 && actors[ i ]->velocity.y < 0 ) { //  up-right
            entities->animatedSprites[ i ]->current_anim_row = 4;
        }
        if( actors[ i ]->velocity.x < 0 && actors[ i ]->velocity.y < 0 ) { // up-left
            entities->animatedSprites[ i ]->current_anim_row = 5;
        }
        if( actors[ i ]->velocity.x > 0 && actors[ i ]->velocity.y > 0 ) { // down-right
            entities->animatedSprites[ i ]->current_anim_row = 6;
        }
        if( actors[ i ]->velocity.x < 0 && actors[ 0 ]->velocity.y > 0 ) { // down-left
            entities->animatedSprites[ i ]->current_anim_row = 7;
        }


    }
    
}


void moveActors( Entities *entities ) {
    Actor *actors[ MAX_NUM_ENTITIES ] = entities->actors;
    for( int i = 0; i < MAX_NUM_ENTITIES; ++i ) {
        // skip if no actor
        if( actors[ i ] == NULL ) {
            continue;
        }
        // process actor
        actors[ i ]->world_position.x += actors[ i ]->velocity.x;
        actors[ i ]->world_position.y += actors[ i ]->velocity.y;

        // MOVE TO OTHER SIDE OF SCREEN IF OFF EDGE
        if( actors[ i ]->world_position.x > SCREEN_WIDTH ) {
            actors[ i ]->world_position.x = -TILE_SIZE ;
        }
        if( actors[ i ]->world_position.y > TILE_ROWS * TILE_SIZE  ) {
            actors[ i ]->world_position.y = -TILE_SIZE;
        }
        if( actors[ i ]->world_position.x < -TILE_SIZE ) {
            actors[ i ]->world_position.x = SCREEN_WIDTH;
        }
        if( actors[ i ]->world_position.y < -TILE_SIZE ) {        
            actors[ i ]->world_position.y = TILE_ROWS * TILE_SIZE;
        }

        actor_align_world_data_based_on_world_position( actors[ i ] );
    }
}

void moveActor( Actor *actor, Vector_f velocity ) {
    // skip if null actor
    if( actor == NULL ) {
        return;
    }
    // process actor
    actor->world_position.x += velocity.x;
    actor->world_position.y += velocity.y;

    // MOVE TO OTHER SIDE OF SCREEN IF OFF EDGE
    if( actor->world_position.x > SCREEN_WIDTH ) {
        actor->world_position.x = -TILE_SIZE ;
    }
    if( actor->world_position.y > TILE_ROWS * TILE_SIZE  ) {
        actor->world_position.y = -TILE_SIZE;
    }
    if( actor->world_position.x < -TILE_SIZE ) {
        actor->world_position.x = SCREEN_WIDTH;
    }
    if( actor->world_position.y < -TILE_SIZE ) {        
        actor->world_position.y = TILE_ROWS * TILE_SIZE;
    }

    actor_align_world_data_based_on_world_position( actor );
}

float g_PAC_DASH_SPEED_MULTR = 2.5f;
float g_PAC_DASH_TIME_MAX = 0.75f;
void dashTimersProcess( Entities *entities, float deltaTime ) {
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++) {
        if( entities->dashTimers[ eid ] == NULL || entities->chargeTimers[ eid ] == NULL || entities->slowTimers[ eid ] == NULL || entities->inputMasks[ eid ] == NULL ) { //skip if doesn't have dash timers
            continue;
        }

        // charge or dash
        
        if( *entities->inputMasks[ eid ] & g_INPUT_ACTION ) {
            *entities->chargeTimers[ eid ] += deltaTime;
        }
        else if( *entities->inputMasks[ eid ] ^ g_INPUT_ACTION && *entities->chargeTimers[ eid ] > 0.0f ) {
            *entities->chargeTimers[ eid ] = *entities->chargeTimers[ eid ] > g_PAC_DASH_TIME_MAX ? g_PAC_DASH_TIME_MAX : *entities->chargeTimers[ eid ];
            *entities->chargeTimers[ eid ] = 0.0f;
        }
        
        entities->actors[ eid ]->speed_multp = 1.0f;
        if( *entities->slowTimers[ eid ] > 0 ) {
            entities->actors[ eid ]->speed_multp = 0.6f;
            *entities->slowTimers[ eid ] -= deltaTime;
        }
        if( *entities->dashTimers[eid] > 0 ) {
            entities->actors[ eid ]->speed_multp = g_PAC_DASH_SPEED_MULTR;
            *entities->dashTimers[ eid ] -= deltaTime;
            SDL_SetTextureAlphaMod( g_texture_atlases[ entities->animatedSprites[ eid ]->texture_atlas_id ].texture, 150 );
        }
        else {
            SDL_SetTextureAlphaMod( g_texture_atlases[ entities->animatedSprites[ eid ]->texture_atlas_id  ].texture, 255 );
        }
        if( *entities->chargeTimers[ eid ] > 0 ) {
            entities->actors[ eid ]->speed_multp = 0.4f;
        }
    }
        
}


void ghost_move( Actor **actors, TileMap *tm, float delta_time ) {

    for( int i = 1; i < 5; ++i ) {
        Vector_f velocity = { 0, 0 };
        if( actors[ i ]->direction == DIR_UP ) {
            // set velocity
            if( actors[ i ]->world_center_point.x >= tile_grid_point_to_world_point( actors[ i ]->current_tile ).x  + ( TILE_SIZE / 2 ) - 2 
            && ( actors[ i ]->world_center_point.x <= tile_grid_point_to_world_point( actors[ i ]->current_tile ).x  + ( TILE_SIZE / 2 ) + 2) ) {
                actors[ i ]->world_center_point.x = tile_grid_point_to_world_point( actors[ i ]->current_tile ).x  + ( TILE_SIZE / 2 );
            }
            if ( actors[ i ]->world_center_point.x == tile_grid_point_to_world_point( actors[ i ]->current_tile ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = -1;
            } 
            else if( actors[ i ]->world_center_point.x < tile_grid_point_to_world_point( actors[ i ]->current_tile ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 1;
                velocity.y = 0;
            }
            else if( actors[ i ]->world_center_point.x > tile_grid_point_to_world_point( actors[ i ]->current_tile ).x  + ( TILE_SIZE / 2 )){
                velocity.x = -1;
                velocity.y = 0;
            }
        }
        else if( actors[ i ]->direction == DIR_DOWN ){
            // set velocity
            if ( actors[ i ]->world_center_point.x == tile_grid_point_to_world_point( actors[ i ]->current_tile ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = 1;
                
            } 
            else if( actors[ i ]->world_center_point.x < tile_grid_point_to_world_point( actors[ i ]->current_tile ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 1;
                velocity.y = 0;

            }
            else if( actors[ i ]->world_center_point.x >tile_grid_point_to_world_point( actors[ i ]->current_tile ).x  + ( TILE_SIZE / 2 )){
                velocity.x = -1;
                velocity.y = 0;

            }
        } 
        else if( actors[ i ]->direction == DIR_LEFT ) {
            // set velocity
            if ( actors[ i ]->world_center_point.y == tile_grid_point_to_world_point( actors[ i ]->current_tile ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = -1;
                velocity.y = 0;

            } 
            else if( actors[ i ]->world_center_point.y < tile_grid_point_to_world_point( actors[ i ]->current_tile ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = 1;
                
            }
            else if( actors[ i ]->world_center_point.y >tile_grid_point_to_world_point( actors[ i ]->current_tile ).y  + ( TILE_SIZE / 2 )){
                velocity.x = 0;
                velocity.y = -1;

            }
        }
        else if(actors[ i ]->direction == DIR_RIGHT ) {
            // set velocity
            if ( actors[ i ]->world_center_point.y == tile_grid_point_to_world_point( actors[ i ]->current_tile ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 1;
                velocity.y = 0;

            } 
            else if( actors[ i ]->world_center_point.y < tile_grid_point_to_world_point( actors[ i ]->current_tile ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = 1;

            }
            else if( actors[ i ]->world_center_point.y >tile_grid_point_to_world_point( actors[ i ]->current_tile ).y  + ( TILE_SIZE / 2 )){
                velocity.x = 0;
                velocity.y = -1;

            }
        }
        velocity.x *= actors[ i ]->base_speed * actors[ i ]->speed_multp * delta_time;
        velocity.y *= actors[ i ]->base_speed * actors[ i ]->speed_multp * delta_time;

        moveActor(actors[ i ], velocity );

        // account for overshooting
        // note velocity will never be in x and y direction.
        if( velocity.x > 0 && !( actors[ i ]->direction == DIR_RIGHT )) { // right
            if( actors[ i ]->world_center_point.x > tile_grid_point_to_world_point( actors[ i ]->current_tile ).x + ( TILE_SIZE / 2 ) ) {
                actors[ i ]->world_position.x = ( tile_grid_point_to_world_point( actors[ i ]->current_tile ).x + ( TILE_SIZE / 2 ) ) - ( ACTOR_SIZE * 0.5 );
                actor_align_world_data_based_on_world_position( actors[ i ] );

            }
        }
        else if( velocity.x < 0 && !( actors[ i ]->direction == DIR_LEFT )) { // left
            if( actors[ i ]->world_center_point.x < tile_grid_point_to_world_point( actors[ i ]->current_tile ).x + ( TILE_SIZE / 2 ) ) {
                actors[ i ]->world_position.x = ( tile_grid_point_to_world_point( actors[ i ]->current_tile ).x + ( TILE_SIZE / 2 ) ) - ( ACTOR_SIZE * 0.5 );
                actor_align_world_data_based_on_world_position( actors[ i ] );

            }
        }
        else if( velocity.y > 0 && !( actors[ i ]->direction == DIR_DOWN )) { // down
            if( actors[ i ]->world_center_point.y > tile_grid_point_to_world_point( actors[ i ]->current_tile ).y + ( TILE_SIZE / 2 ) ) {
                actors[ i ]->world_position.y = ( tile_grid_point_to_world_point( actors[ i ]->current_tile ).y + ( TILE_SIZE / 2 ) ) - ( ACTOR_SIZE * 0.5 );
                actor_align_world_data_based_on_world_position( actors[ i ] );

            }
        }
        else if( velocity.y < 0 && !( actors[ i ]->direction == DIR_UP )) { // up
            if( actors[ i ]->world_center_point.y < tile_grid_point_to_world_point( actors[ i ]->current_tile ).y + ( TILE_SIZE / 2 ) ) {
                actors[ i ]->world_position.y = ( tile_grid_point_to_world_point( actors[ i ]->current_tile ).y + ( TILE_SIZE / 2 ) ) - ( ACTOR_SIZE * 0.5 );
                actor_align_world_data_based_on_world_position( actors[ i ] );

            }
        }
    }
    
}

void pac_try_move( Actor *pacmonster,  TileMap *tm, float delta_time ) {
    

    if( pacmonster->direction == DIR_UP ) {
        pacmonster->next_tile.x = pacmonster->current_tile.x;
        pacmonster->next_tile.y = pacmonster->current_tile.y - 1;
        if( pacmonster->next_tile.y < 0 ){
            pacmonster->next_tile.y = 22;
        } 

        // set velocity
        if ( pacmonster->world_center_point.x == tile_grid_point_to_world_point( pacmonster->current_tile ).x  + ( TILE_SIZE / 2 ) ) {
            pacmonster->velocity.x = 0;
            pacmonster->velocity.y = -1;
        } 
        else if( pacmonster->world_center_point.x < tile_grid_point_to_world_point( pacmonster->current_tile ).x  + ( TILE_SIZE / 2 ) ) {
            pacmonster->velocity.x = 0.7071068;
            pacmonster->velocity.y = -0.7071068;
        }
        else if( pacmonster->world_center_point.x >tile_grid_point_to_world_point( pacmonster->current_tile).x  + ( TILE_SIZE / 2 )){
           pacmonster->velocity.x = -0.7071068;
            pacmonster->velocity.y = -0.7071068;
            
        }

        pacmonster->velocity.x *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        pacmonster->velocity.y *= pacmonster->base_speed * pacmonster->speed_multp  * delta_time;
        // move
        moveActor(pacmonster, pacmonster->velocity );
        
        // make sure pacman doesn't pass centerpt
        if( pacmonster->velocity.x > 0 ) {
            if( pacmonster->world_center_point.x > tile_grid_point_to_world_point(pacmonster->current_tile).x + (TILE_SIZE * 0.5 ) ) {
                pacmonster->world_center_point.x = tile_grid_point_to_world_point(pacmonster->current_tile).x ;
                actor_align_world_data_based_on_world_position( pacmonster );
            }
        }
        else if( pacmonster->velocity.x < 0 ) {
            if( pacmonster->world_center_point.x < tile_grid_point_to_world_point(pacmonster->current_tile).x + (TILE_SIZE * 0.5 ) ) {
                pacmonster->world_position.x = tile_grid_point_to_world_point(pacmonster->current_tile).x;
                actor_align_world_data_based_on_world_position( pacmonster );
            }
        }

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( pacmonster->next_tile );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // top sensor is inside target tile
        if( pacmonster->world_top_sensor.y < target_tile_rect.y + TILE_SIZE 
        && pacmonster->world_top_sensor.x > target_tile_rect.x 
        && pacmonster->world_top_sensor.x < target_tile_rect.x + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_walls[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ] == 'x' ) {
                Vector_f reversed_velocity = { -pacmonster->velocity.x, -pacmonster->velocity.y };
                moveActor(pacmonster, reversed_velocity );
            }
        }
    }

    if( pacmonster->direction == DIR_DOWN ) {
        pacmonster->next_tile.x = pacmonster->current_tile.x;
        pacmonster->next_tile.y = pacmonster->current_tile.y + 1;
        if( pacmonster->next_tile.y >= 23 ){
            pacmonster->next_tile.y = 0;
        } 

        // set velocity
        if ( pacmonster->world_center_point.x == tile_grid_point_to_world_point( pacmonster->current_tile).x  + ( TILE_SIZE / 2 ) ) {
            pacmonster->velocity.x = 0;
            pacmonster->velocity.y = 1;
        } 
        else if( pacmonster->world_center_point.x < tile_grid_point_to_world_point( pacmonster->current_tile ).x  + ( TILE_SIZE / 2 ) ) {
            pacmonster->velocity.x = 0.7071068;
            pacmonster->velocity.y = 0.7071068;
        }
        else if( pacmonster->world_center_point.x >tile_grid_point_to_world_point( pacmonster->current_tile).x  + ( TILE_SIZE / 2 )){
            pacmonster->velocity.x = -0.7071068;
            pacmonster->velocity.y = 0.7071068;
        }
        pacmonster->velocity.x *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        pacmonster->velocity.y *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        // move
        moveActor(pacmonster, pacmonster->velocity );

        // make sure pacman doesn't pass centerpt
        if( pacmonster->velocity.x > 0 ) {
            if( pacmonster->world_center_point.x > tile_grid_point_to_world_point(pacmonster->current_tile).x + (TILE_SIZE * 0.5 ) ) {
                pacmonster->world_position.x = tile_grid_point_to_world_point(pacmonster->current_tile).x ;
                actor_align_world_data_based_on_world_position( pacmonster );
            }
        }
        else if( pacmonster->velocity.x < 0 ) {
            if( pacmonster->world_center_point.x < tile_grid_point_to_world_point(pacmonster->current_tile).x + (TILE_SIZE * 0.5 ) ) {
                pacmonster->world_position.x = tile_grid_point_to_world_point(pacmonster->current_tile).x;
                actor_align_world_data_based_on_world_position( pacmonster );
            }
        }

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( pacmonster->next_tile );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->world_bottom_sensor.y > target_tile_rect.y 
        && pacmonster->world_bottom_sensor.x > target_tile_rect.x 
        && pacmonster->world_bottom_sensor.x < target_tile_rect.x + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_walls[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ] == 'x') {
                Vector_f reversed_velocity = { -pacmonster->velocity.x, -pacmonster->velocity.y };
                moveActor(pacmonster, reversed_velocity );
            }
        }
    }

    if( pacmonster->direction == DIR_LEFT ) {
        pacmonster->next_tile.x = pacmonster->current_tile.x - 1;
        pacmonster->next_tile.y = pacmonster->current_tile.y;
        if( pacmonster->next_tile.x < 0 ){
            pacmonster->next_tile.x = 0;
        } 

        // set velocity
        if ( pacmonster->world_center_point.y == tile_grid_point_to_world_point( pacmonster->current_tile ).y  + ( TILE_SIZE / 2 ) ) {
            pacmonster->velocity.x = -1;
            pacmonster->velocity.y = 0;
        } 
        else if( pacmonster->world_center_point.y < tile_grid_point_to_world_point( pacmonster->current_tile ).y  + ( TILE_SIZE / 2 ) ) {
            pacmonster->velocity.x = -0.7071068;
            pacmonster->velocity.y = 0.7071068;
        }
        else if( pacmonster->world_center_point.y >tile_grid_point_to_world_point( pacmonster->current_tile).y  + ( TILE_SIZE / 2 )){
            pacmonster->velocity.x = -0.7071068;
            pacmonster->velocity.y = -0.7071068;
        }

        pacmonster->velocity.x *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        pacmonster->velocity.y *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        // move
        moveActor(pacmonster, pacmonster->velocity );

        // make sure pacman doesn't pass centerpt
        if( pacmonster->velocity.y > 0 ) {
            if( pacmonster->world_center_point.y > tile_grid_point_to_world_point(pacmonster->current_tile).y + (TILE_SIZE * 0.5 ) ) {
                pacmonster->world_position.y = tile_grid_point_to_world_point(pacmonster->current_tile).y ;
                actor_align_world_data_based_on_world_position( pacmonster );
            }
        }
        else if( pacmonster->velocity.y < 0 ) {
            if( pacmonster->world_center_point.y < tile_grid_point_to_world_point(pacmonster->current_tile).y + (TILE_SIZE * 0.5 ) ) {
                pacmonster->world_position.y = tile_grid_point_to_world_point(pacmonster->current_tile).y ;
                actor_align_world_data_based_on_world_position( pacmonster );
            }
        }

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( pacmonster->next_tile);
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->world_left_sensor.x < target_tile_rect.x + TILE_SIZE
        && pacmonster->world_left_sensor.y > target_tile_rect.y 
        && pacmonster->world_left_sensor.y < target_tile_rect.y + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_walls[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ] == 'x' ) {
                Vector_f reversed_velocity = { -pacmonster->velocity.x, -pacmonster->velocity.y };
                moveActor(pacmonster, reversed_velocity );
            }
        }
    }

    if( pacmonster->direction == DIR_RIGHT ) {
        pacmonster->next_tile.x = pacmonster->current_tile.x + 1;
        pacmonster->next_tile.y = pacmonster->current_tile.y;
        if( pacmonster->next_tile.x >= TILE_COLS ){
            pacmonster->next_tile.x = 0;
        } 
        

        // set velocity
        if ( pacmonster->world_center_point.y == tile_grid_point_to_world_point( pacmonster->current_tile).y  + ( TILE_SIZE / 2 ) ) {
            pacmonster->velocity.x = 1;
            pacmonster->velocity.y = 0;
        } 
        else if( pacmonster->world_center_point.y < tile_grid_point_to_world_point( pacmonster->current_tile ).y  + ( TILE_SIZE / 2 ) ) {
            pacmonster->velocity.x = 0.7071068;
            pacmonster->velocity.y = 0.7071068;
        }
        else if( pacmonster->world_center_point.y >tile_grid_point_to_world_point( pacmonster->current_tile ).y  + ( TILE_SIZE / 2 )){
            pacmonster->velocity.x = 0.7071068;
            pacmonster->velocity.y = -0.7071068;
        }

        pacmonster->velocity.x *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        pacmonster->velocity.y *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        // move
        moveActor(pacmonster, pacmonster->velocity );

        // make sure pacman doesn't pass centerpt
        if( pacmonster->velocity.y > 0 ) {
            if( pacmonster->world_center_point.y > tile_grid_point_to_world_point(pacmonster->current_tile).y + (TILE_SIZE * 0.5 ) ) {
                pacmonster->world_position.y = tile_grid_point_to_world_point(pacmonster->current_tile).y;
                actor_align_world_data_based_on_world_position( pacmonster );
            }
        }
        else if( pacmonster->velocity.y < 0 ) {
            if( pacmonster->world_center_point.y < tile_grid_point_to_world_point(pacmonster->current_tile).y + (TILE_SIZE * 0.5 ) ) {
                pacmonster->world_position.y = tile_grid_point_to_world_point(pacmonster->current_tile).y;
                actor_align_world_data_based_on_world_position( pacmonster );
            }
        }

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( pacmonster->next_tile );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->world_right_sensor.x > target_tile_rect.x 
        && pacmonster->world_right_sensor.y > target_tile_rect.y 
        && pacmonster->world_right_sensor.y < target_tile_rect.y + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_walls[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ] == 'x') {
                Vector_f reversed_velocity = { -pacmonster->velocity.x, -pacmonster->velocity.y };
                moveActor(pacmonster, reversed_velocity );
            }
        }
    }
}