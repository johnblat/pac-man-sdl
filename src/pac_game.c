#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_gamecontroller.h>
#include <stdio.h>
#include "entity.h"
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
#include "renderProcessing.h"
#include "interpolation.h"
#include "sounds.h"
#include "input.h"

SDL_bool gIsFullscreen = SDL_TRUE;

typedef enum {
    MAIN_MENU_GAME_STATE,
    GAME_PLAYING_GAME_STATE,
    EXIT_STATE
} GameState;

typedef enum {
    GAME_PLAYING,
    GAME_PAUSED
} GamePlayingState;

GameState gGameState = MAIN_MENU_GAME_STATE;
GamePlayingState gGamePlayingState = GAME_PLAYING;

// Main Menu
char *gMainMenuText = "Press START to Play!";
SDL_Texture *gMainMenuTextTexture = NULL;

char *gPauseText = "PAUSED";
SDL_Texture *gPauseTextTexture = NULL;

SDL_Texture *gCooldownTexture = NULL;
SDL_Rect gCooldownRect;

// End Main Menu

SDL_bool g_show_debug_info = SDL_FALSE;

SDL_Color pac_color = {200,150,0};
SDL_Color white = {200,200,255};

// used to track progress in level
unsigned int g_NumDots = 0;

uint8_t g_NumGhostsEaten = 0;
unsigned int g_GhostPointValues[] = { 400, 800, 1600, 3200 };


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

// TODO: FIX
// Returns SDL_TRUE if advanced to next level
// Returns SDL_FALSE if no more levels to advance
SDL_bool level_advance(LevelConfig *levelConfig, TileMap *tilemap, SDL_Renderer *renderer, Entities *entities) {
    gCurrentLevel++;

    if( gCurrentLevel > gNumLevels ) {
        return SDL_TRUE;
    }

    load_current_level_off_disk( levelConfig, tilemap, renderer );

    // pacman
    // reset players
    for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
        if( entities->inputMasks[i] == NULL ) {
            continue;
        }
        actor_reset_data( entities->actors[ i ], levelConfig->pacStartingTile );
        *entities->inputMasks[i  ] = 0b0;
        *entities->chargeTimers[i] = 0.0f;
        *entities->dashTimers[i] = 0.0f;
    }
    
    
    //ghosts
    // INIT GHOST
    SDL_Point ghost_pen_tile;
    ghost_pen_tile = levelConfig->ghostPenTile;

    
    // reset ghosts
    for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
        if( entities->ghostStates[ i ] == NULL ) {
            continue;
        }
        actor_reset_data( entities->actors[ i ], ghost_pen_tile );
        entities->animatedSprites[i ]->texture_atlas_id = entities->animatedSprites[i]->default_texture_atlas_id;
        *entities->ghostStates[ i ] = STATE_LEAVE_PEN;
        leave_pen_enter( entities, i );
    }


    // calculate number of dots
    g_NumDots = 0;

    for( int row = 0; row < TILE_ROWS; row++ ) {
        for(int col =0; col < TILE_COLS; col++ ) {
            if( tilemap->tm_dots[ row ][ col ] == 'x' ) {
                g_NumDots++;
            }
        }
    }

    // add power pellets to number of dots
    // set power pellets 
    for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {

    }
    // for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
    //     if( !points_equal( tilemap->tm_power_pellet_tiles[i], TILE_NONE ) ) {
    //         g_NumDots++;
    //     }
    // }

    int ppIdx = 0; 
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->pickupTypes[ eid ] == NULL || *entities->pickupTypes[ eid ] != POWER_PELLET_PICKUP ) {
            continue;
        }
        
        entities->actors[ eid ]->current_tile = levelConfig->powerPelletTiles[ ppIdx ];
        entities->actors[ eid ]->world_position.x = tile_grid_point_to_world_point(entities->actors[ eid ]->current_tile ).x;
        entities->actors[ eid ]->world_position.y = tile_grid_point_to_world_point(entities->actors[ eid ]->current_tile ).y;

        g_NumDots++;
        ppIdx++;
    
    }

    return SDL_FALSE;

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
    TTF_Font *font; 
    TileMap tilemap;
    Entities entities;
    LevelConfig levelConfig;

    // initialize entities
    for( int i = 0; i < MAX_NUM_ENTITIES; i ++ ) { 
        entities.positions        [ i ] = NULL;
        entities.actors           [ i ] = NULL;
        entities.animatedSprites  [ i ] = NULL;
        entities.renderDatas     [ i ] = NULL;
        entities.ghostStates      [ i ] = NULL;
        entities.targetingBehaviors [ i ] = NULL;
        entities.chargeTimers     [ i ] = NULL;
        entities.dashTimers       [ i ] = NULL;
        entities.slowTimers       [ i ] = NULL;
        entities.inputMasks       [ i ] = NULL;
        entities.pickupTypes      [ i ] = NULL;
        entities.dashCooldownStocks [ i ] = NULL;
    }
    
    // TIMER USED FOR VULNERABILITY STATE
    float ghost_vulnerable_timer = 0.0f;
    // GHOST BEHAVIOR TIMER FOR CURRENT GLOBAL GHOST MODE
    float ghost_mode_timer = 0.0f;
    

    int numLevels = determine_number_of_levels_from_dirs();
    printf("Num levels %d\n", numLevels);

    gCurrentLevel++;

    // initialize levelConfig
    SDL_Point zero = {0, 0};
    levelConfig.scatterChasePeriodSeconds = g_scatter_chase_period_seconds;
    levelConfig.numScatterChasePeriods = NUM_SCATTER_CHASE_PERIODS;
    levelConfig.ghostPenTile = zero;
    levelConfig.pacStartingTile = zero;

    // initializing texture atlas pointer in tilemap 
    tilemap.tm_texture_atlas = NULL; 


    // Initializing SDL stuff
    if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf( stderr, "Error %s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    if( TTF_Init() < 0 ) {
        fprintf( stderr, "%s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    window = SDL_CreateWindow( "JB Pacmonster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN );
    if ( window == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
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
    Mix_AllocateChannels( 16 ); // increase number of channels o process more sfx

    // game controller initialization
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
    g_Music = Mix_LoadMUS( gMenuMusicFilename );
    // g_Music = Mix_LoadMUS( "res/sounds/test/Dont-Worry-We-Got-Warp-Spe.mp3" );
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

    Mix_VolumeMusic( 0 );

    Mix_PlayMusic(g_Music, -1 );
    
    // initialize font
    font = TTF_OpenFont("res/gomarice_no_continue.ttf", 30 );
    if ( font == NULL ) {
        fprintf(stderr, "%s\n", TTF_GetError());
        exit( EXIT_FAILURE );
    }

    // INIT TEXTURE ATLASES
    load_global_texture_atlases_from_config_file( renderer );

    // setup first level data
    load_current_level_off_disk( &levelConfig, &tilemap, renderer);

    
    for( int i = 0; i < 4; i++ ) {
        AnimatedSprite *powerPelletSprite = init_animation( 5, 12, 1, 6 );
        createPowerPellet( &entities, powerPelletSprite, levelConfig.powerPelletTiles[ i ] );
    }




    // INIT TILEMAP
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
    for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
        if( entities.pickupTypes[ i ] == NULL || *entities.pickupTypes[ i ] != POWER_PELLET_PICKUP ) {
            continue;
        }
        if( !points_equal( entities.actors[ i ]->current_tile, TILE_NONE ) ) {
            g_NumDots++;
        }
    }

    // INIT Playwe
    initializePlayersFromFiles( &entities, &levelConfig, 2 );
    
    // init ghosts
    initializeGhostsFromFile( &entities, &levelConfig, "res/ghost_animated_sprites");

    // load everything for entity data from config

    EntityId playerIds[ 2 ];
    unsigned int numPlayers = 0;
    for( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
        if( entities.inputMasks[ eid ] != NULL ) {
            playerIds[ numPlayers ] = eid;
            numPlayers++;
        }
    }

    // add game controllers to players
    for( int i = 0; i < g_NumGamepads; i++ ) {
        if( i > numPlayers ) {
            break;
        }
        entities.gameControllers[ playerIds[ i ] ] = g_GameControllers[ i ];
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
    float fpsTimerStart = SDL_GetTicks();
    float fpsTimerCurrent = fpsTimerStart - SDL_GetTicks()  ;
    unsigned int countedFrames = 0;
    float avgFps = countedFrames / (fpsTimerCurrent*0.001f);
    if( avgFps > 20000 ) {
        avgFps = 0.0f;
    } 
    SDL_bool charge_button_up = SDL_FALSE;
    SDL_bool charge_button_down = SDL_FALSE;


    SDL_Surface *mainMenuTextSurface = TTF_RenderText_Solid( font, gMainMenuText, white );
    gMainMenuTextTexture = SDL_CreateTextureFromSurface( renderer, mainMenuTextSurface );
    SDL_Rect gMainMenuTextDestRect = {SCREEN_WIDTH / 2 - mainMenuTextSurface->w/2 ,SCREEN_HEIGHT/2 - mainMenuTextSurface->h/2, mainMenuTextSurface->w, mainMenuTextSurface->h};
    SDL_FreeSurface( mainMenuTextSurface );
    mainMenuTextSurface = NULL;

    SDL_Surface *pauseTextSurface = TTF_RenderText_Solid( font, gPauseText, pac_color );
    gPauseTextTexture = SDL_CreateTextureFromSurface( renderer, pauseTextSurface);
    SDL_Rect pauseTextDestRect = { SCREEN_WIDTH / 2 - pauseTextSurface->w/2 ,SCREEN_HEIGHT/2 - pauseTextSurface->h/2, pauseTextSurface->w, pauseTextSurface->h};
    SDL_FreeSurface( pauseTextSurface );
    pauseTextSurface = NULL;

    Blink startMenuBlink = blinkInit( 0.33f, 255, 50);

    while (!quit) {
        // semi-fixed timestep
        float current_frame_ticks = SDL_GetTicks() / 1000.0;
        float deltaTime = current_frame_ticks - previous_frame_ticks;
        previous_frame_ticks = current_frame_ticks;
        // adjust for any pauses, debugging breaks, etc
        deltaTime = deltaTime < max_delta_time ?  deltaTime : max_delta_time;
        ++countedFrames;
        fpsTimerCurrent = SDL_GetTicks() - fpsTimerStart ;
        avgFps = countedFrames/(fpsTimerCurrent*0.001f);
        if( avgFps > 20000 ) {
            avgFps = 0.0f;
        } 
        if( (countedFrames % 200 ) == 0 ) { // only display every so often and update the counter to get a new read on avg. This way spikes won't effect it that much
            printf("FPS:%f\n", avgFps);
            fpsTimerStart = SDL_GetTicks(); //reset timer to get new avg
            countedFrames = 0;

        }
        


        switch( gGameState ) {
            /**************
             * ************
             * *** MAIN MENU
             * *****************/
            case MAIN_MENU_GAME_STATE:
                while( SDL_PollEvent( &event ) != 0 ) {
                    if( event.type == SDL_QUIT ) {
                        gGameState = EXIT_STATE;
                        break;
                    }
                    if( event.type == SDL_KEYUP ) {
                        if( event.key.keysym.sym == SDLK_RETURN ) {
                            Mix_HaltMusic();
                            Mix_FreeMusic( g_Music );
                            g_Music = Mix_LoadMUS( gGameMusicFilename );
                            Mix_PlayMusic( g_Music, -1 );
                            gGameState = GAME_PLAYING_GAME_STATE;
                            break;
                        }
                        if( event.key.keysym.sym == SDLK_ESCAPE ) {
                            quit = 1;
                            break;
                        }
                        if( event.key.keysym.sym == SDLK_F11 ) {
                            Uint32 windowFlags = SDL_GetWindowFlags( window );
                            SDL_SetWindowFullscreen( window, windowFlags ^= SDL_WINDOW_FULLSCREEN );
                        }
                    }
                    
                        
                    if( event.type == SDL_CONTROLLERBUTTONUP ) {
                        for( int i = 0; i < g_NumGamepads; i++ ) {
                            
                            if( event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_GameControllers[ i ]))) {
                                if( event.cbutton.button == SDL_CONTROLLER_BUTTON_START ) {
                                    Mix_HaltMusic();
                                    Mix_FreeMusic( g_Music );
                                    g_Music = Mix_LoadMUS( gGameMusicFilename );
                                    Mix_PlayMusic( g_Music, -1 );
                                    gGameState = GAME_PLAYING_GAME_STATE;
                                    break;                                    
                                }
                            }
                        }
                    }
                }

                blinkProcess( &startMenuBlink, deltaTime );

                SDL_SetTextureAlphaMod( gMainMenuTextTexture, startMenuBlink.values[ startMenuBlink.current_value_idx ] );

                SDL_SetRenderDrawColor( renderer, 20,20,20,255);
                SDL_RenderClear( renderer );

                // display message

                SDL_RenderCopy( renderer, gMainMenuTextTexture, NULL, &gMainMenuTextDestRect);
                SDL_RenderPresent( renderer );
                break;

            /**************
             * ************
             * *** GAME PLAYING
             * *****************/

            case GAME_PLAYING_GAME_STATE:

                switch(gGamePlayingState) {
                    /**************
                     * ************
                     * *** GAME PLAYING
                     * *****************/
                    case GAME_PLAYING:

                                //gamePlayingProcess( &entities, &tilemap, deltaTime );
                            // EVENTS
                        while (SDL_PollEvent( &event ) != 0 ) {
                            //int gameControllerState = SDL_GameControllerEventState( SDL_QUERY );
                            
                            if( event.type == SDL_QUIT ) {
                                gGameState = EXIT_STATE;
                                break;
                            }
                            if ( event.type == SDL_KEYDOWN ) {
                                if ( event.key.keysym.sym == SDLK_b ) {
                                    g_show_debug_info = !g_show_debug_info;
                                }
                                if (event.key.keysym.sym == SDLK_v ) {
                                    // ghost_vulnerable_timer = 20.0f;
                                    // for( int i = 1; i < 5; ++i ) {
                                    //     ghost_states[ i ] = STATE_VULNERABLE;
                                    //     vulnerable_enter( actors, animations, i, renderDatas[ i ] );
                                    // }
                                    
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
                                if( event.key.keysym.sym == SDLK_RETURN ) {
                                    gGamePlayingState = GAME_PAUSED;
                                    break;
                                }
                                if( event.key.keysym.sym == SDLK_ESCAPE ) {
                                    quit = 1;
                                    break;
                                }
                                if( event.key.keysym.sym == SDLK_F11 ) {
                                    Uint32 windowFlags = SDL_GetWindowFlags( window );
                                    SDL_SetWindowFullscreen( window, windowFlags ^= SDL_WINDOW_FULLSCREEN );
                                }
                        
                            
                            }
                            if( event.type == SDL_CONTROLLERBUTTONDOWN ) {
                                for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
                                    if( entities.gameControllers[ i ] == NULL || entities.inputMasks[ i ] == NULL ) {
                                        continue;
                                    }
                                    if( event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(entities.gameControllers[ i ]))) {
                                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP ) {
                                            *entities.inputMasks[ i ] |= g_INPUT_UP;
                                        }
                                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT ) {
                                            *entities.inputMasks[ i ] |= g_INPUT_LEFT;
                                        }
                                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT ) {
                                            *entities.inputMasks[ i ] |= g_INPUT_RIGHT;
                                        }
                                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN ) {
                                            *entities.inputMasks[ i ] |= g_INPUT_DOWN;
                                        }
                                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_X ) {
                                            *entities.inputMasks[ i ] |= g_INPUT_ACTION;
                                            charge_button_down = SDL_TRUE;
                                            charge_button_up = SDL_FALSE;
                                        }
                                    }
                                }
                                
                            }
                            if( event.type == SDL_CONTROLLERBUTTONUP ) {
                                for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
                                    if( entities.gameControllers[ i ] == NULL || entities.inputMasks[ i ] == NULL ) {
                                        continue;
                                    }
                                    if( event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(entities.gameControllers[ i ]))) {
                                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP ) {
                                            *entities.inputMasks[ i ] ^= g_INPUT_UP;
                                        }
                                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT ) {
                                            *entities.inputMasks[ i ] ^= g_INPUT_LEFT;
                                        }
                                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT ) {
                                            *entities.inputMasks[ i ] ^= g_INPUT_RIGHT;
                                        }
                                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN ) {
                                            *entities.inputMasks[ i ] ^= g_INPUT_DOWN;
                                        }
                                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_X ) {
                                            *entities.inputMasks[ i ] ^= g_INPUT_ACTION;
                                            charge_button_up = SDL_TRUE;
                                            charge_button_down = SDL_FALSE;
                                        }
                                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_START ) {
                                            gGamePlayingState = GAME_PAUSED;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        if(quit) break;

                        // NEXT LEVEL?
                        if( g_NumDots <= 0 ) {
                            
                            SDL_bool gameCleared = level_advance( &levelConfig, &tilemap, renderer, &entities );
                            if( gameCleared ) {
                                Mix_HaltChannel( GHOST_SOUND_CHANNEL );
                                Mix_HaltChannel( GHOST_VULN_CHANNEL );
                                Mix_HaltMusic();
                                Mix_FreeMusic( g_Music );
                                g_Music = Mix_LoadMUS( gMenuMusicFilename );
                                Mix_PlayMusic( g_Music, -1 );
                                gGameState = MAIN_MENU_GAME_STATE;
                                gCurrentLevel = 1;
                                load_current_level_off_disk( &levelConfig, &tilemap, renderer);
                                break;
                            }
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
                            SDL_Delay(300);
                            SDL_bool gameCleared = level_advance( &levelConfig, &tilemap, renderer, &entities );
                            if( gameCleared ) {
                                Mix_HaltChannel( GHOST_SOUND_CHANNEL );
                                Mix_HaltChannel( GHOST_VULN_CHANNEL );
                                Mix_HaltMusic();
                                Mix_FreeMusic( g_Music );
                                g_Music = Mix_LoadMUS( gMenuMusicFilename );
                                Mix_PlayMusic( g_Music, -1 );
                                gGameState = MAIN_MENU_GAME_STATE;
                                gCurrentLevel = 1;
                                load_current_level_off_disk( &levelConfig, &tilemap, renderer);
                                break;
                            }
                        }
                        
                        

                        // UPDATE SIMULATION

                        inputToTryMoveProcess( &entities, &tilemap, deltaTime);

                        dashTimersProcess( &entities, deltaTime );
                        cooldownProcess( &entities, deltaTime );

                        SDL_DestroyTexture( gCooldownTexture );
                        char coolDownNumberText[2];
                        snprintf(coolDownNumberText, 2, "%d", entities.dashCooldownStocks[ 4 ]->currentNumStock );
                        SDL_Surface *cooldownSurface = TTF_RenderText_Solid( font, coolDownNumberText, pac_color );
                        gCooldownTexture = SDL_CreateTextureFromSurface( renderer, cooldownSurface );
                        gCooldownRect.x =  SCREEN_WIDTH/2 - cooldownSurface->w/2;
                        gCooldownRect.y =  20;
                        gCooldownRect.w = cooldownSurface->w; 
                        gCooldownRect.h = cooldownSurface->h ;
                        SDL_FreeSurface( cooldownSurface );
                        

                        animatedSpriteIncProcess( entities.animatedSprites , deltaTime); 
                        
                        // slow down pacman if in pac-pellet-tile
        
                        // process eating dots slow timers
                        for( int eid = 0 ; eid < MAX_NUM_ENTITIES; eid++ ) {
                            if( entities.slowTimers[ eid ] == NULL ) {
                                continue;
                            }
                            if( tilemap.tm_dots[ entities.actors[ eid ]->current_tile.y ][ entities.actors[ eid ]->current_tile.x ] == 'x' ) {
                                *entities.slowTimers[ eid ] = 0.075f;
                            }
                        }

                        collectDotProcess( &entities, tilemap.tm_dots, &g_NumDots, &score, renderer );

                        
                        // CHECK STATE TRANSITIONS FOR GHOSTS
                        for(int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
                            if( entities.ghostStates[ eid ] == NULL ) {
                                continue;
                            }
                            switch( *entities.ghostStates[ eid ] ) {
                                case STATE_VULNERABLE :
                                    // return all ghosts that are still vulnerable to normal
                                    // when the timer runs out
                                    if (ghost_vulnerable_timer <= 0.0f ) {
                                        g_NumGhostsEaten = 0;
                                        *entities.ghostStates[ eid ] = STATE_NORMAL;
                                        normal_enter( &entities, eid );
                                        // for( int i = 1; i < 5; ++i ) {
                                        //     if( *entities.ghostStates[ eid ] == STATE_VULNERABLE ) {
                                        //         *entities.ghostStates[ eid ] = STATE_NORMAL;
                                        //         normal_enter( &entities, eid );
                                        //     }
                                        // }
                                    }

                                    EntityId playerId;
                                    for( int i = 0; i < numPlayers; ++i ) {
                                        playerId = playerIds[ i ];

                                        // eat ghost if pacman touches
                                        if ( entities.actors[ playerId ]->current_tile.x == entities.actors[ eid ]->current_tile.x 
                                        && entities.actors[ playerId ]->current_tile.y == entities.actors[ eid ]->current_tile.y ) {
                                            Mix_PlayChannel( -1, g_PacChompSound, 0 );
                                            Mix_PlayChannel( -1, g_GhostEatenSounds[ g_NumGhostsEaten ], 0);
                                            score.score_number+=g_GhostPointValues[ g_NumGhostsEaten ];
                                            g_NumGhostsEaten++;
                                            *entities.ghostStates[ eid ] = STATE_GO_TO_PEN;
                                            uint8_t texture_atlas_id = 4;
                                            entities.animatedSprites[ eid ]->texture_atlas_id = texture_atlas_id;
                                            entities.actors[ eid ]->next_tile = entities.actors[ eid ]->current_tile;
                                            entities.actors[ eid ]->target_tile = ghost_pen_tile;
                                            entities.actors[ eid ]->speed_multp = 1.6f;

                                            // show message
                                            for( int i = 0; i < g_NumTimedMessages; i++ ) {
                                                if( g_TimedMessages[ i ].remainingTime <= 0.0f ) {
                                                    g_TimedMessages[ i ].remainingTime = 0.85f;
                                                    g_TimedMessages[ i ].world_position = tile_grid_point_to_world_point( entities.actors[ playerId ]->current_tile );
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
                                    }
                                    break;
                                    
                                case STATE_GO_TO_PEN :
                                    // ghost is in pen
                                    
                                    if( points_equal(entities.actors[ eid ]->current_tile, ghost_pen_tile ) && entities.actors[ eid ]->world_center_point.y >= ghost_pen_center_point.y) {

                                        *entities.ghostStates[ eid ] = STATE_LEAVE_PEN;
                                        leave_pen_enter( &entities, eid );
                                    }
                                    break;

                                
                                case STATE_NORMAL :
                                    break;
                                case STATE_LEAVE_PEN:
                                    if( points_equal( entities.actors[ eid ]->current_tile, entities.actors[ eid ]->target_tile ) ) {
                                        *entities.ghostStates[ eid ] = STATE_NORMAL;
                                        normal_enter( &entities, eid );
                                    }
                                    
                                    break;
                            }
                        }

                        // pacman eats power pellet
                        // TODO: FIX

                        // process power pellet pickups
                        for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
                            if( entities.pickupTypes[ eid ] == NULL || *entities.pickupTypes[ eid] != POWER_PELLET_PICKUP ) { // its a power pellet pickup
                                continue;
                            }

                            for( int i = 0; i < numPlayers; i++ ) {
                                EntityId playerId = playerIds[ i ];
                                // player eats power pellet
                                if( points_equal( entities.actors[ eid ]->current_tile, entities.actors[ playerId ]->current_tile ) ) {
                                    score.score_number += 20;
                                    g_NumGhostsEaten = 0;
                                    // move it outside of the world area for now.
                                    // TODO: deactivate this somehow
                                    entities.actors[ eid ]->current_tile.x = -1;
                                    entities.actors[ eid ]->current_tile.y = -1;
                                    entities.actors[ eid ]->world_position.x = -100;
                                    entities.actors[ eid ]->world_position.y = -100;
                                    entities.actors[ eid ]->world_center_point.x = -100;
                                    entities.actors[ eid ]->world_center_point.y = -100;
                                    g_NumDots--;

                                    // make ghosts all vulnerable state
                                    for( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
                                        if( entities.ghostStates[ eid ] == NULL ) {
                                            continue;
                                        }
                                        if ( *entities.ghostStates[ eid ] != STATE_GO_TO_PEN && *entities.ghostStates[ eid ] != STATE_LEAVE_PEN ) {

                                            *entities.ghostStates[ eid ] = STATE_VULNERABLE;
                                            vulnerable_enter( &entities, eid );
                                        }
                                        
                                    }   
                                    ghost_vulnerable_timer = 20.0f;   
                                }
                            }

                        }


                        /*******************
                         * PROCESS STATES
                         * ******************/
                        //states_machine_process( actors, ghost_states, &tilemap );

                        //if no ghosts normal, then stop playing ghost sound
                        int any_ghosts_nomral = 0;
                        for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
                            if( entities.ghostStates[ eid ] == STATE_NORMAL ) {
                                any_ghosts_nomral = 1;
                                break;
                            }
                        }
                        if( !any_ghosts_nomral ) {
                            if(Mix_Playing( GHOST_SOUND_CHANNEL ) ) {
                                Mix_HaltChannel( GHOST_SOUND_CHANNEL );
                            }
                        }

                        // PROCESS GLOBAL GAME STATE FOR ANY GHOSTS VULNERABLE
                        int any_ghosts_vuln = 0;
                        for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
                            if( entities.ghostStates[ eid ] == NULL ) {
                                continue;
                            }
                            if( *entities.ghostStates[ eid ] == STATE_VULNERABLE ) {
                                any_ghosts_vuln = 1;
                                break;
                            }
                        }
                        if( !any_ghosts_vuln ) {
                            if(Mix_Playing( GHOST_VULN_CHANNEL ) ) {
                                Mix_HaltChannel( GHOST_VULN_CHANNEL );
                            }
                        }
                        else {
                            ghost_vulnerable_timer -= deltaTime;
                        }

                        ghostsProcess( &entities, playerIds, 2, &tilemap,  deltaTime,&levelConfig);

                        /********************
                         * MOVE GHOSTS
                         * ********************/
                        for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
                            if( entities.targetingBehaviors[ i ] == NULL ) {
                                continue;
                            }
                            ghost_move( entities.actors, i, &tilemap, deltaTime );
                        }
                        
                        /*********************
                         * GHOST MODE PERIOD
                         * *******************/
                        ghost_mode_timer += deltaTime;

                        if( g_current_scatter_chase_period < NUM_SCATTER_CHASE_PERIODS && ghost_mode_timer > g_scatter_chase_period_seconds[ g_current_scatter_chase_period ] ) {
                            if( g_current_ghost_mode == MODE_CHASE ) {
                                g_current_ghost_mode = MODE_SCATTER;
                                for( int i = 0; i < MAX_NUM_ENTITIES; ++i ) {
                                    if( entities.ghostStates[ i ] != NULL && entities.ghostStates[ i ] == STATE_NORMAL ) {
                                        entities.actors[ i ]->direction = opposite_directions[ entities.actors[ i ]->direction ];
                                        entities.actors[ i ]->next_tile = entities.actors[ i ]->current_tile; // need to do this so that the ghost will want to set a new next tile
                                    }
                                    
                                }
                            }
                            else {
                                g_current_ghost_mode = MODE_CHASE;
                                for( int i = 0; i < MAX_NUM_ENTITIES; ++i ) {
                                    if( entities.ghostStates[ i ] != NULL && entities.ghostStates[ i ] == STATE_NORMAL ) {
                                        entities.actors[ i ]->direction = opposite_directions[ entities.actors[ i ]->direction ];
                                        entities.actors[ i ]->next_tile = entities.actors[ i ]->current_tile; // need to do this so that the ghost will want to set a new next tile
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
                                
                                tilemap.tm_dot_particles[ r ][ c ].position.y += tilemap.tm_dot_particles[ r ][ c ].velocity.y * DOT_SPEED * deltaTime ;

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
                        set_render_clip_values_based_on_positions_and_animation( &entities, tilemap.tm_screen_position );

                        

                        SDL_SetRenderDrawColor( renderer, 0,0,0,255);
                        SDL_RenderClear( renderer );    

                        tm_render_with_screen_position_offset( renderer, &tilemap );

                        renderDataForAnimatedSpriteProcess( renderer, &entities );

                        

                        SDL_Rect black_bar = {0,0, 1920, TILE_SIZE * 2};
                        SDL_SetRenderDrawColor( renderer, 0,0,0,255 );
                        SDL_RenderFillRect( renderer, &black_bar);

                        SDL_RenderCopy( renderer, score.score_texture, NULL, &score.score_render_dst_rect);
                        SDL_RenderCopy(renderer, gCooldownTexture, NULL, &gCooldownRect);


                        /****
                         * ANY MESSAGES
                         **/
                        for(int i = 0; i < g_NumTimedMessages; i++ ) {
                            if( g_TimedMessages[ i ].remainingTime > 0.0f ) {
                                
                                if( g_TimedMessages[ i ].l.remainingTime > 0.0f ) {
                                    
                                    
                                    blinkProcess( &g_ScoreBlinks[ i ], deltaTime );
                                    interpolate( &g_TimedMessages[ i ].l, deltaTime );
                                    g_TimedMessages[ i ].world_position.y = g_TimedMessages[ i ].l.value;
                                    g_TimedMessages[ i ].render_dest_rect.y = world_point_to_screen_point( g_TimedMessages[ i ].world_position, tilemap.tm_screen_position ).y;
                                }
                                else {
                                    g_ScoreBlinks[ i ].current_value_idx = 1;
                                }
                                

                                SDL_SetTextureAlphaMod( g_TimedMessages[ i ].messageTexture, g_ScoreBlinks[ i ].values[ g_ScoreBlinks->current_value_idx ] );
                                SDL_RenderCopy( renderer, g_TimedMessages[ i ].messageTexture, NULL, &g_TimedMessages[ i ].render_dest_rect);
                                g_TimedMessages[ i ].remainingTime -= deltaTime;
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
                            
                            SDL_SetRenderDrawColor( renderer,  pac_color.r, pac_color.g, pac_color.b, 150 );

                            SDL_Rect tileRect;
                            SDL_Point screenPoint;
                            // current tile
                            for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
                                if( entities.actors[ i ] == NULL ) {
                                    continue;
                                }
                                screenPoint = tile_grid_point_to_screen_point( entities.actors[ i ]->current_tile, tilemap.tm_screen_position );
                                tileRect.x = screenPoint.x;    
                                tileRect.y = screenPoint.y;    
                                tileRect.w = TILE_SIZE;    
                                tileRect.h = TILE_SIZE;    
                                SDL_RenderFillRect( renderer, &tileRect);

                                screenPoint = tile_grid_point_to_screen_point( entities.actors[ i ]->next_tile, tilemap.tm_screen_position );
                                tileRect.x = screenPoint.x;    
                                tileRect.y = screenPoint.y; 
                                SDL_RenderFillRect( renderer, &tileRect);

                            }
                            // target tile
                            for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
                                if( entities.targetingBehaviors[ i ] == NULL ) {
                                    continue;
                                }

                                switch( *entities.targetingBehaviors[ i ] ) {
                                    case SHADOW_BEHAVIOR:
                                        SDL_SetRenderDrawColor( renderer, 255,0,0,255);
                                        break;
                                    case AMBUSH_BEHAVIOR:
                                        SDL_SetRenderDrawColor( renderer, 255,0,255,255);
                                        break;
                                    case MOODY_BEHAVIOR:
                                        SDL_SetRenderDrawColor( renderer, 0,255,255,255);
                                        break;
                                    case POKEY_BEHAVIOR:
                                        SDL_SetRenderDrawColor( renderer, 150,150,0,255);
                                        break;
                                    default:
                                        SDL_SetRenderDrawColor( renderer, 255,255,255,255);
                                        break;
                                }
                                screenPoint = tile_grid_point_to_screen_point( entities.actors[ i ]->target_tile, tilemap.tm_screen_position );
                                tileRect.x = screenPoint.x;
                                tileRect.y = screenPoint.y;
                                SDL_RenderDrawRect( renderer, &tileRect );
                            }


                            // current_tile
                            // for(int i = 0; i < 4; ++i) {
                            //     SDL_SetRenderDrawColor( renderer, pac_color.r, pac_color.g, pac_color.b,150);
                            //     SDL_Rect tile_rect = { actors[ i ]->current_tile.x * TILE_SIZE + tilemap.tm_screen_position.x, actors[ i ]->current_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE,TILE_SIZE};
                            //     SDL_RenderFillRect( renderer, &tile_rect);
                            // }
                            
                            // // next tile 
                            // for( int i = 0; i < 5; ++i ) {
                            //     SDL_SetRenderDrawColor( renderer,  pac_color.r, pac_color.g, pac_color.b, 225 );
                            //     SDL_Rect next_rect = { actors[ i ]->next_tile.x * TILE_SIZE + tilemap.tm_screen_position.x, actors[ i ]->next_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE, TILE_SIZE };
                            //     SDL_RenderFillRect( renderer, &next_rect );

                            // }

                            // SDL_SetRenderDrawColor( renderer, 255,0,0,255);
                            // SDL_Rect b_target_rect = { actors[ 1 ]->target_tile.x * TILE_SIZE+ tilemap.tm_screen_position.x, actors[ 1 ]->target_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE, TILE_SIZE };
                            // SDL_RenderDrawRect( renderer, &b_target_rect );

                            // SDL_SetRenderDrawColor( renderer, 255,150,255,255);
                            // SDL_Rect p_target_rect = { actors[ 2 ]->target_tile.x * TILE_SIZE+ tilemap.tm_screen_position.x, actors[ 2 ]->target_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE, TILE_SIZE };
                            // SDL_RenderDrawRect( renderer, &p_target_rect );

                            // SDL_SetRenderDrawColor( renderer, 3,252,248,255);
                            // SDL_Rect i_target_rect = { actors[ 3 ]->target_tile.x * TILE_SIZE+ tilemap.tm_screen_position.x, actors[ 3 ]->target_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE, TILE_SIZE };
                            // SDL_RenderDrawRect( renderer, &i_target_rect );

                            // SDL_SetRenderDrawColor( renderer, 235, 155, 52,255);
                            // SDL_Rect c_target_rect = { actors[ 4 ]->target_tile.x * TILE_SIZE+ tilemap.tm_screen_position.x, actors[ 4 ]->target_tile.y * TILE_SIZE + tilemap.tm_screen_position.y, TILE_SIZE, TILE_SIZE };
                            // SDL_RenderDrawRect( renderer, &c_target_rect );
                            
                            // // pacman center point
                            // SDL_SetRenderDrawColor( renderer, 255,255,255,255);
                            // SDL_Point points_to_draw[ 25 ];
                            
                            // //CENTER
                            // set_cross( actors[ 0 ]->world_center_point, 0, tilemap.tm_screen_position, points_to_draw );
                            
                            // // SENSORS
                            // set_cross( actors[ 0 ]->world_top_sensor, 5, tilemap.tm_screen_position, points_to_draw );
                            // set_cross( actors[ 0 ]->world_bottom_sensor, 10, tilemap.tm_screen_position, points_to_draw );
                            // set_cross( actors[ 0 ]->world_left_sensor, 15, tilemap.tm_screen_position,points_to_draw );
                            // set_cross( actors[ 0 ]->world_right_sensor, 20,tilemap.tm_screen_position, points_to_draw );

                            // SDL_RenderDrawPoints( renderer, points_to_draw, 25 );

                            // // GHOST PEN
                            // SDL_SetRenderDrawColor( renderer, 255,255,255,50);
                            // SDL_Point ghost_pen_screen_point = world_point_to_screen_point( ghost_pen_position, tilemap.tm_screen_position );
                            // SDL_Rect pen_rect = { ghost_pen_screen_point.x, ghost_pen_screen_point.y, TILE_SIZE, TILE_SIZE };
                            // SDL_RenderFillRect( renderer, &pen_rect);

                            // // GHOST PEN ENTRANCE
                            // SDL_SetRenderDrawColor( renderer, 255,255,255,50);
                            // SDL_Point ghost_pen_entrance_screen_point = tile_grid_point_to_screen_point(tilemap.one_way_tile, tilemap.tm_screen_position);//world_point_to_screen_point( tilemap.one_way_tile, tilemap.tm_screen_position );
                            // SDL_Rect pen_entrance_rect = { ghost_pen_entrance_screen_point.x, ghost_pen_entrance_screen_point.y, TILE_SIZE, TILE_SIZE };
                            // SDL_RenderFillRect( renderer, &pen_entrance_rect);

                        }
                        SDL_RenderPresent( renderer );
                        break;
                    /**************
                     * ************
                     * *** GAME PAUSED
                     * *****************/
                    case GAME_PAUSED:
                        while (SDL_PollEvent( &event ) != 0 ) {
                            //int gameControllerState = SDL_GameControllerEventState( SDL_QUERY );
                            
                            if( event.type == SDL_QUIT ) {
                                gGameState = EXIT_STATE;
                                break;
                            }
                            if( event.type == SDL_KEYUP ) {
                                if( event.key.keysym.sym == SDLK_RETURN ) {
                                    gGamePlayingState = GAME_PLAYING;
                                }
                                if( event.key.keysym.sym == SDLK_F11 ) {
                                    Uint32 windowFlags = SDL_GetWindowFlags( window );
                                    SDL_SetWindowFullscreen( window, windowFlags ^= SDL_WINDOW_FULLSCREEN );
                                }
                            }
                            if( event.type == SDL_CONTROLLERBUTTONUP ) {
                                for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
                                    if( entities.gameControllers[ i ] == NULL || entities.inputMasks[ i ] == NULL ) {
                                        continue;
                                    }
                                    if( event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(entities.gameControllers[ i ]))) {
                                        if( event.cbutton.button == SDL_CONTROLLER_BUTTON_START ) {
                                            gGamePlayingState = GAME_PLAYING;
                                            break;
                                        }
                                    }
                                }
                                
                            }
                        }

                        const Uint8 *currentKeyState = SDL_GetKeyboardState( NULL );
                        if( currentKeyState[ SDL_SCANCODE_RETURN ] ) {
                            
                        }   

                        SDL_RenderCopy( renderer, gPauseTextTexture, NULL, &pauseTextDestRect);

                        SDL_RenderPresent( renderer );
                        break;
                }
                
                break;
            case EXIT_STATE:
                quit = SDL_TRUE;
                break;
            default:
                printf("something went wrong. gGameState: %d\n", gGameState );
                break;
        }
        

        
        
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

    for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
        if( entities.actors[ i ] != NULL ) {
            free(entities.actors[ i ] );
            entities.actors[ i ] = NULL;
        }
        if( entities.animatedSprites[ i ] != NULL ) {
            free(entities.animatedSprites[ i ] );
            entities.animatedSprites[ i ] = NULL;
        }
        if( entities.chargeTimers[ i ] != NULL ) {
            free(entities.chargeTimers[ i ] );
            entities.chargeTimers[ i ] = NULL;
        }
        if( entities.dashTimers[ i ] != NULL ) {
            free(entities.dashTimers[ i ] );
            entities.dashTimers[ i ] = NULL;
        }
        if( entities.ghostStates[ i ] != NULL ) {
            free(entities.ghostStates[ i ] );
            entities.ghostStates[ i ] = NULL;
        }
        if( entities.inputMasks[ i ] != NULL ) {
            free(entities.inputMasks[ i ] );
            entities.inputMasks[ i ] = NULL;
        }
        if( entities.positions[ i ] != NULL ) {
            free(entities.positions[ i ] );
            entities.positions[ i ] = NULL;
        }
        if( entities.renderDatas[ i ] != NULL ) {
            free(entities.renderDatas[ i ] );
            entities.renderDatas[ i ] = NULL;
        }
        if( entities.slowTimers[ i ] != NULL ) {
            free(entities.slowTimers[ i ] );
            entities.slowTimers[ i ] = NULL;
        }
        if( entities.targetingBehaviors[ i ] != NULL ) {
            free(entities.targetingBehaviors[ i ] );
            entities.targetingBehaviors[ i ] = NULL;
        }
        if( entities.pickupTypes[i] != NULL ) {
            free(entities.pickupTypes[i]);
            entities.pickupTypes[i] = NULL;
        }
        if(entities.dashCooldownStocks[i] != NULL ) {
            free(entities.dashCooldownStocks[i]);
            entities.dashCooldownStocks[i] = NULL;
        }
        
    }
    for( int i = 0; i < MAX_TEXTURE_ATLASES; i++ ) {
        free( g_texture_atlases[ i ].sprite_clips );
        g_texture_atlases[ i ].sprite_clips = NULL;

    }

    SDL_Quit();
    
}
