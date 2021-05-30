#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "render.h"
#include "actor.h"
#include <assert.h>
#include "animation.h"


// const uint8_t MAX_TEXTURE_ATLASES = 10; 
// TextureAtlas g_texture_atlases[ MAX_TEXTURE_ATLASES ];
// uint8_t num_texture_atlases = 0;

uint8_t num_texture_atlases = 0; 

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


RenderClipFromTextureAtlas *init_render_clip( uint8_t texture_atlas_id, uint8_t animation_id ) {
    
    RenderClipFromTextureAtlas *render_clip = ( RenderClipFromTextureAtlas * ) malloc (sizeof(RenderClipFromTextureAtlas ));

    if( texture_atlas_id >= num_texture_atlases)  {
        fprintf(stderr, "Trying to assign texture atlas id %d that's more than the number of texture atlases created %d \n", texture_atlas_id, num_texture_atlases );
        return NULL;
    }
    assert( texture_atlas_id >= num_texture_atlases );

    //render_clip->texture_atlas_id = texture_atlas_id;
    render_clip->animation_id = animation_id;

    render_clip->dest_rect.x = 0;
    render_clip->dest_rect.y = 0;
    render_clip->dest_rect.w = 0;
    render_clip->dest_rect.h = 0;

    render_clip->rotation = 0.0f;

    render_clip->current_sprite_clip = 0;

    render_clip->flip = SDL_FLIP_NONE;


    return render_clip;
}

void render_render_textures( SDL_Renderer *renderer, RenderClipFromTextureAtlas **render_clips, AnimatedSprite **animations, int number_render_textures ) {
    for( int i = 0; i < number_render_textures; ++i ) {
        uint8_t animation_id = render_clips[ i ]->animation_id;
        uint8_t texture_atlas_id = animations[ animation_id ]->texture_atlas_id;
        SDL_Rect src_rect = g_texture_atlases[ texture_atlas_id ].sprite_clips[ animations[ animation_id ]->current_frame ];
        
        SDL_RenderCopyEx( renderer, g_texture_atlases[ texture_atlas_id ].texture, &src_rect, &render_clips[ i ]->dest_rect, render_clips[ i ]->rotation, NULL, render_clips[ i ]->flip);
    }
}


// void set_render_texture_values_based_on_animation( AnimatedSprite **animations, RenderClipFromTextureAtlas **render_clips, int num ) {
//     for( int i = 0; i < num ; ++i ){
//         render_clips[ i ]->current_sprite_clip = animations[ render_clips[ i ]->animation_id ]->current_frame ;
//     }
// }

// void set_render_texture_values_based_on_actor( Actor **actors, int offset_x, int offset_y, RenderClipFromTextureAtlas **render_clips, int num ) {
//     for( int i = 0; i < num; ++i ) {
//         // center the sprite around the actor size
//         render_clips[ i ]->dest_rect.x = 
//               offset_x  
//             + actors[ i ]->world_position.x 
//             - ( g_texture_atlases[ render_clips[ i ]->texture_atlas_id ].sprite_clips[ render_clips[ i ]->current_sprite_clip ].w - ACTOR_SIZE ) / 2;
//         render_clips[ i ]->dest_rect.y = offset_y + actors[ i ]->world_position.y - ( g_texture_atlases[ render_clips[ i ]->texture_atlas_id ].sprite_clips[ render_clips[ i ]->current_sprite_clip ].h  - ACTOR_SIZE ) / 2;
//         render_clips[ i ]->dest_rect.w = g_texture_atlases[ render_clips[ i ]->texture_atlas_id ].sprite_clips[ render_clips[ i ]->current_sprite_clip ].w;
//         render_clips[ i ]->dest_rect.h = g_texture_atlases[ render_clips[ i ]->texture_atlas_id ].sprite_clips[ render_clips[ i ]->current_sprite_clip ].h;

//         render_clips[ i ]->flip = SDL_FLIP_NONE;
//     }
// }

void set_render_clip_values_based_on_actor_and_animation( RenderClipFromTextureAtlas **render_clips, Actor **actors, SDL_Point offset, AnimatedSprite **animated_sprites, int num ) {
    for ( int i = 0; i < num; ++i ) {
        uint8_t animation_id = render_clips[ i ]->animation_id;
        uint8_t texture_atlas_id = animated_sprites[ animation_id ]->texture_atlas_id;
        uint8_t current_frame = animated_sprites[ animation_id ]->current_frame;
        render_clips[ i ]->dest_rect.x =
            offset.x
            + actors[ i ]->world_position.x
            - ( g_texture_atlases[ texture_atlas_id ].sprite_clips[ current_frame ].w - ACTOR_SIZE ) / 2;
        render_clips[ i ]->dest_rect.y = 
            offset.y
            + actors[ i ]->world_position.y 
            - ( g_texture_atlases[ texture_atlas_id ].sprite_clips[ current_frame ].h - ACTOR_SIZE ) / 2;
        render_clips[ i ]->dest_rect.w = g_texture_atlases[ texture_atlas_id ].sprite_clips[ current_frame ].w;
        render_clips[ i ]->dest_rect.h = g_texture_atlases[ texture_atlas_id ].sprite_clips[ current_frame ].h;

        render_clips[ i ]->flip = SDL_FLIP_NONE;

    }
}