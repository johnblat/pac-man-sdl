#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include "actor.h"
#include "animation.h"
#include "jb_types.h"
#include "constants.h"
#include "tiles.h"
#include "render.h"


SDL_bool g_show_debug_info = SDL_TRUE;

SDL_Color pac_color = {200,150,0};

int main( int argc, char *argv[] ) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    Actor **actors = (Actor **) malloc(sizeof(Actor *) * 5);
    Animation *pac_animation;
    RenderTexture **render_textures = ( RenderTexture **) malloc(sizeof( RenderTexture *) * 5);
    TTF_Font *gasted_font; 
    TileMap tilemap;

    // Initializing stuff
    if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf( stderr, "Error %s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    if( TTF_Init() < 0 ) {
        fprintf( stderr, "%s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    window = SDL_CreateWindow( "JB Pacmonster", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
    if ( window == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    if (renderer == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (! ( IMG_Init( IMG_INIT_PNG ) & IMG_INIT_PNG ) ) {
        fprintf( stderr, "Error %s\n ", IMG_GetError() );
        exit( EXIT_FAILURE );
    }
    
    gasted_font = TTF_OpenFont("gomarice_no_continue.ttf", 30 );
    if ( gasted_font == NULL ) {
        fprintf(stderr, "%s\n", TTF_GetError());
        exit( EXIT_FAILURE );
    }

    // INIT PACMONSTER
    Position_f initial_pos = { TILE_SIZE, TILE_SIZE * 17 };
    actors[ 0 ] = init_actor( initial_pos );
    render_textures[ 0 ] = init_render_texture( renderer, "pac_monster.png", 4);
    pac_animation = init_animation( 0, 0.08f, render_textures[ 0 ]->num_sprite_clips );


    // INIT TILEMAP
    tm_init_and_load_texture( renderer, &tilemap, "maze_file" );

    // INIT SCORE
    Score score;
    score.font = gasted_font;
    score.score_color = pac_color;
    score.score_number = 0;
    SDL_Surface *score_surface = TTF_RenderText_Solid( score.font, "Score : 0", score.score_color);
    score.score_texture = SDL_CreateTextureFromSurface( renderer, score_surface );
    score.score_render_dst_rect.x = 10;
    score.score_render_dst_rect.y = 10;
    score.score_render_dst_rect.w = score_surface->w;
    score.score_render_dst_rect.h = score_surface->h;

    SDL_FreeSurface( score_surface );


    // PREPARE VARIABLES FOR LOOP
    SDL_Event event;
    int quit = 0;

    // delta time
    float time = 0.0;
    float max_delta_time = 1 / 60.0;
    float previous_frame_ticks = SDL_GetTicks() / 1000.0;


    while (!quit) {

        // semi-fixed timestep
        float current_frame_ticks = SDL_GetTicks() / 1000.0;
        float delta_time = current_frame_ticks - previous_frame_ticks;
        previous_frame_ticks = current_frame_ticks;
        // adjust for any pauses 
        delta_time = delta_time < max_delta_time ?  delta_time : max_delta_time;

        // EVENTS
        while (SDL_PollEvent( &event ) != 0 ) {
            if( event.type == SDL_QUIT ) {
                quit = 1;
            }
            if ( event.type == SDL_KEYDOWN ) {
                if ( event.key.keysym.sym == SDLK_b ) {
                    g_show_debug_info = !g_show_debug_info;
                }
            }
        }
        if(quit) break;

        // KEYBOARD STATE

        const Uint8 *current_key_states = SDL_GetKeyboardState( NULL );

        // UPDATE PACMONSTER
        pac_try_set_direction( actors[ 0 ], current_key_states, &tilemap);
       
        pac_try_move( actors[ 0 ], &tilemap, delta_time );
       
        inc_animation_frame( pac_animation, 1, delta_time);
        
        pac_collect_dot( actors[ 0 ], tilemap.tm_dots, &score, renderer );

        // UPDATE DOTS ANIMATION

        for( int r = 0; r < TILE_ROWS; ++r ) {
            for( int c = 0; c < TILE_COLS; ++c ) {
                
                tilemap.tm_dot_stuff[ r ][ c ].position.y += tilemap.tm_dot_stuff[ r ][ c ].velocity.y * delta_time ;

                if ( tilemap.tm_dot_stuff[ r ][ c ].position.y < DOT_PADDING ) {
                    tilemap.tm_dot_stuff[ r ][ c ].position.y = DOT_PADDING;
                    tilemap.tm_dot_stuff[ r ][ c ].velocity.y = DOT_SPEED;
                }
                if ( tilemap.tm_dot_stuff[ r ][ c ].position.y > TILE_SIZE - DOT_SIZE - DOT_PADDING) {
                    tilemap.tm_dot_stuff[ r ][ c ].position.y = TILE_SIZE - DOT_SIZE - DOT_PADDING;
                    
                    tilemap.tm_dot_stuff[ r ][ c ].velocity.y = -DOT_SPEED;
                }
            }
        }


        // RENDER

        set_render_texture_values_based_on_actor( actors[ 0 ], render_textures[ 0 ], 1 );
        set_render_texture_values_based_on_animation( pac_animation, render_textures[ 0 ], 1 );

        SDL_SetRenderDrawColor( renderer, 0,0,0,255);
        SDL_RenderClear( renderer );    

        tm_render_with_screen_position_offset( renderer, &tilemap );

        render_render_textures( renderer, render_textures[ 0 ], 1 );

        SDL_RenderCopy( renderer, score.score_texture, NULL, &score.score_render_dst_rect);

        // DEBUG
        if ( g_show_debug_info ) {
            SDL_SetRenderDrawColor( renderer, 150,50,50,255);
            for ( int y = 0; y < SCREEN_HEIGHT; y+= TILE_SIZE ) {
                SDL_RenderDrawLine( renderer, 0, y, SCREEN_WIDTH, y);
            }
            for ( int x = 0; x < SCREEN_WIDTH; x+= TILE_SIZE) {
                SDL_RenderDrawLine( renderer, x, 0, x, SCREEN_HEIGHT );
            }
            

            // current_tile
            SDL_SetRenderDrawColor( renderer, pac_color.r, pac_color.g, pac_color.b,150);
            SDL_Rect tile_rect = { actors[ 0 ]->current_tile.x * TILE_SIZE, actors[ 0 ]->current_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE,TILE_SIZE};
            SDL_RenderFillRect( renderer, &tile_rect);

            // target tile 
            SDL_SetRenderDrawColor( renderer,  pac_color.r, pac_color.g, pac_color.b, 225 );
            SDL_Rect target_rect = { actors[ 0 ]->target_tile.x * TILE_SIZE, actors[ 0 ]->target_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE, TILE_SIZE };
            SDL_RenderFillRect( renderer, &target_rect );
            
            // pacman center point
            SDL_SetRenderDrawColor( renderer, 255,255,255,255);
            SDL_Point points_to_draw[ 25 ];
            
            //CENTER
            points_to_draw[ 0 ].x = actors[ 0 ]->center_point.x;
            points_to_draw[ 0 ].y = actors[ 0 ]->center_point.y;
            //above
            points_to_draw[ 1 ].x = actors[ 0 ]->center_point.x;
            points_to_draw[ 1 ].y = actors[ 0 ]->center_point.y - 1;
            //below
            points_to_draw[ 2 ].x = actors[ 0 ]->center_point.x;
            points_to_draw[ 2 ].y = actors[ 0 ]->center_point.y + 1;
            //left
            points_to_draw[ 3 ].x = actors[ 0 ]->center_point.x - 1;
            points_to_draw[ 3 ].y = actors[ 0 ]->center_point.y;
            //right
            points_to_draw[ 4 ].x = actors[ 0 ]->center_point.x + 1;
            points_to_draw[ 4 ].y = actors[ 0 ]->center_point.y;
            
            // SENSORS

            // TOP SENSOR
            points_to_draw[ 5 ].x = actors[ 0 ]->top_sensor.x;
            points_to_draw[ 5 ].y = actors[ 0 ]->top_sensor.y;
            //above
            points_to_draw[ 6 ].x = actors[ 0 ]->top_sensor.x;
            points_to_draw[ 6 ].y = actors[ 0 ]->top_sensor.y - 1;
            //below
            points_to_draw[ 7 ].x = actors[ 0 ]->top_sensor.x;
            points_to_draw[ 7 ].y = actors[ 0 ]->top_sensor.y + 1;
            //left
            points_to_draw[ 8 ].x = actors[ 0 ]->top_sensor.x - 1;
            points_to_draw[ 8 ].y = actors[ 0 ]->top_sensor.y;
            //right
            points_to_draw[ 9 ].x = actors[ 0 ]->top_sensor.x + 1;
            points_to_draw[ 9 ].y = actors[ 0 ]->top_sensor.y;

            // BOTTOM SENSOR
            points_to_draw[ 10 ].x = actors[ 0 ]->bottom_sensor.x;
            points_to_draw[ 10 ].y = actors[ 0 ]->bottom_sensor.y;
            //above
            points_to_draw[ 11 ].x = actors[ 0 ]->bottom_sensor.x;
            points_to_draw[ 11 ].y = actors[ 0 ]->bottom_sensor.y - 1;
            //below
            points_to_draw[ 12 ].x = actors[ 0 ]->bottom_sensor.x;
            points_to_draw[ 12 ].y = actors[ 0 ]->bottom_sensor.y + 1;
            //left
            points_to_draw[ 13 ].x = actors[ 0 ]->bottom_sensor.x - 1;
            points_to_draw[ 13 ].y = actors[ 0 ]->bottom_sensor.y;
            //right
            points_to_draw[ 14 ].x = actors[ 0 ]->bottom_sensor.x + 1;
            points_to_draw[ 14 ].y = actors[ 0 ]->bottom_sensor.y;

            // LEFT SENSOR
            points_to_draw[ 15 ].x = actors[ 0 ]->left_sensor.x;
            points_to_draw[ 15 ].y = actors[ 0 ]->left_sensor.y;
            //above
            points_to_draw[ 16 ].x = actors[ 0 ]->left_sensor.x;
            points_to_draw[ 16 ].y = actors[ 0 ]->left_sensor.y - 1;
            //below
            points_to_draw[ 17 ].x = actors[ 0 ]->left_sensor.x;
            points_to_draw[ 17 ].y = actors[ 0 ]->left_sensor.y + 1;
            //left
            points_to_draw[ 18 ].x = actors[ 0 ]->left_sensor.x - 1;
            points_to_draw[ 18 ].y = actors[ 0 ]->left_sensor.y;
            //right
            points_to_draw[ 19 ].x = actors[ 0 ]->left_sensor.x + 1;
            points_to_draw[ 19 ].y = actors[ 0 ]->left_sensor.y;

            // RIGHT SENSOR
            points_to_draw[ 20 ].x = actors[ 0 ]->right_sensor.x;
            points_to_draw[ 20 ].y = actors[ 0 ]->right_sensor.y;
            //above
            points_to_draw[ 21 ].x = actors[ 0 ]->right_sensor.x;
            points_to_draw[ 21 ].y = actors[ 0 ]->right_sensor.y - 1;
            //below
            points_to_draw[ 22 ].x = actors[ 0 ]->right_sensor.x;
            points_to_draw[ 22 ].y = actors[ 0 ]->right_sensor.y + 1;
            //left
            points_to_draw[ 23 ].x = actors[ 0 ]->right_sensor.x - 1;
            points_to_draw[ 23 ].y = actors[ 0 ]->right_sensor.y;
            //right
            points_to_draw[ 24 ].x = actors[ 0 ]->right_sensor.x + 1;
            points_to_draw[ 24 ].y = actors[ 0 ]->right_sensor.y;



            SDL_RenderDrawPoints( renderer, points_to_draw, 25 );


        }
        SDL_RenderPresent( renderer );
    }

    // CLOSE DOWN
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_DestroyTexture( render_textures[ 0 ]->texture_atlas );
    SDL_Quit();
    
}
