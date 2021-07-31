#ifndef RESOURCES_H
#define RESOURCES_H

#include "jb_types.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include "animation.h"
#include "entity.h"
#include "render.h"
#include "tiles.h"




unsigned int gNumLevels;

static void build_resource_file_path( ); 

int determine_number_of_levels_from_dirs( );

void initializePlayersFromFiles( Entities *entities, LevelConfig *levelConfig, unsigned int numPlayers ) ;

void initializeGhostsFromFile( Entities *entities, LevelConfig *levelConfig, const char *animatedSpritesFilename  );

void save_current_level_to_disk( LevelConfig *levelConfig, TileMap *tilemap );

void load_current_level_off_disk( LevelConfig *levelConfig, TileMap *tilemap, SDL_Renderer *renderer) ;

void load_global_texture_atlases_from_config_file( SDL_Renderer *renderer );

void load_animations_from_config_file( AnimatedSprite **animated_sprites );

void load_render_xx_from_config_file( RenderData **renderDatas );

void load_ghost_mode_times_from_config_file( uint32_t *ghost_mode_times, int num_periods, char *filename_config );

void tryLoadPickupsFromConfigFile( LevelConfig *levelConfig, const char *fullResourcePath );
/**
 * FILE I/O FOR SAVING/LOADING
 */

void save_resource_to_file( void* resource_ptr, char *filename, size_t resource_size, int num_resources );
void try_load_resource_from_file( void *resource_ptr, char *filename, size_t resource_size, int num_resources );


#endif