#ifndef TILES_H
#define TILES_H


#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "jb_types.h"
#include "constants.h"

/** 
 * TILE VALUES, DIMENSIONS, ETC
 */
const int TILE_SIZE = 40; // change to different size if necessary
const int TILE_COLS = SCREEN_WIDTH / (TILE_SIZE );
const int TILE_ROWS = SCREEN_HEIGHT / ( TILE_SIZE ) - 2;
const int TOTAL_NUMBER_OF_TILES = TILE_ROWS * TILE_COLS;
const int DOT_SIZE = 5;
const int DOT_RADIUS = DOT_SIZE / 2;
const int DOT_PADDING = 15;
const float DOT_SPEED = 12;

/**
 * This is used even for dots that are not on the screen
 * because dots can appear at anytime and they must remain in sync 
 * with the other dots
 */
typedef struct DotStuff {
    Position_f position;
    Vector_f velocity;
} DotStuff;

/**
 * STRUCTS
 */
typedef struct TwoDimensionalArrayIndex {
    int r, c;
} TwoDimensionalArrayIndex;

const TwoDimensionalArrayIndex EMPTY_TILE_TEXTURE_ATLAS_INDEX = { -1, -1 };

typedef struct TileMap {
    /** The screen position where the tilemap begins
     * This can be used for offset calculations
     */
    SDL_Point tm_screen_position;
    /**
     * The entire Texture atlas to be used for the tilemap.
     * Should be 2 x 18 tiles in size
     * To be indexed in row/col fashion
     */
    SDL_Texture *tm_texture_atlas;
    /**
     * This ultimately represents the state of each tile in the tilemap
     * It will either pull from a tile in the texture atlas, or it won't ( it will be (-1, -1 ) )
     */
    TwoDimensionalArrayIndex tm_texture_atlas_indexes[ TILE_ROWS ][ TILE_COLS ];

    /**
     * If there is an 'x' in the position, then that means there's a dot in the tile.
     * This ought to change in the future once that fuck stephen gets me a dot sprite
     * Or , who knows, maybe i can go with this
     */
    char tm_dots[ TILE_ROWS ][ TILE_COLS ];

    /**
     * If the dots animate or move, this will update
     * Think of making this a pointer array for DotStuff, then use NULL to represent no dot at the position
     * However, then i would lose the velocity
     */
    DotStuff tm_dot_stuff[ TILE_ROWS ][ TILE_COLS ];
    /**
     * Members I might want
     * SDL_Rect tm_tile_collision_rects[ TILE_ROWS ][ TILE_COLS ];
     * SDL_Point tm_tile_screen_positions[ TILE_ROWS ][ TILE_COLS ];
     */

} TileMap;

/**
 * FILE I/O FOR SAVING/LOADING
 */

void save_tilemap_texture_atlas_indexes_to_file( TwoDimensionalArrayIndex tm_texture_atlas_indexes[ TILE_ROWS ][ TILE_COLS ] ) ;

void try_load_tilemap_texture_atlas_indexes_from_file( TwoDimensionalArrayIndex tm_texture_atlas_indexes[ TILE_ROWS ][ TILE_COLS ] ) ;

void save_dots_to_file( char dots[ TILE_ROWS ][ TILE_COLS ] ) ;

void try_load_dots_from_file( char dots[ TILE_ROWS ][ TILE_COLS ] );
/**
 * INITIALIZATION for tm ( tilemap )
 * 
 * @level_filename: 
 *  - NULL: will initialize an empty tilemap 
 *  - NOT NULL: will load the tilemap for the level of the filename passed
 */
void tm_init_and_load_texture( SDL_Renderer *renderer, TileMap *tm, char *level_filename ) ;
/**
 * CONVERTING TILE MAP GRID COORDINATES AND SCREEN COORDINATES
 */

SDL_Point tile_grid_point_to_screen_point( SDL_Point tile_grid_point, SDL_Point tile_map_screen_position ) ;

SDL_Point screen_point_to_tile_grid_point( SDL_Point screen_point, SDL_Point tile_map_screen_position ) ;
/**
 * RENDERING
 */
void tm_render_with_screen_position_offset( SDL_Renderer *renderer, TileMap *tm );

#endif