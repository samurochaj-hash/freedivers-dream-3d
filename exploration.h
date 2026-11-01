#ifndef EXPLORATION_H
#define EXPLORATION_H

#include <stdint.h>

#define EXP_MAX_OBJECTIVES 12
#define EXP_MAX_DISCOVERIES 32
#define EXP_MAX_COLLECTIBLES 24
#define EXP_NAME_LENGTH 32

typedef enum {
    EXP_OBJ_DISCOVER = 0,
    EXP_OBJ_COLLECT,
    EXP_OBJ_REACH,
    EXP_OBJ_COMPLETE_PHASE
} ExplorationObjectiveType;

typedef struct {
    ExplorationObjectiveType type;
    char name[EXP_NAME_LENGTH];
    uint16_t target_id;
    uint16_t required_count;
    uint16_t current_count;
    uint8_t completed;
} ExplorationObjective;

typedef struct {
    uint16_t id;
    uint8_t discovered;
} ExplorationDiscovery;

typedef struct {
    uint16_t id;
    uint8_t collected;
} ExplorationCollectible;

typedef struct {
    ExplorationObjective objectives[EXP_MAX_OBJECTIVES];
    ExplorationDiscovery discoveries[EXP_MAX_DISCOVERIES];
    ExplorationCollectible collectibles[EXP_MAX_COLLECTIBLES];
    uint8_t objective_count;
    uint8_t discovery_count;
    uint8_t collectible_count;
    uint8_t phase_complete;
} ExplorationProgress;

void Exploration_Init(ExplorationProgress *progress);
int Exploration_AddObjective(ExplorationProgress *progress,
                             ExplorationObjectiveType type,
                             const char *name, uint16_t target_id,
                             uint16_t required_count);
int Exploration_RegisterDiscovery(ExplorationProgress *progress, uint16_t id);
int Exploration_CollectItem(ExplorationProgress *progress, uint16_t id);
int Exploration_ReachPoint(ExplorationProgress *progress, uint16_t point_id);
void Exploration_SetPhaseComplete(ExplorationProgress *progress);
int Exploration_IsMissionComplete(const ExplorationProgress *progress);
uint8_t Exploration_GetCompletionPercent(const ExplorationProgress *progress);

#endif
