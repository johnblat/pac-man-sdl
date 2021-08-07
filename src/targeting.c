#include <SDL2/SDL.h>
#include "stdio.h"
#include "targeting.h"
#include "actor.h"
#include "states.h"
#include "comparisons.h"
#include "tiles.h"

SDL_Point shadow_home_tile = { 0, 0 };
SDL_Point ambush_home_tile = { 0, TILE_ROWS - 1 };
SDL_Point moody_home_tile = { TILE_COLS - 1, 0 };
SDL_Point pokey_home_tile = { TILE_COLS - 1, TILE_ROWS - 1};

void set_direction_and_next_tile_shortest_to_target( Actor *actor, TileMap *tm, int ghost_state ) {
    SDL_Point tile_above, tile_below, tile_left, tile_right;
    tile_above.x = actor->current_tile.x;
    tile_above.y = actor->current_tile.y - 1;
    tile_below.x = actor->current_tile.x;
    tile_below.y = actor->current_tile.y + 1;
    tile_left.x = actor->current_tile.x - 1;
    tile_left.y = actor->current_tile.y;
    tile_right.x = actor->current_tile.x + 1;
    tile_right.y = actor->current_tile.y;

    SDL_Point surrounding_tiles[ 4 ];
    surrounding_tiles[ DIR_UP ] = tile_above;
    surrounding_tiles[ DIR_DOWN ] = tile_below;
    surrounding_tiles[ DIR_LEFT ] = tile_left;
    surrounding_tiles[ DIR_RIGHT ] = tile_right;

    // wrap around tilemaps Issue 150
    for( int i = 0; i < 4; i++ ) {
        if( surrounding_tiles[ i ].x < 0 ) {
            surrounding_tiles[ i ].x = TILE_COLS-1;
        }
        else if( surrounding_tiles[ i ].x >  TILE_COLS-1 ) {
            surrounding_tiles[ i ].x = 0;
        }
        else if( surrounding_tiles[ i ].y < 0 ) {
            surrounding_tiles[ i ].y = TILE_ROWS-1;
        }
        else if( surrounding_tiles[ i ].y > TILE_ROWS-1) {
            surrounding_tiles[ i ].y = 0;
        }
    }

    float above_to_target_dist = distance(tile_above.x, tile_above.y,  actor->target_tile.x, actor->target_tile.y ); 
    float below_to_target_dist = distance(tile_below.x, tile_below.y,  actor->target_tile.x, actor->target_tile.y ); 
    float left_to_target_dist  = distance(tile_left.x, tile_left.y,  actor->target_tile.x, actor->target_tile.y ); 
    float right_to_target_dist = distance(tile_right.x, tile_right.y,  actor->target_tile.x, actor->target_tile.y ); 


    float lengths[ 4 ];
    lengths[ DIR_UP ] = above_to_target_dist;
    lengths[ DIR_DOWN ] = below_to_target_dist;
    lengths[ DIR_LEFT ] = left_to_target_dist;
    lengths[ DIR_RIGHT ] = right_to_target_dist;

    Direction opposite_directions[ 5 ];
    opposite_directions[ DIR_UP ] = DIR_DOWN;
    opposite_directions[ DIR_DOWN ] = DIR_UP;
    opposite_directions[ DIR_LEFT ] = DIR_RIGHT;
    opposite_directions[ DIR_RIGHT ] = DIR_LEFT;
    opposite_directions[ DIR_NONE ] = DIR_UP; // just something in case their direction is none

    int shortest_direction = opposite_directions[ actor->direction ]; // this will ensure, that if all options are run through and ghost hasnt found a tile NOT behind him, he/she will just turn around
    float shortest_length = 9999.0f; // just some high number


    for( int i = 0; i < 4; ++i ) {
        if( tm->tm_walls[ surrounding_tiles[ i ].y][ surrounding_tiles[ i ].x ] == 'x' ) {
            continue;
        }

        if( 
            points_equal(surrounding_tiles[ i ], tm->one_way_tile) 
            && i == DIR_DOWN 
            && ghost_state != STATE_GO_TO_PEN ) {
                continue;
        }

        if( i == opposite_directions[ actor->direction  ] ) continue;

        if ( lengths[ i ] < shortest_length ) {
            shortest_direction = i;
            shortest_length = lengths[ i ];
        }
    }

    // 23 is the actual tilemap on the screen height - because of offset
    actor->direction = (Direction) shortest_direction;
    actor->next_tile = surrounding_tiles[ shortest_direction ];
    if( actor->next_tile.y > TILE_ROWS-1 ) {
        actor->next_tile.y = 1;
    } 
    if( actor->next_tile.y < 0 ){
        actor->next_tile.y =  TILE_ROWS-1;
    }
    if( actor->next_tile.x >  TILE_COLS-1 ) {
        actor->next_tile.x = 1;
    }
    if( actor->next_tile.x < 0 ) {
        actor->next_tile.x =  TILE_COLS-1;
    }
}


void set_scatter_target_tile(  Entities *entities, EntityId ghostId, SDL_Point home_tile  ) {
    entities->actors[ ghostId ]->target_tile = home_tile;
}

EntityId closestEntityToEntity( Entities *entities, EntityId entityId, EntityId *entityIds, unsigned int numEntities ) {
    if( numEntities == 1 ) {
        return entityIds[ 0 ];
    }
    EntityId closestEntityId = entityIds[ 0 ];
    EntityId currentEntityId = entityIds[ 0 ];

    float closestDistance = 99999.9f;
    float currentDistance = 0.0f;
    for( int i = 0; i < numEntities; i++ ) {
        currentEntityId = entityIds[ i ];
        if ( entities->isActive[currentEntityId] != NULL && *entities->isActive[currentEntityId] == SDL_FALSE ) {
            continue;
        }
        currentDistance = distance( entities->actors[ entityId ]->world_center_point.x, entities->actors[ entityId ]->world_center_point.y, entities->actors[ currentEntityId ]->world_center_point.x, entities->actors[ currentEntityId ]->world_center_point.y );
        if( currentDistance < closestDistance ) {
            closestDistance = currentDistance;
            closestEntityId = currentEntityId;
        }
    }

    return closestEntityId;

}

void set_shadow_target_tile( Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers ){
    Actor *ghostActor = entities->actors[ ghostId ];
    if( numPlayers == 1 ) { // single player? automatically going to be closest player
        ghostActor->target_tile = entities->actors[ playerIds[0] ]->current_tile;
        return;
    }
    // determine which player is closest
    EntityId closestPlayerId = closestEntityToEntity( entities, ghostId, playerIds, numPlayers );

    ghostActor->target_tile = entities->actors[ closestPlayerId ]->current_tile;

}


inline void set_ambush_target_tile( Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers ) {
    Actor *ghostActor = entities->actors[ ghostId ];
    
    // determine which player is closest
    EntityId closestPlayerId = closestEntityToEntity( entities, ghostId, playerIds, numPlayers );

    // target tile is in front of player based on their direction
    switch( entities->actors[ closestPlayerId ]->direction ) {
        case DIR_UP:
            ghostActor->target_tile.x =  entities->actors[ closestPlayerId]->current_tile.x;
            ghostActor->target_tile.y = entities->actors[ closestPlayerId]->current_tile.y - 4;
            break;
        case DIR_DOWN:
            ghostActor->target_tile.x =  entities->actors[ closestPlayerId]->current_tile.x;
            ghostActor->target_tile.y =  entities->actors[ closestPlayerId]->current_tile.y + 4;
            break;
        case DIR_LEFT:
            ghostActor->target_tile.x =  entities->actors[ closestPlayerId]->current_tile.x - 4;
            ghostActor->target_tile.y =  entities->actors[ closestPlayerId]->current_tile.y;
            break;
        case DIR_RIGHT:
            ghostActor->target_tile.x =  entities->actors[ closestPlayerId]->current_tile.x + 4;
            ghostActor->target_tile.y =  entities->actors[ closestPlayerId]->current_tile.y;
            break;
        default :
            ghostActor->target_tile.x =  entities->actors[ closestPlayerId]->current_tile.x;
            ghostActor->target_tile.y =  entities->actors[ closestPlayerId]->current_tile.y;
            break;
    }   
}

inline void set_moody_target_tile(Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers ) {
    // Most complex targeting scheme
    // Needs to have one entity with a targeting behavior of shadow to calculate

    EntityId closestPlayerId = closestEntityToEntity( entities, ghostId, playerIds, numPlayers );

    // find shadowghost to compare with for this targeting algorithm
    Actor *shadowGhostActor = NULL;
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->targetingBehaviors[ eid ] == NULL ) {
            continue;
        }
        if( *entities->targetingBehaviors[ eid ] == SHADOW_BEHAVIOR ) {
            shadowGhostActor = entities->actors[ eid ];
            break;
        }
    }
    // handle any error finding a shadow ghost
    if( shadowGhostActor == NULL ) {
        fprintf( stderr, "Something went wrong. Moody tried to find shadow entity, but not found\n");
        fprintf( stderr, "Just setting moody as if shadow\n");
        entities->actors[ ghostId ]->target_tile = entities->actors[ closestPlayerId ]->current_tile;
        return;
    }

    // determine target tile
    SDL_Point shadowGhostCurrentTile = shadowGhostActor->current_tile;
    SDL_Point offset_tile;
    int tiles_x;
    int tiles_y;

    switch( entities->actors[ closestPlayerId ]->direction ) {
        case DIR_UP:
            offset_tile.x = entities->actors[ closestPlayerId ]->current_tile.x;
            offset_tile.y = entities->actors[ closestPlayerId ]->current_tile.y - 2;
            

            tiles_x = offset_tile.x - shadowGhostCurrentTile.x;
            tiles_y = offset_tile.y - shadowGhostCurrentTile.y;

            entities->actors[ ghostId ]->target_tile.x = offset_tile.x + tiles_x;
            entities->actors[ ghostId ]->target_tile.y = offset_tile.y + tiles_y;

            break;
        case DIR_DOWN:
            offset_tile.x = entities->actors[ closestPlayerId ]->current_tile.x;
            offset_tile.y = entities->actors[ closestPlayerId ]->current_tile.y + 2;

            tiles_x = offset_tile.x - shadowGhostCurrentTile.x;
            tiles_y = offset_tile.y - shadowGhostCurrentTile.y;

            entities->actors[ ghostId ]->target_tile.x = offset_tile.x + tiles_x;
            entities->actors[ ghostId ]->target_tile.y = offset_tile.y + tiles_y;

            break;
        case DIR_LEFT:
            offset_tile.x = entities->actors[ closestPlayerId ]->current_tile.x - 2;
            offset_tile.y = entities->actors[ closestPlayerId ]->current_tile.y;

            tiles_x = offset_tile.x - shadowGhostCurrentTile.x;
            tiles_y = offset_tile.y - shadowGhostCurrentTile.y;

            entities->actors[ ghostId ]->target_tile.x = offset_tile.x + tiles_x;
            entities->actors[ ghostId ]->target_tile.y = offset_tile.y + tiles_y;

            break;
        case DIR_RIGHT:
            offset_tile.x = entities->actors[ closestPlayerId ]->current_tile.x + 2;
            offset_tile.y = entities->actors[ closestPlayerId ]->current_tile.y;

            tiles_x = offset_tile.x - shadowGhostCurrentTile.x;
            tiles_y = offset_tile.y - shadowGhostCurrentTile.y;

            entities->actors[ ghostId ]->target_tile.x = offset_tile.x + tiles_x;
            entities->actors[ ghostId ]->target_tile.y = offset_tile.y + tiles_y;

            break;
        default :
            offset_tile.x = entities->actors[ closestPlayerId ]->current_tile.x;
            offset_tile.y = entities->actors[ closestPlayerId ]->current_tile.y;

            tiles_x = offset_tile.x - shadowGhostCurrentTile.x;
            tiles_y = offset_tile.y - shadowGhostCurrentTile.y;

            entities->actors[ ghostId ]->target_tile.x = offset_tile.x + tiles_x;
            entities->actors[ ghostId ]->target_tile.y = offset_tile.y + tiles_y;

            break;
    }
    if ( entities->actors[ ghostId ]->target_tile.x < 0 ) entities->actors[ ghostId ]->target_tile.x = 0;
    if ( entities->actors[ ghostId ]->target_tile.y < 0 ) entities->actors[ ghostId ]->target_tile.y = 0;

    if ( entities->actors[ ghostId ]->target_tile.x > TILE_COLS - 1 ) entities->actors[ ghostId ]->target_tile.x = TILE_COLS - 1;
    if ( entities->actors[ ghostId ]->target_tile.y > TILE_ROWS - 1 ) entities->actors[ ghostId ]->target_tile.y = TILE_ROWS - 1;

}

inline void set_pokey_target_tile( Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers ) {
   // SDL_Point b_tile_above, b_tile_below, b_tile_left, b_tile_right;
    float shadow_range = 64;
    SDL_Point ghostCurrentTile = entities->actors[ ghostId ]->current_tile;

    EntityId closestPlayerId = closestEntityToEntity( entities, ghostId, playerIds, numPlayers );
    SDL_Point pac_current_tile = entities->actors[ closestPlayerId ]->current_tile;

    float distance_to_pacman = distance( ghostCurrentTile.x, ghostCurrentTile.y, pac_current_tile.x, pac_current_tile.y );

    if( distance_to_pacman >= shadow_range ) {
        set_shadow_target_tile( entities, ghostId, playerIds, numPlayers );
    }
    else {
        set_scatter_target_tile( entities, ghostId, pokey_home_tile );
    }
}