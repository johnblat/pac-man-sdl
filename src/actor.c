#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include "animation.h"
#include "actor.h"
#include "jb_types.h"
#include "constants.h"
#include "tiles.h"

Direction opposite_directions[ 4 ] = { DIR_DOWN, DIR_UP, DIR_RIGHT, DIR_LEFT };

void actor_set_current_tile( Actor *actor ) {
    actor->current_tile.x = ( ( actor->position.x + TILE_SIZE / 2 ) / TILE_SIZE ) ;
    actor->current_tile.y = ( ( ( actor->position.y + TILE_SIZE / 2 ) - (TILE_SIZE * 2 ) ) / TILE_SIZE ) ;
}

Actor *init_actor( Position_f initial_position ) {
    Actor *actor;
    actor = ( Actor *) malloc( sizeof( Actor ) );

    actor->position.x = initial_position.x;
    actor->position.y = initial_position.y;

    actor->center_point.x = ( int ) actor->position.x + ( ACTOR_SIZE / 2 );
    actor->center_point.y = ( int ) actor->position.y + ( ACTOR_SIZE / 2 );

    actor->top_sensor.x = actor->position.x + ( ACTOR_SIZE / 2 );
    actor->top_sensor.y = actor->position.y;

    actor->bottom_sensor.x = actor->position.x + ( ACTOR_SIZE / 2 );
    actor->bottom_sensor.y = actor->position.y + ACTOR_SIZE;

    actor->left_sensor.x = actor->position.x;
    actor->left_sensor.y = actor->position.y + ( ACTOR_SIZE / 2 );

    actor->right_sensor.x = actor->position.x + ACTOR_SIZE;
    actor->right_sensor.y = actor->position.y + ( ACTOR_SIZE / 2 );    

    actor->direction = DIR_NONE;

    actor_set_current_tile( actor );
    actor->next_tile = actor->current_tile;
    actor->next_tile = actor->current_tile;

    actor->speed = 0;

    return actor;
}



void pac_collect_dot( Actor *pacmonster, char dots[ TILE_ROWS ][ TILE_COLS ], Score *score, SDL_Renderer *renderer ) {
    if( dots[ pacmonster->current_tile.y ][ pacmonster->current_tile.x ] == 'x') {
        // get rid of dot marker
        dots[ pacmonster->current_tile.y ][ pacmonster->current_tile.x ] = ' ';
        
        score->score_number += 10;
        
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

}


