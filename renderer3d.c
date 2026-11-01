#include "renderer3d.h"
#include <math.h>
#include <string.h>

#define R3D_PI 3.14159265358979323846f

void Renderer3D_Mat4Identity(RenderMat4 *out) {
    int i;
    if (!out) return;
    for (i = 0; i < 16; ++i) out->m[i] = 0.0f;
    out->m[0] = out->m[5] = out->m[10] = out->m[15] = 1.0f;
}

void Renderer3D_Mat4Multiply(RenderMat4 *out,
                             const RenderMat4 *a,
                             const RenderMat4 *b) {
    RenderMat4 temp;
    int row, col, k;
    if (!out || !a || !b) return;
    for (row = 0; row < 4; ++row) {
        for (col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (k = 0; k < 4; ++k)
                sum += a->m[row * 4 + k] * b->m[k * 4 + col];
            temp.m[row * 4 + col] = sum;
        }
    }
    *out = temp;
}

void Renderer3D_Init(Renderer3D *renderer, uint16_t width, uint16_t height) {
    if (!renderer) return;
    memset(renderer, 0, sizeof(*renderer));
    renderer->width = width;
    renderer->height = height;
    Renderer3D_Mat4Identity(&renderer->view);
    Renderer3D_Mat4Identity(&renderer->projection);
}

void Renderer3D_BeginFrame(Renderer3D *renderer,
                           const RenderMat4 *view,
                           const RenderMat4 *projection) {
    if (!renderer) return;
    if (view) renderer->view = *view;
    if (projection) renderer->projection = *projection;
    renderer->frame_open = 1;
}

void Renderer3D_EndFrame(Renderer3D *renderer) {
    if (renderer) renderer->frame_open = 0;
}

void Renderer3D_BuildWorldMatrix(RenderMat4 *out,
                                 const RenderObject *object) {
    float sx, sy, sz, cx, cy, cz;
    RenderMat4 rx, ry, rz, scale, rotate_xy, rotate, translate;
    if (!out || !object) return;

    sx = object->scale.x; sy = object->scale.y; sz = object->scale.z;
    cx = cosf(object->rotation.x * (R3D_PI / 180.0f));
    cy = cosf(object->rotation.y * (R3D_PI / 180.0f));
    cz = cosf(object->rotation.z * (R3D_PI / 180.0f));

    Renderer3D_Mat4Identity(&scale);
    scale.m[0] = sx; scale.m[5] = sy; scale.m[10] = sz;

    Renderer3D_Mat4Identity(&rx);
    rx.m[5] = cx; rx.m[6] = -sinf(object->rotation.x * (R3D_PI / 180.0f));
    rx.m[9] = -rx.m[6]; rx.m[10] = cx;

    Renderer3D_Mat4Identity(&ry);
    ry.m[0] = cy; ry.m[2] = sinf(object->rotation.y * (R3D_PI / 180.0f));
    ry.m[8] = -ry.m[2]; ry.m[10] = cy;

    Renderer3D_Mat4Identity(&rz);
    rz.m[0] = cz; rz.m[1] = -sinf(object->rotation.z * (R3D_PI / 180.0f));
    rz.m[4] = -rz.m[1]; rz.m[5] = cz;

    Renderer3D_Mat4Multiply(&rotate_xy, &ry, &rx);
    Renderer3D_Mat4Multiply(&rotate, &rz, &rotate_xy);
    Renderer3D_Mat4Multiply(&translate, &rotate, &scale);

    translate.m[3] = object->position.x;
    translate.m[7] = object->position.y;
    translate.m[11] = object->position.z;
    *out = translate;
}

void RenderQueue_Init(RenderQueue *queue) {
    if (queue) memset(queue, 0, sizeof(*queue));
}

int RenderQueue_Add(RenderQueue *queue, const RenderObject *object) {
    RenderCommand *cmd;
    if (!queue || !object || !object->visible || !object->mesh)
        return 0;
    if (object->mesh->vertex_count == 0 ||
        object->mesh->vertex_count > RENDERER_MAX_VERTICES ||
        object->mesh->index_count > RENDERER_MAX_INDICES ||
        queue->count >= RENDERER_MAX_OBJECTS)
        return 0;

    cmd = &queue->commands[queue->count];
    cmd->object = object;
    cmd->material_id = object->material_id;
    Renderer3D_BuildWorldMatrix(&cmd->world, object);
    queue->count++;
    return 1;
}

void RenderQueue_Submit(const RenderQueue *queue,
                        const MaterialRegistry *materials,
                        RendererSubmitFn submit,
                        void *user_data) {
    uint16_t i;
    if (!queue || !materials || !submit) return;
    for (i = 0; i < queue->count; ++i) {
        const RenderCommand *cmd = &queue->commands[i];
        const Material *mat = 0;
        if (cmd->material_id >= 0 &&
            cmd->material_id < materials->count) {
            const Material *candidate = &materials->items[cmd->material_id];
            if (candidate->valid) mat = candidate;
        }
        submit(cmd, mat, user_data);
    }
}
