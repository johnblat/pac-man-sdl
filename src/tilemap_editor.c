// MAIN
#include <SDL2/SDL.h>
#include <stdio.h>
#include "tiles.h"
#include "jb_types.h"



int main( int argc, char *argv[] ) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Point mouse_point;
    TileMap tilemap;
    TwoDimensionalArrayIndex selected_texture_atlas_index = {0, 0};
    
    char *filename = "level";

    // Initializing stuff
    if( SDL_Init( SDL_INIT_VIDEO ) < 0) {
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

    tm_init_and_load_texture( renderer, &tilemap, "level02" );
   
    SDL_Event event;
    int quit = 0;

    int left_button_pressed = 0;
    int right_button_pressed = 0;

    while (!quit) {
        while (SDL_PollEvent( &event ) != 0 ) {
            if( event.type == SDL_QUIT ) {
                quit = 1;
            }
            if ( event.type == SDL_KEYDOWN ) {
                if ( event.key.keysym.sym == SDLK_SPACE ) {
                    save_tilemap_texture_atlas_indexes_to_file( tilemap.tm_texture_atlas_indexes );
                }
            }
            if ( event.type == SDL_MOUSEBUTTONDOWN ) {
                if (  event.button.button == ( SDL_BUTTON_LEFT ) ) {
                    left_button_pressed = 1;
                }
                else if ( event.button.button == ( SDL_BUTTON_RIGHT ) ) {
                    right_button_pressed = 1;
                }
                
            }
            if ( event.type == SDL_MOUSEBUTTONUP ) {
                if (  event.button.button == ( SDL_BUTTON_LEFT ) ) {
                    left_button_pressed = 0;
                }
                else if ( event.button.button == ( SDL_BUTTON_RIGHT ) ) {
                    right_button_pressed = 0;
                }
            }
        }

        if ( left_button_pressed ) {
            SDL_GetMouseState( &mouse_point.x, &mouse_point.y );
            SDL_Point tile_grid_point;
            tile_grid_point = screen_point_to_tile_grid_point( mouse_point, tilemap.tm_screen_position );

            if ( tile_grid_point.x > TILE_COLS - 1) {
                tile_grid_point.x = TILE_COLS - 1;
            }
            if ( tile_grid_point.y > TILE_ROWS - 1) {
                tile_grid_point.y = TILE_ROWS - 1;
            }
            
            //TODO: add selected tile to position
            //tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = 'x';
        }
        else if ( right_button_pressed ) {
            SDL_GetMouseState( &mouse_point.x, &mouse_point.y );
            SDL_Point tile_grid_point;
            tile_grid_point.x = mouse_point.x / TILE_SIZE;//* 0.015625;
            tile_grid_point.y = mouse_point.y / TILE_SIZE;//* 0.015625;
            if ( tile_grid_point.x > TILE_COLS - 1) {
                tile_grid_point.x = TILE_COLS - 1;
            }
            if ( tile_grid_point.y > TILE_ROWS - 1) {
                tile_grid_point.y = TILE_ROWS - 1;
            }
            //TODO Add selected tile to position
            //tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = '\0';
        }

        SDL_SetRenderDrawColor( renderer, 0,0,0,255);
        SDL_RenderClear( renderer );

        tm_render_with_screen_position_offset( renderer, &tilemap );

        SDL_RenderPresent(renderer);
        
    
    }
    // CLOSE DOWN
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();
}