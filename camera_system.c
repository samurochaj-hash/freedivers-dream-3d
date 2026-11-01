#include "camera_system.h"
#include <math.h>

#define CAM_PI 3.14159265358979323846f

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float smooth_factor(float smoothing, float dt) {
    return clampf(smoothing * dt, 0.0f, 1.0f);
}

void Camera3D_Init(Camera3D *camera) {
    if (!camera) return;
    camera->position.x = 0.0f;
    camera->position.y = -5.0f;
    camera->position.z = 2.0f;
    camera->target.x = 0.0f;
    camera->target.y = 0.0f;
    camera->target.z = 0.0f;
    camera->up.x = 0.0f;
    camera->up.y = 0.0f;
    camera->up.z = 1.0f;
    camera->yaw = 0.0f;
    camera->pitch = 0.18f;
    camera->distance = 5.0f;
    camera->min_distance = 2.0f;
    camera->max_distance = 9.0f;
    camera->follow_smoothing = 5.0f;
    camera->look_smoothing = 7.0f;
}

void Camera3D_SetOrbit(Camera3D *camera, float yaw_delta,
                       float pitch_delta, float zoom_delta) {
    if (!camera) return;
    camera->yaw += yaw_delta;
    camera->pitch = clampf(camera->pitch + pitch_delta, -0.15f, 1.0f);
    camera->distance = clampf(camera->distance + zoom_delta,
                              camera->min_distance, camera->max_distance);
}

void Camera3D_Update(Camera3D *camera, CameraVec3 player_position,
                     float dt) {
    float desired_x, desired_y, desired_z;
    float cp, sp, cy, sy;
    float f;
    if (!camera || dt <= 0.0f) return;
    dt = clampf(dt, 0.0f, 0.05f);

    cp = cosf(camera->pitch);
    sp = sinf(camera->pitch);
    cy = cosf(camera->yaw);
    sy = sinf(camera->yaw);

    desired_x = player_position.x - cy * cp * camera->distance;
    desired_y = player_position.y - sy * cp * camera->distance;
    desired_z = player_position.z + sp * camera->distance + 0.6f;

    f = smooth_factor(camera->follow_smoothing, dt);
    camera->position.x += (desired_x - camera->position.x) * f;
    camera->position.y += (desired_y - camera->position.y) * f;
    camera->position.z += (desired_z - camera->position.z) * f;

    f = smooth_factor(camera->look_smoothing, dt);
    camera->target.x += (player_position.x - camera->target.x) * f;
    camera->target.y += (player_position.y - camera->target.y) * f;
    camera->target.z += (player_position.z - camera->target.z) * f;
}

/* Resolução provisória: se a câmera estiver dentro da caixa, aproxima-a
   do jogador. O sistema final deverá usar raycast/colisor do cenário. */
void Camera3D_ResolveWall(Camera3D *camera,
                          CameraVec3 player_position,
                          CameraVec3 wall_min,
                          CameraVec3 wall_max) {
    int inside;
    if (!camera) return;

    inside = camera->position.x >= wall_min.x &&
             camera->position.x <= wall_max.x &&
             camera->position.y >= wall_min.y &&
             camera->position.y <= wall_max.y &&
             camera->position.z >= wall_min.z &&
             camera->position.z <= wall_max.z;

    if (inside) {
        camera->position.x = player_position.x;
        camera->position.y = player_position.y;
        camera->position.z = player_position.z + 0.5f;
    }
}
