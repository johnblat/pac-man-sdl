#ifndef PAC_RENDER_H
#define PAC_RENDER_H

#include "stdlib.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "actor.h"
#include "entity.h"
#include "animation.h"

/**
 * Responsible for holding render details of textures to be rendered in each cycle through the game loop
 */

typedef struct {
    SDL_Texture *texture;
    SDL_Rect *sprite_clips;
    uint8_t rows;
    uint8_t cols;
} TextureAtlas;

#define MAX_TEXTURE_ATLASES 10 
TextureAtlas g_texture_atlases[ MAX_TEXTURE_ATLASES ];
extern uint8_t num_texture_atlases;

typedef struct RenderData {
    Uint8 current_sprite_clip;
    float rotation;
    SDL_Rect dest_rect;
    SDL_RendererFlip flip;
} RenderData;


int addTextureAtlas( SDL_Renderer *renderer, const char *filename, int num_rows, int num_cols );

RenderData *renderDataInit(  ) ;


#endif