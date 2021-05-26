#include <SDL2/SDL.h>
#include "actor.h"
#include "tiles.h"
#include "jb_types.h"



void pac_try_set_direction( Actor *pacmonster, const Uint8 *current_key_states, TileMap *tm ) {

    // don't allow changing direciton if pacman is more than half of the tile
    if( current_key_states[ SDL_SCANCODE_UP ] ) {
        SDL_Point tile_above = { pacmonster->current_tile.x, pacmonster->current_tile.y - 1 };
        SDL_Rect tile_above_rect = {tile_grid_point_to_world_point( tile_above ).x, tile_grid_point_to_world_point( tile_above ).y, TILE_SIZE, TILE_SIZE };

        if( pacmonster->world_center_point.x > tile_above_rect.x + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_RIGHT ) {
            return;
        }

        if( pacmonster->world_center_point.x < tile_above_rect.x + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_LEFT ) {
            return;
        }

        if(tm->tm_walls[ tile_above.y ][ tile_above.x ] != 'x' ) 
        {
            pacmonster->direction = DIR_UP;
        }
        else {
            return;
        }
    }

    if( current_key_states[ SDL_SCANCODE_DOWN ] ) {
        SDL_Point tile_below = { pacmonster->current_tile.x, pacmonster->current_tile.y + 1 };
        SDL_Rect tile_below_rect = {tile_grid_point_to_world_point( tile_below ).x, tile_grid_point_to_world_point( tile_below ).y, TILE_SIZE, TILE_SIZE };

        if( pacmonster->world_center_point.x > tile_below_rect.x + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_RIGHT ) {
            return;
        }
        if( pacmonster->world_center_point.x < tile_below_rect.x + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_LEFT ) {
            return;
        }

        if(tm->tm_walls[ tile_below.y ][ tile_below.x ] != 'x' ) 
        {
            pacmonster->direction = DIR_DOWN;
        }
        else {
            return;
        }
    }

    if( current_key_states[ SDL_SCANCODE_LEFT ] ) {
        SDL_Point tile_to_left = { pacmonster->current_tile.x - 1, pacmonster->current_tile.y  };
        SDL_Rect tile_to_left_rect = {tile_grid_point_to_world_point( tile_to_left ).x, tile_grid_point_to_world_point( tile_to_left ).y, TILE_SIZE, TILE_SIZE };

        if( pacmonster->world_center_point.y > tile_to_left_rect.y + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_DOWN ) {
            return;
        }
        if( pacmonster->world_center_point.y < tile_to_left_rect.y + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_UP ) {
            return;
        }

        if(tm->tm_walls[ tile_to_left.y ][ tile_to_left.x ] != 'x'  ) 
        {
            pacmonster->direction = DIR_LEFT;
        }
        else {
            return;
        }
    }

    if( current_key_states[ SDL_SCANCODE_RIGHT ] ) {
        SDL_Point tile_to_right = { pacmonster->current_tile.x + 1, pacmonster->current_tile.y };
        SDL_Rect tile_to_right_rect = {tile_grid_point_to_world_point( tile_to_right ).x, tile_grid_point_to_world_point( tile_to_right ).y, TILE_SIZE, TILE_SIZE };

        if( pacmonster->world_center_point.y > tile_to_right_rect.y + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_DOWN ) {
            return;
        }
        if( pacmonster->world_center_point.y < tile_to_right_rect.y + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_UP ) {
            return;
        }

        if(tm->tm_walls[ tile_to_right.y ][ tile_to_right.x ] != 'x'  ) 
        {
            pacmonster->direction = DIR_RIGHT;
        }
        else {
            return;
        }
    }
}


void move( Actor *actor, Vector_f velocity ) {
    actor->world_position.x += velocity.x;
    actor->world_position.y += velocity.y;

    // MOVE TO OTHER SIDE OF SCREEN IF OFF EDGE
    if( actor->world_position.x > SCREEN_WIDTH ) {
        actor->world_position.x = 0 ;//- TILE_SIZE;
    }
    if( actor->world_position.y > 1000 ) {
        actor->world_position.y = 0;// + 80 - TILE_SIZE;
    }
    if( actor->world_position.x < 0 ) {//+ TILE_SIZE < 0 ) {
        actor->world_position.x = SCREEN_WIDTH;
    }
    if( actor->world_position.y < 0 ) { //} + TILE_SIZE < 80 ) {
        actor->world_position.y = 1000;
    }

    actor_align_world_data_based_on_world_position( actor );
    // actor->world_center_point.x = ( int ) actor->world_position.x + ( TILE_SIZE / 2 );
    // actor->world_center_point.y = ( int ) actor->world_position.y + ( TILE_SIZE / 2 );

    // actor->current_tile.x = ( ( actor->world_position.x + TILE_SIZE / 2 ) / TILE_SIZE ) ;
    // actor->current_tile.y = ( ( ( actor->world_position.y + TILE_SIZE / 2 ) ) / TILE_SIZE ) ;

    // actor->world_top_sensor.x = actor->world_position.x + ( TILE_SIZE / 2 );
    // actor->world_top_sensor.y = actor->world_position.y;

    // actor->world_bottom_sensor.x = actor->world_position.x + ( TILE_SIZE / 2 );
    // actor->world_bottom_sensor.y = actor->world_position.y + TILE_SIZE;

    // actor->world_left_sensor.x = actor->world_position.x;
    // actor->world_left_sensor.y = actor->world_position.y + ( TILE_SIZE / 2 );

    // actor->world_right_sensor.x = actor->world_position.x + TILE_SIZE;
    // actor->world_right_sensor.y = actor->world_position.y + ( TILE_SIZE / 2 );
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

        move(actors[ i ], velocity );

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
    Vector_f velocity = { 0, 0 };

    if( pacmonster->direction == DIR_UP ) {
        pacmonster->next_tile.x = pacmonster->current_tile.x;
        pacmonster->next_tile.y = pacmonster->current_tile.y - 1;
        if( pacmonster->next_tile.y < 0 ){
            pacmonster->next_tile.y = 22;
        } 

        // set velocity
        if ( pacmonster->world_center_point.x == tile_grid_point_to_world_point( pacmonster->current_tile ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0;
            velocity.y = -1;
        } 
        else if( pacmonster->world_center_point.x < tile_grid_point_to_world_point( pacmonster->current_tile ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0.7071068;
            velocity.y = -0.7071068;
        }
        else if( pacmonster->world_center_point.x >tile_grid_point_to_world_point( pacmonster->current_tile).x  + ( TILE_SIZE / 2 )){
            velocity.x = -0.7071068;
            velocity.y = -0.7071068;
            
        }

        velocity.x *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        velocity.y *= pacmonster->base_speed * pacmonster->speed_multp  * delta_time;
        // move
        move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( pacmonster->next_tile );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // top sensor is inside target tile
        if( pacmonster->world_top_sensor.y < target_tile_rect.y + TILE_SIZE 
        && pacmonster->world_top_sensor.x > target_tile_rect.x 
        && pacmonster->world_top_sensor.x < target_tile_rect.x + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_walls[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ] == 'x' ) {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                move(pacmonster, reversed_velocity );
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
            velocity.x = 0;
            velocity.y = 1;
        } 
        else if( pacmonster->world_center_point.x < tile_grid_point_to_world_point( pacmonster->current_tile ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0.7071068;
            velocity.y = 0.7071068;
        }
        else if( pacmonster->world_center_point.x >tile_grid_point_to_world_point( pacmonster->current_tile).x  + ( TILE_SIZE / 2 )){
            velocity.x = -0.7071068;
            velocity.y = 0.7071068;
        }
        velocity.x *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        velocity.y *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        // move
        move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( pacmonster->next_tile );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->world_bottom_sensor.y > target_tile_rect.y 
        && pacmonster->world_bottom_sensor.x > target_tile_rect.x 
        && pacmonster->world_bottom_sensor.x < target_tile_rect.x + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_walls[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ] == 'x') {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                move(pacmonster, reversed_velocity );
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
            velocity.x = -1;
            velocity.y = 0;
        } 
        else if( pacmonster->world_center_point.y < tile_grid_point_to_world_point( pacmonster->current_tile ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = -0.7071068;
            velocity.y = 0.7071068;
        }
        else if( pacmonster->world_center_point.y >tile_grid_point_to_world_point( pacmonster->current_tile).y  + ( TILE_SIZE / 2 )){
            velocity.x = -0.7071068;
            velocity.y = -0.7071068;
        }

        velocity.x *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        velocity.y *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        // move
        move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( pacmonster->next_tile);
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->world_left_sensor.x < target_tile_rect.x + TILE_SIZE
        && pacmonster->world_left_sensor.y > target_tile_rect.y 
        && pacmonster->world_left_sensor.y < target_tile_rect.y + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_walls[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ] == 'x' ) {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                move(pacmonster, reversed_velocity );
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
            velocity.x = 1;
            velocity.y = 0;
        } 
        else if( pacmonster->world_center_point.y < tile_grid_point_to_world_point( pacmonster->current_tile ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0.7071068;
            velocity.y = 0.7071068;
        }
        else if( pacmonster->world_center_point.y >tile_grid_point_to_world_point( pacmonster->current_tile ).y  + ( TILE_SIZE / 2 )){
            velocity.x = 0.7071068;
            velocity.y = -0.7071068;
        }

        velocity.x *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        velocity.y *= pacmonster->base_speed * pacmonster->speed_multp * delta_time;
        // move
        move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_world_point( pacmonster->next_tile );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->world_right_sensor.x > target_tile_rect.x 
        && pacmonster->world_right_sensor.y > target_tile_rect.y 
        && pacmonster->world_right_sensor.y < target_tile_rect.y + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_walls[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ] == 'x') {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                move(pacmonster, reversed_velocity );
            }
        }
    }
}