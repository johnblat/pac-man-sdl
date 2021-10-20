#include "flecs.h"

typedef struct Message{
    char message[64];
    ecs_entity_t from;
    ecs_time_t time;
}Message;

typedef struct MessageQueue{
    Message *messages;
    size_t count;
} MessageQueue;