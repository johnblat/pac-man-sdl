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

typedef struct RenderTexture {
    SDL_Texture *texture_atlas;
    SDL_Rect *sprite_clips;
    Uint8 num_sprite_clips;
    Uint8 current_sprite_clip;
    float rotation;
    SDL_Rect dest_rect;
    SDL_RendererFlip flip;
} RenderTexture;

RenderTexture *init_render_texture(SDL_Renderer *renderer, const char *filename, int num_sprites ) {
    RenderTexture *render_texture = ( RenderTexture * ) malloc(sizeof(RenderTexture));
    
    SDL_Surface *surface = IMG_Load( filename );
    if( surface == NULL ) {
        fprintf( stderr, "Error %s\n ", IMG_GetError() );
        exit( EXIT_FAILURE );
    }

    render_texture->texture_atlas = SDL_CreateTextureFromSurface( renderer, surface );
    if( render_texture->texture_atlas == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    int stride = surface->w / num_sprites;

    render_texture->sprite_clips = (SDL_Rect *) malloc(sizeof(SDL_Rect) * num_sprites);
    for( int i = 0; i < num_sprites; ++i ) {
        render_texture->sprite_clips[ i ].x = i * stride;
        render_texture->sprite_clips[ i ].y = 0;
        render_texture->sprite_clips[ i ].w = stride;
        render_texture->sprite_clips[ i ].h = stride;
    }
    // render_texture->sprite_clips[ 0 ].x = 0;
    // render_texture->sprite_clips[ 0 ].y = 0;
    // render_texture->sprite_clips[ 0 ].w = stride;
    // render_texture->sprite_clips[ 0 ].h = stride;
    // render_texture->sprite_clips[ 1 ].x = stride;
    // render_texture->sprite_clips[ 1 ].y = 0;
    // render_texture->sprite_clips[ 1 ].w = stride;
    // render_texture->sprite_clips[ 1 ].h = stride;
    // render_texture->sprite_clips[ 2 ].x = stride * 2;
    // render_texture->sprite_clips[ 2 ].y = 0;
    // render_texture->sprite_clips[ 2 ].w = stride;
    // render_texture->sprite_clips[ 2 ].h = stride;
    // render_texture->sprite_clips[ 3 ].x = stride * 3;
    // render_texture->sprite_clips[ 3 ].y = 0;
    // render_texture->sprite_clips[ 3 ].w = stride;
    // render_texture->sprite_clips[ 3 ].h = stride;

    render_texture->dest_rect.x = 0;
    render_texture->dest_rect.y = 0;
    render_texture->dest_rect.w = 0;
    render_texture->dest_rect.h = 0;

    render_texture->rotation = 0.0f;

    render_texture->num_sprite_clips = num_sprites;
    render_texture->current_sprite_clip = 0;

    render_texture->flip = SDL_FLIP_NONE;

    SDL_FreeSurface( surface );

    return render_texture;
}

void render_render_textures( SDL_Renderer *renderer, RenderTexture **render_textures, int number_render_textures ) {
    for( int i = 0; i < number_render_textures; ++i ) {
        SDL_Rect src_rect = render_textures[ i ]->sprite_clips[ render_textures[ i ]->current_sprite_clip ];
        SDL_RenderCopyEx( renderer, render_textures[ i ]->texture_atlas, &src_rect, &render_textures[ i ]->dest_rect, render_textures[ i ]->rotation, NULL, render_textures[ i ]->flip);
    }
}


void set_render_texture_values_based_on_animation( Animation **animations, RenderTexture **render_textures, int num ) {
    for( int i = 0; i < num; ++i ){
        render_textures[ i ]->current_sprite_clip = animations[ i ]->current_frame;
    }
}

void set_render_texture_values_based_on_actor( Actor **actors, RenderTexture **render_textures, int num ) {
    for( int i = 0; i < num; ++i ) {
        // center the sprite around the actor size
        render_textures[ i ]->dest_rect.x = actors[ i ]->position.x - ( render_textures[ i ]->sprite_clips[ render_textures[ i ]->current_sprite_clip ].w - ACTOR_SIZE ) / 2;
        render_textures[ i ]->dest_rect.y = actors[ i ]->position.y - ( render_textures[ i ]->sprite_clips[ render_textures[ i ]->current_sprite_clip ].h  - ACTOR_SIZE ) / 2;
        render_textures[ i ]->dest_rect.w = render_textures[ i ]->sprite_clips[ render_textures[ i ]->current_sprite_clip ].w;
        render_textures[ i ]->dest_rect.h = render_textures[ i ]->sprite_clips[ render_textures[ i ]->current_sprite_clip ].h;

        render_textures[ i ]->flip = actors[ i ]->direction == DIR_LEFT ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    }
}

#endif