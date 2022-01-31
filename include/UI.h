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

typedef struct {
    SDL_Rect rects[ 3 ];
    SDL_Color color;
    unsigned int numRectsToShow;
} DashStockRects;

DashStockRects gDashStockRects[ 4 ];
unsigned int gNumDashStockRects = 0; 
const unsigned int MAX_NUM_DASH_STOCK_RECTS = 3;

// Lives
typedef struct LivesRemainingUI {
    unsigned int livesRemaining;
    char text[ 16 ];
    SDL_Rect destRect;
    SDL_Texture *texture;
    TTF_Font *font;
    SDL_Color color;
} LivesRemainingUI;



void initializeDashStockRects( unsigned int numPlayers ) {
    gNumDashStockRects = numPlayers;
    
    const unsigned int RECT_HEIGHT = 40;
    const unsigned int RECT_SPACING = 4;
    const unsigned int RECT_WIDTH = 20;

    //const unsigned int FULL_WIDTH = 20 + 4 + 20 + 4 + 20;

    const unsigned int PLAYER_RECT_SPACING = 80;

    const SDL_Color p1_color = {200,200,0};
    const SDL_Color p2_color = { 20,255,20};
    const SDL_Color p3_color = {253,201,255};
    const SDL_Color p4_color = {200,200,200};

    const SDL_Color colors[ 4 ] = {p1_color, p2_color, p3_color, p4_color};

    unsigned int currentX = 300; // original offset
    for( int i = 0; i < gNumDashStockRects; i++ ) {
        for( int numRect = 0; numRect < 3; numRect++ ) {
            gDashStockRects[ i ].rects[numRect].x = currentX + RECT_SPACING;
            gDashStockRects[ i ].rects[ numRect ].y = 20;
            gDashStockRects[ i ].rects[numRect].w = RECT_WIDTH;
            gDashStockRects[i].rects[numRect].h = RECT_HEIGHT;

            gDashStockRects[i].color = colors[ i ];

            gDashStockRects[i].numRectsToShow = MAX_NUM_DASH_STOCK_RECTS;

            currentX = gDashStockRects[ i ].rects[numRect].x + RECT_WIDTH;
        }
        currentX += PLAYER_RECT_SPACING;
    }
}


#endif