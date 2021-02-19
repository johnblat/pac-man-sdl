// MAIN
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "tiles.h"
#include "jb_types.h"


const int ROWS = 31;
const int COLS = 28;
//const int TOTAL_NUMBER_OF_TILES = ROWS * COLS;

void save_level( char tile_map[ ROWS ][ COLS ], char *contents ) {
    //char contents[ MAX_FILESIZE ];
    int row = 0;
    int col = 0;

    int i = 0;
    for ( int row = 0; row < ROWS; ++row ) {
        for ( int col = 0; col < COLS; ++col ) {
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
    //memcpy( addr, contents, MAX_FILESIZE);
    // Why isn't this syncing with the file
    int result = msync( contents, MAX_TILEMAP_FILESIZE, MS_SYNC );
    printf("Result : %d\n", result);
}

int file_size( int fd ) {
    return lseek(fd, 0, SEEK_END);
}

int main() {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Point mouse_point;
    char tile_map[ROWS][COLS];
    char *filename = "level";
    int fd = open(filename,  O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR );

    // Initializing stuff
    if( SDL_Init( SDL_INIT_VIDEO ) < 0) {
        fprintf( stderr, "Error %s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    // TODO Fill tilemap with null characters 
    for ( int row = 0; row < ROWS; ++row ) {
                for( int col = 0; col < COLS; ++col ) {
                    tile_map[ row ][ col ] = '\0';
                }
            } 
    // is file empty?
    if ( file_size(fd) == 0 ) {
        for ( int i = 0; i < MAX_TILEMAP_FILESIZE; ++i) {
            write(fd, " ", 1);
        }
    }
    

    char *contents = mmap( NULL, MAX_TILEMAP_FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if ( contents == NULL ) {
        printf("here\n");
    }
    // read in file to 2D tilemap
    int row = 0;
    int col = 0;
    for( int i = 0; i < MAX_TILEMAP_FILESIZE; ++i ) {
        if ( contents[ i ] == 'x' ) {
            tile_map[ row ][ col ] = 'x';
            ++col;
        }
        else if ( contents[ i ] == ' ') {
            tile_map[ row ][ col ] = '\0';
            ++col;
        }
        else if ( contents[ i ] == '\n') {
            ++row;
            col = 0;
        }
        else {
            break;
        }
        printf("%c", contents[ i ]);
        
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
                    save_level( tile_map, contents );
                }
            }
            if ( event.type == SDL_MOUSEBUTTONDOWN ) {
                if (  event.button.button == ( SDL_BUTTON_LEFT ) ) {
                    left_button_pressed = 1;
                    // SDL_GetMouseState( &mouse_point.x, &mouse_point.y );
                    // SDL_Point tile_grid_point;
                    // tile_grid_point.x = mouse_point.x / 32;//* 0.015625;
                    // tile_grid_point.y = mouse_point.y / 32;//* 0.015625;
                    // tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = 'x';
                }
                else if ( event.button.button == ( SDL_BUTTON_RIGHT ) ) {
                    right_button_pressed = 1;
                    // SDL_GetMouseState( &mouse_point.x, &mouse_point.y );
                    // SDL_Point tile_grid_point;
                    // tile_grid_point.x = mouse_point.x / 32;//* 0.015625;
                    // tile_grid_point.y = mouse_point.y / 32;//* 0.015625;
                    // tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = '\0';
                }
                // SDL_GetMouseState( &mouse_point.x, &mouse_point.y );
                // SDL_Point tile_grid_point;
                // tile_grid_point.x = mouse_point.x / 32;//* 0.015625;
                // tile_grid_point.y = mouse_point.y / 32;//* 0.015625;
                // if ( tile_map[ tile_grid_point.y ][ tile_grid_point.x ] == '\0' ) {
                //     tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = 'x';
                // } 
                // else {
                //     tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = '\0';
                // }
                
            }
            if ( event.type == SDL_MOUSEBUTTONUP ) {
                if (  event.button.button == ( SDL_BUTTON_LEFT ) ) {
                    left_button_pressed = 0;
                    // SDL_GetMouseState( &mouse_point.x, &mouse_point.y );
                    // SDL_Point tile_grid_point;
                    // tile_grid_point.x = mouse_point.x / 32;//* 0.015625;
                    // tile_grid_point.y = mouse_point.y / 32;//* 0.015625;
                    // tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = 'x';
                }
                else if ( event.button.button == ( SDL_BUTTON_RIGHT ) ) {
                    right_button_pressed = 0;
                    // SDL_GetMouseState( &mouse_point.x, &mouse_point.y );
                    // SDL_Point tile_grid_point;
                    // tile_grid_point.x = mouse_point.x / 32;//* 0.015625;
                    // tile_grid_point.y = mouse_point.y / 32;//* 0.015625;
                    // tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = '\0';
                }
            }
        }

        if ( left_button_pressed ) {
            SDL_GetMouseState( &mouse_point.x, &mouse_point.y );
            SDL_Point tile_grid_point;
            tile_grid_point.x = mouse_point.x / TILE_SIZE;//* 0.015625;
            tile_grid_point.y = mouse_point.y / TILE_SIZE;//* 0.015625;
            if ( tile_grid_point.x > COLS - 1) {
                tile_grid_point.x = COLS - 1;
            }
            if ( tile_grid_point.y > ROWS - 1) {
                tile_grid_point.y = ROWS - 1;
            }
            tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = 'x';
        }
        else if ( right_button_pressed ) {
            SDL_GetMouseState( &mouse_point.x, &mouse_point.y );
            SDL_Point tile_grid_point;
            tile_grid_point.x = mouse_point.x / TILE_SIZE;//* 0.015625;
            tile_grid_point.y = mouse_point.y / TILE_SIZE;//* 0.015625;
            if ( tile_grid_point.x > COLS - 1) {
                tile_grid_point.x = COLS - 1;
            }
            if ( tile_grid_point.y > ROWS - 1) {
                tile_grid_point.y = ROWS - 1;
            }
            tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = '\0';
        }


        //Uint32 mouse_button_state = SDL_GetMouseState(&mouse_point.x, &mouse_point.y);
        //if( mouse_button_state ) {
            // if( mouse_button_state & SDL_BUTTON( SDL_BUTTON_LEFT ) ) {
            //     printf("here!\n");
            //     SDL_Point tile_grid_point;
            //     tile_grid_point.x = mouse_point.x * 0.015625; // 1/64
            //     tile_grid_point.y = mouse_point.y * 0.015625; // 1/64
            //     tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = 'x';

            //     for ( int row = 0; row < 14; ++row ) {
            //         for( int col = 0; col < 15; ++col ) {
            //             printf("%c", tile_map[ row ][ col ] );
            //         }
            //         printf("\n");
            //     }
            // } 
            // else if ( mouse_button_state & SDL_BUTTON( SDL_BUTTON_RIGHT ) ) {
            //     SDL_Point tile_grid_point;
            //     tile_grid_point.x = mouse_point.x * 0.015625; // 1/64
            //     tile_grid_point.y = mouse_point.y * 0.015625; // 1/64
            //     tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = '\0';
            // }
        //printf("%s\n", SDL_GetError());
        SDL_SetRenderDrawColor( renderer, 0,0,0,255);
        SDL_RenderClear( renderer );

        SDL_SetRenderDrawColor( renderer, 255,255,255,255);
        for ( int row = 0; row < ROWS; ++row ) {
            for( int col = 0; col < COLS; ++col ) {
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