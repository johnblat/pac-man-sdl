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

//unsigned int gNumLevels = 0;



Score gScore;

typedef enum {
    MAIN_MENU_PROGRAM_STATE,
    GAME_PLAYING_PROGRAM_STATE,
    EXIT_STATE
} ProgramState;

typedef enum {
    GAME_PLAYING,
    GAME_PAUSED,
    LEVEL_START,
    LEVEL_END
} GamePlayingState;

ProgramState gProgramState = MAIN_MENU_PROGRAM_STATE;
GamePlayingState gGamePlayingState = GAME_PLAYING;

// level transition variables
const char* gLevelStartText = "READY!";
SDL_Texture *gLevelStartTextTexture = NULL;
SDL_Rect gLevelStartTextRect;

const char *gLevelEndText = "LEVEL CLEARED!";
SDL_Texture *gLevelEndTextTexture = NULL;
SDL_Rect gLevelEndTextRect;

const float gLevelStartDuration = 3.0f; 
float gLevelStartTimer = 0.0f;
const float gLevelEndDuration = 1.5f;
float gLevelEndTimer = 0.0f;

// Main Menu
char *gMainMenuText = "Press START to Play!";
SDL_Texture *gMainMenuTextTexture = NULL;
SDL_Rect gMainMenuTextDestRect;

char *gPauseText = "PAUSED";
SDL_Texture *gPauseTextTexture = NULL;
SDL_Rect gPauseTextDestRect;

SDL_Texture *gCooldownTexture = NULL;
SDL_Rect gCooldownRect;

// End Main Menu


// TODO: FIX
// Returns SDL_TRUE if advanced to next level
// Returns SDL_FALSE if no more levels to advance
SDL_bool level_advance(LevelConfig *levelConfig, TileMap *tilemap, SDL_Renderer *renderer, Entities *entities) {

    gCurrentLevel++;

    if( gCurrentLevel > gNumLevels ) {
        return SDL_TRUE;
    }

    // reset ghost periods 
    g_current_ghost_mode = MODE_SCATTER;
    g_current_scatter_chase_period = 0;
    gGhostModeTimer = 0.0f;

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
        *entities->inputMasks[i]= 0b0;
        entities->actors[i]->direction = DIR_NONE;
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
        if(entities->numDots[eid] != NULL ) {
            continue;
        }
        
        entities->actors[ eid ]->current_tile = levelConfig->powerPelletTiles[ ppIdx ];
        entities->actors[ eid ]->world_position.x = tile_grid_point_to_world_point(entities->actors[ eid ]->current_tile ).x;
        entities->actors[ eid ]->world_position.y = tile_grid_point_to_world_point(entities->actors[ eid ]->current_tile ).y;

        g_NumDots++;
        ppIdx++;
    
    }

    g_StartingNumDots = g_NumDots;

    // " zero out " all pickups. Makes sure they don't get processed
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->pickupTypes[ eid ] == NULL || *entities->pickupTypes[ eid ] == POWER_PELLET_PICKUP ) {
            continue;
        }

        *entities->pickupTypes[ eid ] = NONE_PICKUP;
        *entities->activeTimers[ eid ] = 0.0f;

    }

    // set all of the pickups to what the levelConfgi has
    unsigned int pickupIdx = 0;

    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->pickupTypes[ eid ] == NULL || entities->numDots[eid] == NULL ) {
            continue;
        }

        if( pickupIdx >= levelConfig->numPickupConfigs ) {
            break; // loaded in all the pickup configs for the level
        }

        entities->actors[ eid ]->current_tile = levelConfig->pacStartingTile;

        entities->actors[ eid ]->world_position.x = tile_grid_point_to_world_point(entities->actors[ eid ]->current_tile ).x;
        entities->actors[ eid ]->world_position.y = tile_grid_point_to_world_point(entities->actors[ eid ]->current_tile ).y;

        *entities->pickupTypes[ eid ] = levelConfig->pickupConfigs[ pickupIdx ].pickupType;
        *entities->activeTimers[ eid ] = levelConfig->pickupConfigs[ pickupIdx ].activeTime;
        *entities->numDots[ eid ] = levelConfig->pickupConfigs[ pickupIdx ].numDots;
        *entities->scores[ eid ] = levelConfig->pickupConfigs[ pickupIdx ].scoreReward;
        entities->animatedSprites[ eid ]->texture_atlas_id = levelConfig->pickupConfigs[ pickupIdx ].textureAtlasId;
        entities->animatedSprites[ eid ]->default_texture_atlas_id = entities->animatedSprites[ eid ]->texture_atlas_id;

        pickupIdx++; // will go to the next one if any

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

void mainMenuProcess(LevelConfig *levelConfig, Entities *entities, TileMap *tilemap, SDL_Event *event, Blink *startMenuBlink, float deltaTime );
inline void mainMenuProcess( LevelConfig *levelConfig, Entities *entities, TileMap *tilemap, SDL_Event *event, Blink *startMenuBlink, float deltaTime ) {
    while( SDL_PollEvent( event ) != 0 ) {
        if( event->type == SDL_QUIT ) {
            gProgramState = EXIT_STATE;
            break;
        }
        if( event->type == SDL_KEYUP ) {
            if( event->key.keysym.sym == SDLK_RETURN ) {
                Mix_HaltMusic();
                Mix_FreeMusic( g_Music );
                g_Music = Mix_LoadMUS( gGameMusicFilename );
                if( g_Music == NULL ) {
                    fprintf(stderr, "Could not load music from file %s. Error: %s\n\n", gGameMusicFilename, Mix_GetError() );
                }
                int channel = Mix_PlayMusic( g_Music, -1 );
                if( !( channel >= 0 ) ) {
                    fprintf(stderr, "Could not play %s. Error: %s\n", gGameMusicFilename, Mix_GetError() );
                }
                gProgramState = GAME_PLAYING_PROGRAM_STATE;
                gGamePlayingState = LEVEL_START;
                level_advance( levelConfig, tilemap, gRenderer, entities );
                break;
            }
            if( event->key.keysym.sym == SDLK_ESCAPE ) {
                gQuit = 1;
                break;
            }
            if( event->key.keysym.sym == SDLK_F11 ) {
                Uint32 windowFlags = SDL_GetWindowFlags( gWindow );
                SDL_SetWindowFullscreen( gWindow, windowFlags ^= SDL_WINDOW_FULLSCREEN );
            }
        }
        if( event->type == SDL_CONTROLLERDEVICEADDED ) {
            if( ! (g_NumGamepads>=MAX_NUM_GAME_CONTROLLERS ) ) {
                Sint32 joyStickDeviceId = event->cdevice.which;
                if(SDL_IsGameController(joyStickDeviceId)) {
                    // is the game controller already exist
                    
                    
                    // look for first open gamepad slot
                    for( int i = 0; i < MAX_NUM_GAME_CONTROLLERS; i++ ) {

                        if( g_GameControllers[ i ] == NULL ) {
                            g_GameControllers[ i ] = SDL_GameControllerOpen( joyStickDeviceId );
                            if( !SDL_GameControllerGetAttached( g_GameControllers[ g_NumGamepads ] ) ) {
                                fprintf(stderr, "Wrong!\n");
                            }
                            printf("Controller added: %s\n", SDL_GameControllerName(g_GameControllers[ g_NumGamepads ] ));
                            g_NumGamepads++;
                            break;
                        }
                    }
                    
                }
            }
            else {
                printf("Too Many Controllers Added! Can't add this one!\n");
            }
        }
        if( event->type == SDL_CONTROLLERDEVICEREMOVED ) {
            Sint32 joyStickDeviceId = event->cdevice.which;
            //if(SDL_IsGameController(joyStickDeviceId)) {
                for( int i = 0; i < MAX_NUM_GAME_CONTROLLERS; i++ ) {
                    SDL_Joystick *joy = SDL_GameControllerGetJoystick( g_GameControllers[ i ] );
                    
                    if( joyStickDeviceId == SDL_JoystickInstanceID( joy ) ) {
                        printf("Controller removed\n" );
                        SDL_GameControllerClose( g_GameControllers[ i ] );
                        g_GameControllers[ i ] = NULL;
                        g_NumGamepads--;

                    }
                }
            //}
        }
        
            
        if( event->type == SDL_CONTROLLERBUTTONUP ) {
            for( int i = 0; i < g_NumGamepads; i++ ) {
                
                if( event->cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_GameControllers[ i ]))) {
                    if( event->cbutton.button == SDL_CONTROLLER_BUTTON_START ) {
                        Mix_HaltMusic();
                        Mix_FreeMusic( g_Music );
                        g_Music = Mix_LoadMUS( gGameMusicFilename );
                        Mix_PlayMusic( g_Music, -1 );
                        gProgramState = GAME_PLAYING_PROGRAM_STATE;
                        gGamePlayingState = LEVEL_START;
                        level_advance( levelConfig, tilemap, gRenderer, entities );
                        break;                                    
                    }
                }
            }
        }
    }

    blinkProcess( startMenuBlink, deltaTime );

    SDL_SetTextureAlphaMod( gMainMenuTextTexture, startMenuBlink->values[ startMenuBlink->current_value_idx ] );

    SDL_SetRenderDrawColor( gRenderer, 20,20,20,255);
    SDL_RenderClear( gRenderer );

    // display message

    SDL_RenderCopy( gRenderer, gMainMenuTextTexture, NULL, &gMainMenuTextDestRect);
    SDL_RenderPresent( gRenderer );
}

void gamePlayingProcess( Entities *entities, TileMap *tilemap, SDL_Event *event, LevelConfig *levelConfig, float deltaTime) ;
inline void gamePlayingProcess( Entities *entities, TileMap *tilemap, SDL_Event *event, LevelConfig *levelConfig, float deltaTime) {

    while (SDL_PollEvent( event ) != 0 ) {
        //int gameControllerState = SDL_GameControllerEventState( SDL_QUERY );
        
        if( event->type == SDL_QUIT ) {
            gProgramState = EXIT_STATE;
            break;
        }
        if ( event->type == SDL_KEYDOWN ) {
            if ( event->key.keysym.sym == SDLK_b ) {
                g_show_debug_info = !g_show_debug_info;
            }
            if (event->key.keysym.sym == SDLK_v ) {
                // ghost_vulnerable_timer = 20.0f;
                // for( int i = 1; i < 5; ++i ) {
                //     ghost_states[ i ] = STATE_VULNERABLE;
                //     vulnerable_enter( actors, animations, i, renderDatas[ i ] );
                // }
                
            }

        }
        if( event->type == SDL_KEYUP ) {

            if( event->key.keysym.sym == SDLK_RETURN ) {
                gGamePlayingState = GAME_PAUSED;
                break;
            }
            if( event->key.keysym.sym == SDLK_ESCAPE ) {
                gQuit = 1;
                break;
            }
            if( event->key.keysym.sym == SDLK_F11 ) {
                Uint32 windowFlags = SDL_GetWindowFlags( gWindow );
                SDL_SetWindowFullscreen( gWindow, windowFlags ^= SDL_WINDOW_FULLSCREEN );
            }
    
        
        }
        if( event->type == SDL_CONTROLLERBUTTONDOWN ) {
            for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
                if( entities->gameControllers[ i ] == NULL || entities->inputMasks[ i ] == NULL ) {
                    continue;
                }
                if( event->cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(entities->gameControllers[ i ]))) {
                    if( event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP ) {
                        *entities->inputMasks[ i ] |= g_INPUT_UP;
                    }
                    if( event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT ) {
                        *entities->inputMasks[ i ] |= g_INPUT_LEFT;
                    }
                    if( event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT ) {
                        *entities->inputMasks[ i ] |= g_INPUT_RIGHT;
                    }
                    if( event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN ) {
                        *entities->inputMasks[ i ] |= g_INPUT_DOWN;
                    }
                    if( event->cbutton.button == SDL_CONTROLLER_BUTTON_X ) {
                        *entities->inputMasks[ i ] |= g_INPUT_ACTION;

                    }
                }
            }
            
        }
        if( event->type == SDL_CONTROLLERBUTTONUP ) {
            for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
                if( entities->gameControllers[ i ] == NULL || entities->inputMasks[ i ] == NULL ) {
                    continue;
                }
                if( event->cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(entities->gameControllers[ i ]))) {
                    if( event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP ) {
                        if( *entities->inputMasks[i] & g_INPUT_UP )
                            *entities->inputMasks[ i ] ^= g_INPUT_UP;
                    }
                    if( event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT ) {
                        if( *entities->inputMasks[i] & g_INPUT_LEFT )
                            *entities->inputMasks[ i ] ^= g_INPUT_LEFT;
                    }
                    if( event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT ) {
                        if( *entities->inputMasks[i] & g_INPUT_RIGHT )
                            *entities->inputMasks[ i ] ^= g_INPUT_RIGHT;
                    }
                    if( event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN ) {
                        if( ( *entities->inputMasks[ i ] & g_INPUT_DOWN ) ) {
                            *entities->inputMasks[ i ] ^= g_INPUT_DOWN;
                        }
                        
                    }
                    if( event->cbutton.button == SDL_CONTROLLER_BUTTON_X ) {
                        *entities->inputMasks[ i ] ^= g_INPUT_ACTION;

                    }
                    if( event->cbutton.button == SDL_CONTROLLER_BUTTON_START ) {
                        gGamePlayingState = GAME_PAUSED;
                        break;
                    }
                }
            }
        }
    }
    if(gQuit) return;

    // NEXT LEVEL?
    if( g_NumDots <= 0 ) {
        
        SDL_bool gameCleared = level_advance( levelConfig, tilemap, gRenderer, entities );
        if( gameCleared ) {
            Mix_HaltChannel( GHOST_SOUND_CHANNEL );
            Mix_HaltChannel( GHOST_VULN_CHANNEL );
            Mix_HaltMusic();
            Mix_FreeMusic( g_Music );
            g_Music = Mix_LoadMUS( gMenuMusicFilename );
            Mix_PlayMusic( g_Music, -1 );
            gProgramState = MAIN_MENU_PROGRAM_STATE;
            gCurrentLevel = 0;
            //load_current_level_off_disk( levelConfig, tilemap, gRenderer);
            return;
        }
        Mix_HaltChannel( GHOST_SOUND_CHANNEL );
        Mix_HaltChannel( GHOST_VULN_CHANNEL );
        gGamePlayingState = LEVEL_END;
        return;
    }

    // KEYBOARD STATE

    const Uint8 *current_key_states = SDL_GetKeyboardState( NULL );
    //int gameControllerState = SDL_GameControllerEventState( SDL_QUERY );
    
    // set input masks for keybound players
    for( int i = 0 ; i < gNumPlayers; i++ ) {
        EntityId playerId = gPlayerIds[ i ];
        if( entities->keybinds[ playerId ] == NULL ) {
            continue;
        }

        for( int i = 0; i < 5; i++ ) { // 5 is the number of keybinds possible right now.
            SDL_Scancode scancode = entities->keybinds[ playerId ][i].scancode;
            InputMask inputMask = entities->keybinds[ playerId ][i].inputMask;
            if(current_key_states[ scancode ]) {
                *entities->inputMasks[ playerId ] |= inputMask;
            } 
            else { // key not pressed
                if( *entities->inputMasks[playerId] & inputMask ) {// make sure its not in mask {
                    *entities->inputMasks[ playerId ] ^= inputMask;
                }
            }
        }
    }
    
    // adjust tilemap
    if( current_key_states[ SDL_SCANCODE_S ] ) {
        tilemap->tm_screen_position.y++;
        // increase double speed if shift held down
        if( current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
            tilemap->tm_screen_position.y+=4;
        }
    }
    if( current_key_states[ SDL_SCANCODE_W ] ) {
        tilemap->tm_screen_position.y--;
        // increase double speed if shift held down

        if( current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
            tilemap->tm_screen_position.y-=4;
        }
    }
    if( current_key_states[ SDL_SCANCODE_D ] ) {
        tilemap->tm_screen_position.x++;
        // increase double speed if shift held down
        if( current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
            tilemap->tm_screen_position.x+=4;
        }
    }
    if( current_key_states[ SDL_SCANCODE_A ] ) {
        tilemap->tm_screen_position.x--;
        // increase double speed if shift held down
        if( current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
            tilemap->tm_screen_position.x-=4;
        }
    }
    if( current_key_states[ SDL_SCANCODE_COMMA ] ) {
        SDL_Delay(300);
        SDL_bool gameCleared = level_advance( levelConfig, tilemap, gRenderer, entities );
        if( gameCleared ) {
            Mix_HaltChannel( GHOST_SOUND_CHANNEL );
            Mix_HaltChannel( GHOST_VULN_CHANNEL );
            Mix_HaltMusic();
            Mix_FreeMusic( g_Music );
            g_Music = Mix_LoadMUS( gMenuMusicFilename );
            Mix_PlayMusic( g_Music, -1 );
            gProgramState = MAIN_MENU_PROGRAM_STATE;
            gCurrentLevel = 0;
            //load_current_level_off_disk( levelConfig, tilemap, gRenderer);
            return;
        }
    }
    
    

    // UPDATE SIMULATION

    //reset alpha mods. Will be adjusted accordingly by other process functions
    for( int eid = 0; eid < g_NumEntities; eid++ ) {
        entities->renderDatas[eid]->alphaMod = 255;
    }

    inputToTryMoveProcess( entities, tilemap, deltaTime);
    dashTimersProcess( entities, deltaTime );
    cooldownProcess( entities, deltaTime );
    processTempMirrorPlayers( entities, deltaTime);
    processSpeedBoostTimer( entities, deltaTime );
    processStopTimers( entities, deltaTime);
    processInvincibilityTimers(entities, deltaTime);


    SDL_DestroyTexture( gCooldownTexture );
    char coolDownNumberText[2];
    snprintf(coolDownNumberText, 2, "%d", entities->dashCooldownStocks[ 4 ]->currentNumStock );
    SDL_Surface *cooldownSurface = TTF_RenderText_Solid( gFont, coolDownNumberText, pac_color );
    gCooldownTexture = SDL_CreateTextureFromSurface( gRenderer, cooldownSurface );
    gCooldownRect.x =  SCREEN_WIDTH/2 - cooldownSurface->w/2;
    gCooldownRect.y =  20;
    gCooldownRect.w = cooldownSurface->w; 
    gCooldownRect.h = cooldownSurface->h ;
    SDL_FreeSurface( cooldownSurface );
    

    animatedSpriteIncProcess( entities->animatedSprites , deltaTime); 
    
    // slow down pacman if in pac-pellet-tile

    // process eating dots slow timers
    for( int eid = 0 ; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->slowTimers[ eid ] == NULL ) {
            continue;
        }
        if( tilemap->tm_dots[ entities->actors[ eid ]->current_tile.y ][ entities->actors[ eid ]->current_tile.x ] == 'x' ) {
            *entities->slowTimers[ eid ] = 0.05f;
        }
    }

    collectDotProcess( entities, tilemap->tm_dots, &g_NumDots, &gScore, gRenderer );
    tempMirrorPlayerCollectDotProcess( entities, tilemap->tm_dots, &gScore ) ;

    updateScoreTexture( &gScore, gRenderer );

    
    // CHECK STATE TRANSITIONS FOR GHOSTS
    for(int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
        if( entities->ghostStates[ eid ] == NULL ) {
            continue;
        }
        switch( *entities->ghostStates[ eid ] ) {
            case STATE_VULNERABLE :
                // return all ghosts that are still vulnerable to normal
                // when the timer runs out
                if (gGhostVulnerableTimer <= 0.0f ) {
                    g_NumGhostsEaten = 0;
                    *entities->ghostStates[ eid ] = STATE_NORMAL;
                    normal_enter( entities, eid );
                    // for( int i = 1; i < 5; ++i ) {
                    //     if( *entities->ghostStates[ eid ] == STATE_VULNERABLE ) {
                    //         *entities->ghostStates[ eid ] = STATE_NORMAL;
                    //         normal_enter( &entities-> eid );
                    //     }
                    // }
                }
                SDL_bool stateChanged = SDL_FALSE;
                // collide with players
                EntityId playerId;
                for( int i = 0; i < gNumPlayers; ++i ) {
                    playerId = gPlayerIds[ i ];

                    // eat ghost if pacman touches
                    if ( entities->actors[ playerId ]->current_tile.x == entities->actors[ eid ]->current_tile.x 
                    && entities->actors[ playerId ]->current_tile.y == entities->actors[ eid ]->current_tile.y ) {
                        Mix_PlayChannel( -1, g_PacChompSound, 0 );
                        Mix_PlayChannel( -1, g_GhostEatenSounds[ g_NumGhostsEaten ], 0);
                        gScore.score_number+=g_GhostPointValues[ g_NumGhostsEaten ];
                        g_NumGhostsEaten++;
                        *entities->ghostStates[ eid ] = STATE_GO_TO_PEN;
                        uint8_t texture_atlas_id = 4;
                        entities->animatedSprites[ eid ]->texture_atlas_id = texture_atlas_id;
                        entities->actors[ eid ]->next_tile = entities->actors[ eid ]->current_tile;
                        entities->actors[ eid ]->target_tile = ghost_pen_tile;
                        entities->actors[ eid ]->speed_multp = 1.6f;

                        // show message
                        for( int i = 0; i < g_NumTimedMessages; i++ ) {
                            if( g_TimedMessages[ i ].remainingTime <= 0.0f ) {
                                g_TimedMessages[ i ].remainingTime = 0.85f;
                                g_TimedMessages[ i ].world_position = tile_grid_point_to_world_point( entities->actors[ playerId ]->current_tile );
                                snprintf( g_TimedMessages[ i ].message, 8, "%d", g_GhostPointValues[ g_NumGhostsEaten - 1 ] );
                                g_TimedMessages[ i ].color = white;
                                SDL_Surface *msgSurface = TTF_RenderText_Solid( g_TimedMessages[ i ].font,  g_TimedMessages[ i ].message, g_TimedMessages[ i ].color );
                                g_TimedMessages[ i ].messageTexture = SDL_CreateTextureFromSurface( gRenderer, msgSurface );
                                g_TimedMessages[ i ].render_dest_rect.x = world_point_to_screen_point(g_TimedMessages[ i ].world_position, tilemap->tm_screen_position).x;
                                g_TimedMessages[ i ].render_dest_rect.y = world_point_to_screen_point(g_TimedMessages[ i ].world_position, tilemap->tm_screen_position).y;
                                g_TimedMessages[ i ].render_dest_rect.w = msgSurface->w;
                                g_TimedMessages[ i ].render_dest_rect.h = msgSurface->h;
                                SDL_FreeSurface( msgSurface );
                                g_TimedMessages[ i ].l = lerpInit( g_TimedMessages[ i ].world_position.y - TILE_SIZE*1.25, g_TimedMessages[ i ].world_position.y, 0.33f );

                                break;

                            }
                        }
                        stopGhostsForDuration( entities, 0.2f);
                        stopPlayersForDuration(entities, gPlayerIds, gNumPlayers, 0.2f );
                        //stopEntityForDuration( entities, playerId, 0.2f);
                        stateChanged = SDL_TRUE;
                        break;
                    }
                    
                }

                // don't run again cause in different state
                if( stateChanged ) {
                    break;
                }
                // collide with ghost mirror player
                //collide with mirror temp players
                for( int i = 0; i < g_NumEntities; i++ ) {
                    
                    if( entities->mirrorEntityRefs[ i ] == NULL ) {
                        continue;
                    }
                    if( *entities->activeTimers[i] <= 0.0f ) {
                        continue;
                    }

                    EntityId mirroredTempPlayerId = i;
                        // eat ghost if pacman touches
                    if ( points_equal( entities->actors[ mirroredTempPlayerId ]->current_tile, entities->actors[ eid ]->current_tile ) ) {
                        Mix_PlayChannel( -1, g_PacChompSound, 0 );
                        Mix_PlayChannel( -1, g_GhostEatenSounds[ g_NumGhostsEaten ], 0);
                        gScore.score_number+=g_GhostPointValues[ g_NumGhostsEaten ];
                        g_NumGhostsEaten++;
                        *entities->ghostStates[ eid ] = STATE_GO_TO_PEN;
                        uint8_t texture_atlas_id = 4;
                        entities->animatedSprites[ eid ]->texture_atlas_id = texture_atlas_id;
                        entities->actors[ eid ]->next_tile = entities->actors[ eid ]->current_tile;
                        entities->actors[ eid ]->target_tile = ghost_pen_tile;
                        entities->actors[ eid ]->speed_multp = 1.6f;

                        // show message
                        for( int i = 0; i < g_NumTimedMessages; i++ ) {
                            if( g_TimedMessages[ i ].remainingTime <= 0.0f ) {
                                g_TimedMessages[ i ].remainingTime = 0.85f;
                                g_TimedMessages[ i ].world_position = tile_grid_point_to_world_point( entities->actors[ mirroredTempPlayerId ]->current_tile );
                                snprintf( g_TimedMessages[ i ].message, 8, "%d", g_GhostPointValues[ g_NumGhostsEaten - 1 ] );
                                g_TimedMessages[ i ].color = white;
                                SDL_Surface *msgSurface = TTF_RenderText_Solid( g_TimedMessages[ i ].font,  g_TimedMessages[ i ].message, g_TimedMessages[ i ].color );
                                g_TimedMessages[ i ].messageTexture = SDL_CreateTextureFromSurface( gRenderer, msgSurface );
                                g_TimedMessages[ i ].render_dest_rect.x = world_point_to_screen_point(g_TimedMessages[ i ].world_position, tilemap->tm_screen_position).x;
                                g_TimedMessages[ i ].render_dest_rect.y = world_point_to_screen_point(g_TimedMessages[ i ].world_position, tilemap->tm_screen_position).y;
                                g_TimedMessages[ i ].render_dest_rect.w = msgSurface->w;
                                g_TimedMessages[ i ].render_dest_rect.h = msgSurface->h;
                                SDL_FreeSurface( msgSurface );
                                g_TimedMessages[ i ].l = lerpInit( g_TimedMessages[ i ].world_position.y - TILE_SIZE*1.25, g_TimedMessages[ i ].world_position.y, 0.33f );

                                break;

                            }
                        }
                        stopGhostsForDuration( entities, 0.2f);
                        stopPlayersForDuration(entities, gPlayerIds, gNumPlayers, 0.2f );
                    }
                    break;
                }
                
                
                
            case STATE_GO_TO_PEN :
                // ghost is in pen
                
                if( points_equal(entities->actors[ eid ]->current_tile, ghost_pen_tile ) && entities->actors[ eid ]->world_center_point.y >= tile_grid_point_to_world_point(levelConfig->ghostPenTile).y/2) {

                    *entities->ghostStates[ eid ] = STATE_LEAVE_PEN;
                    leave_pen_enter( entities, eid );
                }
                break;

            
            case STATE_NORMAL :
                break;
            case STATE_LEAVE_PEN:
                if( points_equal( entities->actors[ eid ]->current_tile, entities->actors[ eid ]->target_tile ) ) {
                    *entities->ghostStates[ eid ] = STATE_NORMAL;
                    normal_enter( entities, eid );
                }
                
                break;
        }
    }

    // pacman eats power pellet
    // TODO: FIX

    // process power pellet pickups
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->pickupTypes[ eid ] == NULL || *entities->pickupTypes[ eid] != POWER_PELLET_PICKUP  || entities->numDots[eid] != NULL) { // its a power pellet pickup. Not a temporary one
            continue;
        }

        // collide with players
        for( int i = 0; i < gNumPlayers; i++ ) {
            EntityId playerId = gPlayerIds[ i ];
            // player eats power pellet
            if( points_equal( entities->actors[ eid ]->current_tile, entities->actors[ playerId ]->current_tile ) ) {
                gScore.score_number += *entities->scores[eid];
                g_NumGhostsEaten = 0;
                // move it outside of the world area for now.
                // TODO: deactivate this somehow
                entities->actors[ eid ]->current_tile.x = -1;
                entities->actors[ eid ]->current_tile.y = -1;
                entities->actors[ eid ]->world_position.x = -100;
                entities->actors[ eid ]->world_position.y = -100;
                entities->actors[ eid ]->world_center_point.x = -100;
                entities->actors[ eid ]->world_center_point.y = -100;
                g_NumDots--;

                // make ghosts all vulnerable state
                for( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
                    if( entities->ghostStates[ eid ] == NULL ) {
                        continue;
                    }
                    if ( *entities->ghostStates[ eid ] != STATE_GO_TO_PEN && *entities->ghostStates[ eid ] != STATE_LEAVE_PEN ) {

                        *entities->ghostStates[ eid ] = STATE_VULNERABLE;
                        vulnerable_enter( entities, eid );
                    }
                    
                }   
                gGhostVulnerableTimer = 10.0f;   
            }
        }
        
        //collide with mirror temp players
        for( int i = 0; i < g_NumEntities; i++ ) {
            
            if( entities->mirrorEntityRefs[ i ] == NULL ) {
                continue;
            }
            if( *entities->activeTimers[i] <= 0.0f ) {
                continue;
            }

            EntityId mirroredTempPlayerId = i;
            // mirrored player eats power pellet
            if( points_equal( entities->actors[ eid ]->current_tile, entities->actors[ mirroredTempPlayerId ]->current_tile ) ) {
                gScore.score_number += *entities->scores[eid];
                g_NumGhostsEaten = 0;
                // move it outside of the world area for now.
                // TODO: deactivate this somehow
                entities->actors[ eid ]->current_tile.x = -1;
                entities->actors[ eid ]->current_tile.y = -1;
                entities->actors[ eid ]->world_position.x = -100;
                entities->actors[ eid ]->world_position.y = -100;
                entities->actors[ eid ]->world_center_point.x = -100;
                entities->actors[ eid ]->world_center_point.y = -100;
                g_NumDots--;

                // make ghosts all vulnerable state
                for( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
                    if( entities->ghostStates[ eid ] == NULL ) {
                        continue;
                    }
                    if ( *entities->ghostStates[ eid ] != STATE_GO_TO_PEN && *entities->ghostStates[ eid ] != STATE_LEAVE_PEN ) {

                        *entities->ghostStates[ eid ] = STATE_VULNERABLE;
                        vulnerable_enter( entities, eid );
                    }
                    
                }   
                gGhostVulnerableTimer = 10.0f;   
            }

        }

    }

    // process temp power pellets
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->pickupTypes[ eid ] == NULL || *entities->pickupTypes[ eid] != POWER_PELLET_PICKUP  || entities->numDots[eid] == NULL) { // its a temporary power pellet pickup. 
            continue;
        }
        int numDotsEaten = g_StartingNumDots - g_NumDots;
        if( *entities->numDots[eid] >= numDotsEaten ) { // not ready
            continue;
        }

        if( *entities->activeTimers[eid] <= 0.0f) { // its' dead
            continue;
        }

        *entities->activeTimers[eid] -= deltaTime;

        // collide with players
        for( int i = 0; i < gNumPlayers; i++ ) {
            EntityId playerId = gPlayerIds[ i ];
            // player eats power pellet
            if( points_equal( entities->actors[ eid ]->current_tile, entities->actors[ playerId ]->current_tile ) ) {
                gScore.score_number += *entities->scores[ eid ];
                g_NumGhostsEaten = 0;
                // move it outside of the world area for now.
                // TODO: deactivate this somehow
                entities->actors[ eid ]->current_tile.x = -1;
                entities->actors[ eid ]->current_tile.y = -1;
                entities->actors[ eid ]->world_position.x = -100;
                entities->actors[ eid ]->world_position.y = -100;
                entities->actors[ eid ]->world_center_point.x = -100;
                entities->actors[ eid ]->world_center_point.y = -100;
                //g_NumDots--;

                // make ghosts all vulnerable state
                for( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
                    if( entities->ghostStates[ eid ] == NULL ) {
                        continue;
                    }
                    if ( *entities->ghostStates[ eid ] != STATE_GO_TO_PEN && *entities->ghostStates[ eid ] != STATE_LEAVE_PEN ) {

                        *entities->ghostStates[ eid ] = STATE_VULNERABLE;
                        vulnerable_enter( entities, eid );
                    }
                    
                }   
                gGhostVulnerableTimer = 10.0f;   
            }
        }
        
        //collide with mirror temp players
        for( int i = 0; i < g_NumEntities; i++ ) {
            
            if( entities->mirrorEntityRefs[ i ] == NULL ) {
                continue;
            }
            if( *entities->activeTimers[i] <= 0.0f ) {
                continue;
            }

            EntityId mirroredTempPlayerId = i;
            // mirrored player eats power pellet
            if( points_equal( entities->actors[ eid ]->current_tile, entities->actors[ mirroredTempPlayerId ]->current_tile ) ) {
                gScore.score_number += *entities->scores[eid];
                g_NumGhostsEaten = 0;
                // move it outside of the world area for now.
                // TODO: deactivate this somehow
                entities->actors[ eid ]->current_tile.x = -1;
                entities->actors[ eid ]->current_tile.y = -1;
                entities->actors[ eid ]->world_position.x = -100;
                entities->actors[ eid ]->world_position.y = -100;
                entities->actors[ eid ]->world_center_point.x = -100;
                entities->actors[ eid ]->world_center_point.y = -100;
                g_NumDots--;

                // make ghosts all vulnerable state
                for( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
                    if( entities->ghostStates[ eid ] == NULL ) {
                        continue;
                    }
                    if ( *entities->ghostStates[ eid ] != STATE_GO_TO_PEN && *entities->ghostStates[ eid ] != STATE_LEAVE_PEN ) {

                        *entities->ghostStates[ eid ] = STATE_VULNERABLE;
                        vulnerable_enter( entities, eid );
                    }
                    
                }   
                gGhostVulnerableTimer = 10.0f;   
            }

        }

    }

    processTemporaryPickup( entities, gPlayerIds, gNumPlayers, &gScore, tilemap, g_NumDots, deltaTime ) ;



    /*******************
     * PROCESS STATES
     * ******************/
    //states_machine_process( actors, ghost_states, &tilemap );

    //if no ghosts normal, then stop playing ghost sound
    int any_ghosts_nomral = 0;
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->ghostStates[ eid ] == STATE_NORMAL ) {
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
        if( entities->ghostStates[ eid ] == NULL ) {
            continue;
        }
        if( *entities->ghostStates[ eid ] == STATE_VULNERABLE ) {
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
        gGhostVulnerableTimer -= deltaTime;
    }

    ghostsProcess( entities, gPlayerIds, gNumPlayers, tilemap,  deltaTime,levelConfig);

    /********************
     * MOVE GHOSTS
     * ********************/
    for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
        if( entities->targetingBehaviors[ i ] == NULL ) {
            continue;
        }
        // don't move if stopped
        if( *entities->stopTimers[i] > 0.0f ) {
            continue;
        }
        ghost_move( entities->actors, i, tilemap, deltaTime );
    }
    
    /*********************
     * GHOST MODE PERIOD
     * *******************/
    gGhostModeTimer += deltaTime;

    if( g_current_scatter_chase_period < NUM_SCATTER_CHASE_PERIODS && gGhostModeTimer > g_scatter_chase_period_seconds[ g_current_scatter_chase_period ] ) {
        if( g_current_ghost_mode == MODE_CHASE ) {
            g_current_ghost_mode = MODE_SCATTER;
            for( int i = 0; i < MAX_NUM_ENTITIES; ++i ) {
                if( entities->ghostStates[ i ] != NULL && entities->ghostStates[ i ] == STATE_NORMAL ) {
                    entities->actors[ i ]->direction = opposite_directions[ entities->actors[ i ]->direction ];
                    entities->actors[ i ]->next_tile = entities->actors[ i ]->current_tile; // need to do this so that the ghost will want to set a new next tile
                }
                
            }
        }
        else {
            g_current_ghost_mode = MODE_CHASE;
            for( int i = 0; i < MAX_NUM_ENTITIES; ++i ) {
                if( entities->ghostStates[ i ] != NULL && *entities->ghostStates[ i ] == STATE_NORMAL ) {
                    entities->actors[ i ]->direction = opposite_directions[ entities->actors[ i ]->direction ];
                    entities->actors[ i ]->next_tile = entities->actors[ i ]->current_tile; // need to do this so that the ghost will want to set a new next tile
                }

            }
        }
        gGhostModeTimer = 0.0f;
        g_current_scatter_chase_period++;
    }

    
    /**********
     * UPDATE DOTS ANIMATION
     * **********/
    int top_bound = DOT_PADDING;
    int bottom_bound = TILE_SIZE - DOT_PADDING;
    for( int r = 0; r < TILE_ROWS; ++r ) {
        for( int c = 0; c < TILE_COLS; ++c ) {
            
            tilemap->tm_dot_particles[ r ][ c ].position.y += tilemap->tm_dot_particles[ r ][ c ].velocity.y * DOT_SPEED * deltaTime ;

            if ( tilemap->tm_dot_particles[ r ][ c ].position.y < top_bound ) {
                tilemap->tm_dot_particles[ r ][ c ].position.y = top_bound;
                tilemap->tm_dot_particles[ r ][ c ].velocity.y = 1;
            }
            if ( tilemap->tm_dot_particles[ r ][ c ].position.y > bottom_bound) {
                tilemap->tm_dot_particles[ r ][ c ].position.y = bottom_bound;
                
                tilemap->tm_dot_particles[ r ][ c ].velocity.y = -1;
            }
        }
    }

    /**********
     * RENDER
     * **********/
    set_render_clip_values_based_on_positions_and_animation( entities, tilemap->tm_screen_position );

    

    SDL_SetRenderDrawColor( gRenderer, 0,0,0,255);
    SDL_RenderClear( gRenderer );    

    tm_render_with_screen_position_offset( gRenderer, tilemap );

    renderDataForAnimatedSpriteProcess( gRenderer, entities );

    

    SDL_Rect black_bar = {0,0, 1920, TILE_SIZE * 2};
    SDL_SetRenderDrawColor( gRenderer, 0,0,0,255 );
    SDL_RenderFillRect( gRenderer, &black_bar);

    SDL_RenderCopy( gRenderer, gScore.score_texture, NULL, &gScore.score_render_dst_rect);
    //SDL_RenderCopy(gRenderer, gCooldownTexture, NULL, &gCooldownRect);

    renderDashStockRects( ) ;


    /****
     * ANY MESSAGES
     **/
    for(int i = 0; i < g_NumTimedMessages; i++ ) {
        if( g_TimedMessages[ i ].remainingTime > 0.0f ) {
            
            if( g_TimedMessages[ i ].l.remainingTime > 0.0f ) {
                
                
                blinkProcess( &g_ScoreBlinks[ i ], deltaTime );
                interpolate( &g_TimedMessages[ i ].l, deltaTime );
                g_TimedMessages[ i ].world_position.y = g_TimedMessages[ i ].l.value;
                g_TimedMessages[ i ].render_dest_rect.y = world_point_to_screen_point( g_TimedMessages[ i ].world_position, tilemap->tm_screen_position ).y;
            }
            else {
                g_ScoreBlinks[ i ].current_value_idx = 1;
            }
            

            SDL_SetTextureAlphaMod( g_TimedMessages[ i ].messageTexture, g_ScoreBlinks[ i ].values[ g_ScoreBlinks[ i ].current_value_idx ] );
            SDL_RenderCopy( gRenderer, g_TimedMessages[ i ].messageTexture, NULL, &g_TimedMessages[ i ].render_dest_rect);
            g_TimedMessages[ i ].remainingTime -= deltaTime;
            if( g_TimedMessages[ i ].remainingTime <= 0.0f ) {
                SDL_DestroyTexture( g_TimedMessages[ i ].messageTexture );
                g_TimedMessages[ i ].messageTexture = NULL;
            }
        }
    }

    updateDashStockRects( entities, gPlayerIds, gNumPlayers ) ;



    // DEBUG
    if ( g_show_debug_info ) {
        //grid
        SDL_SetRenderDrawColor( gRenderer, 50,50,50,255);
        for ( int y = tilemap->tm_screen_position.y; y < SCREEN_HEIGHT; y+= TILE_SIZE ) {
            SDL_RenderDrawLine( gRenderer, 0, y, SCREEN_WIDTH, y);
        }
        for ( int x = tilemap->tm_screen_position.x; x < SCREEN_WIDTH; x+= TILE_SIZE) {
            SDL_RenderDrawLine( gRenderer, x, 0, x, SCREEN_HEIGHT );
        }

        SDL_SetRenderDrawColor( gRenderer, 255, 80, 50, 30 );
        for( int row = 0; row < TILE_ROWS; row++ ) {
            for( int col = 0; col < TILE_COLS; col ++ ) {

                if( tilemap->tm_walls[ row ][ col ] == 'x' ) {

                    SDL_Rect wall_rect = {
                        tilemap->tm_screen_position.x + ( TILE_SIZE * col ),
                        tilemap->tm_screen_position.y + ( TILE_SIZE * row ),
                        TILE_SIZE,
                        TILE_SIZE
                    };
                    
                    SDL_RenderFillRect( gRenderer, &wall_rect );

                } 
            }
        }
        
        SDL_SetRenderDrawColor( gRenderer,  pac_color.r, pac_color.g, pac_color.b, 150 );

        SDL_Rect tileRect;
        SDL_Point screenPoint;
        // current tile
        for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
            if( entities->actors[ i ] == NULL ) {
                continue;
            }
            screenPoint = tile_grid_point_to_screen_point( entities->actors[ i ]->current_tile, tilemap->tm_screen_position );
            tileRect.x = screenPoint.x;    
            tileRect.y = screenPoint.y;    
            tileRect.w = TILE_SIZE;    
            tileRect.h = TILE_SIZE;    
            SDL_RenderFillRect( gRenderer, &tileRect);

            screenPoint = tile_grid_point_to_screen_point( entities->actors[ i ]->next_tile, tilemap->tm_screen_position );
            tileRect.x = screenPoint.x;    
            tileRect.y = screenPoint.y; 
            SDL_RenderFillRect( gRenderer, &tileRect);

        }
        // target tile
        for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
            if( entities->targetingBehaviors[ i ] == NULL ) {
                continue;
            }

            switch( *entities->targetingBehaviors[ i ] ) {
                case SHADOW_BEHAVIOR:
                    SDL_SetRenderDrawColor( gRenderer, 255,0,0,255);
                    break;
                case AMBUSH_BEHAVIOR:
                    SDL_SetRenderDrawColor( gRenderer, 255,0,255,255);
                    break;
                case MOODY_BEHAVIOR:
                    SDL_SetRenderDrawColor( gRenderer, 0,255,255,255);
                    break;
                case POKEY_BEHAVIOR:
                    SDL_SetRenderDrawColor( gRenderer, 150,150,0,255);
                    break;
                default:
                    SDL_SetRenderDrawColor( gRenderer, 255,255,255,255);
                    break;
            }
            screenPoint = tile_grid_point_to_screen_point( entities->actors[ i ]->target_tile, tilemap->tm_screen_position );
            tileRect.x = screenPoint.x;
            tileRect.y = screenPoint.y;
            SDL_RenderDrawRect( gRenderer, &tileRect );
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
    SDL_RenderPresent( gRenderer );
}
void gamePausedProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime ) ;
inline void gamePausedProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime ) {
    while (SDL_PollEvent( event ) != 0 ) {
        //int gameControllerState = SDL_GameControllerEventState( SDL_QUERY );
        
        if( event->type == SDL_QUIT ) {
            gProgramState = EXIT_STATE;
            break;
        }
        if( event->type == SDL_KEYUP ) {
            if( event->key.keysym.sym == SDLK_RETURN ) {
                gGamePlayingState = GAME_PLAYING;
            }
            if( event->key.keysym.sym == SDLK_F11 ) {
                Uint32 windowFlags = SDL_GetWindowFlags( gWindow );
                SDL_SetWindowFullscreen( gWindow, windowFlags ^= SDL_WINDOW_FULLSCREEN );
            }
        }
        if( event->type == SDL_CONTROLLERBUTTONUP ) {
            for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
                if( entities->gameControllers[ i ] == NULL || entities->inputMasks[ i ] == NULL ) {
                    continue;
                }
                if( event->cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(entities->gameControllers[ i ]))) {
                    if( event->cbutton.button == SDL_CONTROLLER_BUTTON_START ) {
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

    SDL_RenderCopy( gRenderer, gPauseTextTexture, NULL, &gPauseTextDestRect);

    SDL_RenderPresent( gRenderer );
}

void gameLevelStartProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime );
inline void gameLevelStartProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime ) {
    
    SDL_SetRenderDrawColor( gRenderer, 20,20,20,255 );
    SDL_RenderClear( gRenderer );
    SDL_RenderCopy( gRenderer, gLevelStartTextTexture, NULL, &gLevelStartTextRect );
    SDL_RenderPresent( gRenderer );

    gLevelStartTimer += deltaTime;
    if( gLevelStartTimer >= gLevelStartDuration ) {
        gGamePlayingState = GAME_PLAYING;
        gLevelStartTimer = 0.0f;
        return;
    }
}

void gameLevelEndProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime ) ;
inline void gameLevelEndProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime ) {
    
    SDL_SetRenderDrawColor( gRenderer, 20,20,20,255 );
    SDL_RenderClear( gRenderer );
    SDL_RenderCopy( gRenderer, gLevelEndTextTexture, NULL, &gLevelEndTextRect );
    SDL_RenderPresent( gRenderer );

    gLevelEndTimer += deltaTime;
    if( gLevelEndTimer >= gLevelEndDuration ) {
        gGamePlayingState = LEVEL_START;
        gLevelEndTimer = 0.0f;
        return;
    }
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
    int numPlayers = 2;
    initializePlayersFromFiles( &entities, &levelConfig, numPlayers );
    
    // init ghosts
    initializeGhostsFromFile( &entities, &levelConfig, "res/ghost_animated_sprites");


    initializeDashStockRects( numPlayers );

    // load everything for entity data from config
    //level_advance( &levelConfig, &tilemap, gRenderer, &entities );


    
    for( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
        if( entities.inputMasks[ eid ] != NULL ) {
            gPlayerIds[ gNumPlayers ] = eid;
            gNumPlayers++;
        }
    }

    // add controllers to players
    uint32_t playerIdx = 0;

    for( int i = playerIdx; i < g_NumGamepads; i++ ) {

        if( i > gNumPlayers ) {
            break;
        }
        entities.gameControllers[ gPlayerIds[ i ] ] = g_GameControllers[ i ];
        playerIdx++;
    }

    int32_t numPlayersLeftToAssignControls = gNumPlayers - g_NumGamepads;
    if(numPlayersLeftToAssignControls > 0 ) {
        for( int i = playerIdx; i < gNumPlayers; i++ ) {
            EntityId playerId = gPlayerIds[ i ];
            entities.keybinds[ playerId ] = gkeyBindings;
        }
    }

    // pre-initialize pickup entities ( non power pellet )
    for( int i = 0; i < MAX_PICKUPS_PER_LEVEL; i++ ) { // start with 4
        createInitialTemporaryPickup( &entities, &levelConfig );
    }
    // AnimatedSprite *fruitAnimatedSprite = init_animation(9,1,1,1);
    // createFruit( &entities, &levelConfig, fruitAnimatedSprite, 60 );
    

    SDL_Point ghost_pen_position = tile_grid_point_to_world_point( ghost_pen_tile ); 
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
    SDL_Surface *mainMenuTextSurface = TTF_RenderText_Solid( gFont, gMainMenuText, white );
    gMainMenuTextTexture = SDL_CreateTextureFromSurface( gRenderer, mainMenuTextSurface );
    gMainMenuTextDestRect.x = SCREEN_WIDTH / 2 - mainMenuTextSurface->w/2 ;
    gMainMenuTextDestRect.y =  SCREEN_HEIGHT/2 - mainMenuTextSurface->h/2;
    gMainMenuTextDestRect.w =  mainMenuTextSurface->w;
    gMainMenuTextDestRect.h =  mainMenuTextSurface->h;
    SDL_FreeSurface( mainMenuTextSurface );
    mainMenuTextSurface = NULL;

    SDL_Surface *pauseTextSurface = TTF_RenderText_Solid( gFont, gPauseText, pac_color );
    gPauseTextTexture = SDL_CreateTextureFromSurface( gRenderer, pauseTextSurface);
    gPauseTextDestRect.x = SCREEN_WIDTH / 2 - pauseTextSurface->w/2; 
    gPauseTextDestRect.y = SCREEN_HEIGHT/2 - pauseTextSurface->h/2;
    gPauseTextDestRect.w = pauseTextSurface->w;
    gPauseTextDestRect.h = pauseTextSurface->h;
    SDL_FreeSurface( pauseTextSurface );
    pauseTextSurface = NULL;

    SDL_Surface *levelStartTextSurface = TTF_RenderText_Solid( gFont, gLevelStartText, white );
    gLevelStartTextTexture = SDL_CreateTextureFromSurface( gRenderer, levelStartTextSurface);
    gLevelStartTextRect.x = SCREEN_WIDTH / 2 -levelStartTextSurface->w/2; 
    gLevelStartTextRect.y = SCREEN_HEIGHT/2 - levelStartTextSurface->h/2;
    gLevelStartTextRect.w = levelStartTextSurface->w;
    gLevelStartTextRect.h = levelStartTextSurface->h;
    SDL_FreeSurface( levelStartTextSurface );
    levelStartTextSurface = NULL;

    SDL_Surface *levelEndTextSurface = TTF_RenderText_Solid( gFont, gLevelEndText, white );
    gLevelEndTextTexture = SDL_CreateTextureFromSurface( gRenderer, levelEndTextSurface);
    gLevelEndTextRect.x = SCREEN_WIDTH / 2 -levelEndTextSurface->w/2; 
    gLevelEndTextRect.y = SCREEN_HEIGHT/2 - levelEndTextSurface->h/2;
    gLevelEndTextRect.w = levelEndTextSurface->w;
    gLevelEndTextRect.h = levelEndTextSurface->h;
    SDL_FreeSurface( levelEndTextSurface );
    levelEndTextSurface = NULL;



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
            case MAIN_MENU_PROGRAM_STATE:
                mainMenuProcess( &levelConfig, &entities, &tilemap, &event, &startMenuBlink, deltaTime );
                break;
            /**************
             * ************
             * *** GAME PLAYING
             * *****************/
            case GAME_PLAYING_PROGRAM_STATE:

                switch(gGamePlayingState) {
                    /**************
                     * ************
                     * *** GAME PLAYING
                     * *****************/
                    case GAME_PLAYING:
                        gamePlayingProcess( &entities, &tilemap, &event, &levelConfig, deltaTime );
                        break;
                    /**************
                     * ************
                     * *** GAME PAUSED
                     * *****************/
                    case GAME_PAUSED:
                        gamePausedProcess( &entities, &event, &levelConfig, deltaTime );
                        break;
                    case LEVEL_START:
                        gameLevelStartProcess( &entities, &event, &levelConfig, deltaTime );
                        break;
                    case LEVEL_END:
                        gameLevelEndProcess( &entities, &event, &levelConfig, deltaTime );
                        break;
                }
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
        
    }
    for( int i = 0; i < MAX_TEXTURE_ATLASES; i++ ) {
        free( g_texture_atlases[ i ].sprite_clips );
        g_texture_atlases[ i ].sprite_clips = NULL;

    }

    SDL_Quit();
    
}
