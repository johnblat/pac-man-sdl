#include <dirent.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "animation.h"
#include "render.h"
#include "resources.h"
#include "tiles.h"


const int MAX_FILENAME_SIZE = 64;

int g_current_level = 0;

int determine_number_of_levels_from_dirs( ) {
    int num_levels = 0;
    DIR *res_dir = opendir("res");
    struct dirent *d;
    char *first_five_chars = "level";
    while( ( d = readdir( res_dir ) ) != NULL ) {
        if( strncmp( first_five_chars, d->d_name, 5 ) == 0 ) {
            num_levels++;
        }

    } 
    return num_levels;
}

void load_level_off_disk( int level_num, int num_levels ) {
    if( level_num > num_levels ) {
        fprintf( stderr, "Could not load level because requested level number greater than %d. Max is %d\n", num_levels, level_num );
    }
    char level_dirname[ MAX_FILENAME_SIZE ];
    memset( level_dirname, '\0', MAX_FILENAME_SIZE );
    snprintf(level_dirname, MAX_FILENAME_SIZE, "level-%d/", level_num );
    
    // ACTUAL RESOURCES
   // char *pac_starting_tile = "pac_starting_tile";
    char path_to_thing[ MAX_FILENAME_SIZE ];
    strcpy( path_to_thing, level_dirname );
    //char *pac_starting_tile_path = strcat( path_to_thing, pac_starting_tile  );

}

void load_global_texture_atlases_from_config_file( SDL_Renderer *renderer ) {
    char *filename_config = "res/texture_atlases";
    FILE *f;
    f = fopen(filename_config, "r");
    if( f == NULL ) {
        fprintf(stderr, "Error opening file %s\n", filename_config );
    }
    
    char filename_texture_atlas[ MAX_FILENAME_SIZE ];

    int num_sprites_texture_atlas;

    char current_line[ 256 ];
    
    /**
     * Very specific and not general-purpose at all
    */
    while( fgets( current_line, 256, f ) != NULL ) {
        if( current_line[ 0 ] == '#' ){
            continue;
        }
        
        // zero these out
        memset(filename_texture_atlas, '\0', MAX_FILENAME_SIZE );
        num_sprites_texture_atlas = 0;

        int beg_value_idx = 0;
        int line_idx = 0;
        int size_bytes_value = 0;
        
        while( current_line[ line_idx ] != ' ' ) {
            line_idx++;
        }

        size_bytes_value = line_idx;
        memcpy( filename_texture_atlas, current_line + beg_value_idx, size_bytes_value );

        beg_value_idx = line_idx + 1;
        line_idx = beg_value_idx;

        // NUMERIC VALUES
        int numeric_values[ 2 ];
        for( int i = 0; i < 2 ; i++ ) {
            numeric_values[ i ] = strtol( current_line + line_idx , NULL, 10 );
            while( current_line[ line_idx ] != ' ' && current_line[ line_idx ] != '\n') {
                line_idx++;
            }
            line_idx++;
        }

        int num_rows = numeric_values[ 0 ];
        int num_cols = numeric_values[ 1 ];
        add_texture_atlas( renderer, filename_texture_atlas, num_rows, num_cols );

    }
}

void load_animations_from_config_file( AnimatedSprite **animated_sprites ) { 

    int num_animated_sprites = 0;

    char *filename_config = "res/animated_sprites";
    FILE *f;
    f = fopen(filename_config, "r");
    if( f == NULL ) {
        fprintf(stderr, "Error opening file %s\n", filename_config );
    }
    
    char current_line[ 256 ];

    /**
     * Very specific and not general-purpose at all
    */
    while( fgets( current_line, 256, f ) != NULL ) {
        if( current_line[ 0 ] == '#') { // COMMENTS
            continue;
        }
        // zero these out
        int texture_atlas_id = -1;
        int number_cols = -1;
        int number_rows = -1;
        int fps = -1;

        int values[ 4 ];

        int line_idx = 0;
        
        for( int i = 0; i < 4; i++ ) {
            //char *end_ptr;
            values[ i ] = strtol( current_line + line_idx , NULL, 10 );

            while( current_line[ line_idx ] != ' ' && current_line[ line_idx ] != '\n') {
                line_idx++;
            }
            line_idx++;
        }
        
        texture_atlas_id = values[ 0 ];
        fps = values[ 1 ];
        number_rows = values[ 2 ];
        number_cols = values[ 3 ];

        animated_sprites[ num_animated_sprites ] = init_animation( texture_atlas_id, fps, number_rows, number_cols );
        num_animated_sprites++;
        //assert( *end_ptr == '\n' || *end_ptr == ' ' || *end_ptr == '\0');

    }
}

void load_render_xx_from_config_file( RenderClipFromTextureAtlas **render_clips ) {

    int num_render_clips = 0;

    char *filename_config = "res/render_anims";
    FILE *f;
    f = fopen(filename_config, "r");
    if( f == NULL ) {
        fprintf(stderr, "Error opening file %s\n", filename_config );
    }
    
    char current_line[ 256 ];

    /**
     * Very specific and not general-purpose at all
    */
    while( fgets( current_line, 256, f ) != NULL ) {
        if( current_line[ 0 ] == '#') { // COMMENT
            continue;
        }
        //int texture_atlas_id = -1;
        int animation_id = -1;

        int values[ 2 ];

        int line_idx = 0;

        for( int i = 0; i < 1; i++ ) {
            values[ i ] = strtol( current_line + line_idx , NULL, 10 );

            while( current_line[ line_idx ] != ' ' && current_line[ line_idx ] != '\n') {
                line_idx++;
            }
            line_idx++;
        }

        //texture_atlas_id = values[ 0 ];
        animation_id = values[ 0 ];

        render_clips[ num_render_clips ] = init_render_clip( 0, animation_id );
        num_render_clips++;
    }
}
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