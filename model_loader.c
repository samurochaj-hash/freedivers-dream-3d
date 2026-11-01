#include "model_loader.h"
#include <string.h>

#define FDM3_HEADER_SIZE 16u
#define FDM3_VERTEX_SIZE 36u
#define FDM3_INDEX_SIZE 2u

static uint16_t read_u16_le(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t read_u32_le(const uint8_t *p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static float read_f32_le(const uint8_t *p) {
    uint32_t bits = read_u32_le(p);
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

ModelLoadResult Model_LoadFDM3(const uint8_t *data, size_t data_size,
                               ModelBuffer *out_model) {
    uint32_t magic;
    uint16_t version, vertex_count, index_count, reserved;
    size_t required, offset;
    uint16_t i;

    if (!data || !out_model || !out_model->vertices || !out_model->indices)
        return MODEL_LOAD_BAD_ARGUMENT;
    out_model->vertex_count = 0;
    out_model->index_count = 0;

    if (data_size < FDM3_HEADER_SIZE) return MODEL_LOAD_TRUNCATED;

    magic = read_u32_le(data);
    version = read_u16_le(data + 4);
    vertex_count = read_u16_le(data + 6);
    index_count = read_u16_le(data + 8);
    reserved = read_u16_le(data + 10);

    if (magic != MODEL_FILE_MAGIC || reserved != 0)
        return MODEL_LOAD_BAD_HEADER;
    if (version != MODEL_FILE_VERSION)
        return MODEL_LOAD_UNSUPPORTED_VERSION;
    if (vertex_count == 0 || vertex_count > MODEL_MAX_VERTICES ||
        index_count == 0 || index_count > MODEL_MAX_INDICES ||
        (index_count % 3) != 0)
        return MODEL_LOAD_BAD_HEADER;
    if (vertex_count > out_model->vertex_capacity ||
        index_count > out_model->index_capacity)
        return MODEL_LOAD_CAPACITY;

    required = FDM3_HEADER_SIZE +
               (size_t)vertex_count * FDM3_VERTEX_SIZE +
               (size_t)index_count * FDM3_INDEX_SIZE;
    if (required > data_size) return MODEL_LOAD_TRUNCATED;

    offset = FDM3_HEADER_SIZE;
    for (i = 0; i < vertex_count; ++i) {
        RenderVertex *v = &out_model->vertices[i];
        v->position.x = read_f32_le(data + offset); offset += 4;
        v->position.y = read_f32_le(data + offset); offset += 4;
        v->position.z = read_f32_le(data + offset); offset += 4;
        v->normal.x = read_f32_le(data + offset); offset += 4;
        v->normal.y = read_f32_le(data + offset); offset += 4;
        v->normal.z = read_f32_le(data + offset); offset += 4;
        v->u = read_f32_le(data + offset); offset += 4;
        v->v = read_f32_le(data + offset); offset += 4;
        v->color = read_u32_le(data + offset); offset += 4;
    }

    for (i = 0; i < index_count; ++i) {
        uint16_t index = read_u16_le(data + offset);
        offset += 2;
        if (index >= vertex_count) {
            out_model->vertex_count = 0;
            out_model->index_count = 0;
            return MODEL_LOAD_BAD_INDEX;
        }
        out_model->indices[i] = index;
    }

    out_model->vertex_count = vertex_count;
    out_model->index_count = index_count;
    return MODEL_LOAD_OK;
}
