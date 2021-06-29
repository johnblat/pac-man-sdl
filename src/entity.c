#include <SDL2/SDL.h>
#include "stdio.h"
#include "jb_types.h"
#include "animation.h"
#include "states.h"
#include "render.h"
#include "targeting.h"
#include "resources.h"
#include "actor.h"
#include "entity.h"


unsigned int g_NumEntities = 0;

EntityId createPlayer( Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite, RenderClipFromTextureAtlas *renderClip ) {
    EntityId entityId = g_NumEntities;
    g_NumEntities++;

    // allocate
    entities->positions      [ entityId ] = (Position *)                malloc(sizeof(Position));
    entities->actors         [ entityId ] = ( Actor * )                 malloc( sizeof( Actor ) );
    entities->animatedSprites[ entityId ] = (AnimatedSprite * )         malloc( sizeof( Actor ) );
    entities->render_clips   [ entityId ] = (RenderClipFromTextureAtlas *)malloc(sizeof(RenderClipFromTextureAtlas ) );
    entities->chargeTimers   [ entityId ] = (float * )                  malloc(sizeof(float));
    entities->dashTimers     [ entityId ] = (float * )                  malloc(sizeof(float ) );
    entities->slowTimers     [ entityId ] = ( float * )malloc(sizeof(float ) );
    entities->inputMasks     [ entityId ] = (uint8_t * ) malloc(sizeof( uint8_t ) );

    //initialize
    // position
    entities->positions[ entityId ]->current_tile = levelConfig->pacStartingTile;
    entities->positions[ entityId ]->world_position.x = levelConfig->pacStartingTile.x * TILE_SIZE;
    entities->positions[ entityId ]->world_position.y = levelConfig->pacStartingTile.y * TILE_SIZE;

    entities->positions[ entityId ]->world_center_point.x = ( int ) entities->positions[ entityId ]->world_position.x + ( ACTOR_SIZE / 2 );
    entities->positions[ entityId ]->world_center_point.y = ( int ) entities->positions[ entityId ]->world_position.y + ( ACTOR_SIZE / 2 );
    //actor
    entities->actors[ entityId ]->current_tile = levelConfig->pacStartingTile;
    entities->actors[ entityId ]->world_position.x = levelConfig->pacStartingTile.x * TILE_SIZE;
    entities->actors[ entityId ]->world_position.y = levelConfig->pacStartingTile.y * TILE_SIZE;

    entities->actors[ entityId ]->world_center_point.x = ( int ) entities->actors[ entityId ]->world_position.x + ( ACTOR_SIZE / 2 );
    entities->actors[ entityId ]->world_center_point.y = ( int ) entities->actors[ entityId ]->world_position.y + ( ACTOR_SIZE / 2 );

    entities->actors[ entityId ]->world_top_sensor.x = entities->positions[ entityId ]->world_position.x + ( ACTOR_SIZE / 2 );
    entities->actors[ entityId ]->world_top_sensor.y = entities->positions[ entityId ]->world_position.y;

    entities->actors[ entityId ]->world_bottom_sensor.x = entities->positions[ entityId ]->world_position.x + ( ACTOR_SIZE / 2 );
    entities->actors[ entityId ]->world_bottom_sensor.y = entities->positions[ entityId ]->world_position.y + ACTOR_SIZE;

    entities->actors[ entityId ]->world_left_sensor.x = entities->positions[ entityId ]->world_position.x;
    entities->actors[ entityId ]->world_left_sensor.y = entities->positions[ entityId ]->world_position.y + ( ACTOR_SIZE / 2 );

    entities->actors[ entityId ]->world_right_sensor.x = entities->positions[ entityId ]->world_position.x + ACTOR_SIZE;
    entities->actors[ entityId ]->world_right_sensor.y = entities->positions[ entityId ]->world_position.y + ( ACTOR_SIZE / 2 );    
    
    entities->actors[ entityId ]->velocity.x = 0.0f;
    entities->actors[ entityId ]->velocity.y = 0.0f;

    entities->actors[ entityId ]->direction = DIR_NONE;

    entities->actors[ entityId ]->next_tile = entities->positions[ entityId ]->current_tile;
    entities->actors[ entityId ]->next_tile = entities->positions[ entityId ]->current_tile;

    entities->actors[ entityId ]->base_speed = levelConfig->baseSpeed;
    entities->actors[ entityId ]->speed_multp = 1.0f;

    entities->animatedSprites[ entityId ] = animatedSprite;
    entities->render_clips[ entityId ] = renderClip;

    *entities->chargeTimers[ entityId ] = 0.0f;
    *entities->dashTimers[ entityId ] = 0.0f;
    *entities->slowTimers[ entityId ] = 0.0f;

    *entities->inputMasks[ entityId ] = 0b00000;

    return entityId;
}

EntityId createGhost(  Entities *entities, LevelConfig *levelConfig, AnimatedSprite *animatedSprite, RenderClipFromTextureAtlas *renderClip, TargetingBehavior targetingBehavior ) {
    EntityId entityId = g_NumEntities;
    g_NumEntities++;

    // allocate
    entities->positions      [ entityId ] = (Position *)                malloc(sizeof(Position));
    entities->actors         [ entityId ] = ( Actor * )                 malloc( sizeof( Actor ) );
    entities->animatedSprites[ entityId ] = (AnimatedSprite * )         malloc( sizeof( Actor ) );
    entities->render_clips   [ entityId ] = (RenderClipFromTextureAtlas *)malloc(sizeof(RenderClipFromTextureAtlas ) );
    entities->targetingBehaviors     [ entityId ] = (TargetingBehavior *)              malloc(sizeof(TargetingBehavior));
    entities->ghostStates[ entityId] = (GhostState *)malloc(sizeof(GhostState));

    //initialize
    // position
    entities->positions[ entityId ]->current_tile = levelConfig->pacStartingTile;
    entities->positions[ entityId ]->world_position.x = levelConfig->pacStartingTile.x * TILE_SIZE;
    entities->positions[ entityId ]->world_position.y = levelConfig->pacStartingTile.y * TILE_SIZE;

    entities->positions[ entityId ]->world_center_point.x = ( int ) entities->positions[ entityId ]->world_position.x + ( ACTOR_SIZE / 2 );
    entities->positions[ entityId ]->world_center_point.y = ( int ) entities->positions[ entityId ]->world_position.y + ( ACTOR_SIZE / 2 );
    //actor
    entities->actors[ entityId ]->current_tile = levelConfig->pacStartingTile;
    entities->actors[ entityId ]->world_position.x = levelConfig->pacStartingTile.x * TILE_SIZE;
    entities->actors[ entityId ]->world_position.y = levelConfig->pacStartingTile.y * TILE_SIZE;

    entities->actors[ entityId ]->world_center_point.x = ( int ) entities->actors[ entityId ]->world_position.x + ( ACTOR_SIZE / 2 );
    entities->actors[ entityId ]->world_center_point.y = ( int ) entities->actors[ entityId ]->world_position.y + ( ACTOR_SIZE / 2 );

    entities->actors[ entityId ]->velocity.x = 0.0f;
    entities->actors[ entityId ]->velocity.y = 0.0f;

    entities->actors[ entityId ]->direction = DIR_NONE;

    entities->actors[ entityId ]->next_tile = entities->positions[ entityId ]->current_tile;
    entities->actors[ entityId ]->next_tile = entities->positions[ entityId ]->current_tile;

    entities->actors[ entityId ]->base_speed = levelConfig->baseSpeed;
    entities->actors[ entityId ]->speed_multp = 0.85f;

    entities->animatedSprites[ entityId ] = animatedSprite;
    entities->render_clips[ entityId ] = renderClip;

    entities->ghostStates[ entityId ] = STATE_NORMAL;
    entities->targetingBehaviors[ entityId ]->targetingBehavior = targetingBehavior;

    return entityId;
}

void ghostsProcess( Entities *entities, EntityId *playerIds, unsigned int numPlayers, TileMap *tilemap, float deltaTime ) {
    Actor *actors[ MAX_NUM_ENTITIES ] = entities->actors;
    TargetingBehavior *targetingBehaviors[ MAX_NUM_ENTITIES ] = entities->targetingBehaviors;
    GhostState *ghostStates[ MAX_NUM_ENTITIES ] = entities->ghostStates;


    for( int eid = 0; eid < g_NumEntities; eid++ ) {
        // skip non ghosts
        if( actors[ eid ] == NULL || targetingBehaviors[ eid ] == NULL || ghostStates[ eid ] == NULL ) {
            continue;
        }
        //Ghost state machine processing
        switch( *ghostStates[ eid ] ) {
            case STATE_NORMAL:
                normal_process( entities, eid, playerIds, numPlayers, tilemap );
                break;
            case STATE_VULNERABLE:
                vulnerable_process(entities, eid , tilemap );
                break;
            case STATE_GO_TO_PEN:
                go_to_pen_process(entities, eid, tilemap );
                break;
            case STATE_LEAVE_PEN:
                leave_pen_process( entities, eid, tilemap );
                break;
            default:
                // something went wrong
                fprintf(stderr, "Entity %d not set to valid state. State: %d\n", eid, *ghostStates[ eid ] );
                break;
        }
        
        set_animation_row( entities->animatedSprites[ eid ], entities->actors[ eid ] );

    }
}