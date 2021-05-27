#include <SDL2/SDL.h>
#include <stdio.h>
#include "resources.h"
#include "tiles.h"
/**
 * FILE I/O FOR SAVING/LOADING
 */

void save_resource_to_file( void* resource_ptr, char *filename, size_t resource_size, int num_resources ) {
    const char *write_binary_mode = "wb";
    SDL_RWops *write_context = SDL_RWFromFile( filename , write_binary_mode );
    if( write_context == NULL ) {
        fprintf(stderr, "%s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }
    SDL_RWwrite( write_context, resource_ptr, resource_size, num_resources );
    SDL_RWclose( write_context );
}

void try_load_resource_from_file( void *resource_ptr, char *filename, size_t resource_size, int num_resources ) {
    const char *read_binary_mode = "rb";

    SDL_RWops *read_context = SDL_RWFromFile( filename, read_binary_mode );
    // File does not exist
    if( read_context == NULL ) {
        fprintf(stderr, "%s\n", SDL_GetError() );
        return; // we'll just not do anything
    }

    SDL_RWread( read_context, resource_ptr,  resource_size , num_resources );
    SDL_RWclose( read_context );
}

// void try_load_texture_atlases_from_config_file( char *filename ) {
//     int count = 0;
//     FILE *f;
//     f = fopen(filename, "r");
//     if( f == NULL ) {
//         fprintf(stderr, "Error opening file %s\n", filename );
//     }
//     fgets()
// }