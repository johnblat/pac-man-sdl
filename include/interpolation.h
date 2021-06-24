#include <SDL2/SDL.h>

typedef struct{
    float value;
    float step;
    float remainingTime;
} Lerp;

typedef struct {
    float values[2];
    uint8_t current_value_idx;
    float blinkRate;
    float remainingTime;
} Blink;

Lerp lerpInit( float to, float from, float time ) {
    Lerp l;
    l.remainingTime = time;
    l.value = from;
    l.step = ( to - from ) / time;
    return l;
}

SDL_bool interpolate( Lerp *l, float deltaTime ) {
    l->remainingTime -= deltaTime;
    l->value += l->step * deltaTime;
    return ( l->remainingTime <= 0 );
}

float blinkProcess( Blink *b, float deltaTime ) {
    b->remainingTime -= deltaTime;
    if( b->remainingTime <= 0.0f ) {
        b->current_value_idx = !b->current_value_idx;
        b->remainingTime = b->blinkRate;
    }
    return b->values[ b->current_value_idx ];
}

Blink blinkInit( float time, float value1, float value2 ) {
    Blink b;
    b.blinkRate = time;
    b.current_value_idx = 0;
    b.remainingTime = time;
    b.values[ 0 ] = value1;
    b.values[ 1 ] = value2;

    return b;
}