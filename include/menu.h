#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include "programState.h"
#include "interpolation.h"
#include "globalData.h"
#include "render.h"
#include "commonEventHandling.h"
#include "menuState.h"

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


SDL_Texture *gTitleScreenImageTexture = NULL;
const char *gTitleScreenImageFilename = "res/img/TitleScreenImage.png";

// Main Menu Screen
char *gMainMenuPlayGameText = "Play Game";
char *gMainMenuExitText = "Exit";
SDL_Texture *gMainMenuPlayGameTextTexture = NULL;
SDL_Texture *gMainMenuExitTextTexture = NULL;
SDL_Rect gMainMenuPlayGameTextDestRect;
SDL_Rect gMainMenuExitTextDestRect;



// End Main Menu

// Join Game

typedef struct UIComponent {
    char text[32];
    SDL_Rect destRect;
    SDL_Texture *texture;
} UIComponent;

SDL_Rect gP1JoinPanelRect;
SDL_Rect gP2JoinPanelRect;
SDL_Rect gP3JoinPanelRect;
SDL_Rect gP4JoinPanelRect;
char *gJoinGameText = "Join Game";
char *gP1Text = "P1";
char *gP2Text = "P2";
char *gP3Text = "P3";
char *gP4Text = "P4";
char *gPressAToJoinText = "Press Any Button to Join!";
char *gReadyText = "Ready!";
char *gPressStartWhenReadyText = "Press Start When All Players Ready!";
SDL_Texture *gJoinGameTextTexture = NULL;
SDL_Texture *gP1TextTexture = NULL;
SDL_Texture *gP2TextTexture = NULL;
SDL_Texture *gP3TextTexture = NULL;
SDL_Texture *gP4TextTexture = NULL;
SDL_Texture *gPressAToJoinTextTexture = NULL;
SDL_Texture *gReadyTextTexture = NULL;
SDL_Texture *gPressStartWhenReadyTextTexture = NULL;
SDL_Rect gJoinGameTextRect;
SDL_Rect gP1TextRect;
SDL_Rect gP2TextRect;
SDL_Rect gP3TextRect;
SDL_Rect gP4TextRect;
SDL_Rect gP1PressAToJoinTextRect;
SDL_Rect gP2PressAToJoinTextRect;
SDL_Rect gP3PressAToJoinTextRect;
SDL_Rect gP4PressAToJoinTextRect;
SDL_Rect gPressAToJoinTextRects[4];
SDL_Rect gP1ReadyTextRect;
SDL_Rect gP2ReadyTextRect;
SDL_Rect gP3ReadyTextRect;
SDL_Rect gP4ReadyTextRect;
SDL_Rect gReadyRects[4];
SDL_Rect gPressStartWhenReadyTextRect;





// End Join Game

void playGameChosen( ) {

}

void exitGameChosen( ) {

}



void initJoinGameStuff() {
    gJoinGameTextTexture = createTextTexture(&gJoinGameTextRect, gJoinGameText, black, gMedFont, SCREEN_WIDTH/2, 50 );
    gP1TextTexture = createTextTexture(&gP1TextRect, gP1Text, black, gMedFont, SCREEN_WIDTH/4, SCREEN_HEIGHT/4 );
    gP2TextTexture = createTextTexture(&gP2TextRect, gP2Text, black, gMedFont, (SCREEN_WIDTH/4) * 3 , SCREEN_HEIGHT/4 );
    gP3TextTexture = createTextTexture(&gP3TextRect, gP3Text, black, gMedFont, SCREEN_WIDTH/4, ( SCREEN_HEIGHT/4) * 3 );
    gP4TextTexture = createTextTexture(&gP4TextRect, gP4Text, black,  gMedFont,(SCREEN_WIDTH/4) * 3, (SCREEN_HEIGHT/4) * 3 );

    gReadyTextTexture = createTextTexture(&gP1ReadyTextRect, gReadyText, green, gMedFont, gP1TextRect.x+(gP1TextRect.w/2), gP1TextRect.y+ 50 );
    gP2ReadyTextRect.x = gP2TextRect.x+(gP2TextRect.w/2);
    gP2ReadyTextRect.y = gP2TextRect.y+50;
    gP2ReadyTextRect.w = gP1ReadyTextRect.w;
    gP2ReadyTextRect.h = gP1ReadyTextRect.h;


    gP3ReadyTextRect.x = gP3TextRect.x+(gP3TextRect.w/2);
    gP3ReadyTextRect.y = gP3TextRect.y+50;
    gP3ReadyTextRect.w = gP1ReadyTextRect.w;
    gP3ReadyTextRect.h = gP1ReadyTextRect.h;
    
    gP4ReadyTextRect.x = gP4TextRect.x+(gP4TextRect.w/2);
    gP4ReadyTextRect.y = gP4TextRect.y+50;
    gP4ReadyTextRect.w = gP1ReadyTextRect.w;
    gP4ReadyTextRect.h = gP1ReadyTextRect.h;

    gReadyRects[ 0 ] = gP1ReadyTextRect;
    gReadyRects[ 1 ] = gP2ReadyTextRect;
    gReadyRects[ 2 ] = gP3ReadyTextRect;
    gReadyRects[ 3 ] = gP4ReadyTextRect;

    gPressAToJoinTextTexture = createTextTexture(&gP1PressAToJoinTextRect, gPressAToJoinText, white, gMedFont, gP1TextRect.x+(gP1TextRect.w/2), gP1TextRect.y+ 50  );
    gP2PressAToJoinTextRect.x = gP2TextRect.x-(gP1PressAToJoinTextRect.w/2);
    gP2PressAToJoinTextRect.y = gP2TextRect.y+50;
    gP2PressAToJoinTextRect.w = gP1PressAToJoinTextRect.w;
    gP2PressAToJoinTextRect.h = gP1PressAToJoinTextRect.h;

    gP3PressAToJoinTextRect.x = gP3TextRect.x-(gP1PressAToJoinTextRect.w/2);
    gP3PressAToJoinTextRect.y = gP3TextRect.y+50;
    gP3PressAToJoinTextRect.w = gP1PressAToJoinTextRect.w;
    gP3PressAToJoinTextRect.h = gP1PressAToJoinTextRect.h;

    gP4PressAToJoinTextRect.x = gP4TextRect.x-(gP1PressAToJoinTextRect.w/2);
    gP4PressAToJoinTextRect.y = gP4TextRect.y+50;
    gP4PressAToJoinTextRect.w = gP1PressAToJoinTextRect.w;
    gP4PressAToJoinTextRect.h = gP1PressAToJoinTextRect.h;


    gPressAToJoinTextRects[ 0 ] = gP1PressAToJoinTextRect;
    gPressAToJoinTextRects[ 1 ] = gP2PressAToJoinTextRect;
    gPressAToJoinTextRects[ 2 ] = gP3PressAToJoinTextRect;
    gPressAToJoinTextRects[ 3 ] = gP4PressAToJoinTextRect;



    gPressStartWhenReadyTextTexture = createTextTexture(&gPressStartWhenReadyTextRect, gPressStartWhenReadyText, white, gMedFont, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
}

void initMainMenuScreenStuff(){
    gMainMenuPlayGameTextTexture = createTextTexture( &gMainMenuPlayGameTextDestRect, gMainMenuPlayGameText, black, gLargeFont, SCREEN_WIDTH / 2, 700 );
    gMainMenuExitTextTexture = createTextTexture( &gMainMenuExitTextDestRect, gMainMenuExitText, black, gLargeFont, SCREEN_WIDTH / 2, 800 );
}


void mainMenuScreenProcess( SDL_Event *event, Entities *entities, TileMap *tilemap, LevelConfig *levelConfig, Blink *blink, float deltaTime ) {
    SDL_bool move_up = SDL_FALSE;
    SDL_bool move_down = SDL_FALSE;
    SDL_bool selected = SDL_FALSE;

    while( SDL_PollEvent( event ) != 0 ) {
        SDL_bool shouldBreak = commonEventHandling(event);
        if( shouldBreak ) break;

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
            else if( event->key.keysym.sym == MoreLivesCheatCode[moreLivesCheatCodeIdx]) {
                moreLivesCheatCodeIdx++;
                if( moreLivesCheatCodeIdx == numMoreLivesCheatCodeKeys ) {
                    moreLivesCheatCodeEnabled = SDL_TRUE;
                    Mix_PlayChannel(EXTRA_LIFE_CHANNEL, g_ExtraLifeSound, 0);
                }
            }
            else if( event->key.keysym.sym == SDLK_KP_1 ) {
                Mix_PlayChannel(EXTRA_LIFE_CHANNEL, g_ExtraLifeSound, 0);
                gCheatAdditionalLivesRemaining++;
            }
            else {
                moreLivesCheatCodeIdx = 0;
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
                gMenuState = JOIN_GAME_MENU_STATE;
                //gamePlayProgramStateEnter(entities, tilemap, levelConfig);
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

    SDL_RenderCopy(gRenderer, gTitleScreenImageTexture, NULL, NULL );


    SDL_RenderCopy( gRenderer, gMainMenuPlayGameTextTexture, NULL, &gMainMenuPlayGameTextDestRect);
    SDL_RenderCopy( gRenderer, gMainMenuExitTextTexture, NULL, &gMainMenuExitTextDestRect);

    SDL_RenderPresent( gRenderer );


}

void joinGameProcess( SDL_Event *event, LevelConfig *levelConfig, Entities *entities, TileMap *tilemap, Blink *blink, float deltaTime ) {
    SDL_bool back_release = SDL_FALSE;
    SDL_bool start_release = SDL_FALSE;

    while( SDL_PollEvent( event ) != 0 ) {
        SDL_bool shouldBreak = commonEventHandling( event );
        if( shouldBreak ) break;

        else if( event->type == SDL_CONTROLLERDEVICEADDED ) {
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
        else if( event->type == SDL_CONTROLLERDEVICEREMOVED ) {
            Sint32 joyStickDeviceId = event->cdevice.which;
            // look through global gameController array for matching joystick
            for( int i = 0; i < MAX_NUM_GAME_CONTROLLERS; i++ ) {
                SDL_Joystick *joy = SDL_GameControllerGetJoystick( g_GameControllers[ i ] );
                
                if( joyStickDeviceId == SDL_JoystickInstanceID( joy ) ) {
                    printf("Controller removed\n" );
                    SDL_GameControllerClose( g_GameControllers[ i ] );
                    g_GameControllers[ i ] = NULL;
                    g_NumGamepads--;

                }
            }
        }

        else if( event->type == SDL_KEYUP ) {
            if( event->key.keysym.sym == SDLK_x ) {
                back_release = SDL_TRUE;
            }
            if ( event->key.keysym.sym == SDLK_RETURN ) {
                start_release = SDL_TRUE;
            }
            if( event->key.keysym.sym != SDLK_RETURN && event->key.keysym.sym != SDLK_x && event->key.keysym.sym != SDLK_F11 ) {
                // full players?
                if( gNumPlayers >= 4 ) {
                    break;
                }
                // do any players already have a keybind
                // if so don't assign
                SDL_bool KeyboardAlreadyAssigned = SDL_FALSE;
                for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
                    if( entities->keybinds[eid] == NULL ) {
                        continue;
                    }
                    if( entities->keybinds[eid] != NULL ) {
                        KeyboardAlreadyAssigned = SDL_TRUE;
                    }
                }

                if( KeyboardAlreadyAssigned ) {
                    break;
                }

                // keyboard not assigned
                // entities->keybinds[]

                // find first viable entity that doesn't have a keybind
                for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++ ) {
                    if( entities->inputMasks[eid] == NULL ) {
                        continue;
                    }
                    // make sure it doesn't have a controller
                    if( entities->gameControllerIds[eid] == NULL ) {
                        entities->keybinds[eid] = gkeyBindings;
                        gPlayerIds[ gNumPlayers ] = eid;
                        gNumPlayers++; // add number of players
                        break;
                    }
                }
            }
        }
        else if( event->type == SDL_CONTROLLERBUTTONUP ) {
            if( event->cbutton.button == SDL_CONTROLLER_BUTTON_B ) {
                back_release = SDL_TRUE;
            }
            else if( event->cbutton.button != SDL_CONTROLLER_BUTTON_B && event->cbutton.button != SDL_CONTROLLER_BUTTON_START ) { // a player wants to join
                // full players?
                if( gNumPlayers >= 4 ) {
                    continue;
                }
                SDL_bool assigned = SDL_FALSE;

                SDL_JoystickID eventJoyStickDeviceId = event->cdevice.which;
                GameControllerId eventGameControllerId;

                // which game controller pressed this button?
                for( unsigned int gcid = 0; gcid < MAX_NUM_GAME_CONTROLLERS; gcid++ ) {
                    
                    SDL_JoystickID gameControllerJoyStickDeviceId = SDL_JoystickInstanceID( SDL_GameControllerGetJoystick( g_GameControllers[gcid] ) );
                    // the event matched a registered game controller in the global array
                    if( eventJoyStickDeviceId == gameControllerJoyStickDeviceId ) {
                        eventGameControllerId = gcid;
                        break;
                    }
                    //assigned = SDL_TRUE;
                    
                }

                // is this game controller already assigned to a player?
                for( int eid = 0; eid < MAX_NUM_ENTITIES; eid++) {
                    if( entities->inputMasks[eid] == NULL ) {
                        continue;
                    }
                    if( entities->gameControllerIds[eid] == NULL ) {
                        continue;
                    }
                    // its already assigned
                    if( *entities->gameControllerIds[eid] == eventGameControllerId ) {
                        assigned = SDL_TRUE;
                        break;
                    }
                    
                }

                if( assigned ) {
                    break;
                }

                // not already assigned
                // look for a player entity
                for( int eid = 0; eid < MAX_NUM_ENTITIES; ++eid ) {
                    if( entities->inputMasks[ eid ] == NULL ) { // can be controlled
                        continue;
                    }
                    if( entities->gameControllerIds[ eid ] != NULL ) { // already taken
                        continue;
                    }
                    // it has a keyboard instead. Its basically assigned.
                    if( entities->keybinds[eid] != NULL ) {
                        assigned = SDL_TRUE;
                        continue;
                    }
                    gPlayerIds[ gNumPlayers ] = eid;
                    gNumPlayers++; // add number of players
                    
                    
                    

                    entities->gameControllerIds[ eid ] = (GameControllerId *)malloc(sizeof(GameControllerId));
                    *entities->gameControllerIds[eid] = eventGameControllerId;
                    break;
                }
                
            }
            else if( event->cbutton.button == SDL_CONTROLLER_BUTTON_START ) {
                start_release = SDL_TRUE;
            }
        }
    }

    blinkProcess( blink, deltaTime );
    SDL_SetTextureAlphaMod(gPressAToJoinTextTexture, blink->values[ blink->current_value_idx ] );

    // create player if button pressed
    // increase gNumPlayers += 1
    // give player the controller's id
    //  or a keybind if the keyboard was pressed
    if( back_release == SDL_TRUE ) {
        gMenuState = MAIN_MENU_SCREEN_MENU_STATE;
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
        return;
    }
    if( start_release == SDL_TRUE && gNumPlayers > 0 ) {
        gamePlayProgramStateEnter( entities, tilemap, levelConfig );
        return;
    }

    // render
    SDL_SetRenderDrawColor( gRenderer, pac_color.r,pac_color.g,pac_color.b,255);
    SDL_RenderClear( gRenderer );

    SDL_RenderCopy( gRenderer, gJoinGameTextTexture, NULL, &gJoinGameTextRect);
    SDL_RenderCopy( gRenderer, gP1TextTexture, NULL, &gP1TextRect);
    SDL_RenderCopy( gRenderer, gP2TextTexture, NULL, &gP2TextRect);
    SDL_RenderCopy( gRenderer, gP3TextTexture, NULL, &gP3TextRect);
    SDL_RenderCopy( gRenderer, gP4TextTexture, NULL, &gP4TextRect);

    for( int i = 0; i < gNumPlayers; i++ ) {
        SDL_Rect readyRect = gReadyRects[ i ];
        SDL_RenderCopy( gRenderer, gReadyTextTexture, NULL,  &readyRect );
    }
    for( int i = gNumPlayers; i < 4; i++ ) {
        SDL_Rect pressAToJoinRect = gPressAToJoinTextRects[ i ];
        SDL_RenderCopy( gRenderer, gPressAToJoinTextTexture, NULL, &pressAToJoinRect );
    }

    SDL_RenderCopy(gRenderer, gPressStartWhenReadyTextTexture, NULL, &gPressStartWhenReadyTextRect );


    SDL_RenderPresent( gRenderer );


}

void titleScreenProcess(LevelConfig *levelConfig, Entities *entities, TileMap *tilemap, SDL_Event *event, Blink *startMenuBlink, float deltaTime );
inline void titleScreenProcess( LevelConfig *levelConfig, Entities *entities, TileMap *tilemap, SDL_Event *event, Blink *startMenuBlink, float deltaTime ) {
    while( SDL_PollEvent( event ) != 0 ) {
        SDL_bool shouldBreak = commonEventHandling(event);
        if( shouldBreak ) break;

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

        }
        if( event->type == SDL_CONTROLLERDEVICEADDED ) {
            if( ! (g_NumGamepads>=MAX_NUM_GAME_CONTROLLERS ) ) {
                SDL_JoystickID joyStickDeviceId = event->cdevice.which;
                if(SDL_IsGameController(joyStickDeviceId)) {
                    // is the game controller already exist
                    for( int i = 0; i < MAX_NUM_GAME_CONTROLLERS;i++) {
                        if( g_GameControllers[i] == NULL ) {
                            continue;
                        }
                        SDL_JoystickID gcJoystickId = SDL_JoystickInstanceID( SDL_GameControllerGetJoystick( g_GameControllers[ i ] ) );
                        if( gcJoystickId == joyStickDeviceId ) {
                            return;
                        }
                    }
                    
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

    SDL_RenderCopy(gRenderer, gTitleScreenImageTexture, NULL, NULL );
    
    // display message

    SDL_RenderCopy( gRenderer, gTitleScreenTextTexture, NULL, &gTitleScreenTextDestRect);
    SDL_RenderPresent( gRenderer );
}


#endif