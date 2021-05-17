#include <SDL2/SDL.h>
#include "actor.h"
#include "tiles.h"
#include "jb_types.h"



void pac_try_set_direction( Actor *pacmonster, const Uint8 *current_key_states, TileMap *tm ) {

    // don't allow changing direciton if pacman is more than half of the tile
    if( current_key_states[ SDL_SCANCODE_UP ] ) {
        SDL_Point tile_above = { pacmonster->current_tile.x, pacmonster->current_tile.y - 1 };
        SDL_Rect tile_above_rect = {tile_grid_point_to_screen_point( tile_above, tm->tm_screen_position ).x, tile_grid_point_to_screen_point( tile_above, tm->tm_screen_position ).y, TILE_SIZE, TILE_SIZE };

        if( pacmonster->center_point.x > tile_above_rect.x + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_RIGHT ) {
            return;
        }

        if( pacmonster->center_point.x < tile_above_rect.x + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_LEFT ) {
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
        SDL_Rect tile_below_rect = {tile_grid_point_to_screen_point( tile_below, tm->tm_screen_position ).x, tile_grid_point_to_screen_point( tile_below, tm->tm_screen_position ).y, TILE_SIZE, TILE_SIZE };

        if( pacmonster->center_point.x > tile_below_rect.x + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_RIGHT ) {
            return;
        }
        if( pacmonster->center_point.x < tile_below_rect.x + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_LEFT ) {
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
        SDL_Rect tile_to_left_rect = {tile_grid_point_to_screen_point( tile_to_left, tm->tm_screen_position ).x, tile_grid_point_to_screen_point( tile_to_left, tm->tm_screen_position ).y, TILE_SIZE, TILE_SIZE };

        if( pacmonster->center_point.y > tile_to_left_rect.y + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_DOWN ) {
            return;
        }
        if( pacmonster->center_point.y < tile_to_left_rect.y + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_UP ) {
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
        SDL_Rect tile_to_right_rect = {tile_grid_point_to_screen_point( tile_to_right, tm->tm_screen_position ).x, tile_grid_point_to_screen_point( tile_to_right, tm->tm_screen_position ).y, TILE_SIZE, TILE_SIZE };

        if( pacmonster->center_point.y > tile_to_right_rect.y + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_DOWN ) {
            return;
        }
        if( pacmonster->center_point.y < tile_to_right_rect.y + ( TILE_SIZE / 2 ) && pacmonster->direction == DIR_UP ) {
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
    actor->position.x += velocity.x;
    actor->position.y += velocity.y;

    actor->center_point.x = ( int ) actor->position.x + ( TILE_SIZE / 2 );
    actor->center_point.y = ( int ) actor->position.y + ( TILE_SIZE / 2 );

    actor->current_tile.x = ( ( actor->position.x + TILE_SIZE / 2 ) / TILE_SIZE ) ;
    actor->current_tile.y = ( ( ( actor->position.y + TILE_SIZE / 2 ) - (TILE_SIZE * 2 ) ) / TILE_SIZE ) ;

    actor->top_sensor.x = actor->position.x + ( TILE_SIZE / 2 );
    actor->top_sensor.y = actor->position.y;

    actor->bottom_sensor.x = actor->position.x + ( TILE_SIZE / 2 );
    actor->bottom_sensor.y = actor->position.y + TILE_SIZE;

    actor->left_sensor.x = actor->position.x;
    actor->left_sensor.y = actor->position.y + ( TILE_SIZE / 2 );

    actor->right_sensor.x = actor->position.x + TILE_SIZE;
    actor->right_sensor.y = actor->position.y + ( TILE_SIZE / 2 );
}

void ghost_move( Actor **actors, TileMap *tm, float delta_time ) {
    
    // if( g_current_ghost_mode == MODE_CHASE ) {
    //     if( actors[ 1 ]->next_tile.x == actors[ 1 ]->current_tile.x
    //     && actors[ 1 ]->next_tile.y == actors[ 1 ]->current_tile.y ) {

    //         set_shadow_target_tile( actors, 1, tm );
    //         set_direction_and_next_tile_shortest_to_target( actors[ 1 ], tm );
    //     }

    //     if( actors[ 2 ]->next_tile.x == actors[ 2 ]->current_tile.x
    //         && actors[ 2 ]->next_tile.y == actors[ 2 ]->current_tile.y ) {

    //         set_ambush_target_tile( actors, tm );
    //         set_direction_and_next_tile_shortest_to_target( actors[ 2 ], tm );
    //     }
    //     if( actors[ 3 ]->next_tile.x == actors[ 3 ]->current_tile.x
    //         && actors[ 3 ]->next_tile.y == actors[ 3 ]->current_tile.y ) {

    //         set_moody_target_tile( actors, tm );
    //         set_direction_and_next_tile_shortest_to_target( actors[ 3 ], tm );
    //     }
    //     if( actors[ 4 ]->next_tile.x == actors[ 4 ]->current_tile.x
    //         && actors[ 4 ]->next_tile.y == actors[ 4 ]->current_tile.y ) {

    //         set_pokey_target_tile( actors, tm );
    //         set_direction_and_next_tile_shortest_to_target( actors[ 4 ], tm );
    //     }
    // }
    // else if( g_current_ghost_mode == MODE_SCATTER ) {
    //     set_all_scatter_target_tile( actors );
    //     if( actors[ 1 ]->next_tile.x == actors[ 1 ]->current_tile.x
    //     && actors[ 1 ]->next_tile.y == actors[ 1 ]->current_tile.y ) {

    //         set_direction_and_next_tile_shortest_to_target( actors[ 1 ], tm );
    //     }

    //     if( actors[ 2 ]->next_tile.x == actors[ 2 ]->current_tile.x
    //         && actors[ 2 ]->next_tile.y == actors[ 2 ]->current_tile.y ) {

    //         set_direction_and_next_tile_shortest_to_target( actors[ 2 ], tm );
    //     }
    //     if( actors[ 3 ]->next_tile.x == actors[ 3 ]->current_tile.x
    //         && actors[ 3 ]->next_tile.y == actors[ 3 ]->current_tile.y ) {

    //         set_direction_and_next_tile_shortest_to_target( actors[ 3 ], tm );
    //     }
    //     if( actors[ 4 ]->next_tile.x == actors[ 4 ]->current_tile.x
    //         && actors[ 4 ]->next_tile.y == actors[ 4 ]->current_tile.y ) {

    //         set_direction_and_next_tile_shortest_to_target( actors[ 4 ], tm );
    //     }

    // }
    

    for( int i = 1; i < 5; ++i ) {
        Vector_f velocity = { 0, 0 };
        if( actors[ i ]->direction == DIR_UP ) {
            // set velocity
            if( actors[ i ]->center_point.x >= tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) - 2 
            && ( actors[ i ]->center_point.x <= tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) + 2) ) {
                actors[ i ]->center_point.x = tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 );
            }
            if ( actors[ i ]->center_point.x == tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = -1;
                velocity.x *= actors[ i ]->speed * delta_time;
                velocity.y *= actors[ i ]->speed * delta_time;
            } 
            else if( actors[ i ]->center_point.x < tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 1;
                velocity.y = 0;
                velocity.x *= actors[ i ]->speed * delta_time;
                velocity.y *= actors[ i ]->speed * delta_time;
            }
            else if( actors[ i ]->center_point.x > tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 )){
                velocity.x = -1;
                velocity.y = 0;
                velocity.x *= actors[ i ]->speed * delta_time;
                velocity.y *= actors[ i ]->speed * delta_time;
            }
        }
        else if( actors[ i ]->direction == DIR_DOWN ){
            // set velocity
            if ( actors[ i ]->center_point.x == tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = 1;
                velocity.x *= actors[ i ]->speed * delta_time;
                velocity.y *= actors[ i ]->speed * delta_time;
            } 
            else if( actors[ i ]->center_point.x < tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 1;
                velocity.y = 0;
                velocity.x *= actors[ i ]->speed * delta_time;
                velocity.y *= actors[ i ]->speed * delta_time;
            }
            else if( actors[ i ]->center_point.x >tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 )){
                velocity.x = -1;
                velocity.y = 0;
                velocity.x *= actors[ i ]->speed * delta_time;
                velocity.y *= actors[ i ]->speed * delta_time;
            }
        } 
        else if( actors[ i ]->direction == DIR_LEFT ) {
            // set velocity
            if ( actors[ i ]->center_point.y == tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = -1;
                velocity.y = 0;
                velocity.x *= actors[ i ]->speed * delta_time;
                velocity.y *= actors[ i ]->speed * delta_time;
            } 
            else if( actors[ i ]->center_point.y < tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = 1;
                velocity.x *= actors[ i ]->speed * delta_time;
                velocity.y *= actors[ i ]->speed * delta_time;
            }
            else if( actors[ i ]->center_point.y >tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 )){
                velocity.x = 0;
                velocity.y = -1;
                velocity.x *= actors[ i ]->speed * delta_time;
                velocity.y *= actors[ i ]->speed * delta_time;
            }
        }
        else if(actors[ i ]->direction == DIR_RIGHT ) {
            // set velocity
            if ( actors[ i ]->center_point.y == tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 1;
                velocity.y = 0;
                velocity.x *= actors[ i ]->speed * delta_time;
                velocity.y *= actors[ i ]->speed * delta_time;
            } 
            else if( actors[ i ]->center_point.y < tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
                velocity.x = 0;
                velocity.y = 1;
                velocity.x *= actors[ i ]->speed * delta_time;
                velocity.y *= actors[ i ]->speed * delta_time;
            }
            else if( actors[ i ]->center_point.y >tile_grid_point_to_screen_point( actors[ i ]->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 )){
                velocity.x = 0;
                velocity.y = -1;
                velocity.x *= actors[ i ]->speed * delta_time;
                velocity.y *= actors[ i ]->speed * delta_time;
            }
        }
        move(actors[ i ], velocity );
    }
    
}

void pac_try_move( Actor *pacmonster,  TileMap *tm, float delta_time ) {
    Vector_f velocity = { 0, 0 };

    if( pacmonster->direction == DIR_UP ) {
        pacmonster->next_tile.x = pacmonster->current_tile.x;
        pacmonster->next_tile.y = pacmonster->current_tile.y - 1;

        // set velocity
        if ( pacmonster->center_point.x == tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0;
            velocity.y = -1;
            velocity.x *= pacmonster->speed * delta_time;
            velocity.y *= pacmonster->speed * delta_time;
        } 
        else if( pacmonster->center_point.x < tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0.7071068;
            velocity.y = -0.7071068;
            velocity.x *= pacmonster->speed * delta_time;
            velocity.y *= pacmonster->speed * delta_time;
        }
        else if( pacmonster->center_point.x >tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 )){
            velocity.x = -0.7071068;
            velocity.y = -0.7071068;
            velocity.x *= pacmonster->speed * delta_time;
            velocity.y *= pacmonster->speed * delta_time;
        }

        // move
        move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_screen_point( pacmonster->next_tile, tm->tm_screen_position );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // top sensor is inside target tile
        if( pacmonster->top_sensor.y < target_tile_rect.y + TILE_SIZE 
        && pacmonster->top_sensor.x > target_tile_rect.x 
        && pacmonster->top_sensor.x < target_tile_rect.x + TILE_SIZE ) {
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

        // set velocity
        if ( pacmonster->center_point.x == tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0;
            velocity.y = 1;
            velocity.x *= pacmonster->speed * delta_time;
            velocity.y *= pacmonster->speed * delta_time;
        } 
        else if( pacmonster->center_point.x < tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0.7071068;
            velocity.y = 0.7071068;
            velocity.x *= pacmonster->speed * delta_time;
            velocity.y *= pacmonster->speed * delta_time;
        }
        else if( pacmonster->center_point.x >tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 )){
            velocity.x = -0.7071068;
            velocity.y = 0.7071068;
            velocity.x *= pacmonster->speed * delta_time;
            velocity.y *= pacmonster->speed * delta_time;
        }

        // move
        move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_screen_point( pacmonster->next_tile, tm->tm_screen_position );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->bottom_sensor.y > target_tile_rect.y 
        && pacmonster->bottom_sensor.x > target_tile_rect.x 
        && pacmonster->bottom_sensor.x < target_tile_rect.x + TILE_SIZE ) {
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

        // set velocity
        if ( pacmonster->center_point.y == tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = -1;
            velocity.y = 0;
            velocity.x *= pacmonster->speed * delta_time;
            velocity.y *= pacmonster->speed * delta_time;
        } 
        else if( pacmonster->center_point.y < tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = -0.7071068;
            velocity.y = 0.7071068;
            velocity.x *= pacmonster->speed * delta_time;
            velocity.y *= pacmonster->speed * delta_time;
        }
        else if( pacmonster->center_point.y >tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 )){
            velocity.x = -0.7071068;
            velocity.y = -0.7071068;
            velocity.x *= pacmonster->speed * delta_time;
            velocity.y *= pacmonster->speed * delta_time;
        }

        // move
        move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_screen_point( pacmonster->next_tile, tm->tm_screen_position );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->left_sensor.x < target_tile_rect.x + TILE_SIZE
        && pacmonster->left_sensor.y > target_tile_rect.y 
        && pacmonster->left_sensor.y < target_tile_rect.y + TILE_SIZE ) {
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

        // set velocity
        if ( pacmonster->center_point.y == tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 1;
            velocity.y = 0;
            velocity.x *= pacmonster->speed * delta_time;
            velocity.y *= pacmonster->speed * delta_time;
        } 
        else if( pacmonster->center_point.y < tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0.7071068;
            velocity.y = 0.7071068;
            velocity.x *= pacmonster->speed * delta_time;
            velocity.y *= pacmonster->speed * delta_time;
        }
        else if( pacmonster->center_point.y >tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 )){
            velocity.x = 0.7071068;
            velocity.y = -0.7071068;
            velocity.x *= pacmonster->speed * delta_time;
            velocity.y *= pacmonster->speed * delta_time;
        }

        // move
        move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_screen_point( pacmonster->next_tile, tm->tm_screen_position );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->right_sensor.x > target_tile_rect.x 
        && pacmonster->right_sensor.y > target_tile_rect.y 
        && pacmonster->right_sensor.y < target_tile_rect.y + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_walls[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ] == 'x') {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                move(pacmonster, reversed_velocity );
            }
        }
    }
}