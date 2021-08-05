#ifndef GHOST_STATES_H
#define GHOST_STATES_H

typedef enum {
    STATE_NORMAL,
    STATE_VULNERABLE,
    STATE_GO_TO_PEN,
    STATE_LEAVE_PEN,
    STATE_STAY_PEN // will wander around pen
} GhostState;

typedef enum {
    SHADOW_BEHAVIOR,
    AMBUSH_BEHAVIOR,
    MOODY_BEHAVIOR,
    POKEY_BEHAVIOR
} TargetingBehavior;

#endif