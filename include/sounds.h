#ifndef SOUNDS_H
#define SOUNDS_H

#include <SDL2/SDL_mixer.h>

Mix_Music *g_Music = NULL;

//const char *gGameMusicFilename = "res/sounds/D"
//const char *gGameMusicFilename = "res/sounds/Dont-Worry-We-Got-Warp-Speed.mp3";
char gGameMusicFilename[ 64 ] = "res/sounds/That Andy Guy - DisFunkTional.ogg";

const char *gMenuMusicFilename = "res/sounds/optionstest.mp3";

Mix_Chunk *g_GhostSound = NULL;
Mix_Chunk *g_GhostVulnerableSound = NULL;
Mix_Chunk *g_PacChompSound = NULL;
Mix_Chunk *g_GhostEatenYeahSound = NULL;
Mix_Chunk *g_GhostEatenSweetSound = NULL;
Mix_Chunk *g_GhostEatenCoolSound = NULL;
Mix_Chunk *g_GhostEatenGroovySound = NULL;
Mix_Chunk *g_PacDieSound = NULL;
Mix_Chunk *g_PacRespawnSound = NULL;
Mix_Chunk *g_PacChompSound2 = NULL;
Mix_Chunk *g_GhostEatSound = NULL;
Mix_Chunk *g_PickupEaten = NULL;
Mix_Chunk *g_ExtraLifeSound = NULL;

SDL_bool gPacChomp2 = SDL_FALSE;

Mix_Chunk *g_GhostEatenSounds[4];

typedef enum {
    GHOST_SOUND_CHANNEL = 7,// = 20,
    GHOST_VULN_CHANNEL = 6,// = 21,
    PAC_CHOMP_CHANNEL = 15,
    PAC_CHOMP_CHANNEL2 = 14,
    GHOST_EAT_CHANNEL = 13,
    PICKUP_EAT_CHANNEL = 12,
    PAC_DIE_CHANNEL = 11,
    PAC_RESPAWN_CHANNEL = 10,
    EXTRA_LIFE_CHANNEL = 9
} SoundChannel;

#endif