#ifndef TARGETING_H
#define TARGETING_H

#include "actor.h"
#include <inttypes.h>
#include "tiles.h"
#include "states.h"

extern SDL_Point shadow_home_tile; 
extern SDL_Point ambush_home_tile; 
extern SDL_Point moody_home_tile; 
extern SDL_Point pokey_home_tile;

void set_direction_and_next_tile_shortest_to_target( Actor *actor, TileMap *tm, GhostState ghost_state ) ;

void set_scatter_target_tile( Actor **actors, uint8_t actor_id, TileMap *tm, SDL_Point home_tile )  ;

void set_all_scatter_target_tile( Actor **actors );

void set_shadow_target_tile( Actor **actors, uint8_t actor_id, TileMap *tm );

void set_ambush_target_tile( Actor **actors, TileMap *tm ) ;

void set_moody_target_tile(Actor **actors, TileMap *tm) ;

void set_pokey_target_tile( Actor **actors, TileMap *tm ) ;

#endif