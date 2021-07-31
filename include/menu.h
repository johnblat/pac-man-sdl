#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include "programState.h"
#include "interpolation.h"
#include "globalData.h"

typedef enum MainMenuSelection {
    PLAY_GAME,
    EXIT,
    NUM_MAIN_MENU_SELECTIONS
} MainMenuSelection;

int gCurrentMainMenuSelection = PLAY_GAME;


// Title Screen
char *gTitleScreenText = "Press START to Play!";
SDL_Texture *gTitleScreenTextTexture = NULL;
SDL_Rect gTitleScreenTextDestRect;

// Main Menu Screen
char *gMainMenuPlayGameText = "Play Game";
char *gMainMenuExitText = "Exit";
SDL_Texture *gMainMenuPlayGameTextTexture = NULL;
SDL_Texture *gMainMenuExitTextTexture = NULL;
SDL_Rect gMainMenuPlayGameTextDestRect;
SDL_Rect gMainMenuExitTextDestRect;

// End Main Menu

void playGameChosen( ) {

}

void exitGameChosen( ) {

}


void initMainMenuScreenStuff(){
    SDL_Surface *mainMenuPlayGameTextSurface = TTF_RenderText_Solid( gFont, gMainMenuPlayGameText, white );
    gMainMenuPlayGameTextTexture = SDL_CreateTextureFromSurface( gRenderer, mainMenuPlayGameTextSurface );
    gMainMenuPlayGameTextDestRect.w = mainMenuPlayGameTextSurface->w;
    gMainMenuPlayGameTextDestRect.h = mainMenuPlayGameTextSurface->h;
    gMainMenuPlayGameTextDestRect.x = SCREEN_WIDTH / 2 - ( gMainMenuPlayGameTextDestRect.w / 2 );
    gMainMenuPlayGameTextDestRect.y = 450;
    SDL_FreeSurface( mainMenuPlayGameTextSurface );
    mainMenuPlayGameTextSurface = NULL;

    SDL_Surface *mainMenuExitTextSurface = TTF_RenderText_Solid( gFont, gMainMenuExitText, white );
    gMainMenuExitTextTexture = SDL_CreateTextureFromSurface( gRenderer, mainMenuExitTextSurface );
    gMainMenuExitTextDestRect.w = mainMenuExitTextSurface->w;
    gMainMenuExitTextDestRect.h = mainMenuExitTextSurface->h;
    gMainMenuExitTextDestRect.x = SCREEN_WIDTH / 2 - ( gMainMenuExitTextDestRect.w / 2 );
    gMainMenuExitTextDestRect.y = 450 + 100 ;
    SDL_FreeSurface( mainMenuExitTextSurface );
    mainMenuExitTextSurface = NULL;
}


void mainMenuScreenProcess( SDL_Event *event, Entities *entities, TileMap *tilemap, LevelConfig *levelConfig, Blink *blink, float deltaTime ) {
    SDL_bool move_up = SDL_FALSE;
    SDL_bool move_down = SDL_FALSE;
    SDL_bool selected = SDL_FALSE;

    while( SDL_PollEvent( event ) != 0 ) {
        if( event->type == SDL_CONTROLLERBUTTONUP ) {
            if( event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP ) {
                move_up = SDL_TRUE;
            }
            else if( event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN ) {
                move_down = SDL_TRUE;
            }
            else if( event->cbutton.button == SDL_CONTROLLER_BUTTON_A ) {
                selected = SDL_TRUE;
            }
        }
        if( event->type == SDL_KEYUP ) {
            if( event->key.keysym.sym == SDLK_UP ) {
                move_up = SDL_TRUE;
            }
            else if( event->key.keysym.sym == SDLK_DOWN) {
                move_up = SDL_TRUE;
            }
            else if( event->key.keysym.sym == SDLK_RETURN ) {
                selected = SDL_TRUE;
            }
        }
    }

    if( move_up ) {
        if( gCurrentMainMenuSelection == 0 ) {
            gCurrentMainMenuSelection = NUM_MAIN_MENU_SELECTIONS - 1;
        }
        else {
            gCurrentMainMenuSelection--;
        }
    }
    else if (move_down) {
        gCurrentMainMenuSelection++;
        if( gCurrentMainMenuSelection == NUM_MAIN_MENU_SELECTIONS ) {
            gCurrentMainMenuSelection = 0;
        }
    }
    else if( selected) {
        switch( gCurrentMainMenuSelection ) {
            case PLAY_GAME:
                gamePlayProgramStateEnter(entities, tilemap, levelConfig);
                break;
            case EXIT:
                gProgramState = EXIT_STATE;
                printf("clicked exit!\n");
                break;
            default:
                printf("Main Menu: Don't know what to select! Selected value: %d\n", gCurrentMainMenuSelection );
                break;
        }
    }

    // blink the selection that is the current one
    blinkProcess( blink, deltaTime );

    SDL_SetTextureAlphaMod( gMainMenuPlayGameTextTexture, 255 );
    SDL_SetTextureAlphaMod( gMainMenuExitTextTexture, 255 );

    switch( gCurrentMainMenuSelection ) {
        case PLAY_GAME:
            SDL_SetTextureAlphaMod(gMainMenuPlayGameTextTexture, blink->values[ blink->current_value_idx ] );
            break;
        case EXIT:
            SDL_SetTextureAlphaMod( gMainMenuExitTextTexture, blink->values[ blink->current_value_idx ] );
            break;
        default:
            printf("not good value for selection. gCurrentMainMenuSelection = %d\n", gCurrentMainMenuSelection );
            break;
    }

    // render
    SDL_SetRenderDrawColor( gRenderer, 20,20,20,255);
    SDL_RenderClear( gRenderer );

    SDL_RenderCopy( gRenderer, gMainMenuPlayGameTextTexture, NULL, &gMainMenuPlayGameTextDestRect);
    SDL_RenderCopy( gRenderer, gMainMenuExitTextTexture, NULL, &gMainMenuExitTextDestRect);

    SDL_RenderPresent( gRenderer );


}


void titleScreenProcess(LevelConfig *levelConfig, Entities *entities, TileMap *tilemap, SDL_Event *event, Blink *startMenuBlink, float deltaTime );
inline void titleScreenProcess( LevelConfig *levelConfig, Entities *entities, TileMap *tilemap, SDL_Event *event, Blink *startMenuBlink, float deltaTime ) {
    while( SDL_PollEvent( event ) != 0 ) {
        if( event->type == SDL_QUIT ) {
            gProgramState = EXIT_STATE;
            break;
        }
        if( event->type == SDL_KEYUP ) {
            if( event->key.keysym.sym == SDLK_RETURN ) {
                startMenuBlink->blinkRate = 0.2;
                gMenuState = MAIN_MENU_SCREEN_MENU_STATE;
                //gamePlayProgramStateEnter( entities, tilemap, levelConfig );
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
                        gMenuState = MAIN_MENU_SCREEN_MENU_STATE;
                        //gamePlayProgramStateEnter( entities, tilemap, levelConfig );
                        break;                                    
                    }
                }
            }
        }
    }

    blinkProcess( startMenuBlink, deltaTime );

    SDL_SetTextureAlphaMod( gTitleScreenTextTexture, startMenuBlink->values[ startMenuBlink->current_value_idx ] );

    SDL_SetRenderDrawColor( gRenderer, 20,20,20,255);
    SDL_RenderClear( gRenderer );

    // display message

    SDL_RenderCopy( gRenderer, gTitleScreenTextTexture, NULL, &gTitleScreenTextDestRect);
    SDL_RenderPresent( gRenderer );
}


#endif