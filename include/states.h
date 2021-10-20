#ifndef STATES_H
#define STATES_H

#include "entity.h"
#include "resources.h"
#include "tiles.h"
#include "userTypeDefinitions.h"

extern SDL_Point ghost_pen_tile;


/**
 * Will be global for all ghosts.
 * Will determine their more specific
 * behavior
 */
/*****************
 * MODES *********
 * *************/
#define NUM_SCATTER_CHASE_PERIODS 9
extern uint8_t g_current_scatter_chase_period;
extern uint32_t g_scatter_chase_period_seconds[ NUM_SCATTER_CHASE_PERIODS ];
//uint8_t g_scatter_chase_period_seconds[ NUM_SCATTER_CHASE_PERIODS ] = { 2, 2, 2, 2, 2, 2, 2, 0 };
typedef enum {
    MODE_CHASE,     // use default behavior
    MODE_SCATTER,   // use scatter behavior
} GhostMode;

extern GhostMode g_current_ghost_mode;


void vulnerable_enter_all( Entities *entities );

void vulnerableEnter(AnimatedSprite *as, Direction *d, CurrentTile *ct, NextTile *nt, SpeedMultiplier *sm);

// void vulnerable_enter( Entities *entities, EntityId ghostId );

void set_random_direction_and_next_tile(ecs_entity_t eid, TileMap *tm  );

/**
 * Ghosts will basically wander aimlessly in the priority tiles. They will not choose targets.
 */
void vulnerable_process( Entities *entities, EntityId ghostId, TileMap *tm );

void normal_enter( Entities *entities, EntityId ghostId ) ;

/**
 * If in chase mode, will target closest player and process using the entity's target behavior.
 * If in scatter mode, will go to home tile assigned.
 * 
 * playerIds: list of players to target. Will target closest player. 
 */
void normal_process( Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers, TileMap *tm, LevelConfig *levelConfig );

/**
 * Sets the animation texture atlas to be the eyes
 */
void go_to_pen_enter( Entities *entities, EntityId ghostId );

/**
 * Will target the ghost pen tile
 */
void go_to_pen_process( Entities *entities, LevelConfig *levelConfig, EntityId ghostId, TileMap *tm );


void leave_pen_enter( Entities *entities, EntityId ghostId );

/**
 * Targets a few tiles above ghost pen
 */
void leave_pen_process( Entities *entities, EntityId ghostId, TileMap *tm );


void stayPenEnter( Entities *entities, LevelConfig *levelConfig, EntityId ghostId );

void stayPenProcess( Entities *entities, LevelConfig *levelConfig, TileMap *tilemap, EntityId ghostId );

#endif