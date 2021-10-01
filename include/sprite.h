#ifndef sprite_h
#define sprite_h

#include "inttypes.h"

typedef struct AnimatedSprite {
    uint8_t texture_atlas_id;
    uint8_t default_texture_atlas_id;
    uint8_t current_anim_row;
    uint8_t current_frame_col;
    float frame_interval;
    float accumulator;
    uint8_t num_frames_col;
} AnimatedSprite;



#endif