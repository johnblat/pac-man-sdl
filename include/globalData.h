#ifndef GLOBAL_H
#define GLOBAL_H

#include <SDL2/SDL.h>
#include <inttypes.h>
#include <entity.h>
#include "jb_types.h"
#include "programState.h"
#include "gamePlayingState.h"
#include "UI.h"

SDL_bool g_show_debug_info = SDL_FALSE;

SDL_bool gIsFullscreen = SDL_TRUE;

unsigned int gCurrentLevel;

Score gScore;

int gQuit = 0;

// TIMER USED FOR VULNERABILITY STATE
float gGhostVulnerableTimer = 0.0f;
// GHOST BEHAVIOR TIMER FOR CURRENT GLOBAL GHOST MODE
float gGhostModeTimer = 0.0f;

EntityId gPlayerIds[ 4 ];
unsigned int gNumPlayers = 0;

EntityId gGhostIds[ 8 ];
unsigned int gNumGhosts = 0;

SDL_Color pac_color = {200,150,0};
SDL_Color white = {200,200,255};


// used to track progress in level
unsigned int g_NumDots = 0;
unsigned int g_StartingNumDots = 0;

uint8_t g_NumGhostsEaten = 0;
unsigned int g_GhostPointValues[] = { 200, 400, 800, 1600 };


unsigned int gBaseSpeed = 0;

unsigned int gLivesRemaining = 0;

LivesRemainingUI gLivesRemainingUI;

SDL_Window *gWindow = NULL;
TTF_Font *gFont = NULL; 

// STATES
ProgramState gProgramState = MENU_PROGRAM_STATE;
MenuState gMenuState = TITLE_SCREEN_MENU_STATE;
GamePlayingState gGamePlayingState = GAME_PLAYING;

int gExtraLifeMilestones[4] = {10000,50000,100000,200000};
unsigned int gNumExtraLifeMilestones = 4;
unsigned int gCurrentExtraLifeMilestoneIdx = 0;


// cheat
SDL_KeyCode MoreLivesCheatCode[5] = {SDLK_c, SDLK_h, SDLK_e, SDLK_a, SDLK_t };
unsigned int moreLivesCheatCodeIdx = 0;
unsigned int numMoreLivesCheatCodeKeys = 5;
SDL_bool moreLivesCheatCodeEnabled = SDL_FALSE;

unsigned int gInitialLivesRemaining = 2;
unsigned int gCheatAdditionalLivesRemaining = 0;
// viewport
// int gViewportWidth = 1920;
// int gViewportHeight = 1080;

// int gCanvasWidth = SCREEN_WIDTH;
// int gCanvasHeight = SCREEN_HEIGHT;


#endif 