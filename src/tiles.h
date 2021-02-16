#ifndef TILES_H
#define TILES_H

#include <SDL2/SDL.h>
#include "jb_types.h"

const int TILE_SIZE = 32;
const int TILE_COLS = SCREEN_WIDTH * ( TILE_SIZE * 0.01 );
const int TILE_ROWS = SCREEN_HEIGHT * ( TILE_SIZE * 0.01 );



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

#endif