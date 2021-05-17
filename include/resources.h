#ifndef RESOURCES_H
#define RESOURCES_H

#include "jb_types.h"
#include "tiles.h"
/**
 * FILE I/O FOR SAVING/LOADING
 */

void save_resource_to_file( void* resource_ptr, char *filename, size_t resource_size, int num_resources );
void try_load_resource_from_file( void *resource_ptr, char *filename, size_t resource_size, int num_resources );

void save_tilemap_texture_atlas_indexes_to_file( TwoDimensionalArrayIndex tm_texture_atlas_indexes[ TILE_ROWS ][ TILE_COLS ] ) ;

void try_load_tilemap_texture_atlas_indexes_from_file( TwoDimensionalArrayIndex tm_texture_atlas_indexes[ TILE_ROWS ][ TILE_COLS ] ) ;

void save_dots_to_file( char dots[ TILE_ROWS ][ TILE_COLS ] ) ;

void try_load_dots_from_file( char dots[ TILE_ROWS ][ TILE_COLS ] );

void save_walls_to_file( char walls[ TILE_ROWS ][ TILE_COLS ] ) ;

void try_load_walls_from_file( char walls[ TILE_ROWS ][ TILE_COLS ] );

#endif