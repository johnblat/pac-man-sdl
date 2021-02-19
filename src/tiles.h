#ifndef TILES_H
#define TILES_H

#include <SDL2/SDL.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>


#include "jb_types.h"

const int TILE_SIZE = 64;
const int TILE_COLS = SCREEN_WIDTH * ( TILE_SIZE * 0.01 );
const int TILE_ROWS = SCREEN_HEIGHT * ( TILE_SIZE * 0.01 );
const int TOTAL_NUMBER_OF_TILES = TILE_ROWS * TILE_COLS;

const int MAX_TILEMAP_FILESIZE = TOTAL_NUMBER_OF_TILES + TILE_ROWS; // ROWS for newline



// typedef struct Tile {
//     SDL_Point tile_grid_position; // will be a point where x is (0..tile_cols) and y is (0..tile_rows)
//     SDL_Rect texture_sheet_clip;
//     SDL_Rect collision_rect; // x & y would be tile_grid_position * 64. Width and height would be 64 x 64
// } Tile;

// typedef struct TileMap{
//     int tile_size;
//     int tile_rows;
//     int tile_cols;
//     SDL_Texture *tile_texture_sheet;
//     Tile **tiles;
// }TileMap;

//SDL_Rect *TileMap[TILE_ROWS][TILE_COLS];

void tile_map_init( SDL_Rect TileMap[TILE_ROWS][TILE_COLS] ) {
    for( int row = 0; row < TILE_ROWS; ++row ) {
        for( int col = 0; col < TILE_COLS; ++col ) {
            TileMap[ row ][ col ].x = col * TILE_SIZE;
            TileMap[ row ][ col ].y = row * TILE_SIZE;
            TileMap[ row ][ col ].w = TILE_SIZE;
            TileMap[ row ][ col ].h = TILE_SIZE;
        }
    }
}

void tiles_render( SDL_Renderer *renderer, char tile_map[ TILE_ROWS ][ TILE_COLS ] ) {
    SDL_SetRenderDrawColor( renderer, 255,255,255,255);
    for ( int row = 0; row < TILE_ROWS; ++row ) {
        for( int col = 0; col < TILE_COLS; ++col ) {
            if ( tile_map[ row ][ col ] == 'x') {
                SDL_Rect rect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                SDL_RenderFillRect( renderer, &rect);
            }
            
        }
    } 
}

void load_tile_map_from_file( char tile_map_to_load[ TILE_ROWS ][ TILE_COLS ] ) {
    char *filename = "level";
    int fd = open(filename,  O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR );

    char *contents = mmap( NULL, MAX_TILEMAP_FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if ( contents == NULL ) {
        printf("here\n");
    }
    // read in file to 2D tilemap
    int row = 0;
    int col = 0;
    for( int i = 0; i < MAX_TILEMAP_FILESIZE; ++i ) {
        if ( contents[ i ] == 'x' ) {
            tile_map_to_load[ row ][ col ] = 'x';
            ++col;
        }
        else if ( contents[ i ] == ' ') {
            tile_map_to_load[ row ][ col ] = '\0';
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

    close( fd );
} 

#endif