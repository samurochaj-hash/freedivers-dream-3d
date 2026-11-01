#ifndef UNDERWATER_FX_H
#define UNDERWATER_FX_H

#include <stdint.h>

#define UWFX_MAX_PARTICLES 48

typedef enum {
    UWFX_BUBBLE = 0,
    UWFX_SEDIMENT,
    UWFX_SPLASH
} UWFXParticleType;

typedef struct {
    UWFXParticleType type;
    float x, y, z;
    float vx, vy, vz;
    float life;
    float max_life;
    float size;
    uint8_t active;
} UWFXParticle;

typedef struct {
    UWFXParticle particles[UWFX_MAX_PARTICLES];
    uint32_t next_slot;
    float elapsed;
    float bubble_timer;
    float sediment_timer;
} UnderwaterFX;

typedef struct {
    float fog_near;
    float fog_far;
    float fog_strength;
    float light_strength;
} UWFXDepthSettings;

void UnderwaterFX_Init(UnderwaterFX *fx);
int UnderwaterFX_Spawn(UnderwaterFX *fx, UWFXParticleType type,
                       float x, float y, float z,
                       float vx, float vy, float vz,
                       float life, float size);
void UnderwaterFX_Update(UnderwaterFX *fx, float dt);
void UnderwaterFX_UpdateBreathing(UnderwaterFX *fx,
                                  float diver_x, float diver_y, float diver_z,
                                  int breathing_event);
void UnderwaterFX_GetDepthSettings(float depth, UWFXDepthSettings *settings);

#endif
