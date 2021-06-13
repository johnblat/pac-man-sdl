#include <SDL2/SDL.h>
#include <stdlib.h>
#include "animation.h"
#include "actor.h"

AnimatedSprite *init_animation( uint8_t texture_atlas_id, uint8_t fps, uint8_t num_anims, Uint8 num_frames_per_anim ){
    AnimatedSprite *animation = (AnimatedSprite *) malloc(sizeof(AnimatedSprite));
    animation->frame_interval = ( (float) num_frames_per_anim / (float) fps ) / (float) num_frames_per_anim ; // length of time for each frame
    animation->accumulator = 0.0f;
    animation->current_anim_row = 0;
    animation->current_frame_col = 0;
    animation->num_frames_col = num_frames_per_anim;
    animation->texture_atlas_id = texture_atlas_id;
    animation->default_texture_atlas_id = texture_atlas_id;

    return animation;
}

float fps_to_frame_interval( int fps, int num_frames ) {
    return ( (float) num_frames / (float) fps ) / (float) num_frames ;
}

void inc_animations( AnimatedSprite **animations, uint8_t num_animations, float delta_time ) {
    for( int i = 0; i < num_animations; ++i ) {

        animations[ i ]->accumulator += delta_time;
        if( animations[ i ]->accumulator > animations[ i ]->frame_interval ) {
            animations[ i ]->accumulator = 0;
            animations[ i ]->current_frame_col++;
        }
        if( animations[ i ]->current_frame_col >= animations[ i ]->num_frames_col ) {
            animations[ i ]->current_frame_col = 0;
        }
    } 
}

// send actors + 1 as argument - will get first ghost
void set_animation_row( AnimatedSprite *animated_sprite, Actor *actor ) {
    animated_sprite->current_anim_row = ( uint8_t ) actor->direction;
}