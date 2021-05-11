#include <SDL2/SDL.h>
#include <stdlib.h>
#include "animation.h"

Animation *init_animation( Uint8 actor_id, float frame_interval, Uint8 num_frames ){
    Animation *animation = (Animation *) malloc(sizeof(Animation));
    animation->actor_id = actor_id;
    animation->playing = SDL_TRUE;
    animation->frame_interval = frame_interval;
    animation->accumulator = 0.0f;
    animation->current_frame = 0;
    animation->num_frames = num_frames;

    return animation;
}

void inc_animation_frame( Animation *animations, uint8_t num_animations, float delta_time ) {
    for( int i = 0; i < num_animations; ++i ) {
        if ( !animations[ i ].playing ) continue;

        animations[ i ].accumulator += delta_time;
        if( animations[ i ].accumulator > animations[ i ].frame_interval ) {
            animations[ i ].accumulator = 0;
            animations[ i ].current_frame++;
        }
        if( animations[ i ].current_frame >= animations[ i ].num_frames ) {
            animations[ i ].current_frame = 0;
        }
    } 
}
