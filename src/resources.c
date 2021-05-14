#include <SDL2/SDL.h>
#include "resources.h"
#include "tiles.h"
/**
 * FILE I/O FOR SAVING/LOADING
 */

void save_tilemap_texture_atlas_indexes_to_file( TwoDimensionalArrayIndex tm_texture_atlas_indexes[ TILE_ROWS ][ TILE_COLS ] ) {
    char *filename = "res/maze_file";
    char *write_binary_mode = "wb";

    SDL_RWops *write_context = SDL_RWFromFile( filename , write_binary_mode );
    if( write_context == NULL ) {
        fprintf(stderr, "%s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }
    SDL_RWwrite( write_context, tm_texture_atlas_indexes, sizeof( TwoDimensionalArrayIndex ), TOTAL_NUMBER_OF_TILES );
    SDL_RWclose( write_context );
}

void try_load_tilemap_texture_atlas_indexes_from_file( TwoDimensionalArrayIndex tm_texture_atlas_indexes[ TILE_ROWS ][ TILE_COLS ] ) {
    char *filename = "res/maze_file";
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


void save_dots_to_file( char dots[ TILE_ROWS ][ TILE_COLS ] ) {
    char *filename = "res/dots";
    char *write_binary_mode = "wb";

    SDL_RWops *write_context = SDL_RWFromFile( filename, write_binary_mode );
    if( write_context == NULL ) {
        fprintf(stderr, "%s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }
    SDL_RWwrite( write_context, dots, sizeof( char ), TOTAL_NUMBER_OF_TILES );
    SDL_RWclose( write_context );
}

void try_load_dots_from_file( char dots[ TILE_ROWS ][ TILE_COLS ] ) {
    char *filename = "res/dots";
    char *read_binary_mode = "rb";

    SDL_RWops *read_context = SDL_RWFromFile( filename, read_binary_mode );
    // File does not exist
    if( read_context == NULL ) {
        fprintf(stderr, "%s\n", SDL_GetError() );
        return; // we'll just not do anything
    }
    SDL_RWread( read_context, dots, sizeof( char ), TOTAL_NUMBER_OF_TILES );
    SDL_RWclose( read_context );
}

void save_walls_to_file( char walls[ TILE_ROWS ][ TILE_COLS ] ) {
    char *filename = "res/walls";
    char *write_binary_mode = "wb";

    SDL_RWops *write_context = SDL_RWFromFile( filename, write_binary_mode );
    if( write_context == NULL ) {
        fprintf(stderr, "%s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }
    SDL_RWwrite( write_context, walls, sizeof( char ), TOTAL_NUMBER_OF_TILES );
    SDL_RWclose( write_context );
}

void try_load_walls_from_file( char walls[ TILE_ROWS ][ TILE_COLS ] ) {
    char *filename = "res/walls";
    char *read_binary_mode = "rb";

    SDL_RWops *read_context = SDL_RWFromFile( filename, read_binary_mode );
    // File does not exist
    if( read_context == NULL ) {
        fprintf(stderr, "%s\n", SDL_GetError() );
        return; // we'll just not do anything
    }
    SDL_RWread( read_context, walls, sizeof( char ), TOTAL_NUMBER_OF_TILES );
    SDL_RWclose( read_context );
}
