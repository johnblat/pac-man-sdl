#ifndef PAC_H
#define PAC_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include "jb_types.h"
#include "constants.h"
#include "tiles.h"

#define NUM_PAC_ANIMATION_FRAMES 4 


typedef enum Direction {
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN,
    DIR_NONE
} Direction;


typedef struct AnimationTimer {
    float frame_interval;
    float accumulator;
} AnimationTimer;



int pac_src_sprite_size = 64; 


typedef struct Pacmonster {
    Position_f  position;
    SDL_Point   center_point;
    SDL_Rect    collision_rect;
    SDL_Point   current_tile;
    SDL_Point   target_tile;
    SDL_Point   top_sensor;
    SDL_Point   bottom_sensor;
    SDL_Point   left_sensor;
    SDL_Point   right_sensor;
    Direction   direction;
    SDL_Texture *texture_atlas;
    SDL_Rect    pac_src_sprite_frames[ NUM_PAC_ANIMATION_FRAMES ];
    int         current_animation_frame_index;

} Pacmonster;

void pac_set_current_tile( Pacmonster *pacmonster ) {
    pacmonster->current_tile.x = ( ( pacmonster->position.x + TILE_SIZE / 2 ) / TILE_SIZE ) ;
    pacmonster->current_tile.y = ( ( ( pacmonster->position.y + TILE_SIZE / 2 ) - (TILE_SIZE * 2 ) ) / TILE_SIZE ) ;
}

Pacmonster *init_pacmonster( SDL_Renderer *renderer ) {
    Pacmonster *pacmonster;
    pacmonster = ( Pacmonster *) malloc( sizeof( Pacmonster ) );

    pacmonster->position.x =  TILE_SIZE ;
    pacmonster->position.y = TILE_SIZE * 17;

    pacmonster->center_point.x = ( int ) pacmonster->position.x + ( TILE_SIZE / 2 );
    pacmonster->center_point.y = ( int ) pacmonster->position.y + ( TILE_SIZE / 2 );

    pacmonster->direction = DIR_NONE;

    pacmonster->collision_rect.x = pacmonster->position.x;
    pacmonster->collision_rect.y = pacmonster->position.y;

    pac_set_current_tile( pacmonster );

    pacmonster->collision_rect.w = TILE_SIZE ;
    pacmonster->collision_rect.h = TILE_SIZE ;
    
    pacmonster->current_animation_frame_index = 0;

    SDL_Surface *pacmonster_surface = IMG_Load("pac_monster.png");
    if( pacmonster_surface == NULL ) {
        fprintf( stderr, "Error %s\n ", IMG_GetError() );
        exit( EXIT_FAILURE );
    }

    pacmonster->texture_atlas = SDL_CreateTextureFromSurface( renderer, pacmonster_surface );
    if( pacmonster->texture_atlas == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    SDL_FreeSurface( pacmonster_surface );

    pacmonster->pac_src_sprite_frames[ 0 ].x = 0;
    pacmonster->pac_src_sprite_frames[ 0 ].y = 0;
    pacmonster->pac_src_sprite_frames[ 0 ].w = 64;
    pacmonster->pac_src_sprite_frames[ 0 ].h = 64;

    pacmonster->pac_src_sprite_frames[ 1 ].x = 64;
    pacmonster->pac_src_sprite_frames[ 1 ].y = 0;
    pacmonster->pac_src_sprite_frames[ 1 ].w = 64;
    pacmonster->pac_src_sprite_frames[ 1 ].h = 64;

    pacmonster->pac_src_sprite_frames[ 2 ].x = 128;
    pacmonster->pac_src_sprite_frames[ 2 ].y = 0;
    pacmonster->pac_src_sprite_frames[ 2 ].w = 64;
    pacmonster->pac_src_sprite_frames[ 2 ].h = 64;

    pacmonster->pac_src_sprite_frames[ 3 ].x = 192;
    pacmonster->pac_src_sprite_frames[ 3 ].y = 0;
    pacmonster->pac_src_sprite_frames[ 3 ].w = 64;
    pacmonster->pac_src_sprite_frames[ 3 ].h = 64;

    return pacmonster;
}


void pac_inc_animation_frame( Pacmonster *pacmonster, AnimationTimer *animation_timer, float delta_time ) {
    animation_timer->accumulator += delta_time ;//* 1000;
    if ( animation_timer->accumulator > animation_timer->frame_interval ) {
        animation_timer->accumulator = 0;
        pacmonster->current_animation_frame_index++;
    }
    if ( pacmonster->current_animation_frame_index >= NUM_PAC_ANIMATION_FRAMES ) {
        pacmonster->current_animation_frame_index = 0;
    }
}


void pac_collect_dot( Pacmonster *pacmonster, char dots[ TILE_ROWS ][ TILE_COLS ], unsigned int *score ) {
    if( dots[ pacmonster->current_tile.y ][ pacmonster->current_tile.x ] == 'x') {
        dots[ pacmonster->current_tile.y ][ pacmonster->current_tile.x ] = ' ';
        *score = *score + 10;
        printf("Score: %d\n", *score);
    }

}


void pac_try_set_direction( Pacmonster *pacmonster, const Uint8 *current_key_states, TileMap *tm ) {

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


void pac_move( Pacmonster *pacmonster, Vector_f velocity ) {
    pacmonster->position.x += velocity.x;
    pacmonster->position.y += velocity.y;

    pacmonster->center_point.x = ( int ) pacmonster->position.x + ( TILE_SIZE / 2 );
    pacmonster->center_point.y = ( int ) pacmonster->position.y + ( TILE_SIZE / 2 );

    pacmonster->collision_rect.x = pacmonster->position.x;
    pacmonster->collision_rect.y = pacmonster->position.y;

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


void pac_try_move( Pacmonster *pacmonster,  TileMap *tm, float delta_time ) {
    
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

    
void pac_render( SDL_Renderer *renderer, Pacmonster *pacmonster ){

    double pac_rotation = 0.f;
    SDL_RendererFlip pac_flip = SDL_FLIP_NONE;
    switch( pacmonster->direction ){
        case DIR_UP:
            pac_rotation = 270.f;
            break;
        case DIR_DOWN:
            pac_rotation = 90.f;
            break;
        case DIR_LEFT:
            pac_flip = SDL_FLIP_HORIZONTAL;
            break;
        case DIR_RIGHT:
            pac_rotation = 0.f;
            break;
        case DIR_NONE:
            break;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    
    SDL_Rect pac_rect = {pacmonster->position.x - 4, pacmonster->position.y - 4, TILE_SIZE + 6, TILE_SIZE+6 };
    
    SDL_RenderCopyEx(renderer, pacmonster->texture_atlas, &pacmonster->pac_src_sprite_frames[ pacmonster->current_animation_frame_index ], &pac_rect, pac_rotation, NULL, pac_flip );
}

#endif
