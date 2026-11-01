#include "cave_system.h"

static int point_in_aabb(const CaveAABB *b, float x, float y, float z) {
    return x >= b->min_x && x <= b->max_x &&
           y >= b->min_y && y <= b->max_y &&
           z >= b->min_z && z <= b->max_z;
}

static int valid_bounds(CaveAABB b) {
    return b.min_x < b.max_x && b.min_y < b.max_y && b.min_z < b.max_z;
}

void CaveSystem_Init(CaveSystem *cave) {
    uint32_t i;
    if (!cave) return;
    cave->room_count = cave->wall_count = 0;
    cave->portal_count = cave->prop_count = 0;
    for (i = 0; i < CAVE_MAX_ROOMS; ++i) cave->rooms[i].active = 0;
    for (i = 0; i < CAVE_MAX_WALLS; ++i) cave->walls[i].active = 0;
    for (i = 0; i < CAVE_MAX_PORTALS; ++i) cave->portals[i].active = 0;
    for (i = 0; i < CAVE_MAX_PROPS; ++i) cave->props[i].active = 0;
}

int CaveSystem_AddRoom(CaveSystem *cave, CaveAABB bounds,
                       float fog_strength, float light_strength) {
    CaveRoom *r;
    if (!cave || cave->room_count >= CAVE_MAX_ROOMS || !valid_bounds(bounds))
        return -1;
    r = &cave->rooms[cave->room_count];
    r->bounds = bounds;
    r->fog_strength = fog_strength < 0.0f ? 0.0f :
                      (fog_strength > 1.0f ? 1.0f : fog_strength);
    r->light_strength = light_strength < 0.0f ? 0.0f :
                        (light_strength > 1.0f ? 1.0f : light_strength);
    r->active = 1;
    return (int)cave->room_count++;
}

int CaveSystem_AddWall(CaveSystem *cave, CaveAABB bounds) {
    CaveWall *w;
    if (!cave || cave->wall_count >= CAVE_MAX_WALLS || !valid_bounds(bounds))
        return 0;
    w = &cave->walls[cave->wall_count++];
    w->bounds = bounds;
    w->active = 1;
    return 1;
}

int CaveSystem_AddPortal(CaveSystem *cave, float x, float y, float z,
                         float radius, int16_t room_a, int16_t room_b) {
    CavePortal *p;
    if (!cave || cave->portal_count >= CAVE_MAX_PORTALS || radius <= 0.0f)
        return 0;
    if (room_a < 0 || room_b < 0 ||
        room_a >= (int16_t)cave->room_count ||
        room_b >= (int16_t)cave->room_count) return 0;
    p = &cave->portals[cave->portal_count++];
    p->x = x; p->y = y; p->z = z; p->radius = radius;
    p->room_a = room_a; p->room_b = room_b; p->active = 1;
    return 1;
}

int CaveSystem_AddProp(CaveSystem *cave, float x, float y, float z,
                       float radius, uint8_t type) {
    CaveProp *p;
    if (!cave || cave->prop_count >= CAVE_MAX_PROPS || radius <= 0.0f)
        return 0;
    p = &cave->props[cave->prop_count++];
    p->x = x; p->y = y; p->z = z;
    p->radius = radius; p->type = type; p->active = 1;
    return 1;
}

int CaveSystem_PointBlocked(const CaveSystem *cave, float x, float y, float z) {
    uint32_t i;
    if (!cave) return 1;
    for (i = 0; i < cave->wall_count; ++i) {
        if (cave->walls[i].active &&
            point_in_aabb(&cave->walls[i].bounds, x, y, z))
            return 1;
    }
    return 0;
}

int CaveSystem_FindRoom(const CaveSystem *cave, float x, float y, float z) {
    uint32_t i;
    if (!cave) return -1;
    for (i = 0; i < cave->room_count; ++i) {
        if (cave->rooms[i].active &&
            point_in_aabb(&cave->rooms[i].bounds, x, y, z))
            return (int)i;
    }
    return -1;
}

int CaveSystem_FindPortal(const CaveSystem *cave, float x, float y, float z) {
    uint32_t i;
    if (!cave) return -1;
    for (i = 0; i < cave->portal_count; ++i) {
        const CavePortal *p = &cave->portals[i];
        float dx, dy, dz;
        if (!p->active) continue;
        dx = x - p->x; dy = y - p->y; dz = z - p->z;
        if (dx*dx + dy*dy + dz*dz <= p->radius*p->radius)
            return (int)i;
    }
    return -1;
}
