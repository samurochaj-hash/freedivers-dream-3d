#ifndef ANIMATION_SYSTEM_H
#define ANIMATION_SYSTEM_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define ANIM_MAX_JOINTS 32
#define ANIM_MAX_CLIPS 16
#define ANIM_MAX_NAME 24
typedef struct { float x,y,z; } AnimVec3;
typedef struct { float x,y,z,w; } AnimQuat;
typedef struct { AnimVec3 position; AnimQuat rotation; AnimVec3 scale; } AnimTransform;
typedef struct { int16_t parent_index; uint16_t flags; AnimTransform bind_local; } AnimJoint;
typedef struct { float time_seconds; AnimTransform transform; } AnimKeyframe;
typedef struct { uint16_t joint_index, key_count; const AnimKeyframe *keys; } AnimJointTrack;
typedef struct {
 char name[ANIM_MAX_NAME]; float duration_seconds;
 uint16_t joint_count, track_count; const AnimJointTrack *tracks; uint8_t looping;
} AnimClip;
typedef enum {
 ANIM_STATE_IDLE=0, ANIM_STATE_SWIM, ANIM_STATE_TURN, ANIM_STATE_ASCEND,
 ANIM_STATE_DESCEND, ANIM_STATE_FISH_SWIM, ANIM_STATE_DOLPHIN_SWIM,
 ANIM_STATE_TURTLE_SWIM
} AnimState;
typedef struct {
 const AnimJoint *skeleton; uint16_t joint_count;
 const AnimClip *clips; uint16_t clip_count;
 AnimTransform *local_pose;
 float time_seconds, playback_speed;
 int16_t active_clip; uint8_t playing;
} AnimPlayer;
void anim_transform_identity(AnimTransform *t);
int anim_player_init(AnimPlayer *p,const AnimJoint *s,uint16_t joints,
 const AnimClip *c,uint16_t clips,AnimTransform *pose,uint16_t capacity);
int anim_player_play(AnimPlayer *p,int16_t clip_index,float speed);
void anim_player_stop(AnimPlayer *p);
void anim_player_update(AnimPlayer *p,float delta_seconds);
void anim_sample_clip(const AnimClip *clip,float time_seconds,
 AnimTransform *out_pose,uint16_t pose_count);
#ifdef __cplusplus
}
#endif
#endif
