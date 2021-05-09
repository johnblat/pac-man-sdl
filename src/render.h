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
    uint8_t num_sprite_clips;
} TextureAtlas;

const uint8_t MAX_TEXTURE_ATLASES = 10; 
TextureAtlas g_texture_atlases[ MAX_TEXTURE_ATLASES ];
uint8_t num_texture_atlases = 0;

typedef struct RenderTexture {
    uint8_t texture_atlas_id;
    Uint8 current_sprite_clip;
    float rotation;
    SDL_Rect dest_rect;
    SDL_RendererFlip flip;
} RenderTexture;

int add_texture_atlas( SDL_Renderer *renderer, const char *filename, int num_sprites ) {
    if( num_texture_atlases >= MAX_TEXTURE_ATLASES ) {
        fprintf(stderr, "Can't add more texture atlases. Limit of %d reached\n", MAX_TEXTURE_ATLASES );
        return -1;
    }

    SDL_Surface *surface = IMG_Load( filename );
    if( surface == NULL ) {
        fprintf( stderr, "Error %s\n ", IMG_GetError() );
        exit( EXIT_FAILURE );
    }

    g_texture_atlases[ num_texture_atlases ].texture = SDL_CreateTextureFromSurface( renderer, surface );
    if( g_texture_atlases[ num_texture_atlases ].texture == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    int stride = surface->w / num_sprites;

    g_texture_atlases[ num_texture_atlases ].sprite_clips = (SDL_Rect *) malloc(sizeof(SDL_Rect) * num_sprites);
    for( int i = 0; i < num_sprites; ++i ) {
        g_texture_atlases[ num_texture_atlases ].sprite_clips[ i ].x = i * stride;
        g_texture_atlases[ num_texture_atlases ].sprite_clips[ i ].y = 0;
        g_texture_atlases[ num_texture_atlases ].sprite_clips[ i ].w = stride;
        g_texture_atlases[ num_texture_atlases ].sprite_clips[ i ].h = stride;
    }

    SDL_FreeSurface( surface );

    g_texture_atlases[ num_texture_atlases ].num_sprite_clips = num_sprites;
    
    return ++num_texture_atlases;
}

RenderTexture *init_render_texture( uint8_t texture_atlas_id ) {
    
    RenderTexture *render_texture = ( RenderTexture * ) malloc (sizeof(RenderTexture ));

    if( texture_atlas_id >= num_texture_atlases)  {
        fprintf(stderr, "Trying to assign texture atlas id %d that's more than the number of texture atlases created %d \n", texture_atlas_id, num_texture_atlases );
        return NULL;
    }

    render_texture->texture_atlas_id = texture_atlas_id;
    
    render_texture->dest_rect.x = 0;
    render_texture->dest_rect.y = 0;
    render_texture->dest_rect.w = 0;
    render_texture->dest_rect.h = 0;

    render_texture->rotation = 0.0f;

    render_texture->current_sprite_clip = 0;

    render_texture->flip = SDL_FLIP_NONE;


    return render_texture;
}

void render_render_textures( SDL_Renderer *renderer, RenderTexture **render_textures, int number_render_textures ) {
    for( int i = 0; i < number_render_textures; ++i ) {
        SDL_Rect src_rect = g_texture_atlases[ render_textures[ i ]->texture_atlas_id ].sprite_clips[ render_textures[ i ]->current_sprite_clip ];
        SDL_RenderCopyEx( renderer, g_texture_atlases[ render_textures[ i ]->texture_atlas_id ].texture, &src_rect, &render_textures[ i ]->dest_rect, render_textures[ i ]->rotation, NULL, render_textures[ i ]->flip);
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
        render_textures[ i ]->dest_rect.x = actors[ i ]->position.x - ( g_texture_atlases[ render_textures[ i ]->texture_atlas_id ].sprite_clips[ render_textures[ i ]->current_sprite_clip ].w - ACTOR_SIZE ) / 2;
        render_textures[ i ]->dest_rect.y = actors[ i ]->position.y - ( g_texture_atlases[ render_textures[ i ]->texture_atlas_id ].sprite_clips[ render_textures[ i ]->current_sprite_clip ].h  - ACTOR_SIZE ) / 2;
        render_textures[ i ]->dest_rect.w = g_texture_atlases[ render_textures[ i ]->texture_atlas_id ].sprite_clips[ render_textures[ i ]->current_sprite_clip ].w;
        render_textures[ i ]->dest_rect.h = g_texture_atlases[ render_textures[ i ]->texture_atlas_id ].sprite_clips[ render_textures[ i ]->current_sprite_clip ].h;

        render_textures[ i ]->flip = actors[ i ]->direction == DIR_LEFT ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    }
}

#endif