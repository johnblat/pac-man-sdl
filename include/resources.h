#ifndef RESOURCES_H
#define RESOURCES_H

#include "jb_types.h"
#include "tiles.h"
/**
 * FILE I/O FOR SAVING/LOADING
 */

void save_resource_to_file( void* resource_ptr, char *filename, size_t resource_size, int num_resources );
void try_load_resource_from_file( void *resource_ptr, char *filename, size_t resource_size, int num_resources );


#endif