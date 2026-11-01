#include "marine_life.h"
#include <math.h>

#define PI_F 3.14159265358979323846f

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float wrap_angle(float a) {
    while (a > PI_F) a -= 2.0f * PI_F;
    while (a < -PI_F) a += 2.0f * PI_F;
    return a;
}

void MarineLife_Init(MarineLife *life) {
    uint32_t i;
    if (!life) return;
    life->count = 0;
    for (i = 0; i < MARINE_MAX_ENTITIES; ++i)
        life->entities[i].active = 0;
}

int MarineLife_Add(MarineLife *life, MarineType type,
                   float x, float y, float z, float heading) {
    MarineEntity *e;
    float speed;
    if (!life || life->count >= MARINE_MAX_ENTITIES) return 0;

    switch (type) {
        case MARINE_TURTLE: speed = 0.75f; break;
        case MARINE_DOLPHIN: speed = 1.65f; break;
        case MARINE_SHARK: speed = 1.15f; break;
        case MARINE_FISH:
        default: speed = 1.0f; break;
    }

    e = &life->entities[life->count++];
    e->type = type;
    e->state = MARINE_SWIM_CRUISE;
    e->x = e->home_x = x;
    e->y = e->home_y = y;
    e->z = e->home_z = z;
    e->heading = heading;
    e->speed = speed;
    e->turn_timer = 0.0f;
    e->active = 1;
    return 1;
}

void MarineLife_Update(MarineLife *life, float dt,
                       float diver_x, float diver_y, float diver_z) {
    uint32_t i;
    if (!life || dt <= 0.0f) return;
    dt = clampf(dt, 0.0f, 0.05f);

    for (i = 0; i < life->count; ++i) {
        MarineEntity *e = &life->entities[i];
        float dx, dy, dz, dist2, desired, turn;
        if (!e->active) continue;

        dx = diver_x - e->x;
        dy = diver_y - e->y;
        dz = diver_z - e->z;
        dist2 = dx*dx + dy*dy + dz*dz;

        /* Comportamento simples e não agressivo: animais se afastam
           quando o mergulhador chega muito perto. */
        if (dist2 < 9.0f && e->type != MARINE_DOLPHIN) {
            e->state = MARINE_SWIM_FLEE;
            desired = atan2f(-dy, -dx);
            e->speed = (e->type == MARINE_FISH) ? 1.8f : 1.4f;
        } else {
            e->state = MARINE_SWIM_CRUISE;
            desired = atan2f(e->home_y - e->y, e->home_x - e->x);
            if ((e->home_x - e->x)*(e->home_x - e->x) +
                (e->home_y - e->y)*(e->home_y - e->y) < 4.0f) {
                desired = e->heading + 0.25f;
            }
        }

        turn = wrap_angle(desired - e->heading);
        e->heading = wrap_angle(e->heading + clampf(turn, -1.5f*dt, 1.5f*dt));

        e->x += cosf(e->heading) * e->speed * dt;
        e->y += sinf(e->heading) * e->speed * dt;

        /* Oscilação vertical leve para dar vida ao movimento. */
        e->z += (e->home_z - e->z) * (0.8f * dt);
    }
}
