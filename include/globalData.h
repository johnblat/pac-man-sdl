#ifndef GLOBAL_H
#define GLOBAL_H

#include <SDL2/SDL.h>
#include <inttypes.h>
#include <entity.h>

SDL_bool g_show_debug_info = SDL_FALSE;

SDL_bool gIsFullscreen = SDL_FALSE;

int gQuit = 0;

// TIMER USED FOR VULNERABILITY STATE
float gGhostVulnerableTimer = 0.0f;
// GHOST BEHAVIOR TIMER FOR CURRENT GLOBAL GHOST MODE
float gGhostModeTimer = 0.0f;

EntityId gPlayerIds[ 2 ];
unsigned int gNumPlayers = 0;

SDL_Color pac_color = {200,150,0};
SDL_Color white = {200,200,255};


// used to track progress in level
unsigned int g_NumDots = 0;
unsigned int g_StartingNumDots = 0;

uint8_t g_NumGhostsEaten = 0;
unsigned int g_GhostPointValues[] = { 400, 800, 1600, 3200 };


SDL_Window *gWindow = NULL;
TTF_Font *gFont = NULL; 


#endif 