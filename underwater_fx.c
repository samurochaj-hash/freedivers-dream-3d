#include "underwater_fx.h"
#include <math.h>

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void UnderwaterFX_Init(UnderwaterFX *fx) {
    uint32_t i;
    if (!fx) return;
    fx->next_slot = 0;
    fx->elapsed = 0.0f;
    fx->bubble_timer = 0.0f;
    fx->sediment_timer = 0.0f;
    for (i = 0; i < UWFX_MAX_PARTICLES; ++i)
        fx->particles[i].active = 0;
}

int UnderwaterFX_Spawn(UnderwaterFX *fx, UWFXParticleType type,
                       float x, float y, float z,
                       float vx, float vy, float vz,
                       float life, float size) {
    UWFXParticle *p;
    uint32_t slot, n;
    if (!fx || life <= 0.0f) return 0;

    slot = fx->next_slot;
    for (n = 0; n < UWFX_MAX_PARTICLES; ++n) {
        uint32_t candidate = (slot + n) % UWFX_MAX_PARTICLES;
        if (!fx->particles[candidate].active) {
            slot = candidate;
            break;
        }
    }
    if (n == UWFX_MAX_PARTICLES) {
        slot = fx->next_slot; /* substitui a partícula mais antiga */
    }

    p = &fx->particles[slot];
    p->type = type;
    p->x = x; p->y = y; p->z = z;
    p->vx = vx; p->vy = vy; p->vz = vz;
    p->life = life;
    p->max_life = life;
    p->size = size;
    p->active = 1;
    fx->next_slot = (slot + 1) % UWFX_MAX_PARTICLES;
    return 1;
}

void UnderwaterFX_Update(UnderwaterFX *fx, float dt) {
    uint32_t i;
    if (!fx || dt <= 0.0f) return;
    dt = clampf(dt, 0.0f, 0.05f);
    fx->elapsed += dt;

    for (i = 0; i < UWFX_MAX_PARTICLES; ++i) {
        UWFXParticle *p = &fx->particles[i];
        if (!p->active) continue;

        p->life -= dt;
        if (p->life <= 0.0f) {
            p->active = 0;
            continue;
        }

        p->x += p->vx * dt;
        p->y += p->vy * dt;
        p->z += p->vz * dt;

        if (p->type == UWFX_BUBBLE) {
            p->vz += 0.35f * dt; /* flutuação em direção à superfície */
            p->vx *= 0.995f;
            p->vy *= 0.995f;
        } else if (p->type == UWFX_SEDIMENT) {
            p->vz += 0.015f * dt;
            p->vx *= 0.99f;
            p->vy *= 0.99f;
        }
    }
}

void UnderwaterFX_UpdateBreathing(UnderwaterFX *fx,
                                  float diver_x, float diver_y, float diver_z,
                                  int breathing_event) {
    if (!fx || !breathing_event) return;

    /* Pequeno conjunto de bolhas por evento; a renderização é outro módulo. */
    UnderwaterFX_Spawn(fx, UWFX_BUBBLE, diver_x, diver_y, diver_z,
                       -0.08f, 0.02f, 0.55f, 1.8f, 0.06f);
    UnderwaterFX_Spawn(fx, UWFX_BUBBLE, diver_x + 0.08f, diver_y, diver_z,
                       0.07f, -0.02f, 0.68f, 1.5f, 0.045f);
}

void UnderwaterFX_GetDepthSettings(float depth, UWFXDepthSettings *settings) {
    float d;
    if (!settings) return;
    d = depth < 0.0f ? 0.0f : depth;

    settings->fog_near = 8.0f + d * 0.12f;
    settings->fog_far = 42.0f - clampf(d * 0.18f, 0.0f, 22.0f);
    settings->fog_strength = clampf(0.12f + d * 0.012f, 0.12f, 0.78f);
    settings->light_strength = clampf(1.0f - d * 0.018f, 0.25f, 1.0f);
}
