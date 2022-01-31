#ifndef COMP_H
#define COMP_H

#include <SDL2/SDL.h>

SDL_bool points_equal( SDL_Point p1, SDL_Point p2 ){
    return ( p1.x == p2.x && p1.y == p2.y  );
} 

#define idx2D_equal(a, b) ( \
    (a.r == b.r && a.c == b.c ) \
)

//#define positions_equal points_equal;
//#define vector2s_equal positions_equal; 
//#define two_dimensional_indexes_equal( idx1, idx2) ( idx1.r == idx2.r && idx1.c == idx2.c )

#endif