#ifndef MODEL_H
#define MODEL_H

#include "game.h"

#define DIVER_JOINT_COUNT 8

typedef enum {
    JOINT_ROOT = 0,
    JOINT_SPINE,
    JOINT_HEAD,
    JOINT_LEFT_ARM,
    JOINT_RIGHT_ARM,
    JOINT_LEFT_LEG,
    JOINT_RIGHT_LEG,
    JOINT_FINS
} DiverJoint;

typedef struct {
    float x, y, z;
    float rx, ry, rz;
} JointPose;

typedef struct {
    JointPose joints[DIVER_JOINT_COUNT];
    int clip;
    float clipTime;
    float blend;
} DiverSkeleton;

typedef enum {
    ANIM_IDLE = 0,
    ANIM_SWIM,
    ANIM_DIVE,
    ANIM_RISE,
    ANIM_TURN,
    ANIM_EXPLORE
} DiverAnimation;

void Model_InitSkeleton(DiverSkeleton *skeleton);
void Model_SetAnimation(DiverSkeleton *skeleton, DiverAnimation animation);
void Model_UpdateAnimation(DiverSkeleton *skeleton, float dt);

#endif
