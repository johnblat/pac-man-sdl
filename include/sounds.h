#ifndef SOUNDS_H
#define SOUNDS_H

#include <SDL2/SDL_mixer.h>

Mix_Music *g_Music = NULL;

//const char *gGameMusicFilename = "res/sounds/D"
const char *gGameMusicFilename = "res/sounds/Scruffy - World 0 & 1 (Pac-Man Arrangement) - arranged by Scruffy.ogg";
const char *gMenuMusicFilename = "res/sounds/Trust-Relax-Autopilot.mp3";

Mix_Chunk *g_GhostSound = NULL;
Mix_Chunk *g_GhostVulnerableSound = NULL;
Mix_Chunk *g_PacChompSound = NULL;
Mix_Chunk *g_GhostEatenYeahSound = NULL;
Mix_Chunk *g_GhostEatenSweetSound = NULL;
Mix_Chunk *g_GhostEatenCoolSound = NULL;
Mix_Chunk *g_GhostEatenGroovySound = NULL;
Mix_Chunk *g_PacDieOhNoSound = NULL;

Mix_Chunk *g_GhostEatenSounds[4];

typedef enum {
    GHOST_SOUND_CHANNEL = 7,// = 20,
    GHOST_VULN_CHANNEL = 6,// = 21,
    PAC_CHOMP_CHANNEL = 22
} SoundChannel;

#endif