#ifndef AUTO_TILE_H
#define AUTO_TILE_H

#include <SDL2/SDL.h>
#include "tiles.h"
#include "arrUtil.h"


typedef enum SurroundingTilesValues {
    NW_VALUE = 0x01,
    N_VALUE  = 0x02,
    NE_VALUE = 0x04,
    W_VALUE  = 0x08,
    E_VALUE  = 0x10,
    SW_VALUE = 0x20,
    S_VALUE  = 0x40,
    SE_VALUE = 0x80
} SurroundingTilesValues;


Uint8 
bitmaskSumForSurroundingTiles(char tm_walls[TILE_ROWS][TILE_COLS], int row, int col, int numRows, int numCols) {
    idx2D tNW, tN, tNE;
    idx2D tW,      tE;
    idx2D tSW, tS, tSE;


    tNW.r = row - 1;
    tNW.c = col - 1;

    tN.r = row - 1;
    tN.c = col;

    tNE.r = row - 1;
    tNE.c = col + 1;

    tW.r = row;
    tW.c = col - 1;

    tE.r = row;
    tE.c = col + 1;
    
    tSW.r = row + 1;
    tSW.c = col - 1;

    tS.r = row + 1;
    tS.c = col;


    tSE.r = row + 1;
    tSE.c = col + 1;

    int sum = 0;

    if( tNW.r < 0 || tNW.c < 0) {
        sum += NW_VALUE;
    }
    else if( tm_walls[tNW.r][tNW.c]  == 'x'   ) {
        sum += NW_VALUE;
    }


    if( tN.r < 0) {
        sum += N_VALUE;
    }
    else if( tm_walls[tN.r][tN.c] == 'x'  ) {
        sum += N_VALUE;
    }


    if( tNE.r < 0 || tNE.c >= numCols) {
        sum += NE_VALUE;
    }
    else if( tm_walls[tNE.r][tNE.c]  == 'x' ) {
        sum += NE_VALUE;
    }


    if(  tW.c < 0  ) {
        sum += W_VALUE;
    }
    else if(tm_walls[tW.r][tW.c] == 'x' ) {
        sum += W_VALUE;
    }


    if(  tE.c >= numCols ) {
        sum += E_VALUE;
    }
    else if( tm_walls[tE.r][tE.c] == 'x' ) {
        sum += E_VALUE;
    }


    if(tSW.r >= numRows || tSW.c < 0) {
        sum += SW_VALUE;
    }
    else if( tm_walls[tSW.r][tSW.c]  == 'x' ) {
        sum += SW_VALUE;
    }


    if(  tS.r >= numRows ) {
        sum += S_VALUE;
    }
    else if( tm_walls[tS.r][tS.c] == 'x'  ) {
        sum += S_VALUE;
    }


    if(  tSE.r >= numRows || tSE.c >= numCols  ){
        sum += SE_VALUE;
    }
    else if( tm_walls[tSE.r][tSE.c] == 'x' ) {
        sum += SE_VALUE;
    }


    return sum;


}


void
generateAutoTiledMap(SDL_Renderer *renderer, TileMap *tilemap, idx2D autoTiledTextureAtlasIdxs[TILE_ROWS][TILE_COLS], Uint8 *tsBitmasks, int tsNumRows, int tsNumCols) {

    Uint8 tileBitmask = 0x00;
    Uint8 tilesetBitmask = 0x00;
    SDL_bool breakOut = SDL_FALSE;
    for( int tmR = 0; tmR < TILE_ROWS; tmR++ ) {
        for( int tmC = 0; tmC < TILE_COLS; tmC++ ) {
            tileBitmask = bitmaskSumForSurroundingTiles(tilemap->tm_walls, tmR, tmC, TILE_ROWS, TILE_COLS);
            // look for tile in tileset with this bitmask
            for( int tsR = 0; tsR < tsNumRows; tsR++ ) {
                for( int tsC = 0; tsC < tsNumCols; tsC++ ) {
                    tilesetBitmask = acc2DArray(tsBitmasks, tsR, tsC, tsNumCols);
                    if( tilesetBitmask == tileBitmask ) {
                        // set 2d idx
                        autoTiledTextureAtlasIdxs[tmR][tmC].r = tsR;
                        autoTiledTextureAtlasIdxs[tmR][tmC].c = tsC;
                        breakOut = SDL_TRUE;
                        break;
                    }
                }
                if( breakOut ) {
                    breakOut = SDL_FALSE;
                    break;
                }
            }
        }
    }
}

#endif