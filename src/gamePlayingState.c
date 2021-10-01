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
#include "commonEventHandling.h"

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

const char *gGameOverText = "GAME OVER!";
SDL_Texture *gGameOverTextTexture = NULL;
SDL_Rect gGameOverTextRect;

const char *gGameClearText = "GAME CLEARED!";
SDL_Texture *gGameClearTextTexture = NULL;
SDL_Rect gGameClearTextRect;

const float gLevelStartDuration = 1.5f; 
float gLevelStartTimer = 0.0f;
const float gLevelEndDuration = 1.5f;
float gLevelEndTimer = 0.0f;
const float gGameOverDuration = 3.0f;
float gGameOverTimer = 0.0f;
const float gGameClearDuration = 3.0f;
float gGameClearTimer = 0.0f;


Blink ghostVulnerableBlink;
Blink invincibleTimerAlmostUpBlink;

void initGamePlayingStuff( ) {
    gPauseTextTexture = createTextTexture(&gPauseTextDestRect, gPauseText, pac_color, gLargeFont, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    gLevelStartTextTexture = createTextTexture(&gLevelStartTextRect, gLevelStartText, white, gLargeFont, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
    gLevelEndTextTexture = createTextTexture(&gLevelEndTextRect, gLevelEndText, white, gLargeFont, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 );
    gGameOverTextTexture = createTextTexture(&gGameOverTextRect, gGameOverText, white, gLargeFont, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
    gGameClearTextTexture = createTextTexture(&gGameClearTextRect, gGameClearText, white, gLargeFont, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);

    ghostVulnerableBlink = blinkInit( 0.2, 1, 255 );
    invincibleTimerAlmostUpBlink = blinkInit(0.15,0,150);
    pickupBlink = blinkInit(0.1, 0, 255);
    //SDL_SetTextureColorMod(g_texture_atlases[3].texture, 0,0,0);
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
        case GAME_OVER:
            gameGameOverProcess( entities, event, levelConfig, deltaTime );
            break;
        case GAME_CLEAR:
            gameGameClearProcess( entities, event, levelConfig, deltaTime );
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

void renderDashStockRects( ) {
    for( int i = 0; i < gNumDashStockRects; i++ ) {
        SDL_SetRenderDrawColor( gRenderer, gDashStockRects[ i ].color.r, gDashStockRects[ i ].color.g, gDashStockRects[ i ].color.b, 255 );
        for( int numRect = 0; numRect < gDashStockRects[ i ].numRectsToShow; numRect++ ) {
            SDL_RenderFillRect( gRenderer, &gDashStockRects[i].rects[numRect]);
        }
    }
}

void updateDashStockRects( Entities *entities, EntityId *playerIds, unsigned int numPlayers ) {
    for(int i = 0; i < numPlayers; i++) {
        EntityId playerId = playerIds[ i ];
        if( entities->dashCooldownStocks[ playerId ] == NULL ) {
            continue;
        }
        gDashStockRects[ i ].numRectsToShow = entities->dashCooldownStocks[ playerId ]->currentNumStock;
    }
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

    tilemap->one_way_tile.x = levelConfig->ghostPenTile.x;
    tilemap->one_way_tile.y = levelConfig->ghostPenTile.y - 3;
    

    // pacman
    // reset players
    EntityId pid = 0;
    for( int i = 0; i <gNumPlayers; i++ ) {
        pid = gPlayerIds[i];
        SDL_Point startingTile = levelConfig->pacStartingTile;
        startingTile.x += i;
        actor_reset_data( entities, pid, startingTile);
        
        *entities->inputMasks[pid  ] = 0b0;
        *entities->chargeTimers[pid] = 0.0f;
        *entities->dashTimers[pid] = 0.0f;
        *entities->inputMasks[pid]= 0b0;
        *entities->directions[pid] = DIR_NONE;
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
        SDL_Point startingTile = ghost_pen_tile;

        if( *entities->targetingBehaviors[i] == AMBUSH_BEHAVIOR ) {
            startingTile.x = ghost_pen_tile.x;
            startingTile.y = ghost_pen_tile.y;
        }
        else if( *entities->targetingBehaviors[i] == MOODY_BEHAVIOR ) {
            startingTile.x = ghost_pen_tile.x - 1;
            startingTile.y = ghost_pen_tile.y;
        }
        else if( *entities->targetingBehaviors[i] == POKEY_BEHAVIOR ) {
            startingTile.x = ghost_pen_tile.x + 1;
            startingTile.y = ghost_pen_tile.y;
        }

        actor_reset_data( entities, i, startingTile );
        entities->animatedSprites[i ]->texture_atlas_id = entities->animatedSprites[i]->default_texture_atlas_id;
        *entities->ghostStates[ i ] = STATE_STAY_PEN;
        stayPenEnter( entities, levelConfig, i );
    }
    // adjust blinky position
    for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
        if( entities->targetingBehaviors[i] == NULL ) {
            continue;
        }
        if( *entities->targetingBehaviors[i] == SHADOW_BEHAVIOR ) {
            SDL_Point blinkyTile;
            blinkyTile.x = ghost_pen_tile.x;
            blinkyTile.y = ghost_pen_tile.y - 4; // above pen. outside of gate
            actor_reset_data( entities, i, blinkyTile );
            *entities->ghostStates[i] = STATE_NORMAL;
        }
    }

    // set ghost num dots accordingly to determine when leave pen
    for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
        if( entities->targetingBehaviors[i] == NULL ) {
            continue;
        }
        if( entities->numDots[i] == NULL ) {
            continue;
        }

        if( *entities->targetingBehaviors[i] == AMBUSH_BEHAVIOR ) {
            *entities->numDots[i] = levelConfig->numDotsUntilLeavePen[ 0 ];
        }
        else if( *entities->targetingBehaviors[i] == MOODY_BEHAVIOR ) {
            *entities->numDots[i] = levelConfig->numDotsUntilLeavePen[ 1 ];
        }
        else if( *entities->targetingBehaviors[i] == POKEY_BEHAVIOR ) {
            *entities->numDots[i] = levelConfig->numDotsUntilLeavePen[ 2 ];
        }
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

    // power pellets
    int ppIdx = 0; 
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->pickupTypes[ eid ] == NULL || *entities->pickupTypes[ eid ] != POWER_PELLET_PICKUP ) {
            continue;
        }
        if(entities->numDots[eid] != NULL ) {
            continue;
        }
        
        *entities->currentTiles[eid] = levelConfig->powerPelletTiles[ ppIdx ];
    entities->worldPositions[eid]->x = tile_grid_point_to_world_point(*entities->currentTiles[eid] ).x + (TILE_SIZE*0.5);
    entities->worldPositions[eid]->y = tile_grid_point_to_world_point(*entities->currentTiles[eid] ).y + (TILE_SIZE*0.5);

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

        entities->currentTiles[eid]->x = levelConfig->pacStartingTile.x;
        entities->currentTiles[eid]->y = levelConfig->pacStartingTile.y;
        *entities->nextTiles[eid] = *entities->currentTiles[eid];
        *entities->directions[eid] = DIR_UP;

        

        entities->worldPositions[eid]->x = tile_grid_point_to_world_point(*entities->currentTiles[eid] ).x + TILE_SIZE*0.5;
        entities->worldPositions[eid]->y = tile_grid_point_to_world_point(*entities->currentTiles[eid] ).y + TILE_SIZE*0.5;

        *entities->pickupTypes[ eid ] = levelConfig->pickupConfigs[ pickupIdx ].pickupType;
        *entities->activeTimers[ eid ] = levelConfig->pickupConfigs[ pickupIdx ].activeTime;
        *entities->numDots[ eid ] = levelConfig->pickupConfigs[ pickupIdx ].numDots;
        *entities->scores[ eid ] = levelConfig->pickupConfigs[ pickupIdx ].scoreReward;
        entities->animatedSprites[ eid ]->texture_atlas_id = levelConfig->pickupConfigs[ pickupIdx ].textureAtlasId;
        entities->animatedSprites[ eid ]->default_texture_atlas_id = entities->animatedSprites[ eid ]->texture_atlas_id;


        pickupIdx++; // will go to the next one if any

    }

    // reactivate all players
    //EntityId pid = 0;
    for( int i = 0; i < gNumPlayers; i++ ) {
        pid = gPlayerIds[i];
        *entities->isActive[pid] = SDL_TRUE;
        entities->renderDatas[pid]->alphaMod = 255; // need to be visible too
        entities->renderDatas[pid]->scale = 1.0f;
    }

    // reset all inputmasks
    for( int i = 0; i < MAX_NUM_ENTITIES; i++ ) {
        if(entities->inputMasks[i] == NULL ) {
            continue;
        }
        if( ( *entities->inputMasks[i] & g_INPUT_ACTION ) ) {
            *entities->inputMasks[ i ] ^= g_INPUT_ACTION;
        }
        if( ( *entities->inputMasks[ i ] & g_INPUT_DOWN ) ) {
            *entities->inputMasks[ i ] ^= g_INPUT_DOWN;
        }
        if( *entities->inputMasks[i] & g_INPUT_RIGHT )
        *entities->inputMasks[ i ] ^= g_INPUT_RIGHT;
        if( *entities->inputMasks[i] & g_INPUT_LEFT )
        *entities->inputMasks[ i ] ^= g_INPUT_LEFT;
            if( *entities->inputMasks[i] & g_INPUT_UP )
        *entities->inputMasks[ i ] ^= g_INPUT_UP;
    }

    return SDL_FALSE;

}

void gamePlayingProcess( Entities *entities, TileMap *tilemap, SDL_Event *event, LevelConfig *levelConfig, float deltaTime) ;
inline void gamePlayingProcess( Entities *entities, TileMap *tilemap, SDL_Event *event, LevelConfig *levelConfig, float deltaTime) {

    while (SDL_PollEvent( event ) != 0 ) {
        //int gameControllerState = SDL_GameControllerEventState( SDL_QUERY );
        SDL_bool shouldBreak = commonEventHandling(event);
        if( shouldBreak ) break;
 
        if ( event->type == SDL_KEYDOWN ) {
            
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
                        if( ( *entities->inputMasks[i] & g_INPUT_ACTION ) ) {
                            *entities->inputMasks[ i ] ^= g_INPUT_ACTION;
                        }

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
        gGamePlayingState = GAME_OVER;
        Mix_HaltMusic();
        //mainMenuProgramStateEnter(entities);
        
        return;
    }

    // NEXT LEVEL?
    if( g_NumDots <= 0 ) {
        
        SDL_bool gameCleared = level_advance( levelConfig, tilemap, gRenderer, entities );
        if( gameCleared ) {
            gGamePlayingState = GAME_CLEAR;
            //mainMenuProgramStateEnter(entities);
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
            mainMenuProgramStateEnter(entities);
            // Mix_HaltChannel( GHOST_SOUND_CHANNEL );
            // Mix_HaltChannel( GHOST_VULN_CHANNEL );
            // Mix_HaltMusic();
            // Mix_FreeMusic( g_Music );
            // g_Music = Mix_LoadMUS( gMenuMusicFilename );
            // Mix_PlayMusic( g_Music, -1 );
            // gProgramState = MENU_PROGRAM_STATE;
            // gCurrentLevel = 0;
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
    processDeathTimers( entities, levelConfig, deltaTime );
    processRespawnTimers( entities, deltaTime );

    if( gCurrentExtraLifeMilestoneIdx < gNumExtraLifeMilestones && gScore.score_number > gExtraLifeMilestones[ gCurrentExtraLifeMilestoneIdx ]) {
        gLivesRemaining++;
        gCurrentExtraLifeMilestoneIdx++;
        Mix_PlayChannel(EXTRA_LIFE_CHANNEL, g_ExtraLifeSound, 0 );
        updateLivesRemainingTexture( &gLivesRemainingUI );
    }

    SDL_DestroyTexture( gCooldownTexture );
    char coolDownNumberText[2];
    snprintf(coolDownNumberText, 2, "%d", entities->dashCooldownStocks[ 4 ]->currentNumStock );
    SDL_Surface *cooldownSurface = TTF_RenderText_Solid( gMedFont, coolDownNumberText, pac_color );
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
        if( tilemap->tm_dots[ entities->currentTiles[eid]->y ][ entities->currentTiles[eid]->x ] == 'x' ) {
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
                EntityId playerId = 0;
                for( int i = 0; i < gNumPlayers; ++i ) {
                    playerId = gPlayerIds[ i ];
                    
                    if( entities->isActive[ playerId ] != NULL && *entities->isActive[ playerId ] == SDL_FALSE ) {
                        continue;
                    }

                    // eat ghost if pacman touches
                    if ( entitiesIntersecting(entities, playerId, eid) ) {
                        Mix_PlayChannel( GHOST_EAT_CHANNEL , g_GhostEatSound, 0 );
                        Mix_PlayChannel( -1, g_GhostEatenSounds[ g_NumGhostsEaten ], 0);
                        gScore.score_number+=g_GhostPointValues[ g_NumGhostsEaten ];
                        g_NumGhostsEaten++;
                        *entities->ghostStates[ eid ] = STATE_GO_TO_PEN;
                        uint8_t texture_atlas_id = 4;
                        entities->animatedSprites[ eid ]->texture_atlas_id = texture_atlas_id;
                        *entities->nextTiles[eid] = *entities->currentTiles[eid];
                        *entities->targetTiles[eid] = levelConfig->ghostPenTile;
                        *entities->speedMultipliers[eid] = 1.6f;

                        // show message
                        for( int i = 0; i < g_NumTimedMessages; i++ ) {
                            if( g_TimedMessages[ i ].remainingTime <= 0.0f ) {
                                g_TimedMessages[ i ].remainingTime = 0.85f;
                                g_TimedMessages[ i ].world_position = tile_grid_point_to_world_point( *entities->currentTiles[playerId] );
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
                    if ( entitiesIntersecting(entities, mirroredTempPlayerId, eid) ) {
                        Mix_PlayChannel( -1, g_PacChompSound, 0 );
                        Mix_PlayChannel( -1, g_GhostEatenSounds[ g_NumGhostsEaten ], 0);
                        gScore.score_number+=g_GhostPointValues[ g_NumGhostsEaten ];
                        g_NumGhostsEaten++;
                        *entities->ghostStates[ eid ] = STATE_GO_TO_PEN;
                        uint8_t texture_atlas_id = 4;
                        entities->animatedSprites[ eid ]->texture_atlas_id = texture_atlas_id;
                        *entities->nextTiles[eid] = *entities->currentTiles[eid];
                        *entities->targetTiles[eid] = levelConfig->ghostPenTile;
                        *entities->speedMultipliers[eid] = 1.6f;

                        // show message
                        for( int i = 0; i < g_NumTimedMessages; i++ ) {
                            if( g_TimedMessages[ i ].remainingTime <= 0.0f ) {
                                g_TimedMessages[ i ].remainingTime = 0.85f;
                                g_TimedMessages[ i ].world_position = tile_grid_point_to_world_point( *entities->currentTiles[mirroredTempPlayerId] );
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
                
                if( points_equal(*entities->currentTiles[eid], levelConfig->ghostPenTile ) && entities->worldPositions[eid]->y >= tile_grid_point_to_world_point(levelConfig->ghostPenTile).y/2) {

                    *entities->ghostStates[ eid ] = STATE_LEAVE_PEN;
                    leave_pen_enter( entities, eid );
                }
                break;

            
            case STATE_NORMAL :
                break;
            case STATE_LEAVE_PEN:
                if( points_equal( *entities->currentTiles[eid], *entities->targetTiles[eid] ) ) {
                    *entities->ghostStates[ eid ] = STATE_NORMAL;
                    //*entities->directions[eid] = DIR_LEFT;
                    normal_enter( entities, eid );
                }
                
                break;
            case STATE_STAY_PEN:
                // check if num dots remaining <= entities->numDots[eid]
                if( g_StartingNumDots - g_NumDots >= *entities->numDots[ eid ] ) {
                    *entities->ghostStates[ eid ] = STATE_LEAVE_PEN;
                    leave_pen_enter( entities, eid );
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
            if( entities->isActive[ playerId ] != NULL && *entities->isActive[ playerId ] == SDL_FALSE ) {
                continue;
            }
            // player eats power pellet
            if( points_equal( *entities->currentTiles[eid], *entities->currentTiles[playerId] ) ) {
                gScore.score_number += *entities->scores[eid];

                g_NumGhostsEaten = 0;
                // move it outside of the world area for now.
                // TODO: deactivate this somehow
                entities->currentTiles[eid]->x = -1;
                entities->currentTiles[eid]->y = -1;
                entities->worldPositions[eid]->x = -100;
                entities->worldPositions[eid]->y = -100;
                entities->worldPositions[eid]->x = -100;
                entities->worldPositions[eid]->y = -100;
                g_NumDots--;

                // make ghosts all vulnerable state
                allGhostsVulnerableStateEnter( entities, levelConfig );   
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
            if( points_equal( *entities->currentTiles[eid], *entities->currentTiles[mirroredTempPlayerId] ) ) {
                gScore.score_number += *entities->scores[eid];

                g_NumGhostsEaten = 0;
                // move it outside of the world area for now.
                // TODO: deactivate this somehow
                entities->currentTiles[eid]->x = -1;
                entities->currentTiles[eid]->y = -1;
                entities->worldPositions[eid]->x = -100;
                entities->worldPositions[eid]->y = -100;
                entities->worldPositions[eid]->x = -100;
                entities->worldPositions[eid]->y = -100;
                g_NumDots--;

                // make ghosts all vulnerable state
                allGhostsVulnerableStateEnter( entities, levelConfig );  
            }

        }

    }


    processTemporaryPickup( entities, gPlayerIds, gNumPlayers, levelConfig, &gScore, tilemap, g_NumDots, deltaTime ) ;



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

    // if ghost vulnerability timer almost up, blink them
    if( gGhostVulnerableTimer <= 2.0f ) {
        blinkProcess( &ghostVulnerableBlink, deltaTime );
        float val = ghostVulnerableBlink.values[ghostVulnerableBlink.current_value_idx];
        SDL_SetTextureColorMod( g_texture_atlases[3].texture, 255, val, val);
    }
    else {
        SDL_SetTextureColorMod(g_texture_atlases[3].texture, 255,255,255);
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
        ghost_move( entities, i, tilemap, deltaTime );
        //align
        entities->collisionRects[i]->x = entities->worldPositions[i]->x - ACTOR_SIZE*0.5;
        entities->collisionRects[i]->y = entities->worldPositions[i]->y - ACTOR_SIZE*0.5;
        entities->collisionRects[i]->w = ACTOR_SIZE;
        entities->collisionRects[i]->h = ACTOR_SIZE;
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
                    *entities->directions[i] = opposite_directions[ *entities->directions[i] ];
                    *entities->nextTiles[i] = *entities->currentTiles[i]; // need to do this so that the ghost will want to set a new next tile
                }
                
            }
        }
        else {
            g_current_ghost_mode = MODE_CHASE;
            for( int i = 0; i < MAX_NUM_ENTITIES; ++i ) {
                if( entities->ghostStates[ i ] != NULL && *entities->ghostStates[ i ] == STATE_NORMAL ) {
                    *entities->directions[i] = opposite_directions[ *entities->directions[i] ];
                    *entities->nextTiles[i] = *entities->currentTiles[i]; // need to do this so that the ghost will want to set a new next tile
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

    for(int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if(entities->invincibilityTimers[eid] != NULL && *entities->invincibilityTimers[eid] > 0.0f ) {
            SDL_SetRenderDrawColor(gRenderer, 255,20,255,150);
            blinkProcess(&invincibleTimerAlmostUpBlink, deltaTime);
            if(*entities->invincibilityTimers[eid] < 3.0f) {
                SDL_SetRenderDrawColor(gRenderer, 255,20,255,invincibleTimerAlmostUpBlink.values[invincibleTimerAlmostUpBlink.current_value_idx]);
            }
            SDL_Point canvasPt = world_point_to_screen_point(
                (SDL_Point){entities->worldPositions[eid]->x, entities->worldPositions[eid]->y }
                , tilemap->tm_screen_position
            );
            renderCircleFill(gRenderer, canvasPt.x, canvasPt.y, 50);
        }
    }
    

    SDL_Rect black_bar = {0,0, 1920, TILE_SIZE * 2};
    SDL_SetRenderDrawColor( gRenderer, 0,0,0,255 );
    SDL_RenderFillRect( gRenderer, &black_bar);

    SDL_RenderCopy( gRenderer, gScore.score_texture, NULL, &gScore.score_render_dst_rect);
    SDL_RenderCopy( gRenderer, gLivesRemainingUI.texture, NULL, &gLivesRemainingUI.destRect);
    //SDL_RenderCopy(gRenderer, gCooldownTexture, NULL, &gCooldownRect);

    renderDashStockRects( ) ;

    // SDL_Rect scoreRect = g_texture_atlases[16].sprite_clips[0];
    // scoreRect.x = SCREEN_WIDTH/2 - (scoreRect.w/2);
    // SDL_RenderCopy(gRenderer, g_texture_atlases[16].texture, NULL, &scoreRect  );


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
            if( entities->currentTiles[i] == NULL ) {
                continue;
            }
            screenPoint = tile_grid_point_to_screen_point( *entities->currentTiles[i], tilemap->tm_screen_position );
            tileRect.x = screenPoint.x;    
            tileRect.y = screenPoint.y;    
            tileRect.w = TILE_SIZE;    
            tileRect.h = TILE_SIZE;    
            SDL_RenderFillRect( gRenderer, &tileRect);

            // screenPoint = tile_grid_point_to_screen_point( *entities->nextTiles[i], tilemap->tm_screen_position );
            // tileRect.x = screenPoint.x;    
            // tileRect.y = screenPoint.y; 
            // SDL_RenderFillRect( gRenderer, &tileRect);

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
            screenPoint = tile_grid_point_to_screen_point( *entities->targetTiles[i], tilemap->tm_screen_position );
            tileRect.x = screenPoint.x;
            tileRect.y = screenPoint.y;
            SDL_RenderDrawRect( gRenderer, &tileRect );
        }

        //collisionRect
        for(int eid = 0; eid < MAX_NUM_ENTITIES; eid++){
            if(entities->collisionRects[eid] == NULL){
                continue;
            }

            SDL_SetRenderDrawColor(gRenderer, 0,0,255,150);
            SDL_Point realPt = world_point_to_screen_point((SDL_Point){entities->collisionRects[eid]->x, entities->collisionRects[eid]->y}, tilemap->tm_screen_position);
            SDL_Rect rect = {realPt.x, realPt.y, entities->collisionRects[eid]->w, entities->collisionRects[eid]->h};
            SDL_RenderFillRect(gRenderer, &rect);
        }
        // sensors
        for(int eid = 0; eid < MAX_NUM_ENTITIES; eid++){
            if(entities->worldPositions[eid] == NULL){
                continue;
            }
            SDL_Point realPt = world_point_to_screen_point((SDL_Point){entities->worldPositions[eid]->x, entities->worldPositions[eid]->y}, tilemap->tm_screen_position);
            SDL_SetRenderDrawColor(gRenderer, 255,255,255,255);
            SDL_RenderDrawPoint(gRenderer, realPt.x, realPt.y);
        }

    }
    SDL_RenderPresent( gRenderer );
}


void gamePausedProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime ) ;
inline void gamePausedProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime ) {
    while (SDL_PollEvent( event ) != 0 ) {
        //int gameControllerState = SDL_GameControllerEventState( SDL_QUERY );
        SDL_bool shouldBreak = commonEventHandling(event);
        if( shouldBreak ) break;

        if( event->type == SDL_KEYUP ) {
            if( event->key.keysym.sym == SDLK_RETURN ) {
                gGamePlayingState = GAME_PLAYING;
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

void gameGameOverProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime){
    SDL_SetRenderDrawColor( gRenderer, 200,20,20,255 );
    SDL_RenderClear( gRenderer );
    SDL_RenderCopy( gRenderer, gGameOverTextTexture, NULL, &gGameOverTextRect );
    SDL_RenderPresent( gRenderer );

    gGameOverTimer += deltaTime;
    if( gGameOverTimer >= gGameOverDuration ) {
        gProgramState = MENU_PROGRAM_STATE;
        mainMenuProgramStateEnter(entities);
        gGamePlayingState = LEVEL_START;
        gGameOverTimer = 0.0f;
        return;
    }
}

void gameGameClearProcess(Entities *entities, SDL_Event *event, LevelConfig *evelConfig, float deltaTime){
    SDL_SetRenderDrawColor( gRenderer, 0,200,20,255 );
    SDL_RenderClear( gRenderer );
    SDL_RenderCopy( gRenderer, gGameClearTextTexture, NULL, &gGameClearTextRect );
    SDL_RenderPresent( gRenderer );

    gGameClearTimer += deltaTime;
    if( gGameClearTimer >= gGameClearDuration ) {
        gProgramState = MENU_PROGRAM_STATE;
        mainMenuProgramStateEnter(entities);
        gGamePlayingState = LEVEL_START;
        gGameClearTimer = 0.0f;
        return;
    }
}