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
const int TILE_ROWS = SCREEN_HEIGHT / ( TILE_SIZE );
const int TOTAL_NUMBER_OF_TILES = TILE_ROWS * TILE_COLS;


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
     * Members I might want
     * SDL_Rect tm_tile_collision_rects[ TILE_ROWS ][ TILE_COLS ];
     * SDL_Point tm_tile_screen_positions[ TILE_ROWS ][ TILE_COLS ];
     */

} TileMap;

/**
 * FILE I/O FOR SAVING/LOADING
 */

void save_tilemap_texture_atlas_indexes_to_file( TwoDimensionalArrayIndex tm_texture_atlas_indexes[ TILE_ROWS ][ TILE_COLS ] ) {
    char *filename = "maze_file";
    char *write_binary_mode = "wb";

    SDL_RWops *write_context = SDL_RWFromFile( filename , write_binary_mode );
    if( write_context == NULL ) {
        fprintf(stderr, "%s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }
    SDL_RWwrite( write_context, tm_texture_atlas_indexes, sizeof( TwoDimensionalArrayIndex ), TOTAL_NUMBER_OF_TILES );
    SDL_RWclose( write_context );
}

// TODO: take in a filename as well
void try_load_tilemap_texture_atlas_indexes_from_file( TwoDimensionalArrayIndex tm_texture_atlas_indexes[ TILE_ROWS ][ TILE_COLS ] ) {
    char *filename = "maze_file";
    char *read_binary_mode = "rb";

    SDL_RWops *read_context = SDL_RWFromFile( filename, read_binary_mode );
    // File does not exist
    if( read_context == NULL ) {
        fprintf(stderr, "%s\n", SDL_GetError() );
        return; // we'll just not do anything
    }
    SDL_RWread( read_context, tm_texture_atlas_indexes, sizeof( TwoDimensionalArrayIndex ), TOTAL_NUMBER_OF_TILES );
    SDL_RWclose( read_context );
} 

/**
 * INITIALIZATION for tm ( tilemap )
 * 
 * @level_filename: 
 *  - NULL: will initialize an empty tilemap 
 *  - NOT NULL: will load the tilemap for the level of the filename passed
 */
void tm_init_and_load_texture( SDL_Renderer *renderer, TileMap *tm, char *level_filename ) {
    // LOAD the texture
    SDL_Surface *surface;
    surface = IMG_Load("tileset_40x40.png");
    tm->tm_texture_atlas = SDL_CreateTextureFromSurface( renderer, surface );
    SDL_FreeSurface( surface );

    for( int row = 0; row < TILE_ROWS; ++row ) {
        for ( int col = 0; col < TILE_COLS; ++col ) {
            tm->tm_texture_atlas_indexes[ row ][ col ] = EMPTY_TILE_TEXTURE_ATLAS_INDEX;
        }
    }
        
    // load the texture indexes to use for each tile in the tilemap
    if( level_filename != NULL) {
        try_load_tilemap_texture_atlas_indexes_from_file( tm->tm_texture_atlas_indexes );
    }
    
    // Space for any UI elements at the top of the screen
    tm->tm_screen_position.x = 0;
    tm->tm_screen_position.y = TILE_SIZE * 2; 
}

/**
 * CONVERTING TILE MAP GRID COORDINATES AND SCREEN COORDINATES
 */

SDL_Point tile_grid_point_to_screen_point( SDL_Point tile_grid_point, SDL_Point tile_map_screen_position ) {
    // Thoughts. might want to just store an array of all the screen positions rather than calculate it with this value every iteration.
    // Alternatively, maybe store a bunch of SDL_Rects of the collisions for each tile
    SDL_Point screen_point;
    screen_point.y = tile_grid_point.y * TILE_SIZE + tile_map_screen_position.y;
    screen_point.x = tile_grid_point.x * TILE_SIZE + tile_map_screen_position.x;

    return screen_point;
}

SDL_Point screen_point_to_tile_grid_point( SDL_Point screen_point, SDL_Point tile_map_screen_position ) {

    SDL_Point grid_point;
    grid_point.y = ( screen_point.y - tile_map_screen_position.y ) / TILE_SIZE;
    grid_point.x = ( screen_point.x - tile_map_screen_position.x ) / TILE_SIZE;

    return grid_point; 
}

/**
 * RENDERING
 */
void tm_render_with_screen_position_offset( SDL_Renderer *renderer, TileMap *tm ) {
    for(int row = 0; row < TILE_ROWS; ++row ) {
        for( int col = 0; col < TILE_COLS; ++col ) {
            if( tm->tm_texture_atlas_indexes[ row ][ col ].r != -1 
                || tm->tm_texture_atlas_indexes[ row ][ col ].c != -1 ){
                
                SDL_Point render_position = { 
                    tm->tm_screen_position.x + ( TILE_SIZE * col ), 
                    tm->tm_screen_position.y + ( TILE_SIZE * row ) 
                };
                
                SDL_Point texture_position_in_atlas = { 
                    tm->tm_texture_atlas_indexes[ row ][ col ].c * TILE_SIZE, 
                    tm->tm_texture_atlas_indexes[ row ][ col ].r * TILE_SIZE  
                };
                
                SDL_Rect texture_atlas_src_rect = { 
                    texture_position_in_atlas.x, 
                    texture_position_in_atlas.y, 
                    TILE_SIZE, 
                    TILE_SIZE 
                };

                SDL_Rect render_dst_rect = {
                    render_position.x , 
                    render_position.y, 
                    TILE_SIZE, 
                    TILE_SIZE 
                };

                // Want to probably organize the structure better so that the texture atlas is closer to each 64 byte in the array
                SDL_RenderCopy( renderer, tm->tm_texture_atlas, &texture_atlas_src_rect, &render_dst_rect );
            }
        }
    }
}



#endif