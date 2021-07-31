#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
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


// Main Menu
char *gMainMenuText = "Press START to Play!";
SDL_Texture *gMainMenuTextTexture = NULL;
SDL_Rect gMainMenuTextDestRect;


// End Main Menu

void playGameChosen( ) {

}

void exitGameChosen( ) {

}

void mainMenuInputProcess( SDL_Event *event, Entities *entities, TileMap *tilemap, LevelConfig *levelConfig  ) {

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
                break;
            default:
                printf("Main Menu: Don't know what to select! Selected value: %d\n", gCurrentMainMenuSelection );
                break;
        }
    }
    
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
                gamePlayProgramStateEnter( entities, tilemap, levelConfig );
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
                        gamePlayProgramStateEnter( entities, tilemap, levelConfig );
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


char *gPlayGameText = "Play Game";
char *gExitText = "Exit";

#endif