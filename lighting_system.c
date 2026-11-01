#include "lighting_system.h"
#include <math.h>

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float dot3(LightVec3 a, LightVec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

static float length3(LightVec3 v) {
    return sqrtf(dot3(v, v));
}

static LightVec3 normalize3(LightVec3 v) {
    float len = length3(v);
    if (len < 0.0001f) {
        LightVec3 zero = {0.0f, 0.0f, 0.0f};
        return zero;
    }
    v.x /= len; v.y /= len; v.z /= len;
    return v;
}

static LightVec3 subtract3(LightVec3 a, LightVec3 b) {
    LightVec3 v = {a.x-b.x, a.y-b.y, a.z-b.z};
    return v;
}

void Lighting_Init(UnderwaterLighting *lighting) {
    if (!lighting) return;
    lighting->ambient_color.r = 0.10f;
    lighting->ambient_color.g = 0.30f;
    lighting->ambient_color.b = 0.42f;
    lighting->ambient_strength = 0.72f;

    lighting->depth_color.r = 0.015f;
    lighting->depth_color.g = 0.07f;
    lighting->depth_color.b = 0.12f;
    lighting->depth_fog_strength = 0.35f;
    lighting->depth_fog_start = 8.0f;
    lighting->depth_fog_end = 55.0f;

    lighting->flashlight_color.r = 0.72f;
    lighting->flashlight_color.g = 0.88f;
    lighting->flashlight_color.b = 1.0f;
    lighting->flashlight_intensity = 1.0f;
    lighting->flashlight_range = 14.0f;
    lighting->flashlight_cone_cos = 0.72f;
    lighting->flashlight_position.x = 0.0f;
    lighting->flashlight_position.y = 0.0f;
    lighting->flashlight_position.z = 0.0f;
    lighting->flashlight_direction.x = 0.0f;
    lighting->flashlight_direction.y = 1.0f;
    lighting->flashlight_direction.z = 0.0f;
    lighting->flashlight_enabled = 1;
}

void Lighting_SetDepth(UnderwaterLighting *lighting, float depth) {
    float t;
    if (!lighting) return;
    if (depth < 0.0f) depth = 0.0f;
    t = clampf((depth - lighting->depth_fog_start) /
               (lighting->depth_fog_end - lighting->depth_fog_start),
               0.0f, 1.0f);
    lighting->depth_fog_strength = 0.15f + t * 0.75f;
}

void Lighting_SetFlashlight(UnderwaterLighting *lighting, uint8_t enabled,
                            LightVec3 position, LightVec3 direction) {
    if (!lighting) return;
    lighting->flashlight_enabled = enabled ? 1 : 0;
    lighting->flashlight_position = position;
    lighting->flashlight_direction = normalize3(direction);
}

LightingResult Lighting_Evaluate(const UnderwaterLighting *lighting,
                                 LightVec3 point,
                                 LightVec3 normal) {
    LightingResult result;
    LightVec3 to_point, ray, n;
    float distance, cone, attenuation, diffuse;

    result.color.r = result.color.g = result.color.b = 0.0f;
    result.fog_factor = 0.0f;
    result.flashlight_factor = 0.0f;
    if (!lighting) return result;

    result.color.r = lighting->ambient_color.r * lighting->ambient_strength;
    result.color.g = lighting->ambient_color.g * lighting->ambient_strength;
    result.color.b = lighting->ambient_color.b * lighting->ambient_strength;

    to_point = subtract3(point, lighting->flashlight_position);
    distance = length3(to_point);
    ray = normalize3(to_point);
    n = normalize3(normal);

    if (lighting->flashlight_enabled && distance < lighting->flashlight_range) {
        cone = dot3(lighting->flashlight_direction, ray);
        if (cone > lighting->flashlight_cone_cos) {
            attenuation = 1.0f - distance / lighting->flashlight_range;
            diffuse = clampf(dot3(n, ray) * -1.0f, 0.0f, 1.0f);
            result.flashlight_factor = attenuation * diffuse *
                lighting->flashlight_intensity *
                clampf((cone - lighting->flashlight_cone_cos) /
                       (1.0f - lighting->flashlight_cone_cos), 0.0f, 1.0f);
            result.color.r += lighting->flashlight_color.r * result.flashlight_factor;
            result.color.g += lighting->flashlight_color.g * result.flashlight_factor;
            result.color.b += lighting->flashlight_color.b * result.flashlight_factor;
        }
    }

    result.fog_factor = clampf(lighting->depth_fog_strength, 0.0f, 1.0f);
    result.color.r = result.color.r * (1.0f-result.fog_factor) +
                     lighting->depth_color.r * result.fog_factor;
    result.color.g = result.color.g * (1.0f-result.fog_factor) +
                     lighting->depth_color.g * result.fog_factor;
    result.color.b = result.color.b * (1.0f-result.fog_factor) +
                     lighting->depth_color.b * result.fog_factor;

    result.color.r = clampf(result.color.r, 0.0f, 1.0f);
    result.color.g = clampf(result.color.g, 0.0f, 1.0f);
    result.color.b = clampf(result.color.b, 0.0f, 1.0f);
    return result;
}
