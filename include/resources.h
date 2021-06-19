#ifndef RESOURCES_H
#define RESOURCES_H

#include "jb_types.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include "animation.h"
#include "render.h"
#include "tiles.h"

void load_global_texture_atlases_from_config_file( SDL_Renderer *renderer );

void load_animations_from_config_file( AnimatedSprite **animated_sprites );

void load_render_xx_from_config_file( RenderClipFromTextureAtlas **render_clips );

void load_ghost_mode_times_from_config_file( uint8_t *ghost_mode_times, int num_periods );
/**
 * FILE I/O FOR SAVING/LOADING
 */

void save_resource_to_file( void* resource_ptr, char *filename, size_t resource_size, int num_resources );
void try_load_resource_from_file( void *resource_ptr, char *filename, size_t resource_size, int num_resources );


#endif