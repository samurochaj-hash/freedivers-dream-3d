#ifndef GAME_H
#define GAME_H

typedef struct {
    float x, y, z;
    float yaw;
    float vx, vy, vz;
    float breath;
    float depth;
    int animation;
} Diver;

typedef enum {
    LEVEL_TROPICAL_OCEAN = 0,
    LEVEL_CORAL_REEF,
    LEVEL_DEEP_BLUE,
    LEVEL_UNDERWATER_CAVE,
    LEVEL_SUNKEN_SHIP
} LevelId;

void Player_Init(Diver *player);
void Player_Update(Diver *player, float forward, float vertical, float dt);
void Physics_Update(Diver *player, float dt);
void Camera_Init(void);
void Camera_Update(const Diver *player);
void Levels_Init(void);
void Levels_Update(void);
void Menu_Init(void);
void Menu_Update(void);
void Audio_Init(void);
void Audio_Update(void);

#endif
