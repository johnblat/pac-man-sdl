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
#include <string.h>
#include "globalData.h"
#include "UI.h"


// const uint8_t MAX_TEXTURE_ATLASES = 10; 
// TextureAtlas g_texture_atlases[ MAX_TEXTURE_ATLASES ];
uint8_t num_texture_atlases = 0;

int addTextureAtlas( SDL_Renderer *renderer, const char *textureName, const char *filename, int num_rows, int num_cols ) {
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

    strncpy( g_texture_atlases[ num_texture_atlases ].textureName, textureName, 16); 

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

int render_sort( SDL_bool **isActive, RenderData **renderDatas, RenderData **renderDatasSortArr )
{
	if(MAX_NUM_ENTITIES <= 1)
		return 0 ;
		

    // clear the sort arr
    for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
        renderDatasSortArr[ i ] = NULL;
    }

    // put in initial values into sort arr
    unsigned int numRenderDatasCopied = 0;
    for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
        // don't render things we can't even render
        if( renderDatas[i] == NULL ) {
            continue;
        }
        // don't add deactivated entities' renderDatas
        if( isActive[i] != NULL && *isActive[i] == SDL_FALSE ) {
            continue;
        }
        
        renderDatasSortArr[numRenderDatasCopied] = renderDatas[i];
        numRenderDatasCopied++;
    }
	
    // sort the sort arr
	for(int i = 1; i < numRenderDatasCopied; i++)
	{
		int j = i - 1;
        RenderData *tempPtr = renderDatasSortArr[i];

		while( ( j >= 0 ) && ( renderDatasSortArr[ j ]->dest_rect.y > tempPtr->dest_rect.y ) )
		{
			renderDatasSortArr[j + 1] = renderDatasSortArr[j];
			j--;
		}
		renderDatasSortArr[j+1] = tempPtr;
	}

    return numRenderDatasCopied;
}

RenderData *renderDataInit( ) {
    
    RenderData *render_clip = ( RenderData * ) malloc (sizeof(RenderData ));

    render_clip->dest_rect.x = 0;
    render_clip->dest_rect.y = 0;
    render_clip->dest_rect.w = 0;
    render_clip->dest_rect.h = 0;

    render_clip->rotation = 0.0f;

    render_clip->textureAtlasId = 0;

    render_clip->flip = SDL_FLIP_NONE;

    render_clip->alphaMod = 255;

    render_clip->scale = 1.0f;


    return render_clip;
}

void renderDataForAnimatedSpriteProcess( SDL_Renderer *renderer, Entities *entities)  {
    // set values from animated sprite
    for( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
        if( entities->renderDatas[ eid ] == NULL || entities->animatedSprites[ eid ] == NULL ) {
            continue;
        }
        uint8_t anim_frame_col = entities->animatedSprites[ eid ]->current_frame_col;
        uint8_t anim_row = entities->animatedSprites[ eid ]->current_anim_row;
        uint8_t num_frames_col = entities->animatedSprites[ eid ]->num_frames_col;
        uint8_t clip_idx = (  ( anim_row * num_frames_col ) + anim_frame_col );

        uint8_t texture_atlas_id = entities->animatedSprites[ eid ]->texture_atlas_id;
        
        entities->renderDatas[eid]->textureAtlasId = texture_atlas_id;
        entities->renderDatas[ eid ]->spriteClipIdx = clip_idx;

        
    }
    
    // sort the renderDatas based on how close they are to the bottom of screen
    RenderData *renderDatasSortArr[MAX_NUM_ENTITIES];
    int numSortedRenderDatas = render_sort(entities->isActive, entities->renderDatas, renderDatasSortArr );

    // render
    for( int i = 0; i < numSortedRenderDatas; i++ ) {

        uint8_t textureAtlasId = renderDatasSortArr[ i ]->textureAtlasId;
        Uint8  clip_idx = renderDatasSortArr[i]->spriteClipIdx;
        SDL_Rect src_rect = g_texture_atlases[ textureAtlasId ].sprite_clips[ clip_idx ];
        SDL_Rect *dest_rect = &renderDatasSortArr[ i ]->dest_rect;
        float rotation = renderDatasSortArr[ i ]->rotation;
        SDL_RendererFlip flip = renderDatasSortArr[ i ]->flip;

        SDL_SetTextureAlphaMod( g_texture_atlases[ textureAtlasId ].texture,  renderDatasSortArr[ i ]->alphaMod ); 

        SDL_RenderCopyEx( renderer, g_texture_atlases[ textureAtlasId ].texture, &src_rect, dest_rect, rotation, NULL, flip );
    }
}


void set_render_clip_values_based_on_positions_and_animation( Entities *entities, SDL_Point offset ) {
    for ( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
        if( entities->animatedSprites[ eid ] == NULL || entities->worldPositions[ eid ] == NULL || entities->renderDatas[ eid ] == NULL ) {
            continue;
        }
        uint8_t texture_atlas_id = entities->animatedSprites[ eid ]->texture_atlas_id;
        if( texture_atlas_id == 255 ) continue;
        uint8_t current_col = entities->animatedSprites[ eid ]->current_frame_col;
        uint8_t current_row = entities->animatedSprites[ eid ]->current_anim_row;
        uint8_t num_cols = entities->animatedSprites[ eid ]->num_frames_col; 
        uint8_t clip_idx = ( current_row * num_cols ) + current_col;
        
        entities->renderDatas[ eid ]->dest_rect.w = g_texture_atlases[ texture_atlas_id ].sprite_clips[ clip_idx].w;
        entities->renderDatas[ eid ]->dest_rect.h = g_texture_atlases[ texture_atlas_id ].sprite_clips[ clip_idx].h;

        // adjust for scale
        entities->renderDatas[eid]->dest_rect.w *= entities->renderDatas[eid]->scale;
        entities->renderDatas[eid]->dest_rect.h *= entities->renderDatas[eid]->scale;

        int centerx = entities->worldPositions[eid]->x;
        int centery = entities->worldPositions[eid]->y;


        int w = entities->renderDatas[eid]->dest_rect.w;
        int h = entities->renderDatas[eid]->dest_rect.h;
        entities->renderDatas[ eid ]->dest_rect.x =
            offset.x
            + centerx
            - ( w ) / 2;
        entities->renderDatas[ eid ]->dest_rect.y = 
            offset.y
            + centery 
            - ( h ) / 2;

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

void updateLivesRemainingTexture(LivesRemainingUI *lr ) {
    lr->livesRemaining = gLivesRemaining;
    snprintf( lr->text, 16, "Lives: %d", lr->livesRemaining );
    SDL_Surface *lr_surface = TTF_RenderText_Solid(lr->font, lr->text, lr->color );

    SDL_DestroyTexture( lr->texture );
    lr->texture = SDL_CreateTextureFromSurface( gRenderer, lr_surface );
    lr->destRect.x = 1800;
    lr->destRect.y = 10;
    lr->destRect.w = lr_surface->w;
    lr->destRect.h = lr_surface->h;

    SDL_FreeSurface(lr_surface);
}

SDL_Texture *createTextTexture( SDL_Rect *destRect, const char *text, SDL_Color color, TTF_Font *font, int screenPosCenterX, int screenPosCenterY ) {
    SDL_Surface *surface = TTF_RenderText_Solid( font, text, color );
    SDL_Texture *targetTexture = SDL_CreateTextureFromSurface( gRenderer, surface );
    destRect->w = surface->w;
    destRect->h = surface->h;
    destRect->x = screenPosCenterX - ( destRect->w / 2 ); 
    destRect->y = screenPosCenterY - ( destRect->h / 2 );
    SDL_FreeSurface( surface );
    surface = NULL;

    return targetTexture;
}

void renderCircleFill( SDL_Renderer *renderer, Uint32 centerX, Uint32 centerY, Uint32 radius) {
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    SDL_RenderDrawLine(renderer,centerX + x , centerY +  y, centerX - x , centerY +  y  );
    SDL_RenderDrawLine(renderer, centerX + x , centerY -  y, centerX - x , centerY -  y );
    SDL_RenderDrawLine(renderer, centerX + y , centerY +  x, centerX - y , centerY +  x);
    SDL_RenderDrawLine(renderer, centerX + y , centerY -  x, centerX - y , centerY -  x);

    while( y >= x ) {
        x++;
        if( d > 0 ) {
            y--;
            d = d + 4 * ( x - y ) + 10;
        }
        else {
            d = d + 4 * x + 6;
        }
        SDL_RenderDrawLine( renderer, centerX + x , centerY +  y, centerX - x , centerY +  y);
        SDL_RenderDrawLine( renderer, centerX + x , centerY -  y, centerX - x , centerY -  y );
        SDL_RenderDrawLine( renderer, centerX + y , centerY +  x , centerX - y , centerY +  x);
        SDL_RenderDrawLine( renderer,  centerX + y , centerY -  x, centerX - y , centerY -  x );
    }
}