#ifndef TILES_H
#define TILES_H

/**
 *  TODOs
 *  [x] 2021-02-22 : Need to  make this portable, so use SDL IO functions instead of unix based file access
 */
#include <SDL2/SDL.h>
#include "jb_types.h"

const int TILE_SIZE = 48;
const int TILE_COLS = SCREEN_WIDTH * ( TILE_SIZE * 0.001 );
const int TILE_ROWS = SCREEN_HEIGHT * ( TILE_SIZE * 0.001 );
const int TOTAL_NUMBER_OF_TILES = TILE_ROWS * TILE_COLS;

const int MAX_TILEMAP_CHARS_IN_FILE = TOTAL_NUMBER_OF_TILES + TILE_ROWS; // ROWS for newline



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

void tile_map_init( SDL_Rect tile_map[TILE_ROWS][TILE_COLS] ) {
    for( int row = 0; row < TILE_ROWS; ++row ) {
        for( int col = 0; col < TILE_COLS; ++col ) {
            tile_map[ row ][ col ].x = col * TILE_SIZE;
            tile_map[ row ][ col ].y = row * TILE_SIZE;
            tile_map[ row ][ col ].w = TILE_SIZE;
            tile_map[ row ][ col ].h = TILE_SIZE;
        }
    }
}

void tiles_render( SDL_Renderer *renderer, char tile_map[ TILE_ROWS ][ TILE_COLS ] ) {
    //int padding_top_ui_height = TILE_SIZE * 2;
    
    SDL_SetRenderDrawColor( renderer, 255,255,255,255);
    for ( int row = 0; row < TILE_ROWS; ++row ) {
        for( int col = 0; col < TILE_COLS; ++col ) {
            if ( tile_map[ row ][ col ] == 'x') {
                SDL_Rect rect = {col * TILE_SIZE, row * TILE_SIZE , TILE_SIZE, TILE_SIZE};
                SDL_RenderFillRect( renderer, &rect);
            } 
            
        }
    } 
}

void create_empty_file(  ) {

}

void load_tile_map_from_file( char tile_map_to_load[ TILE_ROWS ][ TILE_COLS ] ) {
    char *filename = "level";
    char *contents = ( char * ) malloc( sizeof( char ) * MAX_TILEMAP_CHARS_IN_FILE );

    SDL_RWops *file = SDL_RWFromFile( filename, "r");
    if ( file == NULL ) {
        // file = SDL_RWFromFile( filename, "W+");
        // char spaces[ MAX_TILEMAP_CHARS_IN_FILE ];
        int i = 0;
        for ( int row = 0; row < TILE_ROWS; ++row ) {
            for ( int col = 0; col < TILE_COLS; ++col ) {
                tile_map_to_load[ row ][ col ] = ' ';
            }
        }

        return;
        
    }

    size_t num_bytes_read = SDL_RWread( file, contents, sizeof( char ), MAX_TILEMAP_CHARS_IN_FILE );
    SDL_RWclose( file );

    // read in file to 2D tilemap
    int row = 0;
    int col = 0;
    for( int i = 0; i < MAX_TILEMAP_CHARS_IN_FILE; ++i ) {
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

    
} 

#endif