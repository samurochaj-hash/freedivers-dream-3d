#ifndef MARINE_LIFE_H
#define MARINE_LIFE_H

#include <stdint.h>

#define MARINE_MAX_ENTITIES 24

typedef enum {
    MARINE_FISH = 0,
    MARINE_TURTLE,
    MARINE_DOLPHIN,
    MARINE_SHARK
} MarineType;

typedef enum {
    MARINE_SWIM_CRUISE = 0,
    MARINE_SWIM_TURN,
    MARINE_SWIM_FLEE,
    MARINE_SWIM_APPROACH
} MarineState;

typedef struct {
    MarineType type;
    MarineState state;
    float x, y, z;
    float heading;
    float speed;
    float turn_timer;
    float home_x, home_y, home_z;
    uint8_t active;
} MarineEntity;

typedef struct {
    MarineEntity entities[MARINE_MAX_ENTITIES];
    uint32_t count;
} MarineLife;

void MarineLife_Init(MarineLife *life);
int MarineLife_Add(MarineLife *life, MarineType type,
                   float x, float y, float z, float heading);
void MarineLife_Update(MarineLife *life, float dt,
                       float diver_x, float diver_y, float diver_z);

#endif
