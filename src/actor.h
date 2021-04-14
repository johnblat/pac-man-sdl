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

/**
 * Actor is responsible for interacting with other actors, objects, and the world itself via simulation
 */

const int ACTOR_SIZE = TILE_SIZE;

typedef enum Direction {
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN,
    DIR_NONE
} Direction;



int pac_src_sprite_size = 64; 



typedef struct Actor {
    Position_f  position;
    SDL_Point   center_point;
    SDL_Point   current_tile;
    SDL_Point   target_tile;
    SDL_Point   top_sensor;
    SDL_Point   bottom_sensor;
    SDL_Point   left_sensor;
    SDL_Point   right_sensor;
    Direction   direction;
} Actor;

void actor_set_current_tile( Actor *actor ) {
    actor->current_tile.x = ( ( actor->position.x + TILE_SIZE / 2 ) / TILE_SIZE ) ;
    actor->current_tile.y = ( ( ( actor->position.y + TILE_SIZE / 2 ) - (TILE_SIZE * 2 ) ) / TILE_SIZE ) ;
}

Actor *init_actor( Position_f initial_position ) {
    Actor *actor;
    actor = ( Actor *) malloc( sizeof( Actor ) );

    // actor->position.x =  TILE_SIZE ;
    // actor->position.y = TILE_SIZE * 17;
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
    actor->target_tile = actor->current_tile;

    
    // actor->current_animation_frame_index = 0;

    // SDL_Surface *pacmonster_surface = IMG_Load("pac_monster.png");
    // if( pacmonster_surface == NULL ) {
    //     fprintf( stderr, "Error %s\n ", IMG_GetError() );
    //     exit( EXIT_FAILURE );
    // }

    // actor->texture_atlas = SDL_CreateTextureFromSurface( renderer, pacmonster_surface );
    // if( actor->texture_atlas == NULL ) {
    //     fprintf( stderr, "Error %s\n ", SDL_GetError() );
    //     exit( EXIT_FAILURE );
    // }

    // SDL_FreeSurface( pacmonster_surface );

    // actor->pac_src_sprite_frames[ 0 ].x = 0;
    // actor->pac_src_sprite_frames[ 0 ].y = 0;
    // actor->pac_src_sprite_frames[ 0 ].w = 64;
    // actor->pac_src_sprite_frames[ 0 ].h = 64;

    // actor->pac_src_sprite_frames[ 1 ].x = 64;
    // actor->pac_src_sprite_frames[ 1 ].y = 0;
    // actor->pac_src_sprite_frames[ 1 ].w = 64;
    // actor->pac_src_sprite_frames[ 1 ].h = 64;

    // actor->pac_src_sprite_frames[ 2 ].x = 128;
    // actor->pac_src_sprite_frames[ 2 ].y = 0;
    // actor->pac_src_sprite_frames[ 2 ].w = 64;
    // actor->pac_src_sprite_frames[ 2 ].h = 64;

    // actor->pac_src_sprite_frames[ 3 ].x = 192;
    // actor->pac_src_sprite_frames[ 3 ].y = 0;
    // actor->pac_src_sprite_frames[ 3 ].w = 64;
    // actor->pac_src_sprite_frames[ 3 ].h = 64;

    return actor;
}


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


void pac_move( Actor *pacmonster, Vector_f velocity ) {
    pacmonster->position.x += velocity.x;
    pacmonster->position.y += velocity.y;

    pacmonster->center_point.x = ( int ) pacmonster->position.x + ( TILE_SIZE / 2 );
    pacmonster->center_point.y = ( int ) pacmonster->position.y + ( TILE_SIZE / 2 );

    pacmonster->current_tile.x = ( ( pacmonster->position.x + TILE_SIZE / 2 ) / TILE_SIZE ) ;
    pacmonster->current_tile.y = ( ( ( pacmonster->position.y + TILE_SIZE / 2 ) - (TILE_SIZE * 2 ) ) / TILE_SIZE ) ;

    pacmonster->top_sensor.x = pacmonster->position.x + ( TILE_SIZE / 2 );
    pacmonster->top_sensor.y = pacmonster->position.y;

    pacmonster->bottom_sensor.x = pacmonster->position.x + ( TILE_SIZE / 2 );
    pacmonster->bottom_sensor.y = pacmonster->position.y + TILE_SIZE;

    pacmonster->left_sensor.x = pacmonster->position.x;
    pacmonster->left_sensor.y = pacmonster->position.y + ( TILE_SIZE / 2 );

    pacmonster->right_sensor.x = pacmonster->position.x + TILE_SIZE;
    pacmonster->right_sensor.y = pacmonster->position.y + ( TILE_SIZE / 2 );
}


void pac_try_move( Actor *pacmonster,  TileMap *tm, float delta_time ) {
    
    int pac_speed = 220;
    Vector_f velocity = { 0, 0 };

    if( pacmonster->direction == DIR_UP ) {
        pacmonster->target_tile.x = pacmonster->current_tile.x;
        pacmonster->target_tile.y = pacmonster->current_tile.y - 1;

        // set velocity
        if ( pacmonster->center_point.x == tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0;
            velocity.y = -1;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        } 
        else if( pacmonster->center_point.x < tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0.7071068;
            velocity.y = -0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }
        else if( pacmonster->center_point.x >tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 )){
            velocity.x = -0.7071068;
            velocity.y = -0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }

        // move
        pac_move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_screen_point( pacmonster->target_tile, tm->tm_screen_position );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // top sensor is inside target tile
        if( pacmonster->top_sensor.y < target_tile_rect.y + TILE_SIZE 
        && pacmonster->top_sensor.x > target_tile_rect.x 
        && pacmonster->top_sensor.x < target_tile_rect.x + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_texture_atlas_indexes[ pacmonster->target_tile.y ][ pacmonster->target_tile.x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
            && tm->tm_texture_atlas_indexes[ pacmonster->target_tile.y ][ pacmonster->target_tile.x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                pac_move(pacmonster, reversed_velocity );
            }
        }
    }

    if( pacmonster->direction == DIR_DOWN ) {
        pacmonster->target_tile.x = pacmonster->current_tile.x;
        pacmonster->target_tile.y = pacmonster->current_tile.y + 1;

        // set velocity
        if ( pacmonster->center_point.x == tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0;
            velocity.y = 1;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        } 
        else if( pacmonster->center_point.x < tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0.7071068;
            velocity.y = 0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }
        else if( pacmonster->center_point.x >tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).x  + ( TILE_SIZE / 2 )){
            velocity.x = -0.7071068;
            velocity.y = 0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }

        // move
        pac_move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_screen_point( pacmonster->target_tile, tm->tm_screen_position );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->bottom_sensor.y > target_tile_rect.y 
        && pacmonster->bottom_sensor.x > target_tile_rect.x 
        && pacmonster->bottom_sensor.x < target_tile_rect.x + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_texture_atlas_indexes[ pacmonster->target_tile.y ][ pacmonster->target_tile.x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
            && tm->tm_texture_atlas_indexes[ pacmonster->target_tile.y ][ pacmonster->target_tile.x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                pac_move(pacmonster, reversed_velocity );
            }
        }
    }

    if( pacmonster->direction == DIR_LEFT ) {
        pacmonster->target_tile.x = pacmonster->current_tile.x - 1;
        pacmonster->target_tile.y = pacmonster->current_tile.y;

        // set velocity
        if ( pacmonster->center_point.y == tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = -1;
            velocity.y = 0;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        } 
        else if( pacmonster->center_point.y < tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = -0.7071068;
            velocity.y = 0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }
        else if( pacmonster->center_point.y >tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 )){
            velocity.x = -0.7071068;
            velocity.y = -0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }

        // move
        pac_move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_screen_point( pacmonster->target_tile, tm->tm_screen_position );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->left_sensor.x < target_tile_rect.x + TILE_SIZE
        && pacmonster->left_sensor.y > target_tile_rect.y 
        && pacmonster->left_sensor.y < target_tile_rect.y + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_texture_atlas_indexes[ pacmonster->target_tile.y ][ pacmonster->target_tile.x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
            && tm->tm_texture_atlas_indexes[ pacmonster->target_tile.y ][ pacmonster->target_tile.x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                pac_move(pacmonster, reversed_velocity );
            }
        }
    }

    if( pacmonster->direction == DIR_RIGHT ) {
        pacmonster->target_tile.x = pacmonster->current_tile.x + 1;
        pacmonster->target_tile.y = pacmonster->current_tile.y;

        // set velocity
        if ( pacmonster->center_point.y == tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 1;
            velocity.y = 0;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        } 
        else if( pacmonster->center_point.y < tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 ) ) {
            velocity.x = 0.7071068;
            velocity.y = 0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }
        else if( pacmonster->center_point.y >tile_grid_point_to_screen_point( pacmonster->current_tile, tm->tm_screen_position ).y  + ( TILE_SIZE / 2 )){
            velocity.x = 0.7071068;
            velocity.y = -0.7071068;
            velocity.x *= pac_speed * delta_time;
            velocity.y *= pac_speed * delta_time;
        }

        // move
        pac_move(pacmonster, velocity );

        // collision

        SDL_Point target_tile_screen_position = tile_grid_point_to_screen_point( pacmonster->target_tile, tm->tm_screen_position );
        SDL_Rect target_tile_rect = { target_tile_screen_position.x, target_tile_screen_position.y, TILE_SIZE, TILE_SIZE  };

        // sensor is inside target tile
        if( pacmonster->right_sensor.x > target_tile_rect.x 
        && pacmonster->right_sensor.y > target_tile_rect.y 
        && pacmonster->right_sensor.y < target_tile_rect.y + TILE_SIZE ) {
            // target tile is a wall
            if ( tm->tm_texture_atlas_indexes[ pacmonster->target_tile.y ][ pacmonster->target_tile.x ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
            && tm->tm_texture_atlas_indexes[ pacmonster->target_tile.y ][ pacmonster->target_tile.x ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) {
                Vector_f reversed_velocity = { -velocity.x, -velocity.y };
                pac_move(pacmonster, reversed_velocity );
            }
        }
    }
}

 
//         case DIR_UP:
//             pac_rotation = 270.f;
//             break;
//         case DIR_DOWN:
//             pac_rotation = 90.f;
//             break;
//         case DIR_LEFT:
//             pac_flip = SDL_FLIP_HORIZONTAL;
//             break;
//         case DIR_RIGHT:
//             pac_rotation = 0.f;
//             break;
//         case DIR_NONE:
//             break;





#endif
