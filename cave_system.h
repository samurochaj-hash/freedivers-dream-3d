#ifndef CAVE_SYSTEM_H
#define CAVE_SYSTEM_H

#include <stdint.h>

#define CAVE_MAX_ROOMS 8
#define CAVE_MAX_WALLS 32
#define CAVE_MAX_PORTALS 12
#define CAVE_MAX_PROPS 32

typedef struct {
    float min_x, min_y, min_z;
    float max_x, max_y, max_z;
} CaveAABB;

typedef struct {
    float x, y, z;
    float radius;
    uint8_t active;
    uint8_t type;
} CaveProp;

typedef struct {
    CaveAABB bounds;
    float fog_strength;
    float light_strength;
    uint8_t active;
} CaveRoom;

typedef struct {
    CaveAABB bounds;
    uint8_t active;
} CaveWall;

typedef struct {
    float x, y, z;
    float radius;
    int16_t room_a;
    int16_t room_b;
    uint8_t active;
} CavePortal;

typedef struct {
    CaveRoom rooms[CAVE_MAX_ROOMS];
    CaveWall walls[CAVE_MAX_WALLS];
    CavePortal portals[CAVE_MAX_PORTALS];
    CaveProp props[CAVE_MAX_PROPS];
    uint32_t room_count;
    uint32_t wall_count;
    uint32_t portal_count;
    uint32_t prop_count;
} CaveSystem;

void CaveSystem_Init(CaveSystem *cave);
int CaveSystem_AddRoom(CaveSystem *cave, CaveAABB bounds,
                       float fog_strength, float light_strength);
int CaveSystem_AddWall(CaveSystem *cave, CaveAABB bounds);
int CaveSystem_AddPortal(CaveSystem *cave, float x, float y, float z,
                         float radius, int16_t room_a, int16_t room_b);
int CaveSystem_AddProp(CaveSystem *cave, float x, float y, float z,
                       float radius, uint8_t type);
int CaveSystem_PointBlocked(const CaveSystem *cave, float x, float y, float z);
int CaveSystem_FindRoom(const CaveSystem *cave, float x, float y, float z);
int CaveSystem_FindPortal(const CaveSystem *cave, float x, float y, float z);

#endif
