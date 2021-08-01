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

SDL_Renderer *gRenderer = NULL;


typedef struct {
    SDL_Texture *texture;
    char textureName[16];
    SDL_Rect *sprite_clips;
    uint8_t rows;
    uint8_t cols;
} TextureAtlas;

#define MAX_TEXTURE_ATLASES 20 
TextureAtlas g_texture_atlases[ MAX_TEXTURE_ATLASES ];
extern uint8_t num_texture_atlases;

typedef struct RenderData {
    Uint8 textureAtlasId;
    Uint8 spriteClipIdx;
    float rotation;
    SDL_Rect dest_rect;
    SDL_RendererFlip flip;
    uint8_t alphaMod;
} RenderData;


int addTextureAtlas( SDL_Renderer *renderer, const char *textureName, const char *filename, int num_rows, int num_cols );

RenderData *renderDataInit(  ) ;

void updateScoreTexture( Score *score, SDL_Renderer *renderer );

/**
 * Sorts the renderDatas based on their Y position in the destRects
 * Smaller Ys are behind Larger Ys
 * The bigger the Y, the closer it is to you!
 * 
 * Returns numebr of sorted renderDatas
 */
int render_sort( SDL_bool **isActive, RenderData **renderDatas, RenderData **renderDatasSortArr );

SDL_Texture *createTextTexture( SDL_Rect *destRect, const char *text, SDL_Color color, int screenPosCenterX, int screenPosCenterY );
#endif