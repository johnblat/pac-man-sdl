#ifndef ANIMATION_H
#define ANIMATION_H

#include <SDL2/SDL.h>
#include <inttypes.h>
#include <stdlib.h>
#include "sprite.h"
#include "entity.h"
/**
 * Keeps track of sprite based animation. Meaning each frame is drawn out
 */




AnimatedSprite *init_animation( uint8_t texture_atlas_id, uint8_t fps, uint8_t num_anims, Uint8 num_frames_per_anim );

float fps_to_frame_interval( int fps, int num_frames );

void animatedSpriteIncProcess( AnimatedSprite **animatedSprites, float delta_time ) ;

void set_animation_row(AnimatedSprite *animated_sprite, Direction direction ) ;


#endif