#ifndef PAC_RENDER_H
#define PAC_RENDER_H

#include "stdlib.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "actor.h"
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

typedef struct RenderClipFromTextureAtlas {
    // uint8_t texture_atlas_id;
    uint8_t animation_id;
    Uint8 current_sprite_clip;
    float rotation;
    SDL_Rect dest_rect;
    SDL_RendererFlip flip;
} RenderClipFromTextureAtlas;


int add_texture_atlas( SDL_Renderer *renderer, const char *filename, int num_rows, int num_cols );

RenderClipFromTextureAtlas *init_render_clip( uint8_t texture_atlas_id, uint8_t animation_id ) ;

void render_render_textures( SDL_Renderer *renderer, RenderClipFromTextureAtlas **render_textures, AnimatedSprite **animations, int number_render_textures ) ;

//void set_render_texture_values_based_on_animation( AnimatedSprite **animations, RenderClipFromTextureAtlas **render_textures, int num ) ;
void set_render_clip_values_based_on_actor_and_animation( RenderClipFromTextureAtlas **render_clips, Actor **actors, SDL_Point offset, AnimatedSprite **animated_sprites, int num ) ;
//void set_render_texture_values_based_on_actor( Actor **actors, int offset_x, int offset_y, RenderClipFromTextureAtlas **render_textures, int num );

#endif