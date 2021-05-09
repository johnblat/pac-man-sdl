#ifndef PAC_H
#define PAC_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include "animation.h"
#include "jb_types.h"
#include "constants.h"
#include "tiles.h"

#define distance(x1, y1, x2, y2) ( (x2 - x1 ) * (x2 - x1) + (y2 - y1) * (y2 - y1) )




/**
 * Actor is responsible for interacting with other actors, objects, and the world itself via simulation
 */

const int ACTOR_SIZE = TILE_SIZE;

typedef enum Direction {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_NONE
} Direction;

Direction opposite_directions[ 4 ] = { DIR_DOWN, DIR_UP, DIR_RIGHT, DIR_LEFT };


typedef struct Actor {
    Position_f  position;     // screen position
    SDL_Point   center_point; // screen position
    SDL_Point   current_tile; // grid coord where the actor currently is
    SDL_Point   next_tile;    // grid coord where the actor will move to next
    SDL_Point   target_tile;  // grid coord where the actor is trying to get to. unused by pac
    SDL_Point   top_sensor;   // screen position
    SDL_Point   bottom_sensor;// screen position
    SDL_Point   left_sensor;  // screen position
    SDL_Point   right_sensor; // screen position
    Direction   direction;    // direction the actor wants to move
    Vector_f    velocity;     // current velocity
    int         speed;        // current speed
} Actor;

// /*****************
//  * MODES *********
//  * *************/
// const uint8_t NUM_SCATTER_CHASE_PERIODS = 9;
// uint8_t g_current_scatter_chase_period = 0;
// uint8_t g_scatter_chase_period_seconds[ NUM_SCATTER_CHASE_PERIODS ] = { 7, 20, 7, 20, 5, 20, 5, 0 };
// //uint8_t g_scatter_chase_period_seconds[ NUM_SCATTER_CHASE_PERIODS ] = { 2, 2, 2, 2, 2, 2, 2, 0 };
// typedef enum {
//     MODE_CHASE,     // use default behavior
//     MODE_SCATTER,   // use scatter behavior
// } GhostMode;

// GhostMode g_current_ghost_mode = MODE_SCATTER;

// /******************
//  * END MODES********
//  * *****************/


void actor_set_current_tile( Actor *actor ) {
    actor->current_tile.x = ( ( actor->position.x + TILE_SIZE / 2 ) / TILE_SIZE ) ;
    actor->current_tile.y = ( ( ( actor->position.y + TILE_SIZE / 2 ) - (TILE_SIZE * 2 ) ) / TILE_SIZE ) ;
}

Actor *init_actor( Position_f initial_position ) {
    Actor *actor;
    actor = ( Actor *) malloc( sizeof( Actor ) );

    actor->position.x = initial_position.x;
    actor->position.y = initial_position.y;

    actor->center_point.x = ( int ) actor->position.x + ( ACTOR_SIZE / 2 );
    actor->center_point.y = ( int ) actor->position.y + ( ACTOR_SIZE / 2 );

    actor->top_sensor.x = actor->position.x + ( ACTOR_SIZE / 2 );
    actor->top_sensor.y = actor->position.y;

    actor->bottom_sensor.x = actor->position.x + ( ACTOR_SIZE / 2 );
    actor->bottom_sensor.y = actor->position.y + ACTOR_SIZE;

    actor->left_sensor.x = actor->position.x;
    actor->left_sensor.y = actor->position.y + ( ACTOR_SIZE / 2 );

    actor->right_sensor.x = actor->position.x + ACTOR_SIZE;
    actor->right_sensor.y = actor->position.y + ( ACTOR_SIZE / 2 );    

    actor->direction = DIR_NONE;

    actor_set_current_tile( actor );
    actor->next_tile = actor->current_tile;
    actor->next_tile = actor->current_tile;

    actor->speed = 0;

    return actor;
}


// void set_direction_and_next_tile_shortest_to_target( Actor *actor, TileMap *tm ) {
//     SDL_Point tile_above, tile_below, tile_left, tile_right;
//     tile_above.x = actor->current_tile.x;
//     tile_above.y = actor->current_tile.y - 1;
//     tile_below.x = actor->current_tile.x;
//     tile_below.y = actor->current_tile.y + 1;
//     tile_left.x = actor->current_tile.x - 1;
//     tile_left.y = actor->current_tile.y;
//     tile_right.x = actor->current_tile.x + 1;
//     tile_right.y = actor->current_tile.y;

//     SDL_Point surrounding_tiles[ 4 ];
//     surrounding_tiles[ DIR_UP ] = tile_above;
//     surrounding_tiles[ DIR_DOWN ] = tile_below;
//     surrounding_tiles[ DIR_LEFT ] = tile_left;
//     surrounding_tiles[ DIR_RIGHT ] = tile_right;

//     float above_to_target_dist = distance(tile_above.x, tile_above.y,  actor->target_tile.x, actor->target_tile.y ); 
//     float below_to_target_dist = distance(tile_below.x, tile_below.y,  actor->target_tile.x, actor->target_tile.y ); 
//     float left_to_target_dist  = distance(tile_left.x, tile_left.y,  actor->target_tile.x, actor->target_tile.y ); 
//     float right_to_target_dist = distance(tile_right.x, tile_right.y,  actor->target_tile.x, actor->target_tile.y ); 


//     float lengths[ 4 ];
//     lengths[ DIR_UP ] = above_to_target_dist;
//     lengths[ DIR_DOWN ] = below_to_target_dist;
//     lengths[ DIR_LEFT ] = left_to_target_dist;
//     lengths[ DIR_RIGHT ] = right_to_target_dist;

//     Direction opposite_directions[ 4 ];
//     opposite_directions[ DIR_UP ] = DIR_DOWN;
//     opposite_directions[ DIR_DOWN ] = DIR_UP;
//     opposite_directions[ DIR_LEFT ] = DIR_RIGHT;
//     opposite_directions[ DIR_RIGHT ] = DIR_LEFT;

//     int shortest_direction = 0;
//     float shortest_length = 9999.0f; // just some high number

//     for( int i = 0; i < 4; ++i ) {
//         if( tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
//             && tm->tm_texture_atlas_indexes[ surrounding_tiles[ i ].y ][ surrounding_tiles[ i ].x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
//                 continue;
//         }

//         if( i == opposite_directions[ actor->direction  ] ) continue;

//         if ( lengths[ i ] < shortest_length ) {
//             shortest_direction = i;
//             shortest_length = lengths[ i ];
//         }
//     }

//     actor->direction = (Direction) shortest_direction;
//     actor->next_tile = surrounding_tiles[ shortest_direction ];
// }



// void set_scatter_target_tile( Actor **actors, uint8_t actor_id, TileMap *tm ) {
//     actors[ actor_id ]->target_tile = pokey_home_tile;
// }

// void set_all_scatter_target_tile( Actor **actors ) {
//     actors[ 1 ]->target_tile = shadow_home_tile;
//     actors[ 2 ]->target_tile = ambush_home_tile;
//     actors[ 3 ]->target_tile = moody_home_tile;
//     actors[ 4 ]->target_tile = pokey_home_tile;
// }

// void set_shadow_target_tile( Actor **actors, uint8_t actor_id, TileMap *tm ) {
//     actors[ actor_id ]->target_tile = actors[ 0 ]->current_tile;
// }


// void set_ambush_target_tile( Actor **actors, TileMap *tm ) {

//     switch( actors[ 0 ]->direction ) {
//         case DIR_UP:
//             actors[ 2 ]->target_tile.x = actors[ 0 ]->current_tile.x;
//             actors[ 2 ]->target_tile.y = actors[ 0 ]->current_tile.y - 4;
//             break;
//         case DIR_DOWN:
//             actors[ 2 ]->target_tile.x = actors[ 0 ]->current_tile.x;
//             actors[ 2 ]->target_tile.y = actors[ 0 ]->current_tile.y + 4;
//             break;
//         case DIR_LEFT:
//             actors[ 2 ]->target_tile.x = actors[ 0 ]->current_tile.x - 4;
//             actors[ 2 ]->target_tile.y = actors[ 0 ]->current_tile.y;
//             break;
//         case DIR_RIGHT:
//             actors[ 2 ]->target_tile.x = actors[ 0 ]->current_tile.x + 4;
//             actors[ 2 ]->target_tile.y = actors[ 0 ]->current_tile.y;
//             break;
//         default :
//             actors[ 2 ]->target_tile.x = actors[ 0 ]->current_tile.x;
//             actors[ 2 ]->target_tile.y = actors[ 0 ]->current_tile.y;
//             break;
//     }   
// }

// void set_moody_target_tile(Actor **actors, TileMap *tm) {

//     SDL_Point other_ghost_current_tile = actors[ 1 ]->current_tile;
//     SDL_Point offset_tile;
//     Line_Points line_from_other_ghost_to_offset;
//     int tiles_x;
//     int tiles_y;

//     switch( actors[ 0 ]->direction ) {
//         case DIR_UP:
//             offset_tile.x = actors[ 0 ]->current_tile.x;
//             offset_tile.y = actors[ 0 ]->current_tile.y - 2;
            
//             line_from_other_ghost_to_offset.a.x = actors[ 1 ]->current_tile.x;
//             line_from_other_ghost_to_offset.a.y = actors[ 1 ]->current_tile.y;
//             line_from_other_ghost_to_offset.b.x = offset_tile.x;
//             line_from_other_ghost_to_offset.b.y = offset_tile.x;

//             tiles_x = offset_tile.x - other_ghost_current_tile.x;
//             tiles_y = offset_tile.y - other_ghost_current_tile.y;

//             actors[ 3 ]->target_tile.x = offset_tile.x + tiles_x;
//             actors[ 3 ]->target_tile.y = offset_tile.y + tiles_y;

//             break;
//         case DIR_DOWN:
//             offset_tile.x = actors[ 0 ]->current_tile.x;
//             offset_tile.y = actors[ 0 ]->current_tile.y + 2;

//              tiles_x = offset_tile.x - other_ghost_current_tile.x;
//              tiles_y = offset_tile.y - other_ghost_current_tile.y;

//             actors[ 3 ]->target_tile.x = offset_tile.x + tiles_x;
//             actors[ 3 ]->target_tile.y = offset_tile.y + tiles_y;

//             break;
//         case DIR_LEFT:
//             offset_tile.x = actors[ 0 ]->current_tile.x - 2;
//             offset_tile.y = actors[ 0 ]->current_tile.y;

//              tiles_x = offset_tile.x - other_ghost_current_tile.x;
//              tiles_y = offset_tile.y - other_ghost_current_tile.y;

//             actors[ 3 ]->target_tile.x = offset_tile.x + tiles_x;
//             actors[ 3 ]->target_tile.y = offset_tile.y + tiles_y;

//             break;
//         case DIR_RIGHT:
//             offset_tile.x = actors[ 0 ]->current_tile.x + 2;
//             offset_tile.y = actors[ 0 ]->current_tile.y;

//              tiles_x = offset_tile.x - other_ghost_current_tile.x;
//              tiles_y = offset_tile.y - other_ghost_current_tile.y;

//             actors[ 3 ]->target_tile.x = offset_tile.x + tiles_x;
//             actors[ 3 ]->target_tile.y = offset_tile.y + tiles_y;

//             break;
//         default :
//             offset_tile.x = actors[ 0 ]->current_tile.x;
//             offset_tile.y = actors[ 0 ]->current_tile.y;

//              tiles_x = offset_tile.x - other_ghost_current_tile.x;
//              tiles_y = offset_tile.y - other_ghost_current_tile.y;

//             actors[ 3 ]->target_tile.x = offset_tile.x + tiles_x;
//             actors[ 3 ]->target_tile.y = offset_tile.y + tiles_y;

//             break;
//     }
//     if ( actors[ 3 ]->target_tile.x < 0 ) actors[ 3 ]->target_tile.x = 0;
//     if ( actors[ 3 ]->target_tile.y < 0 ) actors[ 3 ]->target_tile.y = 0;

//     if ( actors[ 3 ]->target_tile.x > TILE_COLS - 1 ) actors[ 3 ]->target_tile.x = TILE_COLS - 1;
//     if ( actors[ 3 ]->target_tile.y > TILE_ROWS - 1 ) actors[ 3 ]->target_tile.y = TILE_ROWS - 1;

// }

// void set_pokey_target_tile( Actor **actors, TileMap *tm ) {
//     SDL_Point b_tile_above, b_tile_below, b_tile_left, b_tile_right;
//     float shadow_range = 64;
//     SDL_Point b_current_tile = actors[ 4 ]->current_tile;
//     SDL_Point pac_current_tile = actors[ 0 ]->current_tile;

//     float distance_to_pacman = distance( b_current_tile.x, b_current_tile.y, pac_current_tile.x, pac_current_tile.y );

//     if( distance_to_pacman >= shadow_range ) {
//         set_shadow_target_tile( actors, 4, tm );
//     }
//     else {
//         set_scatter_target_tile( actors, 4, tm );
//     }
// }


void pac_collect_dot( Actor *pacmonster, char dots[ TILE_ROWS ][ TILE_COLS ], Score *score, SDL_Renderer *renderer ) {
    if( dots[ pacmonster->current_tile.y ][ pacmonster->current_tile.x ] == 'x') {
        // get rid of dot marker
        dots[ pacmonster->current_tile.y ][ pacmonster->current_tile.x ] = ' ';
        
        score->score_number += 10;
        
        snprintf( score->score_text, 32, "Score : %d", score->score_number );
        SDL_Surface *score_surface = TTF_RenderText_Solid( score->font, score->score_text, score->score_color );

        SDL_DestroyTexture( score->score_texture );
        score->score_texture = SDL_CreateTextureFromSurface( renderer, score_surface );
        score->score_render_dst_rect.x = 10;
        score->score_render_dst_rect.y = 10;
        score->score_render_dst_rect.w = score_surface->w;
        score->score_render_dst_rect.h = score_surface->h;

        SDL_FreeSurface( score_surface );

    }

}


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

        if(tm->tm_texture_atlas_indexes[ tile_above.y ][ tile_above.x ].r == EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
            && tm->tm_texture_atlas_indexes[ tile_above.y ][ tile_above.x ].c == EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) 
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

        if(tm->tm_texture_atlas_indexes[ tile_below.y ][ tile_below.x ].r == EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
            && tm->tm_texture_atlas_indexes[ tile_below.y ][ tile_below.x ].c == EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) 
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

        if(tm->tm_texture_atlas_indexes[ tile_to_left.y ][ tile_to_left.x ].r == EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
            && tm->tm_texture_atlas_indexes[ tile_to_left.y ][ tile_to_left.x ].c == EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) 
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

        if(tm->tm_texture_atlas_indexes[ tile_to_right.y ][ tile_to_right.x ].r == EMPTY_TILE_TEXTURE_ATLAS_INDEX.r 
            && tm->tm_texture_atlas_indexes[ tile_to_right.y ][ tile_to_right.x ].c == EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) 
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
            if ( tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
            && tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
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
            if ( tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
            && tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
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
            if ( tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
            && tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
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
            if ( tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
            && tm->tm_texture_atlas_indexes[ pacmonster->next_tile.y ][ pacmonster->next_tile.x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                move(pacmonster, reversed_velocity );
            }
        }
    }
}

#endif
