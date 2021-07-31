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
    gLivesRemaining = 2 + gNumPlayers;
    level_advance( levelConfig, tilemap, gRenderer, entities );

    // activate all players
    EntityId pid = 0;
    for( int i = 0; i < gNumPlayers; i++ ) {
        pid = gPlayerIds[ i ];
        *entities->isActive[pid] = SDL_TRUE;
    }
}

void mainMenuProgramStateEnter() {
    Mix_HaltChannel( GHOST_SOUND_CHANNEL );
    Mix_HaltChannel( GHOST_VULN_CHANNEL );
    Mix_HaltMusic();
    Mix_FreeMusic( g_Music );
    g_Music = Mix_LoadMUS( gMenuMusicFilename );
    Mix_PlayMusic( g_Music, -1 );
    gProgramState = MENU_PROGRAM_STATE;
    gMenuState = TITLE_SCREEN_MENU_STATE;
    gCurrentLevel = 0;
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
        default:
            printf("Not a valid menu State. State = %d\n", gMenuState );
            break;
    }
}