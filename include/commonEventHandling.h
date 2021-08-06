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
    return SDL_FALSE;
}

#endif