#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "jb_types.h"
#include "constants.h"
#include "pacmonster.h"
#include "tiles.h"

SDL_bool g_show_debug_info = SDL_TRUE;

int main( int argc, char *argv[] ) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    Pacmonster *pacmonster;
    TileMap tilemap;
    unsigned int score = 0;

    // Initializing stuff
    if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf( stderr, "Error %s\n", SDL_GetError() );
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
    
    

    // INIT PACMONSTER
    pacmonster = init_pacmonster( renderer );
    AnimationTimer animation_timer;
    animation_timer.frame_interval = 0.08f;
    animation_timer.accumulator = 0.0f;
    
    // INIT TILEMAP
    tm_init_and_load_texture( renderer, &tilemap, "maze_file" );

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

        // KEYBOARD STATE

        const Uint8 *current_key_states = SDL_GetKeyboardState( NULL );

        // UPDATE PACMONSTER
        pac_try_set_direction( pacmonster, current_key_states, &tilemap);
        pac_try_move( pacmonster, &tilemap, delta_time );

        // RENDER
        SDL_SetRenderDrawColor( renderer, 0,0,0,255);
        SDL_RenderClear( renderer );    

        tm_render_with_screen_position_offset( renderer, &tilemap );

        pac_render( renderer, pacmonster );

        SDL_SetRenderDrawColor( renderer, 255,255,255,255);

        pac_inc_animation_frame( pacmonster, &animation_timer, delta_time);

        pac_collect_dot( pacmonster, tilemap.tm_dots, &score );
        // DEBUG
        if ( g_show_debug_info ) {
            SDL_SetRenderDrawColor( renderer, 150,50,50,255);
            for ( int y = 0; y < SCREEN_HEIGHT; y+= TILE_SIZE ) {
                SDL_RenderDrawLine( renderer, 0, y, SCREEN_WIDTH, y);
            }
            for ( int x = 0; x < SCREEN_WIDTH; x+= TILE_SIZE) {
                SDL_RenderDrawLine( renderer, x, 0, x, SCREEN_HEIGHT );
            }
            
            // pacman rect
            // SDL_SetRenderDrawColor( renderer, 255,255,0,150 );
            // SDL_RenderFillRect( renderer, &pacmonster->collision_rect);

            

            // current_tile
            SDL_SetRenderDrawColor( renderer, 255, 0, 255,120);
            SDL_Rect tile_rect = { pacmonster->current_tile.x * TILE_SIZE, pacmonster->current_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE,TILE_SIZE};
            SDL_RenderFillRect( renderer, &tile_rect);

            // target tile 
            SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
            SDL_Rect target_rect = { pacmonster->target_tile.x * TILE_SIZE, pacmonster->target_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE, TILE_SIZE };
            SDL_RenderFillRect( renderer, &target_rect );
            
            // pacman center point
            SDL_SetRenderDrawColor( renderer, 255,255,255,255);
            SDL_Point points_to_draw[ 25 ];
            
            //CENTER
            points_to_draw[ 0 ].x = pacmonster->center_point.x;
            points_to_draw[ 0 ].y = pacmonster->center_point.y;
            //above
            points_to_draw[ 1 ].x = pacmonster->center_point.x;
            points_to_draw[ 1 ].y = pacmonster->center_point.y - 1;
            //below
            points_to_draw[ 2 ].x = pacmonster->center_point.x;
            points_to_draw[ 2 ].y = pacmonster->center_point.y + 1;
            //left
            points_to_draw[ 3 ].x = pacmonster->center_point.x - 1;
            points_to_draw[ 3 ].y = pacmonster->center_point.y;
            //right
            points_to_draw[ 4 ].x = pacmonster->center_point.x + 1;
            points_to_draw[ 4 ].y = pacmonster->center_point.y;
            
            // SENSORS

            // TOP SENSOR
            points_to_draw[ 5 ].x = pacmonster->top_sensor.x;
            points_to_draw[ 5 ].y = pacmonster->top_sensor.y;
            //above
            points_to_draw[ 6 ].x = pacmonster->top_sensor.x;
            points_to_draw[ 6 ].y = pacmonster->top_sensor.y - 1;
            //below
            points_to_draw[ 7 ].x = pacmonster->top_sensor.x;
            points_to_draw[ 7 ].y = pacmonster->top_sensor.y + 1;
            //left
            points_to_draw[ 8 ].x = pacmonster->top_sensor.x - 1;
            points_to_draw[ 8 ].y = pacmonster->top_sensor.y;
            //right
            points_to_draw[ 9 ].x = pacmonster->top_sensor.x + 1;
            points_to_draw[ 9 ].y = pacmonster->top_sensor.y;

            // BOTTOM SENSOR
            points_to_draw[ 10 ].x = pacmonster->bottom_sensor.x;
            points_to_draw[ 10 ].y = pacmonster->bottom_sensor.y;
            //above
            points_to_draw[ 11 ].x = pacmonster->bottom_sensor.x;
            points_to_draw[ 11 ].y = pacmonster->bottom_sensor.y - 1;
            //below
            points_to_draw[ 12 ].x = pacmonster->bottom_sensor.x;
            points_to_draw[ 12 ].y = pacmonster->bottom_sensor.y + 1;
            //left
            points_to_draw[ 13 ].x = pacmonster->bottom_sensor.x - 1;
            points_to_draw[ 13 ].y = pacmonster->bottom_sensor.y;
            //right
            points_to_draw[ 14 ].x = pacmonster->bottom_sensor.x + 1;
            points_to_draw[ 14 ].y = pacmonster->bottom_sensor.y;

            // LEFT SENSOR
            points_to_draw[ 15 ].x = pacmonster->left_sensor.x;
            points_to_draw[ 15 ].y = pacmonster->left_sensor.y;
            //above
            points_to_draw[ 16 ].x = pacmonster->left_sensor.x;
            points_to_draw[ 16 ].y = pacmonster->left_sensor.y - 1;
            //below
            points_to_draw[ 17 ].x = pacmonster->left_sensor.x;
            points_to_draw[ 17 ].y = pacmonster->left_sensor.y + 1;
            //left
            points_to_draw[ 18 ].x = pacmonster->left_sensor.x - 1;
            points_to_draw[ 18 ].y = pacmonster->left_sensor.y;
            //right
            points_to_draw[ 19 ].x = pacmonster->left_sensor.x + 1;
            points_to_draw[ 19 ].y = pacmonster->left_sensor.y;

            // RIGHT SENSOR
            points_to_draw[ 20 ].x = pacmonster->right_sensor.x;
            points_to_draw[ 20 ].y = pacmonster->right_sensor.y;
            //above
            points_to_draw[ 21 ].x = pacmonster->right_sensor.x;
            points_to_draw[ 21 ].y = pacmonster->right_sensor.y - 1;
            //below
            points_to_draw[ 22 ].x = pacmonster->right_sensor.x;
            points_to_draw[ 22 ].y = pacmonster->right_sensor.y + 1;
            //left
            points_to_draw[ 23 ].x = pacmonster->right_sensor.x - 1;
            points_to_draw[ 23 ].y = pacmonster->right_sensor.y;
            //right
            points_to_draw[ 24 ].x = pacmonster->right_sensor.x + 1;
            points_to_draw[ 24 ].y = pacmonster->right_sensor.y;



            SDL_RenderDrawPoints( renderer, points_to_draw, 25 );


        }
    
    
        SDL_RenderPresent( renderer );
    }



    // CLOSE DOWN
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_DestroyTexture( pacmonster->texture_atlas );
    SDL_Quit();
    
}
