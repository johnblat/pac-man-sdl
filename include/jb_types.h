#ifndef JB_TYPES_H
#define JB_TYPES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct v2d {
    float x, y;
} v2d;

typedef v2d Position;
typedef v2d Velocity;



typedef struct Line_Points {
    SDL_Point a, b;
} Line_Points; 

typedef struct Score {
    unsigned int score_number;
    char score_text[32];
    Position score_position;
    SDL_Rect score_render_dst_rect;
    SDL_Texture *score_texture;
    TTF_Font *font;
    SDL_Color score_color;
} Score;


#endif