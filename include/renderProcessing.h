#include <SDL2/SDL.h>
#include "entity.h"

void renderDataForAnimatedSpriteProcess( SDL_Renderer *renderer, Entities *entities)  ;


/**
 * Changes the sprite clip if on new frame in the animation, also sets the dest rect position based on 
 * the positionof the entity
 */
void set_render_clip_values_based_on_positions_and_animation( Entities *entities, SDL_Point offset )  ;

