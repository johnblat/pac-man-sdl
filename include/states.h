#ifndef STATES_H
#define STATES_H

#include "targeting.h"
#include "actor.h"
#include "render.h"
#include "targeting.h"
#include "movement.h"
#include "comparisons.h"

extern SDL_Point ghost_pen_tile;

typedef enum {
    STATE_NORMAL,
    STATE_VULNERABLE,
    STATE_GO_TO_PEN,
    STATE_LEAVE_PEN
} GhostState;

typedef enum {
    SHADOW_BEHAVIOR,
    AMBUSH_BEHAVIOR,
    MOODY_BEHAVIOR,
    POKEY_BEHAVIOR
} TargetingBehavior;



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
extern uint8_t g_scatter_chase_period_seconds[ NUM_SCATTER_CHASE_PERIODS ];
//uint8_t g_scatter_chase_period_seconds[ NUM_SCATTER_CHASE_PERIODS ] = { 2, 2, 2, 2, 2, 2, 2, 0 };
typedef enum {
    MODE_CHASE,     // use default behavior
    MODE_SCATTER,   // use scatter behavior
} GhostMode;

extern GhostMode g_current_ghost_mode;


void vulnerable_enter_all( Entities *entities );

void vulnerable_enter( Entities *entities, EntityId ghostId );

void set_vulnerable_direction_and_next_tile( Entities *entities, EntityId ghostId, TileMap *tm );

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
void normal_process( Entities *entities, EntityId ghostId, EntityId *playerIds, unsigned int numPlayers, TileMap *tm );

/**
 * Sets the animation texture atlas to be the eyes
 */
void go_to_pen_enter( Entities *entities, EntityId ghostId );

/**
 * Will target the ghost pen tile
 */
void go_to_pen_process( Entities *entities, EntityId ghostId, TileMap *tm );


void leave_pen_enter( Entities *entities, EntityId ghostId );

/**
 * Targets a few tiles above ghost pen
 */
void leave_pen_process( Entities *entities, EntityId ghostId, TileMap *tm );

void states_machine_process( Actor **actors, GhostState *ghost_states, TileMap *tm );


#endif