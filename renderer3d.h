#ifndef RENDERER3D_H
#define RENDERER3D_H

#include <stdint.h>
#include "material_system.h"

#define RENDERER_MAX_OBJECTS 64
#define RENDERER_MAX_VERTICES 2048
#define RENDERER_MAX_INDICES 4096

typedef struct {
    float x, y, z;
} RenderVec3;

typedef struct {
    float m[16];
} RenderMat4;

typedef struct {
    RenderVec3 position;
    RenderVec3 normal;
    float u, v;
    uint32_t color;
} RenderVertex;

typedef struct {
    const RenderVertex *vertices;
    uint16_t vertex_count;
    const uint16_t *indices;
    uint16_t index_count;
} RenderMesh;

typedef struct {
    RenderVec3 position;
    RenderVec3 rotation;
    RenderVec3 scale;
    int16_t material_id;
    const RenderMesh *mesh;
    uint8_t visible;
} RenderObject;

typedef struct {
    RenderMat4 view;
    RenderMat4 projection;
    uint16_t width;
    uint16_t height;
    uint8_t frame_open;
} Renderer3D;

typedef struct {
    const RenderObject *object;
    RenderMat4 world;
    int16_t material_id;
} RenderCommand;

typedef struct {
    RenderCommand commands[RENDERER_MAX_OBJECTS];
    uint16_t count;
} RenderQueue;

typedef void (*RendererSubmitFn)(const RenderCommand *command,
                                 const Material *material,
                                 void *user_data);

void Renderer3D_Init(Renderer3D *renderer, uint16_t width, uint16_t height);
void Renderer3D_BeginFrame(Renderer3D *renderer,
                           const RenderMat4 *view,
                           const RenderMat4 *projection);
void Renderer3D_EndFrame(Renderer3D *renderer);
void Renderer3D_Mat4Identity(RenderMat4 *out);
void Renderer3D_Mat4Multiply(RenderMat4 *out,
                             const RenderMat4 *a,
                             const RenderMat4 *b);
void Renderer3D_BuildWorldMatrix(RenderMat4 *out,
                                 const RenderObject *object);
void RenderQueue_Init(RenderQueue *queue);
int RenderQueue_Add(RenderQueue *queue, const RenderObject *object);
void RenderQueue_Submit(const RenderQueue *queue,
                        const MaterialRegistry *materials,
                        RendererSubmitFn submit,
                        void *user_data);

#endif
