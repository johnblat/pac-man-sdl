#ifndef TARGETING_H
#define TARGETING_H

#include "entity.h"
#include <inttypes.h>
#include "tiles.h"

extern SDL_Point shadow_home_tile; 
extern SDL_Point ambush_home_tile; 
extern SDL_Point moody_home_tile; 
extern SDL_Point pokey_home_tile;

void set_direction_and_next_tile_shortest_to_target( Entities *entities, EntityId eid, TileMap *tm, int ghost_state ) ;

void set_scatter_target_tile( Entities *entities, EntityId ghostId, SDL_Point home_tile )  ;

void set_shadow_target_tile( Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers );

void set_ambush_target_tile( Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers) ;

void set_moody_target_tile(Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers ) ;

void set_pokey_target_tile( Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers ) ;

#endif