#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H

#include <stdint.h>

typedef struct {
    float x, y, z;
} CameraVec3;

typedef struct {
    CameraVec3 position;
    CameraVec3 target;
    CameraVec3 up;
    float yaw;
    float pitch;
    float distance;
    float min_distance;
    float max_distance;
    float follow_smoothing;
    float look_smoothing;
} Camera3D;

void Camera3D_Init(Camera3D *camera);
void Camera3D_SetOrbit(Camera3D *camera, float yaw_delta,
                       float pitch_delta, float zoom_delta);
void Camera3D_Update(Camera3D *camera, CameraVec3 player_position,
                     float dt);
void Camera3D_ResolveWall(Camera3D *camera,
                          CameraVec3 player_position,
                          CameraVec3 wall_min,
                          CameraVec3 wall_max);

#endif
