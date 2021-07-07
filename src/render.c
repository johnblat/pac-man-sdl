#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "render.h"
#include "renderProcessing.h"
#include "actor.h"
#include "entity.h"
#include <assert.h>
#include "animation.h"


// const uint8_t MAX_TEXTURE_ATLASES = 10; 
// TextureAtlas g_texture_atlases[ MAX_TEXTURE_ATLASES ];
// uint8_t num_texture_atlases = 0;

uint8_t num_texture_atlases = 0; 

int addTextureAtlas( SDL_Renderer *renderer, const char *filename, int num_rows, int num_cols ) {
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

    int stride_cols = surface->w / num_cols;
    int stride_rows = surface->h / num_rows;
    int num_sprites = num_rows * num_cols;

    g_texture_atlases[ num_texture_atlases ].sprite_clips = (SDL_Rect *) malloc(sizeof(SDL_Rect) * num_sprites);

    for( int row = 0 ; row < num_rows; row ++ ) {
        for( int col = 0; col < num_cols; col++ ) {
            g_texture_atlases[ num_texture_atlases ].sprite_clips[ ( row * num_cols ) + col  ].x = col * stride_cols;
            g_texture_atlases[ num_texture_atlases ].sprite_clips[ ( row * num_cols ) + col ].y = row * stride_rows;
            g_texture_atlases[ num_texture_atlases ].sprite_clips[ ( row * num_cols ) + col ].w = stride_cols;
            g_texture_atlases[ num_texture_atlases ].sprite_clips[ ( row * num_cols ) + col ].h = stride_rows;
        }
    }


    SDL_FreeSurface( surface );

    g_texture_atlases[ num_texture_atlases ].rows = num_rows;
    g_texture_atlases[ num_texture_atlases ].cols = num_cols;
    
    return ++num_texture_atlases;
}


RenderData *renderDataInit( ) {
    
    RenderData *render_clip = ( RenderData * ) malloc (sizeof(RenderData ));

    render_clip->dest_rect.x = 0;
    render_clip->dest_rect.y = 0;
    render_clip->dest_rect.w = 0;
    render_clip->dest_rect.h = 0;

    render_clip->rotation = 0.0f;

    render_clip->current_sprite_clip = 0;

    render_clip->flip = SDL_FLIP_NONE;


    return render_clip;
}

void renderDataForAnimatedSpriteProcess( SDL_Renderer *renderer, Entities *entities)  {
    for( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
        if( entities->renderDatas[ eid ] == NULL || entities->animatedSprites[ eid ] == NULL ) {
            continue;
        }
        uint8_t anim_frame_col = entities->animatedSprites[ eid ]->current_frame_col;
        uint8_t anim_row = entities->animatedSprites[ eid ]->current_anim_row;
        uint8_t num_frames_col = entities->animatedSprites[ eid ]->num_frames_col;
        uint8_t clip_idx = (  ( anim_row * num_frames_col ) + anim_frame_col );

        uint8_t texture_atlas_id = entities->animatedSprites[ eid ]->texture_atlas_id;
        SDL_Rect src_rect = g_texture_atlases[ texture_atlas_id ].sprite_clips[ clip_idx ];
        SDL_Rect *dest_rect = &entities->renderDatas[ eid ]->dest_rect;
        float rotation = entities->renderDatas[ eid ]->rotation;
        SDL_RendererFlip flip = entities->renderDatas[ eid ]->flip;
        

        SDL_RenderCopyEx( renderer, g_texture_atlases[ texture_atlas_id ].texture, &src_rect, dest_rect, rotation, NULL, flip );
    }
}


void set_render_clip_values_based_on_positions_and_animation( Entities *entities, SDL_Point offset ) {
    for ( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
        if( entities->animatedSprites[ eid ] == NULL || entities->positions[ eid ] == NULL || entities->renderDatas[ eid ] == NULL ) {
            continue;
        }
        uint8_t texture_atlas_id = entities->animatedSprites[ eid ]->texture_atlas_id;
        uint8_t current_col = entities->animatedSprites[ eid ]->current_frame_col;
        uint8_t current_row = entities->animatedSprites[ eid ]->current_anim_row;
        uint8_t num_cols = entities->animatedSprites[ eid ]->num_frames_col; 
        uint8_t clip_idx = ( current_row * num_cols ) + current_col;
        entities->renderDatas[ eid ]->dest_rect.x =
            offset.x
            + entities->actors[ eid ]->world_position.x
            - ( g_texture_atlases[ texture_atlas_id ].sprite_clips[clip_idx ].w - ACTOR_SIZE ) / 2;
        entities->renderDatas[ eid ]->dest_rect.y = 
            offset.y
            + entities->actors[ eid ]->world_position.y 
            - ( g_texture_atlases[ texture_atlas_id ].sprite_clips[clip_idx ].h - ACTOR_SIZE ) / 2;
        entities->renderDatas[ eid ]->dest_rect.w = g_texture_atlases[ texture_atlas_id ].sprite_clips[ clip_idx].w;
        entities->renderDatas[ eid ]->dest_rect.h = g_texture_atlases[ texture_atlas_id ].sprite_clips[ clip_idx].h;

        entities->renderDatas[ eid ]->flip = SDL_FLIP_NONE;

    }
}

void updateScoreTexture( Score *score, SDL_Renderer *renderer ) {
    snprintf( score->score_text, 32, "Score : %d", score->score_number );
    SDL_Surface *score_surface = TTF_RenderText_Solid( score->font, score->score_text, score->score_color );

    SDL_DestroyTexture( score->score_texture );
    score->score_texture = SDL_CreateTextureFromSurface( renderer, score_surface );
    score->score_render_dst_rect.x = 10;
    score->score_render_dst_rect.y = 10;
    score->score_render_dst_rect.w = score_surface->w;
    score->score_render_dst_rect.h = score_surface->h;

    SDL_FreeSurface( score_surface );
}