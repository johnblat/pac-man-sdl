#ifndef utd_h
#define utd_h


#include <SDL2/SDL.h>
#include "flecs.h"


typedef float ChargeTimer;
typedef float BaseSpeed;
typedef float SpeedMultiplier;
typedef SDL_Point CurrentTile;
typedef SDL_Point NextTile;
typedef SDL_Point TargetTile;
typedef SDL_Rect CollisionRect;
typedef float DashTimer;
typedef float SlowTimer;
typedef float ActiveTimer;
typedef float DeathTimer;
typedef float StopTimer;
typedef float RespawnTimer;
typedef float InvincibilityTimer;
typedef float SpeedBoostTimer;
typedef uint32_t DotCount;
typedef ecs_entity_t MirrorEntityRef;
typedef SDL_bool IsActive;
typedef unsigned int ScoreValue;

// tags
typedef struct Ghost {} Ghost;
typedef struct Player {} Player;


#endif