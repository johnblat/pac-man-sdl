#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>
#include <stdio.h>
#include "animation.h"
#include "actor.h"
#include "jb_types.h"
#include "constants.h"
#include "tiles.h"
#include "entity.h"
#include "sounds.h"



Direction opposite_directions[ 4 ] = { DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT };


void actor_align_world_data_based_on_world_position( Entities *entities, EntityId eid ) {

    entities->currentTiles[eid]->x = entities->worldPositions[eid]->x / TILE_SIZE;
    entities->currentTiles[eid]->y = entities->worldPositions[eid]->y / TILE_SIZE;

    if(entities->sensors[eid] != NULL){
        entities->sensors[eid]->worldTopSensor = (SDL_Point){
            entities->worldPositions[eid]->x,
            entities->worldPositions[eid]->y - (ACTOR_SIZE*0.5)
        };

        entities->sensors[eid]->worldBottomSensor = (SDL_Point){
            entities->worldPositions[eid]->x,
            entities->worldPositions[eid]->y + (ACTOR_SIZE*0.5)
        };

        entities->sensors[eid]->worldLeftSensor = (SDL_Point){
            entities->worldPositions[eid]->x - (ACTOR_SIZE*0.5),
            entities->worldPositions[eid]->y 
        };

        entities->sensors[eid]->worldRightSensor = (SDL_Point){
            entities->worldPositions[eid]->x + (ACTOR_SIZE*0.5),
            entities->worldPositions[eid]->y
        };
    }
    

    *entities->collisionRects[eid] = (SDL_Rect){
        entities->worldPositions[eid]->x - ACTOR_SIZE*0.5,
        entities->worldPositions[eid]->y - ACTOR_SIZE*0.5,
        ACTOR_SIZE,
        ACTOR_SIZE
    };
}

void actor_reset_data( Entities *entities, EntityId eid, SDL_Point initial_tile ) {
    *entities->worldPositions[eid] = (Position_f){
        initial_tile.x * TILE_SIZE + (ACTOR_SIZE*0.5), 
        initial_tile.y * TILE_SIZE + (ACTOR_SIZE*0.5)
    };

    if(entities->sensors[eid] != NULL){
        entities->sensors[eid]->worldTopSensor = (SDL_Point){
        entities->worldPositions[eid]->x,
        entities->worldPositions[eid]->y - (ACTOR_SIZE*0.5)
        };

        entities->sensors[eid]->worldBottomSensor = (SDL_Point){
            entities->worldPositions[eid]->x,
            entities->worldPositions[eid]->y + (ACTOR_SIZE*0.5)
        };

        entities->sensors[eid]->worldLeftSensor = (SDL_Point){
            entities->worldPositions[eid]->x - (ACTOR_SIZE*0.5),
            entities->worldPositions[eid]->y 
        };

        entities->sensors[eid]->worldRightSensor = (SDL_Point){
            entities->worldPositions[eid]->x + (ACTOR_SIZE*0.5),
            entities->worldPositions[eid]->y
        };
    }
    

    *entities->collisionRects[eid] = (SDL_Rect){
        entities->worldPositions[eid]->x - ACTOR_SIZE*0.5,
        entities->worldPositions[eid]->y - ACTOR_SIZE*0.5,
        ACTOR_SIZE,
        ACTOR_SIZE
    };

    *entities->directions[eid] = DIR_NONE;

    *entities->currentTiles[eid] = initial_tile;
    *entities->nextTiles[eid] = *entities->currentTiles[eid];
}




