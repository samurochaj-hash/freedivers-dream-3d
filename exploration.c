#include "exploration.h"
#include <string.h>

static void copy_name(char *dst, const char *src) {
    uint32_t i = 0;
    if (!src) src = "Objetivo";
    while (i < EXP_NAME_LENGTH - 1 && src[i]) {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = '\0';
}

static int find_discovery(const ExplorationProgress *p, uint16_t id) {
    uint32_t i;
    for (i = 0; i < p->discovery_count; ++i)
        if (p->discoveries[i].id == id) return (int)i;
    return -1;
}

static int find_collectible(const ExplorationProgress *p, uint16_t id) {
    uint32_t i;
    for (i = 0; i < p->collectible_count; ++i)
        if (p->collectibles[i].id == id) return (int)i;
    return -1;
}

void Exploration_Init(ExplorationProgress *progress) {
    if (!progress) return;
    memset(progress, 0, sizeof(*progress));
}

int Exploration_AddObjective(ExplorationProgress *progress,
                             ExplorationObjectiveType type,
                             const char *name, uint16_t target_id,
                             uint16_t required_count) {
    ExplorationObjective *o;
    if (!progress || progress->objective_count >= EXP_MAX_OBJECTIVES)
        return 0;
    o = &progress->objectives[progress->objective_count++];
    o->type = type;
    copy_name(o->name, name);
    o->target_id = target_id;
    o->required_count = required_count ? required_count : 1;
    o->current_count = 0;
    o->completed = 0;
    return 1;
}

int Exploration_RegisterDiscovery(ExplorationProgress *progress, uint16_t id) {
    uint32_t i;
    if (!progress) return 0;
    i = (uint32_t)find_discovery(progress, id);
    if ((int)i >= 0 && i < progress->discovery_count) return 0;
    if (progress->discovery_count >= EXP_MAX_DISCOVERIES) return 0;

    progress->discoveries[progress->discovery_count].id = id;
    progress->discoveries[progress->discovery_count].discovered = 1;
    progress->discovery_count++;

    for (i = 0; i < progress->objective_count; ++i) {
        ExplorationObjective *o = &progress->objectives[i];
        if (!o->completed && o->type == EXP_OBJ_DISCOVER &&
            o->target_id == id) {
            o->current_count++;
            if (o->current_count >= o->required_count) o->completed = 1;
        }
    }
    return 1;
}

int Exploration_CollectItem(ExplorationProgress *progress, uint16_t id) {
    uint32_t i;
    if (!progress || find_collectible(progress, id) >= 0) return 0;
    if (progress->collectible_count >= EXP_MAX_COLLECTIBLES) return 0;

    i = progress->collectible_count++;
    progress->collectibles[i].id = id;
    progress->collectibles[i].collected = 1;

    for (i = 0; i < progress->objective_count; ++i) {
        ExplorationObjective *o = &progress->objectives[i];
        if (!o->completed && o->type == EXP_OBJ_COLLECT &&
            (o->target_id == 0 || o->target_id == id)) {
            o->current_count++;
            if (o->current_count >= o->required_count) o->completed = 1;
        }
    }
    return 1;
}

int Exploration_ReachPoint(ExplorationProgress *progress, uint16_t point_id) {
    uint32_t i;
    int updated = 0;
    if (!progress) return 0;
    for (i = 0; i < progress->objective_count; ++i) {
        ExplorationObjective *o = &progress->objectives[i];
        if (!o->completed && o->type == EXP_OBJ_REACH &&
            o->target_id == point_id) {
            o->current_count = 1;
            o->completed = 1;
            updated = 1;
        }
    }
    return updated;
}

void Exploration_SetPhaseComplete(ExplorationProgress *progress) {
    uint32_t i;
    if (!progress) return;
    progress->phase_complete = 1;
    for (i = 0; i < progress->objective_count; ++i) {
        ExplorationObjective *o = &progress->objectives[i];
        if (!o->completed && o->type == EXP_OBJ_COMPLETE_PHASE) {
            o->current_count = 1;
            o->completed = 1;
        }
    }
}

int Exploration_IsMissionComplete(const ExplorationProgress *progress) {
    uint32_t i;
    if (!progress || progress->objective_count == 0) return 0;
    for (i = 0; i < progress->objective_count; ++i)
        if (!progress->objectives[i].completed) return 0;
    return 1;
}

uint8_t Exploration_GetCompletionPercent(const ExplorationProgress *progress) {
    uint32_t i, completed = 0;
    if (!progress || progress->objective_count == 0) return 0;
    for (i = 0; i < progress->objective_count; ++i)
        if (progress->objectives[i].completed) completed++;
    return (uint8_t)((completed * 100U) / progress->objective_count);
}
