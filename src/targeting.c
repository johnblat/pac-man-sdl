#include <SDL2/SDL.h>
#include "stdio.h"
#include "targeting.h"
#include "states.h"
#include "comparisons.h"
#include "tiles.h"

SDL_Point shadow_home_tile = { 0, 0 };
SDL_Point ambush_home_tile = { 0, TILE_ROWS - 1 };
SDL_Point moody_home_tile = { TILE_COLS - 1, 0 };
SDL_Point pokey_home_tile = { TILE_COLS - 1, TILE_ROWS - 1};

void set_direction_and_next_tile_shortest_to_target( Entities *entities, EntityId eid, TileMap *tm, int ghost_state ) {
    SDL_Point tile_above, tile_below, tile_left, tile_right;
    tile_above.x = entities->currentTiles[eid]->x;
    tile_above.y = entities->currentTiles[eid]->y - 1;
    tile_below.x = entities->currentTiles[eid]->x;
    tile_below.y = entities->currentTiles[eid]->y + 1;
    tile_left.x = entities->currentTiles[eid]->x - 1;
    tile_left.y = entities->currentTiles[eid]->y;
    tile_right.x = entities->currentTiles[eid]->x + 1;
    tile_right.y = entities->currentTiles[eid]->y;

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

    float above_to_target_dist = distance(
        tile_above.x, 
        tile_above.y,  
        entities->targetTiles[eid]->x, 
        entities->targetTiles[eid]->y 
    ); 
    float below_to_target_dist = distance(
        tile_below.x, 
        tile_below.y,  
        entities->targetTiles[eid]->x, 
        entities->targetTiles[eid]->y 
    ); 
    float left_to_target_dist  = distance(
        tile_left.x, 
        tile_left.y,  
        entities->targetTiles[eid]->x, 
        entities->targetTiles[eid]->y 
    ); 
    float right_to_target_dist = distance(
        tile_right.x, 
        tile_right.y,  
        entities->targetTiles[eid]->x, 
        entities->targetTiles[eid]->y 
    ); 


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

    int shortest_direction = opposite_directions[ *entities->directions[eid] ]; // this will ensure, that if all options are run through and ghost hasnt found a tile NOT behind him, he/she will just turn around
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

        if( i == opposite_directions[ *entities->directions[eid]  ] ) continue;

        if ( lengths[ i ] < shortest_length ) {
            shortest_direction = i;
            shortest_length = lengths[ i ];
        }
    }

    // 23 is the actual tilemap on the screen height - because of offset
    *entities->directions[eid] = (Direction) shortest_direction;
    *entities->nextTiles[eid] = surrounding_tiles[ shortest_direction ];
    if( entities->nextTiles[eid]->y > TILE_ROWS-1 ) {
        entities->nextTiles[eid]->y = 1;
    } 
    if( entities->nextTiles[eid]->y < 0 ){
        entities->nextTiles[eid]->y =  TILE_ROWS-1;
    }
    if( entities->nextTiles[eid]->x >  TILE_COLS-1 ) {
        entities->nextTiles[eid]->x = 1;
    }
    if( entities->nextTiles[eid]->x < 0 ) {
        entities->nextTiles[eid]->x =  TILE_COLS-1;
    }
}


void set_scatter_target_tile(  Entities *entities, EntityId ghostId, SDL_Point home_tile  ) {
    *entities->targetTiles[ghostId] = home_tile;
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
        currentDistance = distance( 
            entities->worldPositions[entityId]->x, 
            entities->worldPositions[entityId]->y, 
            entities->worldPositions[currentEntityId]->x, 
            entities->worldPositions[currentEntityId]->y 
        );
        if( currentDistance < closestDistance ) {
            closestDistance = currentDistance;
            closestEntityId = currentEntityId;
        }
    }

    return closestEntityId;

}

void set_shadow_target_tile( Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers ){
    // Actor *ghostActor = entities->actors[ ghostId ];
    if( numPlayers == 1 ) { // single player? automatically going to be closest player
        *entities->targetTiles[ghostId] = *entities->currentTiles[playerIds[0]];//entities->actors[ playerIds[0] ]->current_tile;
        return;
    }
    // determine which player is closest
    EntityId closestPlayerId = closestEntityToEntity( entities, ghostId, playerIds, numPlayers );

    *entities->targetTiles[ghostId] = *entities->currentTiles[closestPlayerId];//entities->actors[ closestPlayerId ]->current_tile;

}


inline void set_ambush_target_tile( Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers ) {
    // Actor *ghostActor = entities->actors[ ghostId ];
    
    // determine which player is closest
    EntityId closestPlayerId = closestEntityToEntity( entities, ghostId, playerIds, numPlayers );

    // target tile is in front of player based on their direction
    switch( *entities->directions[closestPlayerId] ) {
        case DIR_UP:
            entities->targetTiles[ghostId]->x =  entities->currentTiles[closestPlayerId]->x;
            entities->targetTiles[ghostId]->y = entities->currentTiles[closestPlayerId]->y - 4;
            break;
        case DIR_DOWN:
            entities->targetTiles[ghostId]->x =  entities->currentTiles[closestPlayerId]->x;
            entities->targetTiles[ghostId]->y =  entities->currentTiles[closestPlayerId]->y + 4;
            break;
        case DIR_LEFT:
            entities->targetTiles[ghostId]->x =  entities->currentTiles[closestPlayerId]->x - 4;
            entities->targetTiles[ghostId]->y =  entities->currentTiles[closestPlayerId]->y;
            break;
        case DIR_RIGHT:
            entities->targetTiles[ghostId]->x =  entities->currentTiles[closestPlayerId]->x + 4;
            entities->targetTiles[ghostId]->y =  entities->currentTiles[closestPlayerId]->y;
            break;
        default :
            entities->targetTiles[ghostId]->x =  entities->currentTiles[closestPlayerId]->x;
            entities->targetTiles[ghostId]->y =  entities->currentTiles[closestPlayerId]->y;
            break;
    }   
}

inline void set_moody_target_tile(Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers ) {
    // Most complex targeting scheme
    // Needs to have one entity with a targeting behavior of shadow to calculate

    EntityId closestPlayerId = closestEntityToEntity( entities, ghostId, playerIds, numPlayers );

    // find shadowghost to compare with for this targeting algorithm
    //Actor *shadowGhostActor = NULL;
    EntityId shadowGhostId = INVALID_ENTITY_ID;
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->targetingBehaviors[ eid ] == NULL ) {
            continue;
        }
        if( *entities->targetingBehaviors[ eid ] == SHADOW_BEHAVIOR ) {
            shadowGhostId = eid;
            break;
        }
    }
    // handle any error finding a shadow ghost
    if( shadowGhostId == INVALID_ENTITY_ID ) {
        fprintf( stderr, "Something went wrong. Moody tried to find shadow entity, but not found\n");
        fprintf( stderr, "Just setting moody as if shadow\n");
        entities->targetTiles[ghostId] = entities->currentTiles[closestPlayerId];
        return;
    }

    // determine target tile
    SDL_Point shadowGhostCurrentTile = *entities->currentTiles[shadowGhostId];
    SDL_Point offset_tile;
    int tiles_x;
    int tiles_y;

    switch( *entities->directions[closestPlayerId] ) {
        case DIR_UP:
            offset_tile.x = entities->currentTiles[closestPlayerId]->x;
            offset_tile.y = entities->currentTiles[closestPlayerId]->y - 2;
            

            tiles_x = offset_tile.x - shadowGhostCurrentTile.x;
            tiles_y = offset_tile.y - shadowGhostCurrentTile.y;

            entities->targetTiles[ghostId]->x = offset_tile.x + tiles_x;
            entities->targetTiles[ghostId]->y = offset_tile.y + tiles_y;

            break;
        case DIR_DOWN:
            offset_tile.x = entities->currentTiles[closestPlayerId]->x;
            offset_tile.y = entities->currentTiles[closestPlayerId]->y + 2;

            tiles_x = offset_tile.x - shadowGhostCurrentTile.x;
            tiles_y = offset_tile.y - shadowGhostCurrentTile.y;

            entities->targetTiles[ghostId]->x = offset_tile.x + tiles_x;
            entities->targetTiles[ghostId]->y = offset_tile.y + tiles_y;

            break;
        case DIR_LEFT:
            offset_tile.x = entities->currentTiles[closestPlayerId]->x - 2;
            offset_tile.y = entities->currentTiles[closestPlayerId]->y;

            tiles_x = offset_tile.x - shadowGhostCurrentTile.x;
            tiles_y = offset_tile.y - shadowGhostCurrentTile.y;

            entities->targetTiles[ghostId]->x = offset_tile.x + tiles_x;
            entities->targetTiles[ghostId]->y = offset_tile.y + tiles_y;

            break;
        case DIR_RIGHT:
            offset_tile.x = entities->currentTiles[closestPlayerId]->x + 2;
            offset_tile.y = entities->currentTiles[closestPlayerId]->y;

            tiles_x = offset_tile.x - shadowGhostCurrentTile.x;
            tiles_y = offset_tile.y - shadowGhostCurrentTile.y;

            entities->targetTiles[ghostId]->x = offset_tile.x + tiles_x;
            entities->targetTiles[ghostId]->y = offset_tile.y + tiles_y;

            break;
        default :
            offset_tile.x = entities->currentTiles[closestPlayerId]->x;
            offset_tile.y = entities->currentTiles[closestPlayerId]->y;

            tiles_x = offset_tile.x - shadowGhostCurrentTile.x;
            tiles_y = offset_tile.y - shadowGhostCurrentTile.y;

            entities->targetTiles[ghostId]->x = offset_tile.x + tiles_x;
            entities->targetTiles[ghostId]->y = offset_tile.y + tiles_y;

            break;
    }
    if ( entities->targetTiles[ghostId]->x < 0 ) entities->targetTiles[ghostId]->x = 0;
    if ( entities->targetTiles[ghostId]->y < 0 ) entities->targetTiles[ghostId]->y = 0;

    if ( entities->targetTiles[ghostId]->x > TILE_COLS - 1 ) entities->targetTiles[ghostId]->x = TILE_COLS - 1;
    if ( entities->targetTiles[ghostId]->y > TILE_ROWS - 1 ) entities->targetTiles[ghostId]->y = TILE_ROWS - 1;

}

inline void set_pokey_target_tile( Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers ) {
   // SDL_Point b_tile_above, b_tile_below, b_tile_left, b_tile_right;
    float shadow_range = 64;
    SDL_Point ghostCurrentTile = *entities->currentTiles[ghostId];

    EntityId closestPlayerId = closestEntityToEntity( entities, ghostId, playerIds, numPlayers );
    SDL_Point pac_current_tile = *entities->currentTiles[closestPlayerId];

    float distance_to_pacman = distance( ghostCurrentTile.x, ghostCurrentTile.y, pac_current_tile.x, pac_current_tile.y );

    if( distance_to_pacman >= shadow_range ) {
        set_shadow_target_tile( entities, ghostId, playerIds, numPlayers );
    }
    else {
        set_scatter_target_tile( entities, ghostId, pokey_home_tile );
    }
}