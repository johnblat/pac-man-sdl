#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "interpolation.h"

typedef struct {
    float remainingTime;
    char message[ 8 ];
    SDL_Point world_position;
    SDL_Color color;
    SDL_Texture *messageTexture;
    SDL_Rect render_dest_rect;
    TTF_Font *font;
    Lerp l;

} TimedMessage;

#define g_NumTimedMessages 5
TimedMessage g_TimedMessages[ g_NumTimedMessages ];
Blink g_ScoreBlinks[g_NumTimedMessages ];

#endif