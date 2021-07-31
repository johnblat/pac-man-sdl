#ifndef GAME_PLAYING_H
#define GAME_PLAYING_H

#include "entity.h"
#include "tiles.h"
#include "levelConfig.h"
#include "sounds.h"
#include "ghostStates.h"
#include "states.h"
#include "programState.h"
#include "globalData.h"
#include "menuState.h"
#include "movement.h"
#include "UI.h"
#include "targeting.h"
#include "animation.h"
#include "render.h"
#include "comparisons.h"
#include "resources.h"
#include "renderProcessing.h"

#include <SDL2/SDL.h>


typedef enum {
    GAME_PLAYING,
    GAME_PAUSED,
    LEVEL_START,
    LEVEL_END
} GamePlayingState;





SDL_bool isGameOver( Entities *entities );


void initGamePlayingStuff( ) ;

// Returns SDL_TRUE if advanced to next level
// Returns SDL_FALSE if no more levels to advance
SDL_bool level_advance(LevelConfig *levelConfig, TileMap *tilemap, SDL_Renderer *renderer, Entities *entities);

void gamePlayingProcess( Entities *entities, TileMap *tilemap, SDL_Event *event, LevelConfig *levelConfig, float deltaTime) ;
inline void gamePlayingProcess( Entities *entities, TileMap *tilemap, SDL_Event *event, LevelConfig *levelConfig, float deltaTime) ;

void gamePlayingStateProcess( SDL_Event *event, Entities *entities, TileMap *tilemap, LevelConfig *levelConfig, float deltaTime );

void gamePausedProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime ) ;
inline void gamePausedProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime );
void gameLevelStartProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime );
inline void gameLevelStartProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime ) ;

void gameLevelEndProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime ) ;
inline void gameLevelEndProcess( Entities *entities, SDL_Event *event, LevelConfig *levelConfig, float deltaTime ) ;

#endif