#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "jb_types.h"
#include "pacmonster.h"
#include "tiles.h"


int main() {
    SDL_Window *window;
    SDL_Renderer *renderer;
    Pacmonster *pacmonster;
    char tile_map[ TILE_ROWS ][ TILE_COLS ];
    SDL_Rect rect_test = { SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5, 192, 192}; // to test collisions

    // Initializing stuff
    if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf( stderr, "Error %s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    window = SDL_CreateWindow( "JB Pacmonster", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if ( window == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    if (renderer == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    if (! ( IMG_Init( IMG_INIT_PNG ) & IMG_INIT_PNG ) ) {
        fprintf( stderr, "Error %s\n ", IMG_GetError() );
        exit( EXIT_FAILURE );
    }

    // INIT PACMONSTER
    pacmonster = init_pacmonster( renderer );

    // INIT TILEMAP
    load_tile_map_from_file( tile_map );

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
        }

        // KEYBOARD STATE

        const Uint8 *current_key_states = SDL_GetKeyboardState( NULL );

        // UPDATE PACMONSTER
        pac_try_set_direction( pacmonster, current_key_states, &rect_test, tile_map);
        pac_try_move( pacmonster, &rect_test, tile_map, delta_time );

        // RENDER
        pac_render( renderer, pacmonster );
        tiles_render( renderer, tile_map );

        SDL_SetRenderDrawColor( renderer, 255,255,255,255);
        SDL_RenderFillRect( renderer, &rect_test );

        pac_inc_animation_frame( pacmonster, delta_time);
        SDL_RenderPresent( renderer );
    }



    // CLOSE DOWN
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_DestroyTexture( pacmonster->texture_atlas );
    SDL_Quit();
    
}