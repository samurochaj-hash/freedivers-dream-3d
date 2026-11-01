#include "game.h"

void Player_Init(Diver *player)
{
    player->x = player->y = player->z = 0.0f;
    player->yaw = 0.0f;
    player->vx = player->vy = player->vz = 0.0f;
    player->breath = 100.0f;
    player->depth = 0.0f;
    player->animation = 0;
}
