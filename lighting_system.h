#ifndef LIGHTING_SYSTEM_H
#define LIGHTING_SYSTEM_H

#include <stdint.h>

typedef struct {
    float r, g, b;
} LightColor;

typedef struct {
    float x, y, z;
} LightVec3;

typedef struct {
    LightColor ambient_color;
    float ambient_strength;
    LightColor depth_color;
    float depth_fog_strength;
    float depth_fog_start;
    float depth_fog_end;
    LightColor flashlight_color;
    float flashlight_intensity;
    float flashlight_range;
    float flashlight_cone_cos;
    LightVec3 flashlight_position;
    LightVec3 flashlight_direction;
    uint8_t flashlight_enabled;
} UnderwaterLighting;

typedef struct {
    LightColor color;
    float fog_factor;
    float flashlight_factor;
} LightingResult;

void Lighting_Init(UnderwaterLighting *lighting);
void Lighting_SetDepth(UnderwaterLighting *lighting, float depth);
void Lighting_SetFlashlight(UnderwaterLighting *lighting, uint8_t enabled,
                            LightVec3 position, LightVec3 direction);
LightingResult Lighting_Evaluate(const UnderwaterLighting *lighting,
                                 LightVec3 point,
                                 LightVec3 normal);

#endif
