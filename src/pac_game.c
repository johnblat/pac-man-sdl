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
#include "UI.h"
#include "input.h"
#include "globalData.h"
#include "programState.h"
#include "menu.h"
#include "gamePlayingState.h"

//unsigned int gNumLevels = 0;


void updateDashStockRects( Entities *entities, EntityId *playerIds, unsigned int numPlayers ) {
    for(int i = 0; i < numPlayers; i++) {
        EntityId playerId = playerIds[ i ];
        if( entities->dashCooldownStocks[ playerId ] == NULL ) {
            continue;
        }
        gDashStockRects[ i ].numRectsToShow = entities->dashCooldownStocks[ playerId ]->currentNumStock;
    }
}

void renderDashStockRects( ) {
    for( int i = 0; i < gNumDashStockRects; i++ ) {
        SDL_SetRenderDrawColor( gRenderer, gDashStockRects[ i ].color.r, gDashStockRects[ i ].color.g, gDashStockRects[ i ].color.b, 255 );
        for( int numRect = 0; numRect < gDashStockRects[ i ].numRectsToShow; numRect++ ) {
            SDL_RenderFillRect( gRenderer, &gDashStockRects[i].rects[numRect]);
        }
    }
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
    
    TileMap tilemap;
    Entities entities;
    LevelConfig levelConfig;

    for(int i = 0; i < 8; i++ ){
        gGhostIds[ i ] = INVALID_ENTITY_ID;
    }

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
        entities.keybinds         [ i ] = NULL;
        entities.inputMasks       [ i ] = NULL;
        entities.pickupTypes      [ i ] = NULL;
        entities.dashCooldownStocks [ i ] = NULL;
        entities.activeTimers [ i ] = NULL;
        entities.numDots [ i ] = NULL;
        entities.scores [ i ] = NULL;
        entities.mirrorEntityRefs[ i ] = NULL;
        entities.invincibilityTimers[i] = NULL;
        entities.isActive[i] = NULL;
        entities.gameControllerIds[i] = NULL;
        entities.speedBoostTimers[i] = NULL;
        entities.stopTimers[i] = NULL;
        entities.mirrorEntityRefs[i] = NULL;
        


    }

    
    // initialize default keybindings

    addKeyBinding(0, SDL_SCANCODE_UP, g_INPUT_UP );
    addKeyBinding(1, SDL_SCANCODE_LEFT, g_INPUT_LEFT );
    addKeyBinding(2, SDL_SCANCODE_RIGHT, g_INPUT_RIGHT );
    addKeyBinding(3, SDL_SCANCODE_DOWN, g_INPUT_DOWN );
    addKeyBinding(4, SDL_SCANCODE_Z, g_INPUT_ACTION );


    gNumLevels = determine_number_of_levels_from_dirs();
    printf("Num levels %d\n", gNumLevels);

    //gCurrentLevel++;

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

    Uint32 windowFlags = SDL_WINDOW_SHOWN;
    if( gIsFullscreen ) {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }
    gWindow = SDL_CreateWindow( "JB Pacmonster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);
    if ( gWindow == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if (gRenderer == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    if( SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND) < 0 ){
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
    int gameControllerJoyId[4] = {-1};

    for( int i = 0; i <g_NumJoysticks; i++ ) {
        if( SDL_IsGameController( i ) ) {
            gameControllerJoyId[ g_NumGamepads ] = i;
            g_NumGamepads++;
        }
    }

    if( g_NumGamepads > 0 ) {
        for( int i = 0; i < g_NumGamepads; i++) {
            g_GameControllers[ i ] = SDL_GameControllerOpen( gameControllerJoyId[ i ] );
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

    g_GhostSound = Mix_LoadWAV("res/sounds/ghosts_ambient.wav");
    if( g_GhostSound == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }
    Mix_VolumeChunk( g_GhostSound, 5 );

    g_GhostVulnerableSound = Mix_LoadWAV("res/sounds/ghosts_ambient_scared1.wav");
    if( g_GhostVulnerableSound == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }
    Mix_VolumeChunk( g_GhostVulnerableSound, 10 );

    g_PacChompSound = Mix_LoadWAV("res/sounds/wakka_wakka1.wav");
    if( g_PacChompSound == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }
    Mix_VolumeChunk( g_PacChompSound, 30 );

    g_PacChompSound2 = Mix_LoadWAV("res/sounds/wakka_wakka2.wav");
    if( g_PacChompSound2 == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }
    Mix_VolumeChunk( g_PacChompSound2, 30 );

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

    g_GhostEatSound = Mix_LoadWAV("res/sounds/ghost_eaten.wav");
    if( g_GhostEatSound == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }
    Mix_VolumeChunk( g_GhostEatSound, 20 );

    g_PickupEaten = Mix_LoadWAV("res/sounds/powerup.wav");
    if( g_PickupEaten == NULL ) {
        fprintf( stderr, "failed to load sound: %s\n", Mix_GetError() );
    }
    Mix_VolumeChunk( g_PickupEaten, 20 );
   // Mix_VolumeChunk( g_GhostEatenGroovySound, 100 );

    g_GhostEatenSounds[ 0 ] = g_GhostEatenYeahSound;
    g_GhostEatenSounds[ 1 ] = g_GhostEatenSweetSound;
    g_GhostEatenSounds[ 2 ] = g_GhostEatenCoolSound;
    g_GhostEatenSounds[ 3 ] = g_GhostEatenGroovySound;

    Mix_VolumeMusic( 20 );
    

    Mix_PlayMusic(g_Music, -1 );
    
    // initialize font
    gFont = TTF_OpenFont("res/gomarice_no_continue.ttf", 30 );
    if ( gFont == NULL ) {
        fprintf(stderr, "%s\n", TTF_GetError());
        exit( EXIT_FAILURE );
    }

    // INIT TEXTURE ATLASES
    load_global_texture_atlases_from_config_file( gRenderer );

    // setup first level data
    //load_current_level_off_disk( &levelConfig, &tilemap, gRenderer);

    
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
        g_TimedMessages[ i ].font = gFont;
        g_TimedMessages[ i ].remainingTime = 0.0f;
        g_TimedMessages[ i ].world_position.x = -1;
        g_TimedMessages[ i ].world_position.y = -1;
    }

    for( int i = 0; i < g_NumTimedMessages; i ++ ) {
        g_ScoreBlinks[ i ] = blinkInit( 0.033, 50, 255 );
    }

    // calculate number of dots
    // for( int row = 0; row < TILE_ROWS; row++ ) {
    //     for(int col =0; col < TILE_COLS; col++ ) {
    //         if( tilemap.tm_dots[ row ][ col ] == 'x' ) {
    //             g_NumDots++;
    //         }
    //     }
    // }

    // //try_load_resource_from_file( tilemap.tm_power_pellet_tiles, "res/power_pellets", sizeof( SDL_Point ), 4 );

    // // add power pellets to number of dotss
    // for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
    //     if( entities.pickupTypes[ i ] == NULL || *entities.pickupTypes[ i ] != POWER_PELLET_PICKUP ) {
    //         continue;
    //     }
    //     if( !points_equal( entities.actors[ i ]->current_tile, TILE_NONE ) ) {
    //         g_NumDots++;
    //     }
    // }


    // INIT Players
    int numPlayers = 1;
    initializePlayersFromFiles( &entities, &levelConfig, numPlayers );
    
    // init ghosts
    initializeGhostsFromFile( &entities, &levelConfig, "res/ghost_animated_sprites");


    

    // load everything for entity data from config
    //level_advance( &levelConfig, &tilemap, gRenderer, &entities );


    
    

    // pre-initialize pickup entities ( non power pellet )
    for( int i = 0; i < MAX_PICKUPS_PER_LEVEL; i++ ) { // start with 4
        createInitialTemporaryPickup( &entities, &levelConfig );
    }
    // AnimatedSprite *fruitAnimatedSprite = init_animation(9,1,1,1);
    // createFruit( &entities, &levelConfig, fruitAnimatedSprite, 60 );
    

    SDL_Point ghost_pen_position = tile_grid_point_to_world_point( levelConfig.ghostPenTile  ); 
    SDL_Point ghost_pen_center_point;
    ghost_pen_center_point.x = ghost_pen_position.x + (TILE_SIZE / 2);
    ghost_pen_center_point.y = ghost_pen_position.y + (TILE_SIZE / 2);

    // INIT SCORE
    
    gScore.font = gFont;
    gScore.score_color = pac_color;
    gScore.score_number = 0;
    SDL_Surface *score_surface = TTF_RenderText_Solid( gScore.font, "Score : 0", gScore.score_color);
    gScore.score_texture = SDL_CreateTextureFromSurface( gRenderer, score_surface );
    gScore.score_render_dst_rect.x = 10;
    gScore.score_render_dst_rect.y = 10;
    gScore.score_render_dst_rect.w = score_surface->w;
    gScore.score_render_dst_rect.h = score_surface->h;

    // INIT LIVES UI
    gLivesRemainingUI.font = gFont;
    gLivesRemainingUI.color = pac_color;
    gLivesRemainingUI.livesRemaining = 0;
    SDL_Surface *lr_surface = TTF_RenderText_Solid(gLivesRemainingUI.font, "Lives: 0", gLivesRemainingUI.color );

    SDL_DestroyTexture( gLivesRemainingUI.texture );
    gLivesRemainingUI.texture = SDL_CreateTextureFromSurface( gRenderer, lr_surface );
    gLivesRemainingUI.destRect.x = 1800;
    gLivesRemainingUI.destRect.y = 10;
    gLivesRemainingUI.destRect.w = lr_surface->w;
    gLivesRemainingUI.destRect.h = lr_surface->h;

    SDL_FreeSurface(lr_surface);


    SDL_FreeSurface( score_surface );


    // PREPARE VARIABLES FOR LOOP
    SDL_Event event;
    

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

    // TODO: Compress this initialization into a function
    SDL_Surface *mainMenuTextSurface = TTF_RenderText_Solid( gFont, gTitleScreenText, white );
    gTitleScreenTextTexture = SDL_CreateTextureFromSurface( gRenderer, mainMenuTextSurface );
    gTitleScreenTextDestRect.x = SCREEN_WIDTH / 2 - mainMenuTextSurface->w/2 ;
    gTitleScreenTextDestRect.y =  SCREEN_HEIGHT/2 - mainMenuTextSurface->h/2;
    gTitleScreenTextDestRect.w =  mainMenuTextSurface->w;
    gTitleScreenTextDestRect.h =  mainMenuTextSurface->h;
    SDL_FreeSurface( mainMenuTextSurface );
    mainMenuTextSurface = NULL;

    initMainMenuScreenStuff();
    initJoinGameStuff();
    initGamePlayingStuff();

    Blink startMenuBlink = blinkInit( 0.33f, 255, 50);

    while (!gQuit) {
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
        


        switch( gProgramState ) {
            /**************
             * ************
             * *** MAIN MENU
             * *****************/
            case MENU_PROGRAM_STATE:
                menuProgramStateProcess( &event,  &entities,  &levelConfig,  &tilemap,  &startMenuBlink,  deltaTime  );
                //titleScreenProcess( &levelConfig, &entities, &tilemap, &event, &startMenuBlink, deltaTime );
                break;
            /**************
             * ************
             * *** GAME PLAYING
             * *****************/
            case GAME_PLAYING_PROGRAM_STATE:

                gamePlayingStateProcess( &event, &entities, &tilemap, &levelConfig, deltaTime );
                break;
            case EXIT_STATE:
                gQuit = SDL_TRUE;
                break;
            default:
                printf("something went wrong. gProgramState: %d\n", gProgramState );
                break;
        }
        

        
        
    }

    // CLOSE DOWN
    for( int i = 0; i < num_texture_atlases; ++i ) {
        SDL_DestroyTexture( g_texture_atlases[ i ].texture );
        g_texture_atlases[ i ].texture = NULL;
    }
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    TTF_CloseFont( gFont );

    Mix_FreeChunk(g_GhostSound);
    Mix_FreeChunk(g_GhostVulnerableSound);
    Mix_FreeChunk(g_PacChompSound);
    Mix_FreeChunk(g_GhostEatenYeahSound);
    Mix_FreeChunk(g_GhostEatenSweetSound);
    Mix_FreeChunk(g_GhostEatenCoolSound);
    Mix_FreeChunk(g_GhostEatenGroovySound);
    Mix_FreeChunk(g_PacDieOhNoSound);
    Mix_FreeChunk(g_PacChompSound2);
    Mix_FreeChunk(g_GhostEatSound);
    Mix_FreeChunk(g_PickupEaten);

    for( int i = 0; i < 4; i++ ) {
        if(g_GameControllers[i] != NULL ){
            SDL_GameControllerClose(g_GameControllers[i]);
        }
    }

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
        if( entities.activeTimers[i] != NULL ) {
            free(entities.activeTimers[i]);
            entities.activeTimers[i] = NULL;
        }
        if( entities.numDots[i] != NULL ) {
            free(entities.numDots[i]);
            entities.numDots[i] = NULL;
        }
        if( entities.scores[i] != NULL ) {
            free(entities.scores[i]);
            entities.scores[i] = NULL;
        }
        if( entities.mirrorEntityRefs[i] != NULL ) {
            free(entities.mirrorEntityRefs[i]);
            entities.mirrorEntityRefs[i] = NULL;
        }
        if( entities.speedBoostTimers[i] != NULL ) {
            free(entities.speedBoostTimers[i]);
            entities.speedBoostTimers[i] = NULL;
        }
        if( entities.invincibilityTimers[i] != NULL ) {
            free(entities.invincibilityTimers[i]);
            entities.invincibilityTimers[i] = NULL;
        }
        if( entities.stopTimers[i] != NULL ) {
            free(entities.stopTimers[i]);
            entities.stopTimers[i] = NULL;
        }
        if( entities.isActive[i] != NULL ) {
            free(entities.isActive[i]);
            entities.isActive[i] = NULL;
        }
        
    }
    for( int i = 0; i < MAX_TEXTURE_ATLASES; i++ ) {
        free( g_texture_atlases[ i ].sprite_clips );
        g_texture_atlases[ i ].sprite_clips = NULL;

    }

    SDL_Quit();
    
}
