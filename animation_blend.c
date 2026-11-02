#include "animation_blend.h"

static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}
static float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}
static BlendVec3 vec_lerp(BlendVec3 a, BlendVec3 b, float t) {
    BlendVec3 r = {lerp(a.x,b.x,t),lerp(a.y,b.y,t),lerp(a.z,b.z,t)};
    return r;
}
static BlendQuat quat_normalize(BlendQuat q) {
    float n = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;
    if (n < 0.000001f) {
        BlendQuat id = {0.0f,0.0f,0.0f,1.0f};
        return id;
    }
    {
        float inv = 1.0f;
        int i;
        for (i=0;i<8;i++) inv *= 1.5f - 0.5f*n*inv*inv;
        q.x*=inv; q.y*=inv; q.z*=inv; q.w*=inv;
    }
    return q;
}
static BlendQuat quat_nlerp(BlendQuat a, BlendQuat b, float t) {
    BlendQuat r;
    float dot = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    /* q and -q encode the same rotation; choose the shorter interpolation. */
    if (dot < 0.0f) {
        b.x=-b.x; b.y=-b.y; b.z=-b.z; b.w=-b.w;
    }
    r.x=lerp(a.x,b.x,t); r.y=lerp(a.y,b.y,t);
    r.z=lerp(a.z,b.z,t); r.w=lerp(a.w,b.w,t);
    return quat_normalize(r);
}
static BlendTransform blend_transform(BlendTransform a, BlendTransform b, float t) {
    BlendTransform r;
    r.position=vec_lerp(a.position,b.position,t);
    r.scale=vec_lerp(a.scale,b.scale,t);
    r.rotation=quat_nlerp(a.rotation,b.rotation,t);
    return r;
}
static void apply_factor(AnimationBlend *b, float factor) {
    uint16_t i;
    for (i=0;i<b->joint_count;i++)
        b->output_pose[i]=blend_transform(b->source_pose[i],b->target_pose[i],factor);
}
void anim_blend_begin(AnimationBlend *b, BlendTransform *source,
                      BlendTransform *target, BlendTransform *output,
                      uint16_t count, float duration) {
    if (!b) return;
    b->source_pose=source; b->target_pose=target; b->output_pose=output;
    b->joint_count=count; b->duration_seconds=duration;
    b->elapsed_seconds=0.0f; b->active=0;
    if (!source || !target || !output || count==0 ||
        count>ANIM_BLEND_MAX_JOINTS || duration<=0.0f) return;
    b->active=1;
    apply_factor(b,0.0f);
}
void anim_blend_cancel(AnimationBlend *b) {
    if (!b) return;
    b->active=0;
    b->elapsed_seconds=0.0f;
}
float anim_blend_get_factor(const AnimationBlend *b) {
    if (!b || b->duration_seconds<=0.0f) return 1.0f;
    return clamp01(b->elapsed_seconds/b->duration_seconds);
}
void anim_blend_update(AnimationBlend *b, float dt) {
    float t;
    if (!b || !b->active || dt<=0.0f) return;
    b->elapsed_seconds += dt;
    if (b->elapsed_seconds >= b->duration_seconds) {
        b->elapsed_seconds=b->duration_seconds;
        b->active=0;
    }
    t=anim_blend_get_factor(b);
    /* Smoothstep eases in and out to reduce abrupt changes. */
    t=t*t*(3.0f-2.0f*t);
    apply_factor(b,t);
}
