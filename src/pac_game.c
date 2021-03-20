#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "jb_types.h"
#include "constants.h"
#include "pacmonster.h"
#include "tiles.h"

SDL_bool g_show_debug_info = SDL_FALSE;

int main( int argc, char *argv[] ) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    Pacmonster *pacmonster;
    TileMap tilemap;

    // Initializing stuff
    if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf( stderr, "Error %s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    window = SDL_CreateWindow( "JB Pacmonster", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
    if ( window == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    if (renderer == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (! ( IMG_Init( IMG_INIT_PNG ) & IMG_INIT_PNG ) ) {
        fprintf( stderr, "Error %s\n ", IMG_GetError() );
        exit( EXIT_FAILURE );
    }
    
    

    // INIT PACMONSTER
    pacmonster = init_pacmonster( renderer );
    AnimationTimer animation_timer;
    animation_timer.frame_interval = 0.08f;
    animation_timer.accumulator = 0.0f;
    
    // INIT TILEMAP
    tm_init_and_load_texture( renderer, &tilemap, "maze_file" );

    // PREPARE VARIABLES FOR LOOP
    SDL_Event event;
    int quit = 0;

    // delta time
    float time = 0.0;
    float max_delta_time = 1 / 60.0;
    float previous_frame_ticks = SDL_GetTicks() / 1000.0;

    while (!quit) {

        // semi-fixed timestep
        float current_frame_ticks = SDL_GetTicks() / 1000.0;
        float delta_time = current_frame_ticks - previous_frame_ticks;
        previous_frame_ticks = current_frame_ticks;
        // adjust for any pauses 
        delta_time = delta_time < max_delta_time ?  delta_time : max_delta_time;

        // EVENTS
        while (SDL_PollEvent( &event ) != 0 ) {
            if( event.type == SDL_QUIT ) {
                quit = 1;
            }
            if ( event.type == SDL_KEYDOWN ) {
                if ( event.key.keysym.sym == SDLK_b ) {
                    g_show_debug_info = !g_show_debug_info;
                }
            }
        }

        // KEYBOARD STATE

        const Uint8 *current_key_states = SDL_GetKeyboardState( NULL );

        // UPDATE PACMONSTER
        pac_try_set_direction( pacmonster, current_key_states, &tilemap);
        pac_try_move( pacmonster, &tilemap, delta_time );

        // RENDER
        SDL_SetRenderDrawColor( renderer, 0,0,0,255);
        SDL_RenderClear( renderer );    

        tm_render_with_screen_position_offset( renderer, &tilemap );

        pac_render( renderer, pacmonster );

        SDL_SetRenderDrawColor( renderer, 255,255,255,255);

        pac_inc_animation_frame( pacmonster, &animation_timer, delta_time);


        // DEBUG
        if ( g_show_debug_info ) {
            SDL_SetRenderDrawColor( renderer, 255,100,100,255);
            for ( int y = 0; y < SCREEN_HEIGHT; y+= TILE_SIZE ) {
                SDL_RenderDrawLine( renderer, 0, y, SCREEN_WIDTH, y);
            }
            for ( int x = 0; x < SCREEN_WIDTH; x+= TILE_SIZE) {
                SDL_RenderDrawLine( renderer, x, 0, x, SCREEN_HEIGHT );
            }

            SDL_SetRenderDrawColor( renderer, 255,255,0,150 );
            SDL_RenderFillRect( renderer, &pacmonster->collision_rect);

            SDL_SetRenderDrawColor( renderer, 255, 0, 255,120);
            SDL_Rect tile_rect = { pacmonster->current_tile.x * TILE_SIZE, pacmonster->current_tile.y * TILE_SIZE, TILE_SIZE,TILE_SIZE};
            SDL_RenderFillRect( renderer, &tile_rect);
        }
    
    
        SDL_RenderPresent( renderer );
    }



    // CLOSE DOWN
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_DestroyTexture( pacmonster->texture_atlas );
    SDL_Quit();
    
}
