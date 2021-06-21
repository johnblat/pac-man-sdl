#ifndef RESOURCES_H
#define RESOURCES_H

#include "jb_types.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include "animation.h"
#include "render.h"
#include "tiles.h"

typedef struct LevelConfig {
    uint8_t *scatterChasePeriodSeconds;
    uint8_t numScatterChasePeriods;
    SDL_Point ghostPenTile;
    SDL_Point pacStartingTile;
} LevelConfig;

unsigned int gCurrentLevel;
unsigned int gNumLevels;

static void build_resource_file_path( ); 

int determine_number_of_levels_from_dirs( );

void save_current_level_to_disk( LevelConfig *levelConfig, TileMap *tilemap );

void load_current_level_off_disk( LevelConfig *levelConfig, TileMap *tilemap, SDL_Renderer *renderer) ;

void load_global_texture_atlases_from_config_file( SDL_Renderer *renderer );

void load_animations_from_config_file( AnimatedSprite **animated_sprites );

void load_render_xx_from_config_file( RenderClipFromTextureAtlas **render_clips );

void load_ghost_mode_times_from_config_file( uint8_t *ghost_mode_times, int num_periods, char *filename_config );
/**
 * FILE I/O FOR SAVING/LOADING
 */

void save_resource_to_file( void* resource_ptr, char *filename, size_t resource_size, int num_resources );
void try_load_resource_from_file( void *resource_ptr, char *filename, size_t resource_size, int num_resources );


#endif