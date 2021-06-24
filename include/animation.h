#ifndef ANIMATION_H
#define ANIMATION_H

#include <SDL2/SDL.h>
#include <inttypes.h>
#include <stdlib.h>
#include "actor.h"

/**
 * Keeps track of sprite based animation. Meaning each frame is drawn out
 */

// typedef struct AnimatedSprite {
//     //uint8_t actor_id;
//     //SDL_bool playing;
//     uint8_t texture_atlas_id;
//     float frame_interval;
//     float accumulator;
//     uint8_t current_frame;
//     uint8_t num_frames;
// } AnimatedSprite;

typedef struct AnimatedSprite {
    uint8_t texture_atlas_id;
    uint8_t default_texture_atlas_id;
    uint8_t current_anim_row;
    uint8_t current_frame_col;
    float frame_interval;
    float accumulator;
    uint8_t num_frames_col;

    
} AnimatedSprite;



// typedef struct AnimationRow {

// }

AnimatedSprite *init_animation( uint8_t texture_atlas_id, uint8_t fps, uint8_t num_anims, Uint8 num_frames_per_anim );

float fps_to_frame_interval( int fps, int num_frames );

void inc_animations( AnimatedSprite **animations, uint8_t num_animations, float delta_time ) ;

void set_animation_row(AnimatedSprite *animated_sprite, Actor *actor ) ;


#endif