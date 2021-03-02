// MAIN
#include <SDL2/SDL.h>
#include <stdio.h>
#include "tiles.h"
#include "jb_types.h"


void save_level( char tile_map[ TILE_ROWS ][ TILE_COLS ] ) {
    int row = 0;
    int col = 0;

    char contents[ MAX_TILEMAP_CHARS_IN_FILE ];
    int i = 0;
    for ( int row = 0; row < TILE_ROWS; ++row ) {
        for ( int col = 0; col < TILE_COLS; ++col ) {
            if ( tile_map[ row ][ col ] == 'x' ) {
                contents[ i ] = 'x';
            }
            else {
                contents[ i ] =' ';
            }
            ++i;
        }
        contents[ i ] = '\n';
        ++i;
    }

    SDL_RWops *file = SDL_RWFromFile("level", "w");
    size_t num_bytes_wrote = SDL_RWwrite( file, contents, sizeof( char ), MAX_TILEMAP_CHARS_IN_FILE );
    SDL_RWclose( file );

    printf("Result : %lu\n", num_bytes_wrote);
}

// int file_size( int fd ) {
//     return lseek(fd, 0, SEEK_END);
// }

int main( int argc, char *argv[] ) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Point mouse_point;
    char tile_map[TILE_ROWS][TILE_COLS];
    char *filename = "level";

    // Initializing stuff
    if( SDL_Init( SDL_INIT_VIDEO ) < 0) {
        fprintf( stderr, "Error %s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    for ( int row = 0; row < TILE_ROWS; ++row ) {
        for( int col = 0; col < TILE_COLS; ++col ) {
            tile_map[ row ][ col ] = '\0';
        }
    }

    load_tile_map_from_file( tile_map );
    
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
                    save_level( tile_map );
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
            tile_grid_point.x = mouse_point.x / TILE_SIZE;//* 0.015625;
            tile_grid_point.y = mouse_point.y / TILE_SIZE;//* 0.015625;
            if ( tile_grid_point.x > TILE_COLS - 1) {
                tile_grid_point.x = TILE_COLS - 1;
            }
            if ( tile_grid_point.y > TILE_ROWS - 1) {
                tile_grid_point.y = TILE_ROWS - 1;
            }
            tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = 'x';
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
            tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = '\0';
        }

        SDL_SetRenderDrawColor( renderer, 0,0,0,255);
        SDL_RenderClear( renderer );

        SDL_SetRenderDrawColor( renderer, 255,255,255,255);
        for ( int row = 0; row < TILE_ROWS; ++row ) {
            for( int col = 0; col < TILE_COLS; ++col ) {
                if ( tile_map[ row ][ col ] == 'x') {
                    SDL_Rect rect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    SDL_RenderFillRect( renderer, &rect);
                }
                
            }
        } 

        SDL_RenderPresent(renderer);
        
    
    }
    // CLOSE DOWN
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();
}