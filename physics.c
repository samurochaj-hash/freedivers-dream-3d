#include "game.h"

/* Simplified prototype only; tune after integrating world coordinates and collisions. */
void Physics_Update(Diver *player, float dt)
{
    const float drag = 0.96f;
    player->vx *= drag;
    player->vy *= drag;
    player->vz *= drag;

    player->x += player->vx * dt;
    player->y += player->vy * dt;
    player->z += player->vz * dt;
    player->depth = player->y;
}
