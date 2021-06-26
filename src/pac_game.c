#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_gamecontroller.h>
#include <stdio.h>
#include "actor.h"
#include "movement.h"
#include "animation.h"
#include "states.h"
#include "jb_types.h"
#include "comparisons.h"
#include "resources.h"
#include "constants.h"
#include "tiles.h"
#include "render.h"
#include "interpolation.h"
#include "sounds.h"
#include "input.h"



SDL_bool g_show_debug_info = SDL_TRUE;

SDL_Color pac_color = {200,150,0};
SDL_Color white = {200,200,255};

// used to track progress in level
unsigned int g_NumDots = 0;

uint8_t g_NumGhostsEaten = 0;
unsigned int g_GhostPointValues[] = { 400, 800, 1600, 3200 };


float g_PAC_DASH_SPEED_MULTR = 2.5f;
float g_PAC_DASH_TIME_MAX = 0.75f;

float g_PacSlowTimers[] = {0.0f, 0.0f};
float g_PacChargeTimers[] = {0.0f, 0.0f};
float g_PacDashTimers[] = {0.0f, 0.0f};




typedef struct {
    float remainingTime;
    char message[ 8 ];
    SDL_Point world_position;
    SDL_Color color;
    SDL_Texture *messageTexture;
    SDL_Rect render_dest_rect;
    TTF_Font *font;
    Lerp l;

} TimedMessage;

#define g_NumTimedMessages 4
TimedMessage g_TimedMessages[ g_NumTimedMessages ];
Blink g_ScoreBlinks[g_NumTimedMessages ];

void level_advance(LevelConfig *levelConfig, TileMap *tilemap, SDL_Renderer *renderer, Actor **actors, AnimatedSprite **animatedSprites, GhostState *ghostStates ) {
    gCurrentLevel++;

    if( gCurrentLevel > gNumLevels ) {
        printf("GAME OVER\n");
        exit( EXIT_SUCCESS );
    }

    load_current_level_off_disk( levelConfig, tilemap, renderer );

    // pacman
    actor_reset_data( actors[ 0 ], levelConfig->pacStartingTile );
    
    //ghosts
    // INIT GHOST
    SDL_Point ghost_pen_tile;
    //try_load_resource_from_file( &ghost_pen_tile, "res/ghost_pen_tile", sizeof( SDL_Point ), 1 );
    ghost_pen_tile = levelConfig->ghostPenTile;
    // doing this in case need more ghosts in future. don't want to harder hard code their exact starting positions. Can define their offsets in a config file
    SDL_Point pinky_from_pen = {0, 0};
    SDL_Point blinky_from_pen = {0, -3};
    SDL_Point inky_from_pen = { -1, 0 };
    SDL_Point clyde_from_pen = {1, 0};

    SDL_Point blinky_tile;
    blinky_tile.x = ghost_pen_tile.x + blinky_from_pen.x;
    blinky_tile.y = ghost_pen_tile.y + blinky_from_pen.y;
    actor_reset_data( actors[ 1 ], blinky_tile );

    SDL_Point pinky_tile;
    pinky_tile.x = ghost_pen_tile.x + pinky_from_pen.x;
    pinky_tile.y = ghost_pen_tile.y + pinky_from_pen.y;
    actor_reset_data( actors[ 2 ], pinky_tile );

    SDL_Point inky_tile;
    inky_tile.x = ghost_pen_tile.x + inky_from_pen.x;
    inky_tile.y = ghost_pen_tile.y + inky_from_pen.y;
    actor_reset_data( actors[ 3 ], inky_tile );

    SDL_Point clyde_tile;
    clyde_tile.x = ghost_pen_tile.x + clyde_from_pen.x;
    clyde_tile.y = ghost_pen_tile.y + clyde_from_pen.y;
    actor_reset_data( actors[ 4 ], clyde_tile );

    animatedSprites[ 1 ]->texture_atlas_id = animatedSprites[ 1 ]->default_texture_atlas_id;
    animatedSprites[ 2 ]->texture_atlas_id = animatedSprites[ 2 ]->default_texture_atlas_id;
    animatedSprites[ 3 ]->texture_atlas_id = animatedSprites[ 3 ]->default_texture_atlas_id;
    animatedSprites[ 4 ]->texture_atlas_id = animatedSprites[ 4 ]->default_texture_atlas_id;

    ghostStates[ 1 ] = STATE_NORMAL;
    ghostStates[ 2 ] = STATE_NORMAL;
    ghostStates[ 3 ] = STATE_NORMAL;
    ghostStates[ 4 ] = STATE_NORMAL;

    // calculate number of dots
    g_NumDots = 0;

    for( int row = 0; row < TILE_ROWS; row++ ) {
        for(int col =0; col < TILE_COLS; col++ ) {
            if( tilemap->tm_dots[ row ][ col ] == 'x' ) {
                g_NumDots++;
            }
        }
    }

    // add power pellets to number of dotss
    for( int i = 0; i < 4; i++ ) {
        if( !points_equal( tilemap->tm_power_pellet_tiles[i], TILE_NONE ) ) {
            g_NumDots++;
        }
    }

    actors[ 0 ]->world_position.x = levelConfig->pacStartingTile.x * TILE_SIZE;
    actors[ 0 ]->world_position.y = levelConfig->pacStartingTile.y * TILE_SIZE;
    

    actors[0]->world_center_point.x = ( int ) actors[0]->world_position.x + ( TILE_SIZE / 2 );
    actors[0]->world_center_point.y = ( int ) actors[0]->world_position.y + ( TILE_SIZE / 2 );

    actors[0]->current_tile.x = ( ( actors[0]->world_position.x + TILE_SIZE / 2 ) / TILE_SIZE ) ;
    actors[0]->current_tile.y = ( ( ( actors[0]->world_position.y + TILE_SIZE / 2 ) ) / TILE_SIZE ) ;

    actors[0]->world_top_sensor.x = actors[0]->world_position.x + ( TILE_SIZE / 2 );
    actors[0]->world_top_sensor.y = actors[0]->world_position.y;

    actors[0]->world_bottom_sensor.x = actors[0]->world_position.x + ( TILE_SIZE / 2 );
    actors[0]->world_bottom_sensor.y = actors[0]->world_position.y + TILE_SIZE;

    actors[0]->world_left_sensor.x = actors[0]->world_position.x;
    actors[0]->world_left_sensor.y = actors[0]->world_position.y + ( TILE_SIZE / 2 );

    actors[0]->world_right_sensor.x = actors[0]->world_position.x + TILE_SIZE;
    actors[0]->world_right_sensor.y = actors[0]->world_position.y + ( TILE_SIZE / 2 );

    actors[ 5 ]->world_position.x = levelConfig->pacStartingTile.x * TILE_SIZE;
    actors[ 5 ]->world_position.y = levelConfig->pacStartingTile.y * TILE_SIZE;

    actors[5]->world_center_point.x = ( int ) actors[5]->world_position.x + ( TILE_SIZE / 2 );
    actors[5]->world_center_point.y = ( int ) actors[5]->world_position.y + ( TILE_SIZE / 2 );

    actors[5]->current_tile.x = ( ( actors[5]->world_position.x + TILE_SIZE / 2 ) / TILE_SIZE ) ;
    actors[5]->current_tile.y = ( ( ( actors[5]->world_position.y + TILE_SIZE / 2 ) ) / TILE_SIZE ) ;

    actors[5]->world_top_sensor.x = actors[5]->world_position.x + ( TILE_SIZE / 2 );
    actors[5]->world_top_sensor.y = actors[5]->world_position.y;

    actors[5]->world_bottom_sensor.x = actors[5]->world_position.x + ( TILE_SIZE / 2 );
    actors[5]->world_bottom_sensor.y = actors[5]->world_position.y + TILE_SIZE;

    actors[5]->world_left_sensor.x = actors[5]->world_position.x;
    actors[5]->world_left_sensor.y = actors[5]->world_position.y + ( TILE_SIZE / 2 );

    actors[5]->world_right_sensor.x = actors[5]->world_position.x + TILE_SIZE;
    actors[5]->world_right_sensor.y = actors[5]->world_position.y + ( TILE_SIZE / 2 );





}

void set_cross( SDL_Point center_point, int starting_index, SDL_Point tilemap_screen_position, SDL_Point *points ) {
    points[ starting_index ].x = center_point.x + tilemap_screen_position.x;
    points[ starting_index ].y = center_point.y + tilemap_screen_position.y ;
    //above
    points[ starting_index + 1 ].x = center_point.x + tilemap_screen_position.x ;
    points[ starting_index + 1 ].y = center_point.y - 1 + tilemap_screen_position.y;
    //below
    points[ starting_index + 2 ].x = center_point.x + tilemap_screen_position.x ;
    points[ starting_index + 2 ].y = center_point.y + 1 + tilemap_screen_position.y ;
    //left
    points[ starting_index + 3 ].x = center_point.x - 1 + tilemap_screen_position.x ;
    points[ starting_index + 3 ].y = center_point.y + tilemap_screen_position.y ;
    //right
    points[ starting_index + 4 ].x = center_point.x + 1 + tilemap_screen_position.x ;
    points[ starting_index + 4 ].y = center_point.y + tilemap_screen_position.y ;
}

int main( int argc, char *argv[] ) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    Actor *actors[ 6 ]; 
    AnimatedSprite *animations[ 13 ]; // pac-man, ghosts, power-pellets, eyes, etc
    RenderClipFromTextureAtlas *render_clips[ 10 ]; // for render textures, 5 thru 9 only different is the Rect x and y 
    GhostState ghost_states[ 5 ]; // 1 thru 5
    TTF_Font *font; 
    TileMap tilemap;
    // TIMER USED FOR VULNERABILITY STATE
    float ghost_vulnerable_timer = 0.0f;
    // GHOST BEHAVIOR TIMER FOR CURRENT GLOBAL GHOST MODE
    float ghost_mode_timer = 0.0f;
    

    int numLevels = determine_number_of_levels_from_dirs();
    printf("Num levels %d\n", numLevels);

    gCurrentLevel++;

    SDL_Point zero = {0, 0};
    LevelConfig levelConfig;
    levelConfig.scatterChasePeriodSeconds = g_scatter_chase_period_seconds;
    levelConfig.numScatterChasePeriods = NUM_SCATTER_CHASE_PERIODS;
    levelConfig.ghostPenTile = zero;
    levelConfig.pacStartingTile = zero;

    tilemap.tm_texture_atlas = NULL; // initializing texture atlas pointer in tilemap 

    
    // initialize the ghost states
    for( int i = 1; i < 5; ++i ) {
        ghost_states[ i ] = STATE_NORMAL;
    }

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

    if( SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) < 0 ){
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    if (! ( IMG_Init( IMG_INIT_PNG ) & IMG_INIT_PNG ) ) {
        fprintf( stderr, "Error %s\n ", IMG_GetError() );
        exit( EXIT_FAILURE );
    }

    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) {
        fprintf( stderr, "SDL_mixer could not initialize: %s\n", Mix_GetError());
    }

    // game controller
    if( SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0 ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    g_NumJoysticks = SDL_NumJoysticks();
    for( int i = 0; i <g_NumJoysticks; i++ ) {
        if( SDL_IsGameController( i ) ) {
            g_NumGamepads++;
        }
    }

    if( g_NumGamepads > 0 ) {
        for( int i = 0; i < g_NumGamepads; i++) {
            g_GameControllers[ i ] = SDL_GameControllerOpen( i );
            if( !SDL_GameControllerGetAttached( g_GameControllers[ i ] ) ) {
                fprintf(stderr, "Wrong!\n");
            }
            printf("Controller name: %s\n", SDL_GameControllerName(g_GameControllers[ i ]));
            
        }
        SDL_GameControllerEventState( SDL_ENABLE );
 
    }
    // end game controller

    //Load music
    g_Music = Mix_LoadMUS( "res/sounds/Scruffy - World 0 & 1 (Pac-Man Arrangement) - arranged by Scruffy.ogg" );
    if( g_Music == NULL )
    {
        printf( "Failed to load music! SDL_mixer Error: %s\n", Mix_GetError() );
    }

    g_GhostSound = Mix_LoadWAV("res/sounds/ghost.wav");
    if( g_GhostSound == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }

    g_GhostVulnerableSound = Mix_LoadWAV("res/sounds/vulnerable.wav");
    if( g_GhostVulnerableSound == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }

    g_PacChompSound = Mix_LoadWAV("res/sounds/chomp.wav");
    if( g_PacChompSound == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }

    g_GhostEatenYeahSound = Mix_LoadWAV("res/sounds/yeah.wav");
    if( g_GhostEatenYeahSound == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }

    g_GhostEatenSweetSound = Mix_LoadWAV("res/sounds/sweet.wav");
    if( g_GhostEatenSweetSound == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }

    g_GhostEatenCoolSound = Mix_LoadWAV("res/sounds/cool.wav");
    if( g_GhostEatenCoolSound == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }

    g_GhostEatenGroovySound = Mix_LoadWAV("res/sounds/groovy.wav");
    if( g_GhostEatenGroovySound == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }

    g_PacDieOhNoSound = Mix_LoadWAV("res/sounds/oh-no.wav");
    if( g_PacDieOhNoSound == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }

   // Mix_VolumeChunk( g_GhostEatenGroovySound, 100 );

    g_GhostEatenSounds[ 0 ] = g_GhostEatenYeahSound;
    g_GhostEatenSounds[ 1 ] = g_GhostEatenSweetSound;
    g_GhostEatenSounds[ 2 ] = g_GhostEatenCoolSound;
    g_GhostEatenSounds[ 3 ] = g_GhostEatenGroovySound;

    Mix_VolumeMusic( 50 );

    Mix_PlayMusic(g_Music, -1 );
    
    font = TTF_OpenFont("res/gomarice_no_continue.ttf", 30 );
    if ( font == NULL ) {
        fprintf(stderr, "%s\n", TTF_GetError());
        exit( EXIT_FAILURE );
    }

    load_current_level_off_disk( &levelConfig, &tilemap, renderer);

    // INIT TEXTURE ATLASES

    load_global_texture_atlases_from_config_file( renderer );

    load_animations_from_config_file( animations );

    load_render_xx_from_config_file( render_clips );

    //load_ghost_mode_times_from_config_file( g_scatter_chase_period_seconds, NUM_SCATTER_CHASE_PERIODS, "res/ghost_mode_times" );


    // INIT TILEMAP
    //tm_init_and_load_texture( renderer, &tilemap );
    tilemap.one_way_tile.x = levelConfig.ghostPenTile.x;
    tilemap.one_way_tile.y = levelConfig.ghostPenTile.y - 2;

    // init messages
    for( int i = 0; i < g_NumTimedMessages; i++ ) {
        g_TimedMessages[ i ].color = white;
        g_TimedMessages[ i ].font = font;
        g_TimedMessages[ i ].remainingTime = 0.0f;
        g_TimedMessages[ i ].world_position.x = -1;
        g_TimedMessages[ i ].world_position.y = -1;
    }

    for( int i = 0; i < g_NumTimedMessages; i ++ ) {
        g_ScoreBlinks[ i ] = blinkInit( 0.033, 0, 255 );
    }

    // calculate number of dots
    for( int row = 0; row < TILE_ROWS; row++ ) {
        for(int col =0; col < TILE_COLS; col++ ) {
            if( tilemap.tm_dots[ row ][ col ] == 'x' ) {
                g_NumDots++;
            }
        }
    }

    //try_load_resource_from_file( tilemap.tm_power_pellet_tiles, "res/power_pellets", sizeof( SDL_Point ), 4 );

    // add power pellets to number of dotss
    for( int i = 0; i < 4; i++ ) {
        if( !points_equal( tilemap.tm_power_pellet_tiles[i], TILE_NONE ) ) {
            g_NumDots++;
        }
    }

    // INIT PACMONSTER
    SDL_Point pac_starting_tile;
    //try_load_resource_from_file( &pac_starting_tile, "res/pac_starting_tile", sizeof( SDL_Point ), 1 );
    pac_starting_tile = levelConfig.pacStartingTile;

    // read in file
    SDL_RWops *read_context = SDL_RWFromFile("res/default_pac_speed", "r");
    unsigned long size = read_context->size(read_context);
    char *content = (char *)malloc( size + 1 );
    SDL_RWread( read_context, content, size, 1 );
    content[ size ] = '\0';
    char *end_ptr;
    float base_speed = strtol(content, &end_ptr, 10);
    free( content );
    SDL_RWclose( read_context );

    actors[ 0 ] = init_actor( pac_starting_tile, tilemap.tm_screen_position, base_speed, 1.0f );
   // render_clips[ 0 ] = init_render_clip( 0, 0 );
    actors[ 5 ] = init_actor( pac_starting_tile, tilemap.tm_screen_position, base_speed, 1.0f);
    

    // INIT GHOST
    SDL_Point ghost_pen_tile;
    //try_load_resource_from_file( &ghost_pen_tile, "res/ghost_pen_tile", sizeof( SDL_Point ), 1 );
    ghost_pen_tile = levelConfig.ghostPenTile;
    // doing this in case need more ghosts in future. don't want to harder hard code their exact starting positions. Can define their offsets in a config file
    SDL_Point pinky_from_pen = {0, 0};
    SDL_Point blinky_from_pen = {0, -3};
    SDL_Point inky_from_pen = { -1, 0 };
    SDL_Point clyde_from_pen = {1, 0};

    SDL_Point blinky_tile;
    blinky_tile.x = ghost_pen_tile.x + blinky_from_pen.x;
    blinky_tile.y = ghost_pen_tile.y + blinky_from_pen.y;
    actors[ 1 ] = init_actor( blinky_tile, tilemap.tm_screen_position, base_speed, 0.8f );

    SDL_Point pinky_tile;
    pinky_tile.x = ghost_pen_tile.x + pinky_from_pen.x;
    pinky_tile.y = ghost_pen_tile.y + pinky_from_pen.y;
    actors[ 2 ]= init_actor( pinky_tile, tilemap.tm_screen_position, base_speed, 0.8f  );

    SDL_Point inky_tile;
    inky_tile.x = ghost_pen_tile.x + inky_from_pen.x;
    inky_tile.y = ghost_pen_tile.y + inky_from_pen.y;
    actors[ 3 ]= init_actor( inky_tile, tilemap.tm_screen_position, base_speed, 0.8f  );

    SDL_Point clyde_tile;
    clyde_tile.x = ghost_pen_tile.x + clyde_from_pen.x;
    clyde_tile.y = ghost_pen_tile.y + clyde_from_pen.y;
    actors[ 4 ]= init_actor( clyde_tile, tilemap.tm_screen_position, base_speed, 0.8f  );


    for( int i = 0; i < 4; ++i ) {
        SDL_Point screen_position = tile_grid_point_to_screen_point( tilemap.tm_power_pellet_tiles[ i ], tilemap.tm_screen_position );
        render_clips[ 6 + i ]->dest_rect.x = screen_position.x;
        render_clips[ 6 + i ]->dest_rect.y = screen_position.y;
        render_clips[ 6 + i ]->dest_rect.w = TILE_SIZE;
        render_clips[ 6 + i ]->dest_rect.h = TILE_SIZE;
        render_clips[ 6 + i ]->flip = SDL_FLIP_NONE;
    }

    


    SDL_Point ghost_pen_position = tile_grid_point_to_world_point( ghost_pen_tile ); 
    SDL_Point ghost_pen_center_point;
    ghost_pen_center_point.x = ghost_pen_position.x + (TILE_SIZE / 2);
    ghost_pen_center_point.y = ghost_pen_position.y + (TILE_SIZE / 2);

    // INIT SCORE
    Score score;
    score.font = font;
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

    // delta time - frame rate independent movement
    float max_delta_time = 1 / 60.0;
    float previous_frame_ticks = SDL_GetTicks() / 1000.0;
    SDL_bool charge_button_up = SDL_FALSE;
    SDL_bool charge_button_down = SDL_FALSE;

    g_InputMasks[ 0 ] = 0b00000; // reset
    g_InputMasks[ 1 ] = 0b00000; // reset

    while (!quit) {

        // semi-fixed timestep
        float current_frame_ticks = SDL_GetTicks() / 1000.0;
        float delta_time = current_frame_ticks - previous_frame_ticks;
        previous_frame_ticks = current_frame_ticks;
        // adjust for any pauses, debugging breaks, etc
        delta_time = delta_time < max_delta_time ?  delta_time : max_delta_time;

        
        // EVENTS
        while (SDL_PollEvent( &event ) != 0 ) {
            //int gameControllerState = SDL_GameControllerEventState( SDL_QUERY );
            
            if( event.type == SDL_QUIT ) {
                quit = 1;
            }
            if ( event.type == SDL_KEYDOWN ) {
                if ( event.key.keysym.sym == SDLK_b ) {
                    g_show_debug_info = !g_show_debug_info;
                }
                if (event.key.keysym.sym == SDLK_v ) {
                    ghost_vulnerable_timer = 20.0f;
                    for( int i = 1; i < 5; ++i ) {
                        ghost_states[ i ] = STATE_VULNERABLE;
                        vulnerable_enter( actors, animations, i, render_clips[ i ] );
                    }
                    
                }
                if( event.key.keysym.sym == SDLK_z ) {
                    charge_button_down = SDL_TRUE;
                    charge_button_up = SDL_FALSE;
                }
            }
            if( event.type == SDL_KEYUP ) {
                if( event.key.keysym.sym == SDLK_z ) {
                    charge_button_up = SDL_TRUE;
                    charge_button_down = SDL_FALSE;
                }
            }
            if( event.type == SDL_CONTROLLERBUTTONDOWN ) {
                for( int i = 0; i < g_NumGamepads; i++ ) {
                    if( event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_GameControllers[ i ]))) {
                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP ) {
                            g_InputMasks[ i ] |= g_INPUT_UP;
                        }
                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT ) {
                            g_InputMasks[ i ] |= g_INPUT_LEFT;
                        }
                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT ) {
                            g_InputMasks[ i ] |= g_INPUT_RIGHT;
                        }
                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN ) {
                            g_InputMasks[ i ] |= g_INPUT_DOWN;
                        }
                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_X ) {
                            g_InputMasks[ i ] |= g_INPUT_ACTION;
                            charge_button_down = SDL_TRUE;
                            charge_button_up = SDL_FALSE;
                        }
                    }
                }
                
            }
            if( event.type == SDL_CONTROLLERBUTTONUP ) {
                for( int i = 0; i < g_NumGamepads; i++ ) {
                    if( event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_GameControllers[ i ]))) {
                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP ) {
                            g_InputMasks[ i ] ^= g_INPUT_UP;
                        }
                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT ) {
                            g_InputMasks[ i ] ^= g_INPUT_LEFT;
                        }
                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT ) {
                            g_InputMasks[ i ] ^= g_INPUT_RIGHT;
                        }
                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN ) {
                            g_InputMasks[ i ] ^= g_INPUT_DOWN;
                        }
                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_X ) {
                            g_InputMasks[ i ] ^= g_INPUT_ACTION;
                            charge_button_up = SDL_TRUE;
                            charge_button_down = SDL_FALSE;
                        }
                    }
                }
            }
        }
        if(quit) break;

        // NEXT LEVEL?
        if( g_NumDots <= 0 ) {
            level_advance( &levelConfig, &tilemap, renderer, actors, animations, ghost_states );
        }

        // KEYBOARD STATE

        const Uint8 *current_key_states = SDL_GetKeyboardState( NULL );
        //int gameControllerState = SDL_GameControllerEventState( SDL_QUERY );
        
        // adjust tilemap
        if( current_key_states[ SDL_SCANCODE_S ] ) {
            tilemap.tm_screen_position.y++;
            // increase double speed if shift held down
            if( current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
                tilemap.tm_screen_position.y+=4;
            }
        }
        if( current_key_states[ SDL_SCANCODE_W ] ) {
            tilemap.tm_screen_position.y--;
            // increase double speed if shift held down

            if( current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
                tilemap.tm_screen_position.y-=4;
            }
        }
        if( current_key_states[ SDL_SCANCODE_D ] ) {
            tilemap.tm_screen_position.x++;
            // increase double speed if shift held down
            if( current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
                tilemap.tm_screen_position.x+=4;
            }
        }
        if( current_key_states[ SDL_SCANCODE_A ] ) {
            tilemap.tm_screen_position.x--;
            // increase double speed if shift held down
            if( current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
                tilemap.tm_screen_position.x-=4;
            }
        }
        if( current_key_states[ SDL_SCANCODE_COMMA ] ) {
            SDL_Delay(500);
            level_advance( &levelConfig, &tilemap, renderer, actors, animations, ghost_states );
        }
        // if( current_key_states[ SDL_SCANCODE_Z ] ) {
        //     g_PacSpeedTimer = 0.33f;
        // }
        // charge1 
        if( g_InputMasks[ 0 ] & g_INPUT_ACTION ) {
            g_PacChargeTimers[ 0 ] += delta_time;
        }
        else if( g_InputMasks[ 0 ] ^ g_INPUT_ACTION && g_PacChargeTimers[ 0 ] > 0.0f ) {
            g_PacDashTimers[ 0 ] = g_PacChargeTimers[ 0 ] > g_PAC_DASH_TIME_MAX ? g_PAC_DASH_TIME_MAX : g_PacChargeTimers[ 0 ];
            g_PacChargeTimers[ 0 ] = 0.0f;
        }

        // charge2
        if( g_InputMasks[ 1 ] & g_INPUT_ACTION ) {
            g_PacChargeTimers[ 1 ] += delta_time;
        }
        else if( g_InputMasks[ 1 ] ^ g_INPUT_ACTION && g_PacChargeTimers[ 1 ] > 0.0f ) {
            g_PacDashTimers[ 1 ] = g_PacChargeTimers[ 1 ] > g_PAC_DASH_TIME_MAX ? g_PAC_DASH_TIME_MAX : g_PacChargeTimers[ 1 ];
            g_PacChargeTimers[ 1 ] = 0.0f;
        }

        // UPDATE SIMULATION

        

        pac_try_set_direction( actors[ 0 ], g_InputMasks[ 0 ], &tilemap);
        pac_try_set_direction( actors[ 5 ], g_InputMasks[ 1 ], &tilemap);

        // dash 1
        actors[ 0 ]->speed_multp = 1.0f;
        if( g_PacSlowTimers[ 0 ] > 0 ) {
            actors[ 0 ]->speed_multp = 0.6f;
            g_PacSlowTimers[ 0 ] -= delta_time;
        }
        if( g_PacDashTimers[0] > 0 ) {
            actors[ 0 ]->speed_multp = g_PAC_DASH_SPEED_MULTR;
            g_PacDashTimers[ 0 ] -= delta_time;
            SDL_SetTextureAlphaMod( g_texture_atlases[ 0 ].texture, 150 );
        }
        else {
            SDL_SetTextureAlphaMod( g_texture_atlases[ 0 ].texture, 255 );
        }
        if( g_PacChargeTimers[ 0 ] > 0 ) {
            actors[ 0 ]->speed_multp = 0.4f;
        }

        // dash 2
        actors[ 5 ]->speed_multp = 1.0f;
        if( g_PacSlowTimers[ 1 ] > 0 ) {
            actors[ 5 ]->speed_multp = 0.6f;
            g_PacSlowTimers[ 1 ] -= delta_time;
        }
        if( g_PacDashTimers[1] > 0 ) {
            actors[ 5 ]->speed_multp = g_PAC_DASH_SPEED_MULTR;
            g_PacDashTimers[ 1 ] -= delta_time;
            SDL_SetTextureAlphaMod( g_texture_atlases[ 8 ].texture, 150 );
        }
        else {
            SDL_SetTextureAlphaMod( g_texture_atlases[ 8 ].texture, 255 );
        }
        if( g_PacChargeTimers[ 1 ] > 0 ) {
            actors[ 5 ]->speed_multp = 0.4f;
        }
       
        pac_try_move( actors[ 0 ], &tilemap, delta_time );
        pac_try_move( actors[ 5 ], &tilemap, delta_time );

        inc_animations( animations, 7 , delta_time); 
        
        // slow down pacman if in pac-pellet-tile

        if( tilemap.tm_dots[ actors[ 0 ]->current_tile.y ][ actors[ 0 ]->current_tile.x ] == 'x' ) {
            g_PacSlowTimers[ 0 ] = 0.075f;
        }

        if( tilemap.tm_dots[ actors[ 5 ]->current_tile.y ][ actors[ 5 ]->current_tile.x ] == 'x' ) {
            g_PacSlowTimers[ 1 ] = 0.075f;
        }

        pac_collect_dot( actors[ 0 ], tilemap.tm_dots, &g_NumDots, &score, renderer );
        pac_collect_dot( actors[ 5 ], tilemap.tm_dots, &g_NumDots, &score, renderer );

        // VULNERABLE TIMER
        for( int i = 1; i < 5; ++i ) {
            if( ghost_states[ i ] == STATE_VULNERABLE ) {
                ghost_vulnerable_timer -= delta_time;
                break;
            }
        }
        
        // CHECK STATE TRANSITIONS FOR GHOSTS
        for(int i = 1; i < 5; ++i ) {
            switch( ghost_states[ i ] ) {
                case STATE_VULNERABLE :
                    // return all ghosts that are still vulnerable to normal
                    // when the timer runs out
                    if (ghost_vulnerable_timer <= 0.0f ) {
                        g_NumGhostsEaten = 0;
                        for( int i = 1; i < 5; ++i ) {
                            if( ghost_states[ i ] == STATE_VULNERABLE ) {
                                ghost_states[ i ] = STATE_NORMAL;
                                normal_enter( actors, animations, i , render_clips[ i ], i );
                            }
                        }
                    }
                    
                    // eat ghost if pacman touches
                    if ( actors[ 0 ]->current_tile.x == actors[ i ]->current_tile.x 
                    && actors[ 0 ]->current_tile.y == actors[ i ]->current_tile.y ) {
                        Mix_PlayChannel( -1, g_PacChompSound, 0 );
                        Mix_PlayChannel( -1, g_GhostEatenSounds[ g_NumGhostsEaten ], 0);
                        score.score_number+=g_GhostPointValues[ g_NumGhostsEaten ];
                        g_NumGhostsEaten++;
                        ghost_states[ i ] = STATE_GO_TO_PEN;
                        uint8_t texture_atlas_id = 4;
                        animations[ i ]->texture_atlas_id = texture_atlas_id;
                        actors[ i ]->next_tile = actors[ i ]->current_tile;
                        actors[ i ]->target_tile = ghost_pen_tile;
                        actors[ i ]->speed_multp = 1.6f;

                        // show message
                        for( int i = 0; i < g_NumTimedMessages; i++ ) {
                            if( g_TimedMessages[ i ].remainingTime <= 0.0f ) {
                                g_TimedMessages[ i ].remainingTime = 0.85f;
                                g_TimedMessages[ i ].world_position = tile_grid_point_to_world_point( actors[ 0 ]->current_tile );
                                snprintf( g_TimedMessages[ i ].message, 8, "%d", g_GhostPointValues[ g_NumGhostsEaten - 1 ] );
                                g_TimedMessages[ i ].color = white;
                                SDL_Surface *msgSurface = TTF_RenderText_Solid( g_TimedMessages[ i ].font,  g_TimedMessages[ i ].message, g_TimedMessages[ i ].color );
                                g_TimedMessages[ i ].messageTexture = SDL_CreateTextureFromSurface( renderer, msgSurface );
                                g_TimedMessages[ i ].render_dest_rect.x = world_point_to_screen_point(g_TimedMessages[ i ].world_position, tilemap.tm_screen_position).x;
                                g_TimedMessages[ i ].render_dest_rect.y = world_point_to_screen_point(g_TimedMessages[ i ].world_position, tilemap.tm_screen_position).y;
                                g_TimedMessages[ i ].render_dest_rect.w = msgSurface->w;
                                g_TimedMessages[ i ].render_dest_rect.h = msgSurface->h;
                                SDL_FreeSurface( msgSurface );
                                g_TimedMessages[ i ].l = lerpInit( g_TimedMessages[ i ].world_position.y - TILE_SIZE*1.25, g_TimedMessages[ i ].world_position.y, 0.33f );

                                break;

                            }
                        }
                    }
                    if ( actors[ 5 ]->current_tile.x == actors[ i ]->current_tile.x 
                    && actors[ 5 ]->current_tile.y == actors[ i ]->current_tile.y ) {
                        Mix_PlayChannel( -1, g_PacChompSound, 0 );
                        Mix_PlayChannel( -1, g_GhostEatenSounds[ g_NumGhostsEaten ], 0);
                        score.score_number+=g_GhostPointValues[ g_NumGhostsEaten ];
                        g_NumGhostsEaten++;
                        ghost_states[ i ] = STATE_GO_TO_PEN;
                        uint8_t texture_atlas_id = 4;
                        animations[ i ]->texture_atlas_id = texture_atlas_id;
                        actors[ i ]->next_tile = actors[ i ]->current_tile;
                        actors[ i ]->target_tile = ghost_pen_tile;
                        actors[ i ]->speed_multp = 1.6f;

                        // show message
                        for( int i = 0; i < g_NumTimedMessages; i++ ) {
                            if( g_TimedMessages[ i ].remainingTime <= 0.0f ) {
                                g_TimedMessages[ i ].remainingTime = 0.85f;
                                g_TimedMessages[ i ].world_position = tile_grid_point_to_world_point( actors[ 5 ]->current_tile );
                                snprintf( g_TimedMessages[ i ].message, 8, "%d", g_GhostPointValues[ g_NumGhostsEaten - 1 ] );
                                g_TimedMessages[ i ].color = white;
                                SDL_Surface *msgSurface = TTF_RenderText_Solid( g_TimedMessages[ i ].font,  g_TimedMessages[ i ].message, g_TimedMessages[ i ].color );
                                g_TimedMessages[ i ].messageTexture = SDL_CreateTextureFromSurface( renderer, msgSurface );
                                g_TimedMessages[ i ].render_dest_rect.x = world_point_to_screen_point(g_TimedMessages[ i ].world_position, tilemap.tm_screen_position).x;
                                g_TimedMessages[ i ].render_dest_rect.y = world_point_to_screen_point(g_TimedMessages[ i ].world_position, tilemap.tm_screen_position).y;
                                g_TimedMessages[ i ].render_dest_rect.w = msgSurface->w;
                                g_TimedMessages[ i ].render_dest_rect.h = msgSurface->h;
                                SDL_FreeSurface( msgSurface );
                                g_TimedMessages[ i ].l = lerpInit( g_TimedMessages[ i ].world_position.y - TILE_SIZE*1.25, g_TimedMessages[ i ].world_position.y, 0.33f );

                                break;

                            }
                        }
                    }

                    
                    break;
                    
                case STATE_GO_TO_PEN :
                    // ghost is in pen
                    
                    if( points_equal(actors[ i ]->current_tile, ghost_pen_tile ) && actors[ i ]->world_center_point.y >= ghost_pen_center_point.y) {
                        // actors[ i ]->direction = opposite_directions[ actors[ i ]->direction ];
                        // actors[ i ]->next_tile = actors[ i ]->current_tile;
                        // ghost_states[ i ] = STATE_NORMAL;
                        // normal_enter( actors, animations, i , render_clips[ i ], i );
                        // actors[ i ]->next_tile.y -=3; // makes sure that they go out of 
                        ghost_states[ i ] = STATE_LEAVE_PEN;
                        leave_pen_enter( actors, animations, i );
                    }
                    break;

                
                case STATE_NORMAL :
                    break;
                case STATE_LEAVE_PEN:
                    if( points_equal( actors[ i ]->current_tile, actors[ i ]->target_tile ) ) {
                        ghost_states[ i ] = STATE_NORMAL;
                        normal_enter( actors, animations, i, render_clips[ i ], i );
                    }
                    
                    break;
            }
        }

        // pacman eats power pellet
        for( int power_pellet_indx = 0; power_pellet_indx < 4; ++power_pellet_indx ) {
        // pac-man eats power pellet
            if ( points_equal( actors[ 0 ]->current_tile, tilemap.tm_power_pellet_tiles[ power_pellet_indx ] ) ){
                score.score_number += 20;
                g_NumGhostsEaten = 0;
                tilemap.tm_power_pellet_tiles[ power_pellet_indx ] = TILE_NONE;
                g_NumDots--;

                for( int ghost_state_idx = 1; ghost_state_idx < 5; ++ghost_state_idx ) {

                    if ( ghost_states[ ghost_state_idx ] != STATE_GO_TO_PEN && ghost_states[ ghost_state_idx ] != STATE_LEAVE_PEN ) {

                        ghost_states[ ghost_state_idx ] = STATE_VULNERABLE;
                        vulnerable_enter( actors, animations, ghost_state_idx, render_clips[ ghost_state_idx ] );
                    }
                    
                }   
                ghost_vulnerable_timer = 20.0f;                        
            }
            if ( points_equal( actors[ 5 ]->current_tile, tilemap.tm_power_pellet_tiles[ power_pellet_indx ] ) ){
                score.score_number += 20;
                g_NumGhostsEaten = 0;
                tilemap.tm_power_pellet_tiles[ power_pellet_indx ] = TILE_NONE;
                g_NumDots--;

                for( int ghost_state_idx = 1; ghost_state_idx < 5; ++ghost_state_idx ) {

                    if ( ghost_states[ ghost_state_idx ] != STATE_GO_TO_PEN && ghost_states[ ghost_state_idx ] != STATE_LEAVE_PEN ) {

                        ghost_states[ ghost_state_idx ] = STATE_VULNERABLE;
                        vulnerable_enter( actors, animations, ghost_state_idx, render_clips[ ghost_state_idx ] );
                    }
                    
                }   
                ghost_vulnerable_timer = 20.0f;                        
            }
        
    }

        /*******************
         * PROCESS STATES
         * ******************/
        states_machine_process( actors, ghost_states, &tilemap );

        //if no ghosts normal, then stop playing ghost sound
        int any_ghosts_nomral = 0;
        for( int i = 1; i < 5 ; i++ ) {
            if( ghost_states[ i ] == STATE_NORMAL ) {
                any_ghosts_nomral = 1;
                break;
            }
        }
        if( !any_ghosts_nomral ) {
            if(Mix_Playing( GHOST_SOUND_CHANNEL ) ) {
                Mix_HaltChannel( GHOST_SOUND_CHANNEL );
            }
        }

        int any_ghosts_vuln = 0;
        for( int i = 1; i < 5 ; i++ ) {
            if( ghost_states[ i ] == STATE_VULNERABLE ) {
                any_ghosts_vuln = 1;
                break;
            }
        }
        if( !any_ghosts_vuln ) {
            if(Mix_Playing( GHOST_VULN_CHANNEL ) ) {
                Mix_HaltChannel( GHOST_VULN_CHANNEL );
            }
        }


        /********************
         * MOVE GHOSTS
         * ********************/
        ghost_move( actors, &tilemap, delta_time );

        // ADJUST ANIMATION DEPENDING ON DIRECTION ACTOR IS MOVING
        // hardcoded ids! uh oh!
        // we really just want to do this on all animations that have more than 1 row. It must have 4 rows for ghosts because they can move in 4 directions
        // When pac-man animations are complete, he will have 8 rows because he can move in 8 directions ( he cuts corners diaganolly )
        set_animation_row( animations[ 1 ], actors[ 1 ] );
        set_animation_row( animations[ 3 ], actors[ 3 ] );
        set_animation_row( animations[ 2 ], actors[ 2 ] );
        set_animation_row( animations[ 4 ], actors[ 4 ] );

        // pacman animation row
        if( actors[ 0 ]->velocity.x > 0 && actors[ 0 ]->velocity.y == 0 ) { // right
            animations[ 0 ]->current_anim_row = 0;
        }
        if( actors[ 0 ]->velocity.x < 0 && actors[ 0 ]->velocity.y == 0 ) { // left
            animations[ 0 ]->current_anim_row = 1;
        }
        if( actors[ 0 ]->velocity.x == 0 && actors[ 0 ]->velocity.y > 0 ) { // down
            animations[ 0 ]->current_anim_row = 2;
        }
        if( actors[ 0 ]->velocity.x == 0 && actors[ 0 ]->velocity.y < 0 ) { // up
            animations[ 0 ]->current_anim_row = 3;
        }
        if( actors[ 0 ]->velocity.x > 0 && actors[ 0 ]->velocity.y < 0 ) { //  up-right
            animations[ 0 ]->current_anim_row = 4;
        }
        if( actors[ 0 ]->velocity.x < 0 && actors[ 0 ]->velocity.y < 0 ) { // up-left
            animations[ 0 ]->current_anim_row = 5;
        }
        if( actors[ 0 ]->velocity.x > 0 && actors[ 0 ]->velocity.y > 0 ) { // down-right
            animations[ 0 ]->current_anim_row = 6;
        }
        if( actors[ 0 ]->velocity.x < 0 && actors[ 0 ]->velocity.y > 0 ) { // down-left
            animations[ 0 ]->current_anim_row = 7;
        }

        if( actors[ 5 ]->velocity.x > 0 && actors[ 5 ]->velocity.y == 0 ) { // right
            animations[ 5 ]->current_anim_row = 0;
        }
        if( actors[ 5 ]->velocity.x < 0 && actors[ 5 ]->velocity.y == 0 ) { // left
            animations[ 5 ]->current_anim_row = 1;
        }
        if( actors[ 5 ]->velocity.x == 0 && actors[ 5 ]->velocity.y > 0 ) { // down
            animations[ 5 ]->current_anim_row = 2;
        }
        if( actors[ 5 ]->velocity.x == 0 && actors[ 5 ]->velocity.y < 0 ) { // up
            animations[ 5 ]->current_anim_row = 3;
        }
        if( actors[ 5 ]->velocity.x > 0 && actors[ 5 ]->velocity.y < 0 ) { //  up-right
            animations[ 5 ]->current_anim_row = 4;
        }
        if( actors[ 5 ]->velocity.x < 0 && actors[ 5 ]->velocity.y < 0 ) { // up-left
            animations[ 5 ]->current_anim_row = 5;
        }
        if( actors[ 5 ]->velocity.x > 0 && actors[ 5 ]->velocity.y > 0 ) { // down-right
            animations[ 5 ]->current_anim_row = 6;
        }
        if( actors[ 5 ]->velocity.x < 0 && actors[ 5 ]->velocity.y > 0 ) { // down-left
            animations[ 5 ]->current_anim_row = 7;
        }



        /*********************
         * GHOST MODE PERIOD
         * *******************/
        ghost_mode_timer += delta_time;

        if( g_current_scatter_chase_period < NUM_SCATTER_CHASE_PERIODS && ghost_mode_timer > g_scatter_chase_period_seconds[ g_current_scatter_chase_period ] ) {
            if( g_current_ghost_mode == MODE_CHASE ) {
                g_current_ghost_mode = MODE_SCATTER;
                for( int i = 1; i < 5; ++i ) {
                    if( ghost_states[ i ] == STATE_NORMAL ) {
                        actors[ i ]->direction = opposite_directions[ actors[ i ]->direction ];
                        actors[ i ]->next_tile = actors[ i ]->current_tile; // need to do this so that the ghost will want to set a new next tile
                    }
                    
                }
            }
            else {
                g_current_ghost_mode = MODE_CHASE;
                for( int i = 1; i < 5; ++i ) {
                    if( ghost_states[ i ] == STATE_NORMAL ) {
                        actors[ i ]->direction = opposite_directions[ actors[ i ]->direction ];
                        actors[ i ]->next_tile = actors[ i ]->current_tile; // need to do this so that the ghost will want to set a new next tile
                    }

                }
            }
            ghost_mode_timer = 0.0f;
            g_current_scatter_chase_period++;
        }

        
        /**********
         * UPDATE DOTS ANIMATION
         * **********/
        int top_bound = DOT_PADDING;
        int bottom_bound = TILE_SIZE - DOT_PADDING;
        for( int r = 0; r < TILE_ROWS; ++r ) {
            for( int c = 0; c < TILE_COLS; ++c ) {
                
                tilemap.tm_dot_particles[ r ][ c ].position.y += tilemap.tm_dot_particles[ r ][ c ].velocity.y * DOT_SPEED * delta_time ;

                if ( tilemap.tm_dot_particles[ r ][ c ].position.y < top_bound ) {
                    tilemap.tm_dot_particles[ r ][ c ].position.y = top_bound;
                    tilemap.tm_dot_particles[ r ][ c ].velocity.y = 1;
                }
                if ( tilemap.tm_dot_particles[ r ][ c ].position.y > bottom_bound) {
                    tilemap.tm_dot_particles[ r ][ c ].position.y = bottom_bound;
                    
                    tilemap.tm_dot_particles[ r ][ c ].velocity.y = -1;
                }
            }
        }

        /**********
         * RENDER
         * **********/
        set_render_clip_values_based_on_actor_and_animation( render_clips, actors, tilemap.tm_screen_position, animations, 6 );
        //set_render_texture_values_based_on_actor( actors, tilemap.tm_screen_position.x, tilemap.tm_screen_position.y,render_clips, 5 );
        for( int i = 0; i < 4; ++i ) {
            SDL_Point world_position = tile_grid_point_to_world_point( tilemap.tm_power_pellet_tiles[ i ] );
            SDL_Point screen_point = world_point_to_screen_point( world_position, tilemap.tm_screen_position );
            render_clips[ 6 + i ]->dest_rect.x = screen_point.x;
            render_clips[ 6 + i ]->dest_rect.y = screen_point.y;
            render_clips[ 6 + i ]->dest_rect.w = TILE_SIZE;
            render_clips[ 6 + i ]->dest_rect.h = TILE_SIZE;
            render_clips[ 6 + i ]->flip = SDL_FLIP_NONE;
        }
        //set_render_texture_values_based_on_animation( animations, render_clips, 9 );

        SDL_SetRenderDrawColor( renderer, 0,0,0,255);
        SDL_RenderClear( renderer );    

        tm_render_with_screen_position_offset( renderer, &tilemap );

        render_render_textures( renderer, render_clips, animations, 7 );

        // power pellets
        for( int i = 0; i < 4; ++i ) {
            if( !points_equal( tilemap.tm_power_pellet_tiles[ i ], TILE_NONE ) ) {
                render_render_textures( renderer, render_clips + 6 + i, animations, 1);
            }
        }

        

        SDL_Rect black_bar = {0,0, 1920, TILE_SIZE * 2};
        SDL_SetRenderDrawColor( renderer, 0,0,0,255 );
        SDL_RenderFillRect( renderer, &black_bar);

        SDL_RenderCopy( renderer, score.score_texture, NULL, &score.score_render_dst_rect);

        /****
         * ANY MESSAGES
         **/
        for(int i = 0; i < g_NumTimedMessages; i++ ) {
            if( g_TimedMessages[ i ].remainingTime > 0.0f ) {
                
                if( g_TimedMessages[ i ].l.remainingTime > 0.0f ) {
                    
                    
                    blinkProcess( &g_ScoreBlinks[ i ], delta_time );
                    interpolate( &g_TimedMessages[ i ].l, delta_time );
                    g_TimedMessages[ i ].world_position.y = g_TimedMessages[ i ].l.value;
                    g_TimedMessages[ i ].render_dest_rect.y = world_point_to_screen_point( g_TimedMessages[ i ].world_position, tilemap.tm_screen_position ).y;
                }
                else {
                    g_ScoreBlinks[ i ].current_value_idx = 1;
                }
                

                SDL_SetTextureAlphaMod( g_TimedMessages[ i ].messageTexture, g_ScoreBlinks[ i ].values[ g_ScoreBlinks->current_value_idx ] );
                SDL_RenderCopy( renderer, g_TimedMessages[ i ].messageTexture, NULL, &g_TimedMessages[ i ].render_dest_rect);
                g_TimedMessages[ i ].remainingTime -= delta_time;
                if( g_TimedMessages[ i ].remainingTime <= 0.0f ) {
                    SDL_DestroyTexture( g_TimedMessages[ i ].messageTexture );
                    g_TimedMessages[ i ].messageTexture = NULL;
                }
            }
        }

        if( charge_button_down ) {
            SDL_SetRenderDrawColor( renderer, 255,255,255,255);
            SDL_Rect rect_holding = { 600, 0, TILE_SIZE, TILE_SIZE };
            SDL_RenderDrawRect( renderer, &rect_holding );
        }

        // DEBUG
        if ( g_show_debug_info ) {
            //grid
            SDL_SetRenderDrawColor( renderer, 50,50,50,255);
            for ( int y = tilemap.tm_screen_position.y; y < SCREEN_HEIGHT; y+= TILE_SIZE ) {
                SDL_RenderDrawLine( renderer, 0, y, SCREEN_WIDTH, y);
            }
            for ( int x = tilemap.tm_screen_position.x; x < SCREEN_WIDTH; x+= TILE_SIZE) {
                SDL_RenderDrawLine( renderer, x, 0, x, SCREEN_HEIGHT );
            }

            SDL_SetRenderDrawColor( renderer, 255, 80, 50, 30 );
            for( int row = 0; row < TILE_ROWS; row++ ) {
                for( int col = 0; col < TILE_COLS; col ++ ) {

                    if( tilemap.tm_walls[ row ][ col ] == 'x' ) {

                        SDL_Rect wall_rect = {
                            tilemap.tm_screen_position.x + ( TILE_SIZE * col ),
                            tilemap.tm_screen_position.y + ( TILE_SIZE * row ),
                            TILE_SIZE,
                            TILE_SIZE
                        };
                        
                        SDL_RenderFillRect( renderer, &wall_rect );

                    } 
                }
            }
            
            // current_tile
            for(int i = 0; i < 4; ++i) {
                SDL_SetRenderDrawColor( renderer, pac_color.r, pac_color.g, pac_color.b,150);
                SDL_Rect tile_rect = { actors[ i ]->current_tile.x * TILE_SIZE + tilemap.tm_screen_position.x, actors[ i ]->current_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE,TILE_SIZE};
                SDL_RenderFillRect( renderer, &tile_rect);
            }
            
            // next tile 
            for( int i = 0; i < 5; ++i ) {
                SDL_SetRenderDrawColor( renderer,  pac_color.r, pac_color.g, pac_color.b, 225 );
                SDL_Rect next_rect = { actors[ i ]->next_tile.x * TILE_SIZE + tilemap.tm_screen_position.x, actors[ i ]->next_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE, TILE_SIZE };
                SDL_RenderFillRect( renderer, &next_rect );

            }

            SDL_SetRenderDrawColor( renderer, 255,0,0,255);
            SDL_Rect b_target_rect = { actors[ 1 ]->target_tile.x * TILE_SIZE+ tilemap.tm_screen_position.x, actors[ 1 ]->target_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE, TILE_SIZE };
            SDL_RenderDrawRect( renderer, &b_target_rect );

            SDL_SetRenderDrawColor( renderer, 255,150,255,255);
            SDL_Rect p_target_rect = { actors[ 2 ]->target_tile.x * TILE_SIZE+ tilemap.tm_screen_position.x, actors[ 2 ]->target_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE, TILE_SIZE };
            SDL_RenderDrawRect( renderer, &p_target_rect );

            SDL_SetRenderDrawColor( renderer, 3,252,248,255);
            SDL_Rect i_target_rect = { actors[ 3 ]->target_tile.x * TILE_SIZE+ tilemap.tm_screen_position.x, actors[ 3 ]->target_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE, TILE_SIZE };
            SDL_RenderDrawRect( renderer, &i_target_rect );

            SDL_SetRenderDrawColor( renderer, 235, 155, 52,255);
            SDL_Rect c_target_rect = { actors[ 4 ]->target_tile.x * TILE_SIZE+ tilemap.tm_screen_position.x, actors[ 4 ]->target_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE, TILE_SIZE };
            SDL_RenderDrawRect( renderer, &c_target_rect );
            
            // pacman center point
            SDL_SetRenderDrawColor( renderer, 255,255,255,255);
            SDL_Point points_to_draw[ 25 ];
            
            //CENTER
            set_cross( actors[ 0 ]->world_center_point, 0, tilemap.tm_screen_position, points_to_draw );
            
            // SENSORS
            set_cross( actors[ 0 ]->world_top_sensor, 5, tilemap.tm_screen_position, points_to_draw );
            set_cross( actors[ 0 ]->world_bottom_sensor, 10, tilemap.tm_screen_position, points_to_draw );
            set_cross( actors[ 0 ]->world_left_sensor, 15, tilemap.tm_screen_position,points_to_draw );
            set_cross( actors[ 0 ]->world_right_sensor, 20,tilemap.tm_screen_position, points_to_draw );

            SDL_RenderDrawPoints( renderer, points_to_draw, 25 );

            // GHOST PEN
            SDL_SetRenderDrawColor( renderer, 255,255,255,50);
            SDL_Point ghost_pen_screen_point = world_point_to_screen_point( ghost_pen_position, tilemap.tm_screen_position );
            SDL_Rect pen_rect = { ghost_pen_screen_point.x, ghost_pen_screen_point.y, TILE_SIZE, TILE_SIZE };
            SDL_RenderFillRect( renderer, &pen_rect);

            // GHOST PEN ENTRANCE
            SDL_SetRenderDrawColor( renderer, 255,255,255,50);
            SDL_Point ghost_pen_entrance_screen_point = tile_grid_point_to_screen_point(tilemap.one_way_tile, tilemap.tm_screen_position);//world_point_to_screen_point( tilemap.one_way_tile, tilemap.tm_screen_position );
            SDL_Rect pen_entrance_rect = { ghost_pen_entrance_screen_point.x, ghost_pen_entrance_screen_point.y, TILE_SIZE, TILE_SIZE };
            SDL_RenderFillRect( renderer, &pen_entrance_rect);

        }
        SDL_RenderPresent( renderer );
        SDL_Delay(5);
    }

    // CLOSE DOWN
    for( int i = 0; i < num_texture_atlases; ++i ) {
        SDL_DestroyTexture( g_texture_atlases[ i ].texture );
        g_texture_atlases[ i ].texture = NULL;
    }
        SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    TTF_CloseFont( font );

    Mix_FreeChunk(g_GhostSound);
    Mix_FreeChunk(g_GhostVulnerableSound);
    Mix_FreeChunk(g_PacChompSound);
    Mix_FreeChunk(g_GhostEatenYeahSound);
    Mix_FreeChunk(g_GhostEatenSweetSound);
    Mix_FreeChunk(g_GhostEatenCoolSound);
    Mix_FreeChunk(g_GhostEatenGroovySound);
    Mix_FreeChunk(g_PacDieOhNoSound);


    Mix_FreeMusic(g_Music);

    Mix_CloseAudio();
    Mix_Quit();

/**
 *    This reduces noise if you're running the program through a memory
 *  profiler like valgrind. It won't complain about un-freed memory.
*/
    */
    for( int i = 0; i < 6; i++ ) {
        free(actors[ i ]);
        actors[ i ] = NULL;
    }
    for( int i = 0; i < 6; i++ ) {
        free(animations[ i ]);
        animations[ i ] = NULL;
        //animations[ i ]->texture_atlas_id = -1;
    }
    for( int i = 0; i < 10; i++ ){
        free(render_clips[ i ]);
        render_clips[ i ] = NULL;
        //render_clips[ i ]->animation_id = -1;
    }
    for( int i = 0; i < 10; i++ ) {
        free( g_texture_atlases[ i ].sprite_clips );
        g_texture_atlases[ i ].sprite_clips = NULL;

    }

    SDL_Quit();
    
}
