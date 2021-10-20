#ifndef TILES_H
#define TILES_H


#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "jb_types.h"
#include "constants.h"


/** 
 * TILE VALUES, DIMENSIONS, ETC
 */
#define TILE_SIZE 40 // change to different size if necessary
#define TILE_COLS 48
#define TILE_ROWS 26
#define TOTAL_NUMBER_OF_TILES ( TILE_ROWS * TILE_COLS )

#define DOT_SIZE  8 
#define DOT_RADIUS  ( DOT_SIZE / 2 )
#define DOT_PADDING 15
#define DOT_SPEED  35
#define DOT_SCORE_VALUE 10

/**
 * This is used even for dots that are not on the screen
 * because dots can appear at anytime and they must remain in sync 
 * with the other dots
 */
typedef struct {
    Position position;
    Velocity velocity;
} DotParticle;

/**
 * STRUCTS
 */
typedef struct idx2D {
    int r, c;
} idx2D;

extern const idx2D EMPTY_TILE_TEXTURE_ATLAS_INDEX;
extern const SDL_Point TILE_NONE;
extern const int MAX_SLOW_TILES;


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
     * This ultimately represents the texture clip of each tile in the tilemap
     * It will either pull from a tile in the texture atlas, or it won't ( it will be (-1, -1 ) )
     */
    idx2D tm_texture_atlas_indexes[ TILE_ROWS ][ TILE_COLS ];

    /**
     * Represents a tile that pacman and ghosts cannot move through
     */
    char tm_walls[ TILE_ROWS ][ TILE_COLS ];

    /**
     * If there is an 'x' in the position, then that means there's a dot in the tile.
     * This ought to change in the future once that fuck stephen gets me a dot sprite
     * Or , who knows, maybe i can go with this
     */
    char tm_dots[ TILE_ROWS ][ TILE_COLS ];

    /**
     * If the dots animate or move, this will update
     * Think of making this a pointer array for DotParticle, then use NULL to represent no dot at the position
     * However, then i would lose the velocity
     */
    DotParticle tm_dot_particles[ TILE_ROWS ][ TILE_COLS ];

    /**
     * Contains the tile coordinates of power pellets
     * If there are no power pellets in an index, then the value should be { -1, -1 }
     */
    SDL_Point tm_power_pellet_tiles[ 4 ];

    /**
     * One way tile that will represent the ghost pen entrance
     * actors can only enter through bottom. IOW, if their direction is DIR_UP
     */
    SDL_Point one_way_tile;

    /**
     * Contains coordinates of tiles that are slow moving for ghosts
     */
    SDL_Point tm_slow_tiles[21];

} TileMap;


/**
 * INITIALIZATION for tm ( tilemap )
 * 
 * @level_filename: 
 *  - NULL: will initialize an empty tilemap 
 *  - NOT NULL: will load the tilemap for the level of the filename passed
 */
void tm_init_and_load_texture( SDL_Renderer *renderer, TileMap *tm ) ;
/**
 * CONVERTING TILE MAP GRID COORDINATES AND SCREEN COORDINATES
 */

SDL_Point tile_grid_point_to_screen_point( SDL_Point tile_grid_point, SDL_Point tile_map_screen_position ) ;

SDL_Point tile_grid_point_to_world_point( SDL_Point tile_grid_point );

SDL_Point world_point_to_screen_point( SDL_Point world_point, SDL_Point tile_map_screen_position );

SDL_Point screen_point_to_tile_grid_point( SDL_Point screen_point, SDL_Point tile_map_screen_position ) ;
/**
 * RENDERING
 */
void tm_render_with_screen_position_offset( SDL_Renderer *renderer, TileMap *tm );

#endif