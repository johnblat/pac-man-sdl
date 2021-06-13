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
    STATE_GO_TO_PEN
} GhostState;

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


void vulnerable_enter_all( Actor **ghosts, RenderClipFromTextureAtlas **render_textures );

void vulnerable_enter( Actor **actors, AnimatedSprite **animated_sprites, uint8_t actor_id, RenderClipFromTextureAtlas *render_texture );


void set_vulnerable_direction_and_next_tile( Actor *ghost, TileMap *tm );

void vulnerable_process( Actor *actor, TileMap *tm );

void normal_enter( Actor **actors, AnimatedSprite **animated_sprites, uint8_t actor_id, RenderClipFromTextureAtlas *render_texture, uint8_t texture_atlas_id ) ;

void normal_process( Actor **actors, uint8_t ghost_id, TileMap *tm );

void go_to_pen_enter( Actor **actors, uint8_t actor_id, RenderClipFromTextureAtlas *render_texture, uint8_t id );


void go_to_pen_process( Actor *actor, TileMap *tm );

void states_machine_process( Actor **actors, GhostState *ghost_states, TileMap *tm );


#endif