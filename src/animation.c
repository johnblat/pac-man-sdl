#include <SDL2/SDL.h>
#include <stdlib.h>
#include "animation.h"
#include "entity.h"

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

void animatedSpriteIncProcess( AnimatedSprite **animatedSprites, float delta_time ) {
    for( int i = 0; i < MAX_NUM_ENTITIES; ++i ) {
        if( animatedSprites[ i ] == NULL ) {
            continue;
        }
        animatedSprites[ i ]->accumulator += delta_time;
        if( animatedSprites[ i ]->accumulator > animatedSprites[ i ]->frame_interval ) {
            animatedSprites[ i ]->accumulator = 0;
            animatedSprites[ i ]->current_frame_col++;
        }
        if( animatedSprites[ i ]->current_frame_col >= animatedSprites[ i ]->num_frames_col ) {
            animatedSprites[ i ]->current_frame_col = 0;
        }
    } 
}

// send actors + 1 as argument - will get first ghost
void set_animation_row( AnimatedSprite *animated_sprite, Direction direction ) {
    // switch ghost direction and set to proper anim row
    animated_sprite->current_anim_row = ( uint8_t ) direction;
}