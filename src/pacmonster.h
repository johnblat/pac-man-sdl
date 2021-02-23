#ifndef PAC_H
#define PAC_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "jb_types.h"
#include "tiles.h"
#include <sys/stat.h>


typedef enum Direction {
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN,
    DIR_NONE
} Direction;

const int PAC_ANIMATION_FRAMES = 4;

int pac_size = 64; // pacmonster will be inside of a 48x48 tile, but his sprite overflows the tile

typedef struct Pacmonster {
    Position_f position;
    Direction direction;
    SDL_Texture *texture_atlas;
    SDL_Rect pac_sprite_clips[ PAC_ANIMATION_FRAMES ];
    SDL_Rect collision_rect;
    int current_animation_frame;

} Pacmonster;

Pacmonster *init_pacmonster( SDL_Renderer *renderer ) {
    Pacmonster *pacmonster;
    pacmonster = ( Pacmonster *) malloc( sizeof( Pacmonster ) );

    pacmonster->position.x = 0;
    pacmonster->position.y = 0;
    pacmonster->direction = DIR_NONE;
    pacmonster->collision_rect.x = pacmonster->position.x;
    pacmonster->collision_rect.y = pacmonster->position.y;
    pacmonster->collision_rect.w = 48;
    pacmonster->collision_rect.h = 48;
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


void pac_try_set_direction( Pacmonster *pacmonster, const Uint8 *current_key_states, char tile_map[ TILE_ROWS ][ TILE_COLS ] ) {

    // set quad here. Try to break up the world into chunks and only check the chunk that pacmonster is in


    if( current_key_states[ SDL_SCANCODE_UP ] ) {
        SDL_Rect pac_extension_rect = { pacmonster->collision_rect.x, pacmonster->collision_rect.y - 1, pacmonster->collision_rect.w, 1 };

        // COLLISIONS IN TILEMAP
        SDL_bool collided = SDL_FALSE;
        
        for ( int row = 0; row < TILE_ROWS; ++row ) 
        {
            for ( int col = 0; col < TILE_COLS; ++col ) 
            {
                if( tile_map[ row ][ col ] == 'x') 
                {
                    SDL_Rect current_tile_rect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};

                    if ( SDL_HasIntersection( &pac_extension_rect, &current_tile_rect ) ) 
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

        // if ( ! SDL_HasIntersection( &pac_extension_rect, world_rect ) ) {
            // pacmonster->direction = DIR_UP;
        // }
    }
    else if( current_key_states[ SDL_SCANCODE_DOWN ] ) {
        SDL_Rect pac_extension_rect = { pacmonster->collision_rect.x, pacmonster->collision_rect.y + pacmonster->collision_rect.h, pacmonster->collision_rect.w, 1 };

        // COLLISIONS IN TILEMAP
        SDL_bool collided = SDL_FALSE;
        
        for ( int row = 0; row < TILE_ROWS; ++row ) {
            for ( int col = 0; col < TILE_COLS; ++col ) {
                if( tile_map[ row ][ col ] == 'x') 
                {
                    SDL_Rect current_tile_rect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};

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

        // if ( ! SDL_HasIntersection( &pac_extension_rect, world_rect ) ) {
        //     pacmonster->direction = DIR_DOWN;
        // }
    }
    else if( current_key_states[ SDL_SCANCODE_LEFT ] ) {
        SDL_Rect pac_extension_rect = { pacmonster->collision_rect.x - 1, pacmonster->collision_rect.y, 1, pacmonster->collision_rect.h };

        // COLLISIONS IN TILEMAP
        SDL_bool collided = SDL_FALSE;
        
        for ( int row = 0; row < TILE_ROWS; ++row ) {
            for ( int col = 0; col < TILE_COLS; ++col ) {
                if( tile_map[ row ][ col ] == 'x') 
                {
                    SDL_Rect current_tile_rect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};

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
        // END COLLISIONS IN TILEMAP

        // if ( ! SDL_HasIntersection( &pac_extension_rect, world_rect ) ) {
        //     pacmonster->direction = DIR_LEFT;
        // }
    }
    else if( current_key_states[ SDL_SCANCODE_RIGHT ] ) {
        SDL_Rect pac_extension_rect = { pacmonster->collision_rect.x + pacmonster->collision_rect.w, pacmonster->collision_rect.y, 1, pacmonster->collision_rect.h };

        // COLLISIONS IN TILEMAP
        SDL_bool collided = SDL_FALSE;
        
        for ( int row = 0; row < TILE_ROWS; ++row ) {
            for ( int col = 0; col < TILE_COLS; ++col ) {
                if( tile_map[ row ][ col ] == 'x') 
                {
                    SDL_Rect current_tile_rect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};

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

        // if ( ! SDL_HasIntersection( &pac_extension_rect, world_rect ) ) {
        //     pacmonster->direction = DIR_RIGHT;
        // }
    }
    else if (current_key_states[ SDL_SCANCODE_SPACE ] ){
        pacmonster->direction = DIR_NONE;
    }
}

void pac_try_move( Pacmonster *pacmonster,  char tile_map[ TILE_ROWS ][ TILE_COLS ], float delta_time ) {
    
    int pac_speed = 275;
    switch( pacmonster->direction ){

        case DIR_UP:
            pacmonster->position.y -= pac_speed * delta_time;
            pacmonster->collision_rect.y = pacmonster->position.y;

            // top screen bound
            if( pacmonster->collision_rect.y < 0 ) {
                pacmonster->position.y = 0;
                pacmonster->collision_rect.y = pacmonster->position.y;
            }

            // tile_map bound
            for ( int row = 0; row < TILE_ROWS; ++row ) {
                for ( int col = 0; col < TILE_COLS; ++col ) {
                    if( tile_map[ row ][ col ] == 'x') 
                    {
                        SDL_Rect current_tile_rect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};

                        if ( SDL_HasIntersection( &pacmonster->collision_rect, &current_tile_rect ) ) 
                        {
                            pacmonster->position.y = current_tile_rect.y + current_tile_rect.h;
                            pacmonster->collision_rect.y = pacmonster->position.y;
            
                            break;
                        }
                    }
                }
            }

            // other rect bound
            // if ( SDL_HasIntersection( &pacmonster->collision_rect, world_rect ) ) {
            //     pacmonster->position.y = world_rect->y + world_rect->h;
            //     pacmonster->collision_rect.y = pacmonster->position.y;
            // }
            break;

        case DIR_DOWN:
            pacmonster->position.y += pac_speed * delta_time;
            pacmonster->collision_rect.y = pacmonster->position.y;
            // bottom screen bound
            if( pacmonster->collision_rect.y + pacmonster->collision_rect.h > SCREEN_HEIGHT ) {
                pacmonster->position.y = SCREEN_HEIGHT - pacmonster->collision_rect.h;
                pacmonster->collision_rect.y = pacmonster->position.y;
            }

            // tile_map bound
            for ( int row = 0; row < TILE_ROWS; ++row ) {
                for ( int col = 0; col < TILE_COLS; ++col ) {
                    if( tile_map[ row ][ col ] == 'x') 
                    {
                        SDL_Rect current_tile_rect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};

                        if ( SDL_HasIntersection( &pacmonster->collision_rect, &current_tile_rect ) ) 
                        {
                            pacmonster->position.y = current_tile_rect.y - pacmonster->collision_rect.h;
                            pacmonster->collision_rect.y = pacmonster->position.y;
            
                            break;
                        }
                    }
                }
            }

            // other rect bound
            // if ( SDL_HasIntersection( &pacmonster->collision_rect, world_rect ) ) {
            //     pacmonster->position.y = world_rect->y - pacmonster->collision_rect.h;
            //     pacmonster->collision_rect.y = pacmonster->position.y;
            // }
            break;

        case DIR_LEFT:
            pacmonster->position.x -= pac_speed * delta_time;
            pacmonster->collision_rect.x = pacmonster->position.x;
            // left screen bound
            if( pacmonster->collision_rect.x < 0 ) {
                pacmonster->position.x = 0;
                pacmonster->collision_rect.x = pacmonster->position.x;
            }

            // tile_map bound
            for ( int row = 0; row < TILE_ROWS; ++row ) {
                for ( int col = 0; col < TILE_COLS; ++col ) {
                    if( tile_map[ row ][ col ] == 'x') 
                    {
                        SDL_Rect current_tile_rect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};

                        if ( SDL_HasIntersection( &pacmonster->collision_rect, &current_tile_rect ) ) 
                        {
                            pacmonster->position.x = current_tile_rect.x + current_tile_rect.w;
                            pacmonster->collision_rect.x = pacmonster->position.x;
            
                            break;
                        }
                    }
                }
            }

            // other rect bound
            // if ( SDL_HasIntersection( &pacmonster->collision_rect, world_rect ) ) {
            //     pacmonster->position.x = world_rect->x + world_rect->w;
            //     pacmonster->collision_rect.x = pacmonster->position.x;
            // }
            break;

        case DIR_RIGHT:
            pacmonster->position.x += pac_speed * delta_time;
            pacmonster->collision_rect.x = pacmonster->position.x;
            // right screen bound
            if( pacmonster->collision_rect.x + pacmonster->collision_rect.w > SCREEN_WIDTH  ) {
                pacmonster->position.x = SCREEN_WIDTH - pacmonster->collision_rect.w;
                pacmonster->collision_rect.x = pacmonster->position.x;
            }

            // tile_map bound
            for ( int row = 0; row < TILE_ROWS; ++row ) {
                for ( int col = 0; col < TILE_COLS; ++col ) {
                    if( tile_map[ row ][ col ] == 'x') 
                    {
                        SDL_Rect current_tile_rect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};

                        if ( SDL_HasIntersection( &pacmonster->collision_rect, &current_tile_rect ) ) 
                        {
                            pacmonster->position.x = current_tile_rect.x - pacmonster->collision_rect.w;
                            pacmonster->collision_rect.x = pacmonster->position.x;

                            break;
                        }
                    }
                }
            }

            // other rect bound
            // if ( SDL_HasIntersection( &pacmonster->collision_rect, world_rect ) ) {
            //     pacmonster->position.x = world_rect->x - pacmonster->collision_rect.w;
            //     pacmonster->collision_rect.x = pacmonster->position.x;
            // }
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

    
    // adjust the frame so that it doesn't play too fast. Each  frame will play five times, then move to the next
    //int frame = ( int ) pacmonster->current_animation_frame; // frame is a float because need to account for delta time
    SDL_Rect pac_rect = {pacmonster->position.x - 8, pacmonster->position.y - 8, pac_size, pac_size };
    SDL_RenderCopyEx(renderer, pacmonster->texture_atlas, &pacmonster->pac_sprite_clips[ pacmonster->current_animation_frame ], &pac_rect, pac_rotation, NULL, pac_flip );


    // DEBUG
    
    SDL_SetRenderDrawColor( renderer, 255,255,0,200 );
    SDL_RenderFillRect( renderer, &pacmonster->collision_rect);

}

#endif