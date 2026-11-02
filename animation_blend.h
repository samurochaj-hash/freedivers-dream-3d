#ifndef ANIMATION_BLEND_H
#define ANIMATION_BLEND_H
#include <stdint.h>

#define ANIM_BLEND_MAX_JOINTS 32

typedef struct { float x, y, z; } BlendVec3;
typedef struct { float x, y, z, w; } BlendQuat;
typedef struct {
    BlendVec3 position;
    BlendQuat rotation;
    BlendVec3 scale;
} BlendTransform;

typedef struct {
    BlendTransform *source_pose;
    BlendTransform *target_pose;
    BlendTransform *output_pose;
    uint16_t joint_count;
    float duration_seconds;
    float elapsed_seconds;
    uint8_t active;
} AnimationBlend;

typedef enum {
    ANIM_BLEND_OK = 0,
    ANIM_BLEND_ERROR_NULL = -1,
    ANIM_BLEND_ERROR_COUNT = -2,
    ANIM_BLEND_ERROR_DURATION = -3
} AnimationBlendResult;

void anim_blend_begin(AnimationBlend *blend,
                      BlendTransform *source_pose,
                      BlendTransform *target_pose,
                      BlendTransform *output_pose,
                      uint16_t joint_count,
                      float duration_seconds);
void anim_blend_cancel(AnimationBlend *blend);
void anim_blend_update(AnimationBlend *blend, float delta_seconds);
float anim_blend_get_factor(const AnimationBlend *blend);
#endif
