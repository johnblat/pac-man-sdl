#include "entity.h"
#include "tiles.h"
#include "levelConfig.h"
#include "sounds.h"
#include "ghostStates.h"
#include "states.h"
#include "programState.h"
#include "globalData.h"
#include "menuState.h"
#include "movement.h"
#include "UI.h"
#include "targeting.h"
#include "animation.h"
#include "render.h"
#include "comparisons.h"
#include "resources.h"
#include "renderProcessing.h"
#include "gamePlayingState.h"

#include <SDL2/SDL.h>

// PAUSE
char *gPauseText = "PAUSED";
SDL_Texture *gPauseTextTexture = NULL;
SDL_Rect gPauseTextDestRect;

// COOLDOWN
SDL_Texture *gCooldownTexture = NULL;
SDL_Rect gCooldownRect;

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




void initGamePlayingStuff( ) {
    gPauseTextTexture = createTextTexture(&gPauseTextDestRect, gPauseText, pac_color, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    gLevelStartTextTexture = createTextTexture(&gLevelStartTextRect, gLevelStartText, white, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
    gLevelEndTextTexture = createTextTexture(&gLevelEndTextRect, gLevelEndText, white, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 );
}

void gamePlayingStateProcess( SDL_Event *event, Entities *entities, TileMap *tilemap, LevelConfig *levelConfig, float deltaTime ){
    switch(gGamePlayingState) {
        case GAME_PLAYING:
            gamePlayingProcess( entities, tilemap, event, levelConfig, deltaTime );
            break;
        case GAME_PAUSED:
            gamePausedProcess( entities, event, levelConfig, deltaTime );
            break;
        case LEVEL_START:
            gameLevelStartProcess( entities, event, levelConfig, deltaTime );
            break;
        case LEVEL_END:
            gameLevelEndProcess( entities, event, levelConfig, deltaTime );
            break;
    }
}

SDL_bool isGameOver( Entities *entities ) {

    // if any player isActive, then keep playing
    EntityId pid = 0;
    for( int i = 0; i < gNumPlayers; i++ ) {
        pid = gPlayerIds[ i ];
        if( *entities->isActive[pid] ) {
            return SDL_FALSE;
        }
    }
    
    return SDL_TRUE;
}



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

    // reactivate all players
    EntityId pid = 0;
    for( int i = 0; i < gNumPlayers; i++ ) {
        pid = gPlayerIds[i];
        *entities->isActive[pid] = SDL_TRUE;
    }

    return SDL_FALSE;

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
                if( entities->gameControllerIds[ i ] == NULL || entities->inputMasks[ i ] == NULL ) {
                    continue;
                }
                GameControllerId gcid = *entities->gameControllerIds[i];
                SDL_GameController *gameController = g_GameControllers[ gcid ];
                if( event->cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick( gameController ))) {
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
                if( entities->gameControllerIds[ i ] == NULL || entities->inputMasks[ i ] == NULL ) {
                    continue;
                }
                GameControllerId gcid = *entities->gameControllerIds[i];
                SDL_GameController *gameController = g_GameControllers[ gcid ];
                if( event->cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick( gameController ))) {
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
    if(gQuit) {
        return;
    } 

    // GAME OVER?
    if( isGameOver( entities ) ) {
        mainMenuProgramStateEnter();
        return;
    }

    // NEXT LEVEL?
    if( g_NumDots <= 0 ) {
        
        SDL_bool gameCleared = level_advance( levelConfig, tilemap, gRenderer, entities );
        if( gameCleared ) {
            mainMenuProgramStateEnter();
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
            gProgramState = MENU_PROGRAM_STATE;
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
        if( entities->ghostStates[eid] != NULL && *entities->ghostStates[ eid ] == STATE_NORMAL ) {
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
                if( entities->ghostStates[ i ] != NULL && *entities->ghostStates[ i ] == STATE_NORMAL ) {
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
                if( entities->gameControllerIds[ i ] == NULL || entities->inputMasks[ i ] == NULL ) {
                    continue;
                }
                GameControllerId gcid = *entities->gameControllerIds[i];
                SDL_GameController *gameController = g_GameControllers[ gcid ];
                if( event->cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gameController))) {
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
