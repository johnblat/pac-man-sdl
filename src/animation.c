#include <SDL2/SDL.h>
#include <stdlib.h>
#include "animation.h"

AnimatedSprite *init_animation( uint8_t texture_atlas_id, uint8_t fps, Uint8 num_frames ){
    AnimatedSprite *animation = (AnimatedSprite *) malloc(sizeof(AnimatedSprite));
    animation->frame_interval = ( (float) num_frames / (float) fps ) / (float) num_frames ; // length of time for each frame
    animation->accumulator = 0.0f;
    animation->current_frame = 0;
    animation->num_frames = num_frames;
    animation->texture_atlas_id = texture_atlas_id;

    return animation;
}

void inc_animations( AnimatedSprite **animations, uint8_t num_animations, float delta_time ) {
    for( int i = 0; i < num_animations; ++i ) {

        animations[ i ]->accumulator += delta_time;
        if( animations[ i ]->accumulator > animations[ i ]->frame_interval ) {
            animations[ i ]->accumulator = 0;
            animations[ i ]->current_frame++;
        }
        if( animations[ i ]->current_frame >= animations[ i ]->num_frames ) {
            animations[ i ]->current_frame = 0;
        }
    } 
}
