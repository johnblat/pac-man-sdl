#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include "entity.h"
#include "levelConfig.h"
#include "tiles.h"
#include "sounds.h"
#include "gamePlayingState.h"
#include "menuState.h"
#include "menu.h"
#include "globalData.h"


void gamePlayProgramStateEnter( Entities *entities, TileMap *tilemap, LevelConfig *levelConfig ) {

    // destroy title screen texture
    if( gTitleScreenImageTexture != NULL ) {
        SDL_DestroyTexture(gTitleScreenImageTexture );
        gTitleScreenImageTexture = NULL;
    }

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

    gLivesRemaining = moreLivesCheatCodeEnabled ? 50 :  gInitialLivesRemaining + gCheatAdditionalLivesRemaining + gNumPlayers;
    moreLivesCheatCodeEnabled = SDL_FALSE;
    moreLivesCheatCodeIdx = 0;
    gLivesRemainingUI.livesRemaining = gLivesRemaining;
    updateLivesRemainingTexture(&gLivesRemainingUI);


    // zero out certian stuff
    for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
        if( entities->deathTimers[eid] != NULL ) {
            *entities->deathTimers[eid] = 0.0f;
        }
        if( entities->respawnTimers[eid] != NULL ) {
            *entities->respawnTimers[eid] = 0.0f;
        }
        if( entities->slowTimers[eid] != NULL ) {
            *entities->slowTimers[eid] = 0.0f;
        }
        if( entities->dashTimers[eid] != NULL ) {
            *entities->dashTimers[eid] = 0.0f;
        }
        if( entities->stopTimers[eid] != NULL ) {
            *entities->stopTimers[eid] = 0.0f;
        }
        if( entities->activeTimers[eid] != NULL ) {
            *entities->activeTimers[eid] = 0.0f;
        }
        if( entities->chargeTimers[eid] != NULL ) {
            *entities->chargeTimers[eid] = 0.0f;
        }
        if( entities->invincibilityTimers[eid] != NULL ) {
            *entities->invincibilityTimers[eid] = 0.0f;
        }
        if( entities->speedBoostTimers[eid] != NULL ) {
            *entities->speedBoostTimers[eid] = 0.0f;
        }

        if( entities->dashCooldownStocks[eid] != NULL ) {
            entities->dashCooldownStocks[eid]->cooldownTimer = 0.0f;
            entities->dashCooldownStocks[eid]->currentNumStock = 3;

        }
    }

    level_advance( levelConfig, tilemap, gRenderer, entities );
    initializeDashStockRects( gNumPlayers );

    // activate all players
    EntityId pid = 0;
    for( int i = 0; i < gNumPlayers; i++ ) {
        pid = gPlayerIds[ i ];
        *entities->isActive[pid] = SDL_TRUE;
    }

    
}

void mainMenuProgramStateEnter(Entities *entities) {
    SDL_Surface *tsSurface = IMG_Load(gTitleScreenImageFilename);
    if( tsSurface == NULL ) {
        fprintf(stderr, "Something went wrong trying to open %s\n", gTitleScreenImageFilename);
    }
    gTitleScreenImageTexture = SDL_CreateTextureFromSurface(gRenderer, tsSurface);
    SDL_FreeSurface( tsSurface );
    tsSurface = NULL;

    gScore.score_number = 0;
    Mix_HaltChannel( GHOST_SOUND_CHANNEL );
    Mix_HaltChannel( GHOST_VULN_CHANNEL );
    Mix_HaltMusic();
    Mix_FreeMusic( g_Music );
    g_Music = Mix_LoadMUS( gMenuMusicFilename );
    Mix_PlayMusic( g_Music, -1 );
    gProgramState = MENU_PROGRAM_STATE;
    gMenuState = TITLE_SCREEN_MENU_STATE;
    gCurrentLevel = 0;
    gCurrentExtraLifeMilestoneIdx = 0;
    gLivesRemaining = 3;
    updateLivesRemainingTexture( &gLivesRemainingUI );
    gNumPlayers = 0;
    gCheatAdditionalLivesRemaining = 0;

    // deactivate all players
    EntityId pid;
    for( int i = 0; i < gNumPlayers; i++ ) {
        pid = gPlayerIds[ i ];
        *entities->isActive[ pid ] = SDL_FALSE;
    }
    gNumPlayers = 0;

    // get rid of controllers because we use NULL to tell whether or not a player needs to be assigned a controller in the Join Game Screen
    // TODO: Figure out a better way to do this
    for( int eid = 0; eid < g_NumEntities; eid++ ) {
        if( entities->gameControllerIds[eid]== NULL) {
            continue;
        }
        free(entities->gameControllerIds[eid]);
        entities->gameControllerIds[eid] = NULL;
    }
    // also set keybinds to null. Setting up join game needs this null to reason about it.
    for( int eid = 0; eid < g_NumEntities;eid++ ) {
        if( entities->keybinds[eid] != NULL ) {
            entities->keybinds[eid] = NULL;
        }
    }

    
    //load_current_level_off_disk( levelConfig, tilemap, gRenderer);
    return;
}

void menuProgramStateProcess( SDL_Event *event, Entities *entities, LevelConfig *levelConfig, TileMap *tilemap, Blink *startMenuBlink, float deltaTime  ) {
    switch( gMenuState ) {
        case TITLE_SCREEN_MENU_STATE:
            titleScreenProcess(levelConfig, entities, tilemap, event, startMenuBlink, deltaTime );
            break;
        case MAIN_MENU_SCREEN_MENU_STATE:
            mainMenuScreenProcess(event, entities, tilemap, levelConfig, startMenuBlink, deltaTime);
            break;
        case JOIN_GAME_MENU_STATE:
            joinGameProcess(event, levelConfig, entities, tilemap, startMenuBlink, deltaTime );
            break;
        default:
            printf("Not a valid menu State. State = %d\n", gMenuState );
            break;
    }
}