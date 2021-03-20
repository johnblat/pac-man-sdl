#ifndef PAC_H
#define PAC_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "jb_types.h"
#include "constants.h"
#include "tiles.h"

#define PAC_ANIMATION_FRAMES 4 

typedef enum Direction {
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN,
    DIR_NONE
} Direction;


int pac_size = 64; // pacmonster will be inside of a 48x48 tile, but his sprite overflows the tile

typedef struct Pacmonster {
    Position_f position;
    Direction direction;
    SDL_Texture *texture_atlas;
    SDL_Rect pac_sprite_clips[ PAC_ANIMATION_FRAMES ];
    SDL_Rect collision_rect;
    SDL_Point current_tile;
    int current_animation_frame;

} Pacmonster;

Pacmonster *init_pacmonster( SDL_Renderer *renderer ) {
    Pacmonster *pacmonster;
    pacmonster = ( Pacmonster *) malloc( sizeof( Pacmonster ) );

    pacmonster->position.x =  TILE_SIZE ;
    pacmonster->position.y = TILE_SIZE * 3;
    pacmonster->direction = DIR_NONE;
    pacmonster->collision_rect.x = pacmonster->position.x;
    pacmonster->collision_rect.y = pacmonster->position.y;
    pacmonster->collision_rect.w = TILE_SIZE;
    pacmonster->collision_rect.h = TILE_SIZE;
    pacmonster->current_animation_frame = 0;

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

    pacmonster->pac_sprite_clips[ 0 ].x = 0;
    pacmonster->pac_sprite_clips[ 0 ].y = 0;
    pacmonster->pac_sprite_clips[ 0 ].w = 64;
    pacmonster->pac_sprite_clips[ 0 ].h = 64;

    pacmonster->pac_sprite_clips[ 1 ].x = 64;
    pacmonster->pac_sprite_clips[ 1 ].y = 0;
    pacmonster->pac_sprite_clips[ 1 ].w = 64;
    pacmonster->pac_sprite_clips[ 1 ].h = 64;

    pacmonster->pac_sprite_clips[ 2 ].x = 128;
    pacmonster->pac_sprite_clips[ 2 ].y = 0;
    pacmonster->pac_sprite_clips[ 2 ].w = 64;
    pacmonster->pac_sprite_clips[ 2 ].h = 64;

    pacmonster->pac_sprite_clips[ 3 ].x = 192;
    pacmonster->pac_sprite_clips[ 3 ].y = 0;
    pacmonster->pac_sprite_clips[ 3 ].w = 64;
    pacmonster->pac_sprite_clips[ 3 ].h = 64;


    return pacmonster;
    
}

// TODO - put these variables somewhere else
float frame_interval_seconds = 0.08;
float accumulator_ms = 0;

void pac_inc_animation_frame( Pacmonster *pacmonster, float delta_time ) {
    accumulator_ms += delta_time ;//* 1000;
    if ( accumulator_ms > frame_interval_seconds ) {
        accumulator_ms = 0;
        pacmonster->current_animation_frame++;
    }
    if ( pacmonster->current_animation_frame >= PAC_ANIMATION_FRAMES ) {
        pacmonster->current_animation_frame = 0;
    }
    
}

SDL_bool pac_has_collided_with_any_tile( SDL_Rect pac_collision_check_extension_rect, TileMap *tm ) {
    for ( int row = 0; row < TILE_ROWS; ++row ) 
    {
        for ( int col = 0; col < TILE_COLS; ++col ) 
        {
            if( tm->tm_texture_atlas_indexes[ row ][ col ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
                && tm->tm_texture_atlas_indexes[ row ][ col ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) 
            {
                SDL_Rect current_tile_rect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};

                if ( SDL_HasIntersection( &pac_collision_check_extension_rect, &current_tile_rect ) ) 
                {
                    return SDL_TRUE;
                }
            }
            
        }
        
    }
    return SDL_FALSE;
}


void pac_try_set_direction( Pacmonster *pacmonster, const Uint8 *current_key_states, TileMap *tm ) {

    // set quad here. Try to break up the world into chunks and only check the chunk that pacmonster is in


    if( current_key_states[ SDL_SCANCODE_UP ] ) {
        SDL_Rect pac_collision_check_extension_rect = { pacmonster->collision_rect.x, pacmonster->collision_rect.y - 1, pacmonster->collision_rect.w, 1 };

        // COLLISIONS IN TILEMAP
        SDL_bool collided = SDL_FALSE;
        
        for ( int row = 0; row < TILE_ROWS; ++row ) 
        {
            for ( int col = 0; col < TILE_COLS; ++col ) 
            {
                if( tm->tm_texture_atlas_indexes[ row ][ col ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
                    && tm->tm_texture_atlas_indexes[ row ][ col ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c ) 
                {
                    SDL_Point tile_grid_point = { col, row };

                    SDL_Point current_tile_screen_position = tile_grid_point_to_screen_point( tile_grid_point, tm->tm_screen_position );

                    SDL_Rect current_tile_rect = {current_tile_screen_position.x, current_tile_screen_position.y, TILE_SIZE, TILE_SIZE};

                    if ( SDL_HasIntersection( &pac_collision_check_extension_rect, &current_tile_rect ) ) 
                    {
                        collided = SDL_TRUE;
                        break;
                    }
                }
                
            }
            if (collided) break;
        }
        if ( !collided ) {
            pacmonster->direction = DIR_UP;
            return;
        }
        // END COLLISIONS IN TILEMAP

    }
    else if( current_key_states[ SDL_SCANCODE_DOWN ] ) {
        SDL_Rect pac_extension_rect = { pacmonster->collision_rect.x, pacmonster->collision_rect.y + pacmonster->collision_rect.h, pacmonster->collision_rect.w, 1 };

        // COLLISIONS IN TILEMAP
        SDL_bool collided = SDL_FALSE;
        
        for ( int row = 0; row < TILE_ROWS; ++row ) {
            for ( int col = 0; col < TILE_COLS; ++col ) {
                if( tm->tm_texture_atlas_indexes[ row ][ col ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
                    && tm->tm_texture_atlas_indexes[ row ][ col ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c) 
                {
                    SDL_Point tile_grid_point = { col, row };

                    SDL_Point current_tile_screen_position = tile_grid_point_to_screen_point( tile_grid_point, tm->tm_screen_position );
                    
                    SDL_Rect current_tile_rect = {current_tile_screen_position.x, current_tile_screen_position.y, TILE_SIZE, TILE_SIZE};

                    if ( SDL_HasIntersection( &pac_extension_rect, &current_tile_rect ) ) 
                    {
                        collided = SDL_TRUE;
                        break;
                    }
                }
            }
        }
        if ( !collided ) {
            pacmonster->direction = DIR_DOWN;
            return;
        }
        // END COLLISIONS IN TILEMAP


    }
    else if( current_key_states[ SDL_SCANCODE_LEFT ] ) {
        SDL_Rect pac_extension_rect = { pacmonster->collision_rect.x - 1, pacmonster->collision_rect.y, 1, pacmonster->collision_rect.h };

        // COLLISIONS IN TILEMAP
        SDL_bool collided = SDL_FALSE;
        
        for ( int row = 0; row < TILE_ROWS; ++row ) {
            for ( int col = 0; col < TILE_COLS; ++col ) {
                if( tm->tm_texture_atlas_indexes[ row ][ col ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
                    && tm->tm_texture_atlas_indexes[ row ][ col ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c) 
                {
                    SDL_Point tile_grid_point = { col, row };

                    SDL_Point current_tile_screen_position = tile_grid_point_to_screen_point( tile_grid_point, tm->tm_screen_position );
                    
                    SDL_Rect current_tile_rect = {current_tile_screen_position.x, current_tile_screen_position.y, TILE_SIZE, TILE_SIZE};

                    if ( SDL_HasIntersection( &pac_extension_rect, &current_tile_rect ) ) 
                    {
                        collided = SDL_TRUE;
                        break;
                    }
                }
            }
        }
        if ( !collided ) {
            pacmonster->direction = DIR_LEFT;
            return;
        }

    }
    else if( current_key_states[ SDL_SCANCODE_RIGHT ] ) {
        SDL_Rect pac_extension_rect = { pacmonster->collision_rect.x + pacmonster->collision_rect.w, pacmonster->collision_rect.y, 1, pacmonster->collision_rect.h };

        // COLLISIONS IN TILEMAP
        SDL_bool collided = SDL_FALSE;
        
        for ( int row = 0; row < TILE_ROWS; ++row ) {
            for ( int col = 0; col < TILE_COLS; ++col ) {
                if( tm->tm_texture_atlas_indexes[ row ][ col ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
                    && tm->tm_texture_atlas_indexes[ row ][ col ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c) 
                {
                    SDL_Point tile_grid_point = { col, row };

                    SDL_Point current_tile_screen_position = tile_grid_point_to_screen_point( tile_grid_point, tm->tm_screen_position );
                    
                    SDL_Rect current_tile_rect = {current_tile_screen_position.x, current_tile_screen_position.y, TILE_SIZE, TILE_SIZE};

                    if ( SDL_HasIntersection( &pac_extension_rect, &current_tile_rect ) ) 
                    {
                        collided = SDL_TRUE;
                        break;
                    }
                }
            }
        }
        if ( !collided ) {
            pacmonster->direction = DIR_RIGHT;
            return;
        }
        // END COLLISIONS IN TILEMAP

    }
    else if (current_key_states[ SDL_SCANCODE_SPACE ] ){
        pacmonster->direction = DIR_NONE;
    }
}

void pac_set_current_tile( Pacmonster *pacmonster ) {
    pacmonster->current_tile.x = ( ( pacmonster->position.x + TILE_SIZE / 2 ) / TILE_SIZE ) ;
    pacmonster->current_tile.y = ( ( pacmonster->position.y + TILE_SIZE / 2 ) / TILE_SIZE ) ;
}

void pac_move( Position_f *position, SDL_Rect *collision_rect, Vector_f velocity ) {
    position->x += velocity.x;
    position->y += velocity.y;
    collision_rect->x = position->x;
    collision_rect->y = position->y;
}


void pac_try_move( Pacmonster *pacmonster,  TileMap *tm, float delta_time ) {
    
    int pac_speed = 260;
    Vector_f velocity = { 0, 0 };
    switch( pacmonster->direction ) {
        
        case DIR_UP:
            velocity.y = -pac_speed * delta_time;
            pac_move( &pacmonster->position, &pacmonster->collision_rect, velocity );
            pac_set_current_tile( pacmonster );

            // top screen bound
            if( pacmonster->collision_rect.y < 0 ) {
                pacmonster->position.y = 0;
                pacmonster->collision_rect.y = pacmonster->position.y;
            }

            // tile_map bound
            for ( int row = 0; row < TILE_ROWS; ++row ) {
                for ( int col = 0; col < TILE_COLS; ++col ) {
                    if(tm->tm_texture_atlas_indexes[ row ][ col ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
                    && tm->tm_texture_atlas_indexes[ row ][ col ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c) 
                    {
                        SDL_Point tile_grid_point = { col, row };

                    SDL_Point current_tile_screen_position = tile_grid_point_to_screen_point( tile_grid_point, tm->tm_screen_position );
                    
                    SDL_Rect current_tile_rect = {current_tile_screen_position.x, current_tile_screen_position.y, TILE_SIZE, TILE_SIZE};

                        if ( SDL_HasIntersection( &pacmonster->collision_rect, &current_tile_rect ) ) 
                        {
                            pacmonster->position.y = current_tile_rect.y + current_tile_rect.h;
                            pacmonster->collision_rect.y = pacmonster->position.y;
                            pac_set_current_tile( pacmonster );
            
                            break;
                        }
                    }
                }
            }


            break;

        case DIR_DOWN:
            velocity.y = pac_speed * delta_time;
            pac_move( &pacmonster->position, &pacmonster->collision_rect, velocity );
            pac_set_current_tile( pacmonster );

            // bottom screen bound
            if( pacmonster->collision_rect.y + pacmonster->collision_rect.h > SCREEN_HEIGHT ) {
                pacmonster->position.y = SCREEN_HEIGHT - pacmonster->collision_rect.h;
                pacmonster->collision_rect.y = pacmonster->position.y;
                pac_set_current_tile( pacmonster );            
            }

            // tile_map bound
            for ( int row = 0; row < TILE_ROWS; ++row ) {
                for ( int col = 0; col < TILE_COLS; ++col ) {
                    if( tm->tm_texture_atlas_indexes[ row ][ col ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
                    && tm->tm_texture_atlas_indexes[ row ][ col ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c) 
                    {
SDL_Point tile_grid_point = { col, row };

                    SDL_Point current_tile_screen_position = tile_grid_point_to_screen_point( tile_grid_point, tm->tm_screen_position );
                    
                    SDL_Rect current_tile_rect = {current_tile_screen_position.x, current_tile_screen_position.y, TILE_SIZE, TILE_SIZE};
                        if ( SDL_HasIntersection( &pacmonster->collision_rect, &current_tile_rect ) ) 
                        {
                            pacmonster->position.y = current_tile_rect.y - pacmonster->collision_rect.h;
                            pacmonster->collision_rect.y = pacmonster->position.y;
                            pac_set_current_tile( pacmonster );            
                            break;
                        }
                    }
                }
            }

     
            break;

        case DIR_LEFT:
            velocity.x = -pac_speed * delta_time;
            pac_move( &pacmonster->position, &pacmonster->collision_rect, velocity );
            pac_set_current_tile( pacmonster );

            // left screen bound
            if( pacmonster->collision_rect.x < 0 ) {
                pacmonster->position.x = 0;
                pacmonster->collision_rect.x = pacmonster->position.x;
                pac_set_current_tile( pacmonster );
            }

            // tile_map bound
            for ( int row = 0; row < TILE_ROWS; ++row ) {
                for ( int col = 0; col < TILE_COLS; ++col ) {
                    if( tm->tm_texture_atlas_indexes[ row ][ col ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
                    && tm->tm_texture_atlas_indexes[ row ][ col ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c) 
                    {
SDL_Point tile_grid_point = { col, row };

                    SDL_Point current_tile_screen_position = tile_grid_point_to_screen_point( tile_grid_point, tm->tm_screen_position );
                    
                    SDL_Rect current_tile_rect = {current_tile_screen_position.x, current_tile_screen_position.y, TILE_SIZE, TILE_SIZE};
                        if ( SDL_HasIntersection( &pacmonster->collision_rect, &current_tile_rect ) ) 
                        {
                            pacmonster->position.x = current_tile_rect.x + current_tile_rect.w;
                            pacmonster->collision_rect.x = pacmonster->position.x;
                            pac_set_current_tile( pacmonster );            
                            break;
                        }
                    }
                }
            }


            break;

        case DIR_RIGHT:
            velocity.x = pac_speed * delta_time;
            pac_move( &pacmonster->position, &pacmonster->collision_rect, velocity );
            pac_set_current_tile( pacmonster );

            // right screen bound
            if( pacmonster->collision_rect.x + pacmonster->collision_rect.w > SCREEN_WIDTH  ) {
                pacmonster->position.x = SCREEN_WIDTH - pacmonster->collision_rect.w;
                pacmonster->collision_rect.x = pacmonster->position.x;
                pac_set_current_tile( pacmonster );
            }
            // tile_map bound
            for ( int row = 0; row < TILE_ROWS; ++row ) {
                for ( int col = 0; col < TILE_COLS; ++col ) {
                    if( tm->tm_texture_atlas_indexes[ row ][ col ].r != EMPTY_TILE_TEXTURE_ATLAS_INDEX.r
                    && tm->tm_texture_atlas_indexes[ row ][ col ].c != EMPTY_TILE_TEXTURE_ATLAS_INDEX.c) 
                    {
SDL_Point tile_grid_point = { col, row };

                    SDL_Point current_tile_screen_position = tile_grid_point_to_screen_point( tile_grid_point, tm->tm_screen_position );
                    
                    SDL_Rect current_tile_rect = {current_tile_screen_position.x, current_tile_screen_position.y, TILE_SIZE, TILE_SIZE};
                        if ( SDL_HasIntersection( &pacmonster->collision_rect, &current_tile_rect ) ) 
                        {
                            pacmonster->position.x = current_tile_rect.x - pacmonster->collision_rect.w;
                            pacmonster->collision_rect.x = pacmonster->position.x;
                            pac_set_current_tile( pacmonster );

                            break;
                        }
                    }
                }
            }

   
            break;

        case DIR_NONE:
            break;
    }
}

    


void pac_render( SDL_Renderer *renderer, Pacmonster *pacmonster ){
    // I set this in the move function
    //pacmonster->collision_rect.x = pacmonster->position.x;
    //pacmonster->collision_rect.y = pacmonster->position.y;
    
    // TODO: Rotation needs to remain whatever it used to be if stopping
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
    
    SDL_RenderCopyEx(renderer, pacmonster->texture_atlas, &pacmonster->pac_sprite_clips[ pacmonster->current_animation_frame ], &pac_rect, pac_rotation, NULL, pac_flip );


    

}

#endif