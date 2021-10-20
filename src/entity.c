#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "sounds.h"
#include "comparisons.h"
#include "stdio.h"
#include "jb_types.h"
#include "animation.h"
#include "ghostStates.h"
#include "states.h"
#include "render.h"
#include "targeting.h"
#include "entity.h"
#include "UI.h"
#include "globalData.h"
#include "userTypeDefinitions.h"
#include "flecs.h"

Direction opposite_directions[5] = {DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT, DIR_UP};


void allGhostsVulnerableStateEnter(){
    ECS_COMPONENT(gEcsWorld, GhostState);
    ECS_COMPONENT(gEcsWorld, AnimatedSprite);
    ECS_COMPONENT(gEcsWorld, Direction);
    ECS_COMPONENT(gEcsWorld, NextTile);
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    ECS_COMPONENT(gEcsWorld, SpeedMultiplier);

    ecs_query_t *query = ecs_query_init(gEcsWorld, &(ecs_query_desc_t){
        .filter.terms = {
            {ecs_id(GhostState)},
            {ecs_id(AnimatedSprite)},
            {ecs_id(Direction)},
            {ecs_id(NextTile)},
            {ecs_id(SpeedMultiplier)}
        }
    });

    GhostState *ghostStates;
    AnimatedSprite *animatedSprites;
    Direction *directions;
    NextTile *nextTiles;
    SpeedMultiplier *speedMultipliers;
    CurrentTile *currentTiles;

    ecs_iter_t it = ecs_query_iter(gEcsWorld, query);
    while(ecs_query_next(&it)){
        ghostStates = ecs_term(&it, GhostState, 1);
        nextTiles = ecs_term(&it, AnimatedSprite, 2);
        directions = ecs_term(&it, Direction, 3);
        nextTiles = ecs_term(&it, NextTile, 4);
        speedMultipliers = ecs_term(&it, SpeedMultiplier, 5);

        for(int i = 0; i < it.count; i++){
            if(ghostStates[i] == STATE_NORMAL){
                ghostStates[i] = STATE_VULNERABLE;
                vulnerableEnter(&animatedSprites[i], &directions[i], &currentTiles[i], &nextTiles[i], &speedMultipliers[i]);

            }
        }
    }

    gGhostVulnerableTimer = gLevelConfig->ghostVulnerableDuration;  
    g_NumGhostsEaten = 0;

}


unsigned int g_NumEntities = 0;

ecs_entity_t createPlayer(LevelConfig *gLevelConfig, AnimatedSprite animatedSprite ) {
    ecs_entity_t entityId = ecs_new_id(gEcsWorld);

    ECS_TAG(gEcsWorld, Player);
    ecs_add(gEcsWorld, entityId, Player);

    ECS_COMPONENT(gEcsWorld, Position);
    ECS_COMPONENT(gEcsWorld, Velocity);
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    ECS_COMPONENT(gEcsWorld, NextTile);
    ECS_COMPONENT(gEcsWorld, CollisionRect);
    ECS_COMPONENT(gEcsWorld, Direction);
    ECS_COMPONENT(gEcsWorld, BaseSpeed);
    ECS_COMPONENT(gEcsWorld, SpeedMultiplier);
    ECS_COMPONENT(gEcsWorld, AnimatedSprite);
    ECS_COMPONENT(gEcsWorld, RenderData);
    ECS_COMPONENT(gEcsWorld, ChargeTimer);
    ECS_COMPONENT(gEcsWorld, DashTimer);
    ECS_COMPONENT(gEcsWorld, SlowTimer);
    ECS_COMPONENT(gEcsWorld, InputMask);
    ECS_COMPONENT(gEcsWorld, CooldownStock);
    ECS_COMPONENT(gEcsWorld, InvincibilityTimer);
    ECS_COMPONENT(gEcsWorld, SpeedBoostTimer);
    ECS_COMPONENT(gEcsWorld, DeathTimer);
    ECS_COMPONENT(gEcsWorld, RespawnTimer);
    ECS_COMPONENT(gEcsWorld, Sensor);
    ECS_COMPONENT(gEcsWorld, IsActive);
    ECS_COMPONENT(gEcsWorld, StopTimer);

    Position position = {
        gLevelConfig->pacStartingTile.x * TILE_SIZE + (TILE_SIZE*0.5), 
        gLevelConfig->pacStartingTile.y * TILE_SIZE + (TILE_SIZE*0.5)
    };
    SDL_Point topSensor, bottomSensor, leftSensor, rightSensor;
    topSensor = (SDL_Point){
        position.x,
        position.y - (TILE_SIZE*0.5)
    };
    bottomSensor = (SDL_Point){
        position.x,
        position.y + (TILE_SIZE*0.5)
    };
    leftSensor = (SDL_Point){
        position.x - (TILE_SIZE*0.5),
        position.y 
    };
    rightSensor = (SDL_Point){
        position.x + (TILE_SIZE*0.5),
        position.y
    };
    RenderData rd = renderDataInit();
    unsigned int collisionRectSize = 20;

    ecs_set(gEcsWorld, entityId, Position, position);
    ecs_set(gEcsWorld, entityId, CurrentTile, gLevelConfig->pacStartingTile);
    ecs_set(gEcsWorld, entityId, Sensor, {
        topSensor,
        bottomSensor,
        leftSensor,
        rightSensor
    });
    ecs_set(gEcsWorld, entityId, Velocity, {0.0f, 0.0f});
    ecs_set(gEcsWorld, entityId, Direction, {DIR_NONE});
    ecs_set(gEcsWorld, entityId, NextTile, gLevelConfig->pacStartingTile);
    ecs_set(gEcsWorld, entityId, BaseSpeed, {gLevelConfig->baseSpeed});
    ecs_set(gEcsWorld, entityId, SpeedMultiplier, {1.0f});
    ecs_set(gEcsWorld, entityId, AnimatedSprite, animatedSprite);
    ecs_set(gEcsWorld, entityId, RenderData, rd);
    ecs_set(gEcsWorld, entityId, InputMask, {0b00000});
    ecs_set(gEcsWorld, entityId, ChargeTimer, {0.0f});
    ecs_set(gEcsWorld, entityId, DashTimer, {0.0f});
    ecs_set(gEcsWorld, entityId, SlowTimer, {0.0f});
    ecs_set(gEcsWorld, entityId, ChargeTimer, {0.0f});
    ecs_set(gEcsWorld, entityId, InvincibilityTimer, {0.0f});
    ecs_set(gEcsWorld, entityId, StopTimer, {0.0f});
    ecs_set(gEcsWorld, entityId, DeathTimer, {0.0f});
    ecs_set(gEcsWorld, entityId, RespawnTimer, {0.0f});
    ecs_set(gEcsWorld, entityId, IsActive, {SDL_FALSE});
    ecs_set(gEcsWorld, entityId, CooldownStock, {
        .cooldownDuration = 3.0f,
        .cooldownTimer = 0.0f,
        .currentNumStock = 3,
        .numStockCap = 3
    });
    ecs_set(gEcsWorld, entityId, CollisionRect, {
        position.x - collisionRectSize*0.5,
        position.y - collisionRectSize*0.5,
        collisionRectSize,
        collisionRectSize
    });

    return entityId;
}


ecs_entity_t createGhost(LevelConfig *gLevelConfig, AnimatedSprite animatedSprite, 
                        TargetingBehavior targetingBehavior ) {
    ECS_COMPONENT(gEcsWorld, Position);
    ECS_COMPONENT(gEcsWorld, Velocity);
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    ECS_COMPONENT(gEcsWorld, NextTile);
    ECS_COMPONENT(gEcsWorld, TargetTile);
    ECS_COMPONENT(gEcsWorld, CollisionRect);
    ECS_COMPONENT(gEcsWorld, Direction);
    ECS_COMPONENT(gEcsWorld, BaseSpeed);
    ECS_COMPONENT(gEcsWorld, SpeedMultiplier);
    ECS_COMPONENT(gEcsWorld, AnimatedSprite);
    ECS_COMPONENT(gEcsWorld, RenderData);
    ECS_COMPONENT(gEcsWorld, GhostState);
    ECS_COMPONENT(gEcsWorld, TargetingBehavior);
    ECS_COMPONENT(gEcsWorld, StopTimer);
    ECS_COMPONENT(gEcsWorld, DotCount);

    ecs_entity_t entityId = ecs_new_id(gEcsWorld);

    ECS_TAG(gEcsWorld, Ghost);
    ecs_add(gEcsWorld, entityId, Ghost);
    
    ecs_set(gEcsWorld, entityId, Position, {
        gLevelConfig->ghostPenTile.x * TILE_SIZE + (TILE_SIZE*0.5),
        gLevelConfig->ghostPenTile.y * TILE_SIZE + (TILE_SIZE*0.5)
    });
    ecs_set(gEcsWorld, entityId, CurrentTile, gLevelConfig->ghostPenTile);
    ecs_set(gEcsWorld, entityId, NextTile, gLevelConfig->ghostPenTile);
    ecs_set(gEcsWorld, entityId, TargetTile, gLevelConfig->ghostPenTile);
    Position *p = ecs_get(gEcsWorld, entityId, Position);
    unsigned int collisionRectSize = 20;
    ecs_set(gEcsWorld, entityId, CollisionRect, {
        p->x - collisionRectSize*0.5,
        p->y - collisionRectSize*0.5,
        collisionRectSize,
        collisionRectSize
    });
    ecs_set(gEcsWorld, entityId, Direction, {DIR_NONE});
    ecs_set(gEcsWorld, entityId, BaseSpeed, {gLevelConfig->baseSpeed});
    ecs_set(gEcsWorld, entityId, SpeedMultiplier, {0.85});
    ecs_set(gEcsWorld, entityId, Velocity, {0.0f,0.0f});
    ecs_set(gEcsWorld, entityId, TargetingBehavior, {targetingBehavior});
    ecs_set(gEcsWorld, entityId, AnimatedSprite, animatedSprite);
    RenderData rd = renderDataInit();
    ecs_set(gEcsWorld, entityId, RenderData, rd);
    ecs_set(gEcsWorld, entityId, GhostState, {STATE_LEAVE_PEN});
    ecs_set(gEcsWorld, entityId, StopTimer, {0.0f});
    ecs_set(gEcsWorld, entityId, DotCount, {0});

    return entityId;
}

ecs_entity_t createTempMirrorPlayer(ecs_entity_t playerId, float activeTime) {
    ecs_entity_t entityId = ecs_new_id(gEcsWorld);

    ECS_TAG(gEcsWorld, MirrorPlayer);
    ecs_add(gEcsWorld, entityId, MirrorPlayer);


    ECS_COMPONENT(gEcsWorld, Position);
    ECS_COMPONENT(gEcsWorld, Velocity);
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    ECS_COMPONENT(gEcsWorld, CollisionRect);
    ECS_COMPONENT(gEcsWorld, Direction);
    ECS_COMPONENT(gEcsWorld, AnimatedSprite);
    ECS_COMPONENT(gEcsWorld, RenderData);
    ECS_COMPONENT(gEcsWorld, ActiveTimer);
    ECS_COMPONENT(gEcsWorld, MirrorEntityRef);
    ECS_COMPONENT(gEcsWorld, IsActive);

    AnimatedSprite *playerAnimatedSprite = ecs_get(gEcsWorld, playerId, AnimatedSprite);
    RenderData rd = renderDataInit();
    unsigned int collisionRectSize = 20;
    Position *playerPosition = ecs_get(gEcsWorld, playerId, Position);
    CurrentTile *playerCurrentTile = ecs_get(gEcsWorld, playerId, CurrentTile);
    Direction *playerDirection = ecs_get(gEcsWorld, playerId, Direction);
    RenderData rd = renderDataInit();
    rd.alphaMod = 150;
    unsigned int collisionRectSize = 20;

    ecs_set(gEcsWorld, entityId, Position, *playerPosition);
    ecs_set(gEcsWorld, entityId, CurrentTile, *playerCurrentTile);
    ecs_set(gEcsWorld, entityId, Direction, {*playerDirection});
    ecs_set(gEcsWorld, entityId, Velocity, {0.0f, 0.0f});
    ecs_set(gEcsWorld, entityId, AnimatedSprite, *playerAnimatedSprite);
    ecs_set(gEcsWorld, entityId, CollisionRect, {
        playerPosition->x - collisionRectSize*0.5,
        playerPosition->y - collisionRectSize*0.5,
        collisionRectSize,
        collisionRectSize
    });
    ecs_set(gEcsWorld, entityId, MirrorEntityRef, {playerId});
    ecs_set(gEcsWorld, entityId, ActiveTimer, {activeTime});
    ecs_set(gEcsWorld, entityId, RenderData, rd);


    return entityId;
}

/**
 * Overwrite the first inactive entity. creates if none to overwrite
*/

ecs_entity_t overwriteInactiveTempMirrorPlayer(ecs_entity_t playerId, float activeTime ) {
    ECS_COMPONENT(gEcsWorld, Position);
    ECS_COMPONENT(gEcsWorld, Velocity);
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    ECS_COMPONENT(gEcsWorld, CollisionRect);
    ECS_COMPONENT(gEcsWorld, Direction);
    ECS_COMPONENT(gEcsWorld, AnimatedSprite);
    ECS_COMPONENT(gEcsWorld, RenderData);
    ECS_COMPONENT(gEcsWorld, ActiveTimer);
    ECS_COMPONENT(gEcsWorld, MirrorEntityRef);
    ECS_COMPONENT(gEcsWorld, IsActive);

    ecs_query_t *query = ecs_query_init(gEcsWorld, &(ecs_query_desc_t){
        .filter.terms = {
            {ecs_id(MirrorEntityRef)},
            {ecs_id(AnimatedSprite)},
            {ecs_id(Position)},
            {ecs_id(RenderData)},
            {ecs_id(IsActive)},
            {ecs_id(ActiveTimer)},
        }
    });

    ecs_iter_t it = ecs_query_iter(gEcsWorld, query);
    while(ecs_query_next(&it)){
        Position *positions = ecs_term(&it, Position, 1);
        CurrentTile *currentTiles = ecs_term(&it, CurrentTile, 2);
        Direction *directions = ecs_term(&it, Direction, 3);
        AnimatedSprite *animatedSprites = ecs_term(&it, AnimatedSprite, 4);
        RenderData *renderDatas = ecs_term(&it, RenderData, 5);
        ActiveTimer *activeTimers = ecs_term(&it, ActiveTimer, 6);
        IsActive *isActives = ecs_term(&it, IsActive, 7);
        MirrorEntityRef *mirrorEntityRefs = ecs_term(&it, MirrorEntityRef, 8);

        for(int i = 0; i < it.count; i++){
            if(activeTimers[i] <= 0.0f){
                AnimatedSprite *playerAnimatedSprite = ecs_get(gEcsWorld, playerId, AnimatedSprite);
                CurrentTile *playerCurrentTile = ecs_get(gEcsWorld, playerId, CurrentTile);
                Position *playerPosition = ecs_get(gEcsWorld, playerId, Position);
                Direction *playerDirection = ecs_get(gEcsWorld, playerId, Direction);

                currentTiles[i] = *playerCurrentTile;
                animatedSprites[i] = *playerAnimatedSprite;
                directions[i] = *playerDirection;
                renderDatas[i] = renderDataInit();
                renderDatas[i].alphaMod = 150;
                mirrorEntityRefs[i] = playerId;
                activeTimers[i] = activeTime;

                return i;
            }
        }
        ecs_entity_t entityId = createTempMirrorPlayer(playerId, activeTime);
        return entityId;
    }
}


ecs_entity_t createInitialTemporaryPickup(LevelConfig *gLevelConfig ) {
    ECS_COMPONENT(gEcsWorld, Position);
    ECS_COMPONENT(gEcsWorld, Velocity);
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    ECS_COMPONENT(gEcsWorld, NextTile);
    ECS_COMPONENT(gEcsWorld, CollisionRect);
    ECS_COMPONENT(gEcsWorld, Direction);
    ECS_COMPONENT(gEcsWorld, BaseSpeed);
    ECS_COMPONENT(gEcsWorld, SpeedMultiplier);
    ECS_COMPONENT(gEcsWorld, AnimatedSprite);
    ECS_COMPONENT(gEcsWorld, RenderData);
    ECS_COMPONENT(gEcsWorld, IsActive);
    ECS_COMPONENT(gEcsWorld, PickupType);
    ECS_COMPONENT(gEcsWorld, ActiveTimer);
    ECS_COMPONENT(gEcsWorld, DotCount);
    ECS_COMPONENT(gEcsWorld, ScoreValue);

    ecs_entity_t entityId = ecs_new_id(gEcsWorld);

    ECS_TAG(gEcsWorld, Pickup);
    ecs_add(gEcsWorld, entityId, Pickup);

    RenderData rd = renderDataInit();
    unsigned int collisionRectSize = 20;
    Position position = {
        gLevelConfig->pacStartingTile.x * TILE_SIZE + (TILE_SIZE*0.5), 
        gLevelConfig->pacStartingTile.y * TILE_SIZE + (TILE_SIZE*0.5)
    };
    AnimatedSprite animatedSprite = init_animation(0, 15, 1, 20);

    // initialize
    ecs_set(gEcsWorld, entityId, Position, position);
    ecs_set(gEcsWorld, entityId, Velocity, {0.0f, 0.0f});
    ecs_set(gEcsWorld, entityId, CurrentTile, gLevelConfig->pacStartingTile);
    ecs_set(gEcsWorld, entityId, NextTile, gLevelConfig->pacStartingTile);
    ecs_set(gEcsWorld, entityId, CollisionRect, {
        position.x - collisionRectSize*0.5,
        position.y - collisionRectSize*0.5,
        collisionRectSize,
        collisionRectSize
    });
    ecs_set(gEcsWorld, entityId, Direction, {DIR_LEFT});
    ecs_set(gEcsWorld, entityId, BaseSpeed, {gLevelConfig->baseSpeed});
    ecs_set(gEcsWorld, entityId, SpeedMultiplier, {0.5f});
    ecs_set(gEcsWorld, entityId, AnimatedSprite, animatedSprite);
    ecs_set(gEcsWorld, entityId, IsActive, {SDL_FALSE});
    ecs_set(gEcsWorld, entityId, RenderData, rd);
    ecs_set(gEcsWorld, entityId, PickupType, {POWER_PELLET_PICKUP});
    ecs_set(gEcsWorld, entityId, ScoreValue, {50});
    ecs_set(gEcsWorld, entityId, ActiveTimer, {0.0f});
    ecs_set(gEcsWorld, entityId, DotCount, {0});
    ecs_set(gEcsWorld, entityId, ScoreValue, {0});

    return entityId;
}


void ghostsProcess( Entities *entities, EntityId *playerIds, unsigned int numPlayers, TileMap *tilemap, float deltaTime, LevelConfig *gLevelConfig ) {
    TargetingBehavior **targetingBehaviors = entities->targetingBehaviors;
    GhostState **ghostStates = entities->ghostStates;


    for( int eid = 0; eid < g_NumEntities; eid++ ) {
        // skip non ghosts
        if( targetingBehaviors[ eid ] == NULL || ghostStates[ eid ] == NULL ) {
            continue;
        }
        //Ghost state machine processing
        switch( *ghostStates[ eid ] ) {
            case STATE_NORMAL:
                normal_process( entities, eid, playerIds, numPlayers, tilemap, gLevelConfig );
                break;
            case STATE_VULNERABLE:
                vulnerable_process(entities, eid , tilemap );
                break;
            case STATE_GO_TO_PEN:
                go_to_pen_process(entities, gLevelConfig, eid, tilemap );
                break;
            case STATE_LEAVE_PEN:
                leave_pen_process( entities, eid, tilemap );
                break;
            case STATE_STAY_PEN:
                stayPenProcess( entities, gLevelConfig, tilemap, eid );
                break;
            default:
                // something went wrong
                fprintf(stderr, "Entity %d not set to valid state. State: %d\n", eid, *ghostStates[ eid ] );
                break;
        }
        
        set_animation_row( entities->animatedSprites[ eid ], *entities->directions[eid] );

        

    }
}

ecs_entity_t createPowerPellet(AnimatedSprite animatedSprite, SDL_Point tile ) {
    ECS_COMPONENT(gEcsWorld, Position);
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    ECS_COMPONENT(gEcsWorld, PickupType);
    ECS_COMPONENT(gEcsWorld, ScoreValue);
    ECS_COMPONENT(gEcsWorld, AnimatedSprite);
    ECS_COMPONENT(gEcsWorld, RenderData);


    ecs_entity_t entityId = ecs_new_id(gEcsWorld);
    
    ECS_TAG(gEcsWorld, Pickup);
    ecs_add(gEcsWorld, entityId, Pickup);
    
    ecs_set(gEcsWorld, entityId, Position, {
        tile.x * TILE_SIZE + (TILE_SIZE*0.5),
        tile.y * TILE_SIZE + (TILE_SIZE*0.5),
    });
    ecs_set(gEcsWorld, entityId, CurrentTile, {
        tile.x, 
        tile.y
    });
    ecs_set(gEcsWorld, entityId, AnimatedSprite, animatedSprite);
    RenderData rd = renderDataInit();
    ecs_set(gEcsWorld, entityId, RenderData, rd);
    ecs_set(gEcsWorld, entityId, PickupType, {POWER_PELLET_PICKUP});
    ecs_set(gEcsWorld, entityId, ScoreValue, {50});

    return entityId;
}


void playerDotCollectionSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    char (*dots)[TILE_COLS] = it->ctx;

    CurrentTile *currentTiles = ecs_term(it, CurrentTile, 1);
    for(int i = 0; i < it->count; it++){
        if(dots[currentTiles[i].y][currentTiles[i].x] == 'x'){
            gPacChomp2 = !gPacChomp2;
            if( gPacChomp2 ) {
                Mix_PlayChannel(PAC_CHOMP_CHANNEL, g_PacChompSound2, 0 );
            }
            else {
                Mix_PlayChannel( PAC_CHOMP_CHANNEL2, g_PacChompSound, 0 );
            }
            dots[currentTiles[i].y][currentTiles[i].x] = ' ';
            gScore.score_number += DOT_SCORE_VALUE;
            g_NumDots -= 1;
        }
    }
}


void tempMirrorPlayerCollectDotProcess( Entities *entities, char dots[ TILE_ROWS ][ TILE_COLS ], Score *score ) {
    for( int eid = 0; eid < g_NumEntities; eid++ ) {
        if( entities->mirrorEntityRefs[ eid ] == NULL ) {
            continue;
        }
        // dont process if inactive
        if( *entities->activeTimers[eid] <= 0.0f ) {
            continue;
        }
        if( dots[entities->currentTiles[eid]->y][entities->currentTiles[eid]->x] == 'x') {
     
            //Mix_PlayChannel( -1, g_PacChompSound, 0 );

            // get rid of dot marker
            dots[entities->currentTiles[eid]->y][entities->currentTiles[eid]->x] = ' ';

            g_NumDots -= 1;
            
            score->score_number += 10;
        }
    }
}


void CooldownSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, CooldownStock);
    CooldownStock *cooldownStocks = ecs_term(it, CooldownStock, 1);

    for(int i = 0; i < it->count; i++){
        if( cooldownStocks[i].currentNumStock >= cooldownStocks[i].numStockCap ) {
            continue; 
        }
        if( cooldownStocks[i].currentNumStock < cooldownStocks[i].numStockCap ) {
            cooldownStocks[i].cooldownTimer -= it->delta_time;
        }
        if( cooldownStocks[i].cooldownTimer <= 0.0f ) {
            cooldownStocks[i].currentNumStock++;
            if( cooldownStocks[i].currentNumStock < cooldownStocks[i].numStockCap ) {
                cooldownStocks[i].cooldownTimer = cooldownStocks[i].cooldownDuration;

            }
        }
    }    
}



void overwriteSpeedBoostTimer(ecs_entity_t playerId, float speed, float duration){
    ECS_COMPONENT(gEcsWorld, SpeedBoostTimer);
    ecs_set(gEcsWorld, playerId, SpeedBoostTimer, {duration});
}


void speedBoostSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, SpeedBoostTimer);
    ECS_COMPONENT(gEcsWorld, SpeedMultiplier);

    SpeedBoostTimer *speedBoostTimers = ecs_term(it, SpeedBoostTimer, 1);
    SpeedMultiplier *speedMultipliers = ecs_term(it, SpeedMultiplier, 2);

    for(int i = 0; i < it->count; i++){
        if(speedBoostTimers[i] >= 0.0f ) {
            speedMultipliers[i] += 0.5f;
            speedBoostTimers[i] -= it->delta_time;
        }
    }
}


void deathResetSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, DeathTimer);
    ECS_COMPONENT(gEcsWorld, RenderData);
    ECS_COMPONENT(gEcsWorld, AnimatedSprite);
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    ECS_COMPONENT(gEcsWorld, NextTile);
    ECS_COMPONENT(gEcsWorld, Position);
    ECS_COMPONENT(gEcsWorld, Sensor);
    ECS_COMPONENT(gEcsWorld, RespawnTimer);
    ECS_COMPONENT(gEcsWorld, InvincibilityTimer);
    ECS_COMPONENT(gEcsWorld, Direction);
    ECS_COMPONENT(gEcsWorld, IsActive);

    DeathTimer *deathTimers = ecs_term(it, DeathTimer, 1);
    RenderData *renderDatas = ecs_term(it, RenderData, 2);
    AnimatedSprite *animatedSprites = ecs_term(it, AnimatedSprite, 3);
    CurrentTile *currentTiles = ecs_term(it, CurrentTile, 4);
    NextTile *nextTiles = ecs_term(it, NextTile, 5);
    Position *positions = ecs_term(it, Position, 6);
    Sensor *sensors = ecs_term(it, Sensor, 7);
    RespawnTimer *respawnTimers = ecs_term(it, RespawnTimer, 8);
    InvincibilityTimer *invincibilityTimers = ecs_term(it, InvincibilityTimer, 9);
    Direction *directions = ecs_term(it, Direction, 10);
    IsActive *isActive = ecs_term(it, IsActive, 11);

    LevelConfig *gLevelConfig = it->ctx;

    for(int i = 0; i < it->count; i++){

        if( deathTimers[i] <= 0.0f ){
            continue;
        }

        animatedSprites[i].current_anim_row = 2;
        deathTimers[i] -= it->delta_time;
        if( renderDatas[i].scale > 0.0f ) {
            if( deathTimers[i] < 1.5f ) {
                renderDatas[i].scale -= it->delta_time * 1.5;
            }
        }

        if( deathTimers[i] <= 0.0f && isActive[i] == SDL_TRUE ) {
            currentTiles[i] = gLevelConfig->pacStartingTile;
            //actors[i].current_tile.y -= 1;
            nextTiles[i] = gLevelConfig->pacStartingTile;
            positions[i] = (Position){
                gLevelConfig->pacStartingTile.x * TILE_SIZE + (TILE_SIZE*0.5),
                (gLevelConfig->pacStartingTile.y ) * TILE_SIZE + (TILE_SIZE*0.5),
            };
            sensors[i].worldTopSensor = (SDL_Point){
                positions[i].x,
                positions[i].y - (TILE_SIZE*0.5)
            };
            sensors[i].worldBottomSensor = (SDL_Point){
                positions[i].x,
                positions[i].y + (TILE_SIZE*0.5)
            };
            sensors[i].worldLeftSensor = (SDL_Point){
                positions[i].x - (TILE_SIZE*0.5),
                positions[i].y
            };
            sensors[i].worldRightSensor = (SDL_Point){
                positions[i].x + (TILE_SIZE*0.5),
                positions[i].y
            };

            directions[i] = DIR_NONE;
            nextTiles[i] = currentTiles[i];
            invincibilityTimers[ i ] = 5.0f;
            respawnTimers[i] = 0.5f;

            Mix_PlayChannel(PAC_RESPAWN_CHANNEL, g_PacRespawnSound, 0 );
        }
    }
}



void respawnTimerSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, RespawnTimer);
    ECS_COMPONENT(gEcsWorld, RenderData);

    RespawnTimer *respawnTimers = ecs_term(it, RespawnTimer, 1);
    RenderData *renderDatas = ecs_term(it, RenderData, 2);

    // Is There a way to filter out the timers before calling this system
    // At the system init phase, can we make a query to only
    // get entities where timer > 0.0f
    for(int i = 0; i < it->count; i++){
        if(respawnTimers[i] <= 0.0f ){
            continue;
        }

        respawnTimers[i] -= it->delta_time;
        if(renderDatas[i].scale < 1.0f ) {
            renderDatas[i].scale += it->delta_time * 3;
        }
    }
}


void placeAndFlipMirrorPlayerSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, MirrorEntityRef);
    ECS_COMPONENT(gEcsWorld, AnimatedSprite);
    ECS_COMPONENT(gEcsWorld, RenderData);
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    ECS_COMPONENT(gEcsWorld, Direction);
    ECS_COMPONENT(gEcsWorld, Position);
    ECS_COMPONENT(gEcsWorld, ActiveTimer);
    ECS_COMPONENT(gEcsWorld, IsActive);

    MirrorEntityRef *mirrorEntityRefs = ecs_term(it, MirrorEntityRef, 1);
    AnimatedSprite *animatedSprites = ecs_term(it, AnimatedSprite, 2);
    RenderData *renderDatas = ecs_term(it, RenderData, 3);
    CurrentTile *currentTiles = ecs_term(it, CurrentTile, 4);
    Direction *directions = ecs_term(it, Direction, 5);
    Position *positions = ecs_term(it, Position, 6);
    ActiveTimer *activeTimers = ecs_term(it, ActiveTimer, 7);
    IsActive *isActive = ecs_term(it, IsActive, 8);

    for(int i = 0; i < it->count; i++){
        if(activeTimers[i] <= 0.0f){
            continue;
        }
        ecs_entity_t playerId = mirrorEntityRefs[i];    
        AnimatedSprite *playerAnimatedSprite = ecs_get(gEcsWorld, playerId, AnimatedSprite);
        RenderData *playerRenderData = ecs_get(gEcsWorld, playerId, RenderData);
        CurrentTile *playerCurrentTile = ecs_get(gEcsWorld, playerId, CurrentTile);
        Direction *playerDirection = ecs_get(gEcsWorld, playerId, Direction);
        Position *playerPosition = ecs_get(gEcsWorld, playerId, Position);


        const int tileCols = TILE_COLS - 1; // accidentally gave map one more than it should have
        const int midTileX = tileCols*0.5;

        if( midTileX == playerCurrentTile->x ) {
            currentTiles[i].x = playerCurrentTile->x;
        }
        else if( playerCurrentTile->x < midTileX ) {
            int difference = midTileX - playerCurrentTile->x;
            currentTiles[i].x = midTileX + difference;
        }
        else if( playerCurrentTile->x > midTileX ) {
            int difference = playerCurrentTile->x - midTileX;
            currentTiles[i].x = midTileX - difference;
        }

        const int midWorldPointX = midTileX*TILE_SIZE + TILE_SIZE*0.5;

        directions[i] = *playerDirection;
        if( directions[i] == DIR_LEFT || directions[i] == DIR_RIGHT) {
            directions[i] = opposite_directions[directions[i] ];
        }

        positions[i].y = positions[i].y;

        if( midWorldPointX == playerPosition->x ) {
            positions[i].x = playerPosition->x ;
        }
        else if( playerPosition->x < midWorldPointX ) {
            int difference = midWorldPointX - playerPosition->x;
            positions[i].x = midWorldPointX + difference;
        }
        else if( playerPosition->x > midWorldPointX ) {
            int difference = playerPosition->x - midWorldPointX;
            positions[i].x = midWorldPointX - difference;
        }
        
        positions[i].x -= TILE_SIZE;
        positions[i].x = positions[i].x + TILE_SIZE/2;
        positions[i].y = positions[i].y + TILE_SIZE/2;

        renderDatas[i].alphaMod = 150;

        unsigned int flippedAnimRows[] = {1,0,2,3,5,4,7,6};
        unsigned int playerAnimRow = playerAnimatedSprite->current_anim_row;
        animatedSprites[i].current_anim_row = flippedAnimRows[playerAnimRow];

        animatedSprites[i].accumulator = playerAnimatedSprite->accumulator;
        animatedSprites[i].current_frame_col = playerAnimatedSprite->current_frame_col;
        animatedSprites[i].default_texture_atlas_id = playerAnimatedSprite->default_texture_atlas_id;
        animatedSprites[i].frame_interval = playerAnimatedSprite->frame_interval;
        animatedSprites[i].num_frames_col = playerAnimatedSprite->num_frames_col;
        animatedSprites[i].texture_atlas_id = playerAnimatedSprite->texture_atlas_id;

        activeTimers[i] -= it->delta_time;  

    }
}


void stopGhostsForDuration(float duration){
    ECS_TAG(gEcsWorld, Ghost);
    ECS_COMPONENT(gEcsWorld, StopTimer);

    ecs_query_t *query = ecs_query_init(gEcsWorld, &(ecs_query_desc_t){
        .filter.terms = {
            {Ghost},
            {ecs_id(StopTimer)},
        }
    });

    StopTimer *stopTimers;

    ecs_iter_t it = ecs_query_iter(gEcsWorld, query);
    while(ecs_query_next(&it)){
        stopTimers = ecs_term(&it, StopTimer, 1);
        for(int i = 0; i < it.count; i++){
            stopTimers[i] = duration;
        }
    }
 
}


void stopPlayersForDuration(float duration ){
    ECS_COMPONENT(gEcsWorld, StopTimer);
    StopTimer *playerStopTimer;

    for(int i = 0; i < gNumPlayers; i++){
        ecs_entity_t pid = gPlayerIds[i];
        playerStopTimer = ecs_get(gEcsWorld, pid, StopTimer);
        *playerStopTimer = duration;
    }
}


void stopTimersSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, StopTimer);

    StopTimer *stopTimers = ecs_term(it, StopTimer, 1);
    for(int i = 0; i < it->count; i++){
        if(stopTimers[i] <= 0.0f) continue;
        stopTimers[i] -= it->delta_time;
    }
}

void processStopTimers(Entities *entities, float deltaTime ) {
    for( int eid = 0; eid < g_NumEntities; eid++ ) {
        if( entities->stopTimers[eid] == NULL ) {
            continue;
        }
        if( *entities->stopTimers[eid] <= 0.0f ) {
            continue;
        } 
        *entities->stopTimers[eid] -= deltaTime;
    }
}

void makePlayerInvincibleForDuration(ecs_entity_t eid, float duration){
    ECS_COMPONENT(gEcsWorld, InvincibilityTimer);
    ecs_set(gEcsWorld, eid, InvincibilityTimer, {duration});
}


void invincibilityTimerSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, InvincibilityTimer);
    InvincibilityTimer *invincibilityTimers = ecs_term(gEcsWorld, InvincibilityTimer, 1);

    for(int i = 0; i < it->count; i++){
        if(invincibilityTimers[i] <= 0.0f) continue;
        invincibilityTimers[i] -= it->delta_time;
    }
}


void temporaryPickupSystem(ecs_iter_t *it){
    ECS_COMPONENT(gEcsWorld, PickupType);
    ECS_COMPONENT(gEcsWorld, ActiveTimer);
    ECS_COMPONENT(gEcsWorld, RenderData);
    ECS_COMPONENT(gEcsWorld, ActiveTimer);
    ECS_COMPONENT(gEcsWorld, DotCount);
    ECS_COMPONENT(gEcsWorld, CurrentTile);
    ECS_COMPONENT(gEcsWorld, NextTile);
    ECS_COMPONENT(gEcsWorld, IsActive);
    ECS_COMPONENT(gEcsWorld, CollisionRect);

    PickupType *pickupTypes = ecs_term(it, PickupType, 1);
    ActiveTimer *activeTimers = ecs_term(it, ActiveTimer, 2);
    RenderData *renderDatas = ecs_term(it, RenderData, 3);
    ActiveTimer *activeTimers = ecs_term(it, ActiveTimer, 4);
    DotCount *dotCounts = ecs_term(it, DotCount, 5);
    CurrentTile *currentTiles = ecs_term(it, CurrentTile, 6);
    NextTile *nextTiles = ecs_term(it, NextTile, 7);
    IsActive *isActives = ecs_term(it, IsActive, 8);
    CollisionRect *collisionRects = ecs_term(it, CollisionRect, 8);

    TileMap *tilemap = it->ctx;

    for(int i = 0; i < it->count; i++){
        ecs_entity_t eid = it->entities[i];
        if(activeTimers[i] <= 0.0f){
            renderDatas[i].alphaMod = 0;
            continue;
        }
        // pickup is active
        int numDotsEaten = g_StartingNumDots - g_NumDots;
        if( dotCounts[ i ]  <= numDotsEaten ) {
            activeTimers[ i ] -= it->delta_time;
            renderDatas[ i ].alphaMod = 255;
            // issue if more than one active
            if(activeTimers[i] < 3.0f) {
                blinkProcess(&pickupBlink, it->delta_time);
                renderDatas[i].alphaMod = pickupBlink.values[pickupBlink.current_value_idx];
            }
            // move it
            if( points_equal( nextTiles[i], currentTiles[i] ) ) {
                set_random_direction_and_next_tile( eid, tilemap );    
            }
            EntityId playerId;
            for( int i = 0; i < gNumPlayers; i++ ) {
                playerId = gPlayerIds[ i ];
                if( isActives[playerId] == SDL_FALSE ) {
                    continue;
                }
                // player picks up
                CollisionRect *playerCollisionRect = ecs_get(gEcsWorld, playerId, CollisionRect);
                if(entitiesIntersecting(&collisionRects[i], playerCollisionRect)) {
                    activeTimers[ i ] = 0.0f;
                    gScore.score_number += dotCounts[ i ];

                    Mix_PlayChannel( PICKUP_EAT_CHANNEL, g_PickupEaten, 0 );

                    for( int i = 0; i < g_NumTimedMessages; i++ ) {
                        if( g_TimedMessages[ i ].remainingTime <= 0.0f ) {
                            g_TimedMessages[ i ].remainingTime = 0.85f;
                            g_TimedMessages[ i ].world_position = tile_grid_point_to_world_point( currentTiles[playerId] );
                            snprintf( g_TimedMessages[ i ].message, 8, "%d", dotCounts[i] );
                            g_TimedMessages[ i ].color.r = 255 ;
                            g_TimedMessages[ i ].color.g = 255;
                            g_TimedMessages[ i ].color.b = 255;
                            SDL_Surface *msgSurface = TTF_RenderText_Solid( g_TimedMessages[ i ].font,  g_TimedMessages[ i ].message, g_TimedMessages[ i ].color );
                            g_TimedMessages[ i ].messageTexture = SDL_CreateTextureFromSurface( gRenderer, msgSurface );
                            g_TimedMessages[ i ].render_dest_rect.x = world_point_to_screen_point(g_TimedMessages[ i ].world_position, tilemap->tm_screen_position).x;
                            g_TimedMessages[ i ].render_dest_rect.y = world_point_to_screen_point(g_TimedMessages[ i ].world_position, tilemap->tm_screen_position).y;
                            g_TimedMessages[ i ].render_dest_rect.w = msgSurface->w;
                            g_TimedMessages[ i ].render_dest_rect.h = msgSurface->h;
                            SDL_FreeSurface( msgSurface );
                            g_TimedMessages[ i ].l = lerpInit( g_TimedMessages[ i ].world_position.y - TILE_SIZE*1.25, g_TimedMessages[ i ].world_position.y, 0.33f );

                            break;
                        }
                    }
                    switch( pickupTypes[ i ] ) {
                        case FRUIT_PICKUP:
                            break; // do nothing. Just a score
                        case MIRROR_PICKUP:
                            // look for available entity to overwrite
                            overwriteInactiveTempMirrorPlayer(playerId, 8.0f);
                            break;
                        case POWER_PELLET_PICKUP:
                            allGhostsVulnerableStateEnter();
                            break;
                        case SPEED_BOOST_PICKUP:
                            overwriteSpeedBoostTimer(playerId, gBaseSpeed * 1.2, 8.0f);
                            break;
                        case SHIELD_PICKUP:
                            makePlayerInvincibleForDuration(playerId, 10.0f);
                            break;
                        case STOP_GHOSTS_PICKUP:
                            stopGhostsForDuration(7.0f);
                            break;
                        case NONE_PICKUP:
                            break;
                    }
                    break;
                }
            }
        }
        else {
            renderDatas[i].alphaMod = 0;
        }
    }
}


SDL_bool entitiesIntersecting(CollisionRect *cr1, CollisionRect *cr2){
    SDL_Rect resultRect;
    if ( SDL_IntersectRect(cr1, cr2, &resultRect) ){
        return SDL_TRUE;
    }
    return SDL_FALSE;
}