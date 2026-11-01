#include "model.h"

/*
 * Animation scaffold.
 * Joint rotations are placeholders until an authorized model and animation
 * clips are imported. Angles are stored in radians.
 */

void Model_InitSkeleton(DiverSkeleton *skeleton)
{
    int i;
    for (i = 0; i < DIVER_JOINT_COUNT; ++i) {
        skeleton->joints[i].x = 0.0f;
        skeleton->joints[i].y = 0.0f;
        skeleton->joints[i].z = 0.0f;
        skeleton->joints[i].rx = 0.0f;
        skeleton->joints[i].ry = 0.0f;
        skeleton->joints[i].rz = 0.0f;
    }
    skeleton->clip = ANIM_IDLE;
    skeleton->clipTime = 0.0f;
    skeleton->blend = 1.0f;
}

void Model_SetAnimation(DiverSkeleton *skeleton, DiverAnimation animation)
{
    if (skeleton->clip != (int)animation) {
        skeleton->clip = (int)animation;
        skeleton->clipTime = 0.0f;
        skeleton->blend = 0.0f;
    }
}

void Model_UpdateAnimation(DiverSkeleton *skeleton, float dt)
{
    int i;
    skeleton->clipTime += dt;

    /* Placeholder blend ramp; actual keyframe interpolation comes later. */
    if (skeleton->blend < 1.0f) {
        skeleton->blend += dt * 4.0f;
        if (skeleton->blend > 1.0f)
            skeleton->blend = 1.0f;
    }

    /* Keep joints stable until real animation data is loaded. */
    for (i = 0; i < DIVER_JOINT_COUNT; ++i) {
        (void)skeleton->joints[i];
    }
}
