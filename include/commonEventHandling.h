#ifndef EVENT_HANDLING_H
#define EVENT_HANDLING_H


#include <SDL2/SDL.h>
#include "globalData.h"

// returns should break
SDL_bool commonEventHandling(SDL_Event *event ) {
    if( event->type == SDL_QUIT ) {
        gProgramState = EXIT_STATE;
        return SDL_TRUE;
    }
    if( event->type == SDL_KEYUP ) {
        if( event->key.keysym.sym == SDLK_ESCAPE ) {
            gQuit = 1;
            return SDL_TRUE;
        }
        if( event->key.keysym.sym == SDLK_F11 ) {
            Uint32 windowFlags = SDL_GetWindowFlags( gWindow );
            SDL_SetWindowFullscreen( gWindow, windowFlags ^= SDL_WINDOW_FULLSCREEN );
        }
        if ( event->key.keysym.sym == SDLK_b ) {
            g_show_debug_info =  !g_show_debug_info;
        }

    
    }
    if( event->type == SDL_WINDOWEVENT ) {
        if( event->window.event == SDL_WINDOWEVENT_RESIZED ) {
            SDL_Log("Window resized to %d X %d", event->window.data1, event->window.data2 );
            float canvasWidth = event->window.data1;
            //float canvasHeight = event->window.data2;
            float ratio = (float)((float)canvasWidth/(float)SCREEN_WIDTH);
            SDL_RenderSetScale( gRenderer, ratio, ratio );
        }
            }
    return SDL_FALSE;
}

#endif