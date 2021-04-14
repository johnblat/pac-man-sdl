#include <SDL2/SDL.h>
#include "jb_types.h"
#include "animation.h"

typedef enum Direction {
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN,
    DIR_NONE
} Direction;


typedef struct Entities {
    unsigned int *texture_atlas_ids;
    Position_f *game_positions;
    SDL_Point *center_points;
    unsigned int *current_animation_frame_indeces;
    SDL_Rect *animation_frames_from_src_texture_atlas;
    SDL_Rect *src_texture_atlas_rects;
    SDL_Rect *dst_render_rects;
    Direction *current_directions;
    SDL_Point **sensors;
    SDL_Point *current_tiles;
    SDL_Point *target_tiles;
    unsigned int *movement_behaviors;
    float *speeds;
    Vector_f *current_velocities;
    AnimationTimer *animation_timers;

} Entities;

SDL_Texture **texture_atlases;
int number_of_texture_atlases;

SDL_Texture *get_texture_atalas_by_index( unsigned int index ) {
    if( index >=  number_of_texture_atlases ) {
        printf("Tried to access a texture atlas index %d beyond current number!\n", index);
        // should return some placeholder texture to use instead
        return NULL;
    } 
    SDL_Texture *accessed_texture_atlas = texture_atlases[ index ];
    if( accessed_texture_atlas == NULL ) {
        printf("Texture atlas accessed by %d index is null??\n", index);
        //should return placeholder texture
        return NULL;
    } 
    return accessed_texture_atlas;
}