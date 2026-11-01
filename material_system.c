#include "material_system.h"
#include <string.h>

static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

void MaterialRegistry_Init(MaterialRegistry *registry) {
    if (!registry) return;
    memset(registry, 0, sizeof(*registry));
}

int Texture_ValidateDescriptor(const TextureDescriptor *t) {
    if (!t || t->width == 0 || t->height == 0) return 0;
    if (t->width > 1024 || t->height > 1024) return 0;
    if (t->buffer_width < t->width) return 0;
    if (t->format < TEX_FORMAT_RGBA32 || t->format > TEX_FORMAT_INDEX4) return 0;
    if ((t->format == TEX_FORMAT_INDEX8 || t->format == TEX_FORMAT_INDEX4) &&
        !t->has_palette) return 0;
    return 1;
}

uint32_t Texture_EstimateBytes(const TextureDescriptor *t) {
    uint32_t pixels, bits;
    if (!Texture_ValidateDescriptor(t)) return 0;
    pixels = (uint32_t)t->width * (uint32_t)t->height;
    switch (t->format) {
        case TEX_FORMAT_RGBA32: bits = 32; break;
        case TEX_FORMAT_RGBA16: bits = 16; break;
        case TEX_FORMAT_INDEX8: bits = 8; break;
        case TEX_FORMAT_INDEX4: bits = 4; break;
        default: return 0;
    }
    /* Estimativa do bitmap; CLUT/alinhamento e padding de VRAM são adicionais. */
    return (pixels * bits) / 8;
}

int Material_Create(MaterialRegistry *registry, const char *name,
                    const TextureDescriptor *texture) {
    Material *m;
    unsigned int i;
    if (!registry || !name || !texture ||
        registry->count >= MATERIAL_MAX_COUNT ||
        !Texture_ValidateDescriptor(texture)) return -1;

    i = registry->count++;
    m = &registry->items[i];
    memset(m, 0, sizeof(*m));
    strncpy(m->name, name, MATERIAL_NAME_LENGTH - 1);
    m->texture = *texture;
    m->color_r = m->color_g = m->color_b = m->color_a = 1.0f;
    m->uv_scale_u = m->uv_scale_v = 1.0f;
    m->alpha_mode = MAT_OPAQUE;
    m->lighting_enabled = 1;
    m->valid = 1;
    return (int)i;
}

int Material_SetColor(MaterialRegistry *registry, int id,
                      float r, float g, float b, float a) {
    Material *m;
    if (!registry || id < 0 || id >= registry->count) return 0;
    m = &registry->items[id];
    if (!m->valid) return 0;
    m->color_r = clamp01(r); m->color_g = clamp01(g);
    m->color_b = clamp01(b); m->color_a = clamp01(a);
    return 1;
}

int Material_SetUVScale(MaterialRegistry *registry, int id,
                        float u, float v) {
    Material *m;
    if (!registry || id < 0 || id >= registry->count || u <= 0.0f || v <= 0.0f)
        return 0;
    m = &registry->items[id];
    if (!m->valid) return 0;
    m->uv_scale_u = u;
    m->uv_scale_v = v;
    return 1;
}

int Material_SetAlphaMode(MaterialRegistry *registry, int id,
                          MaterialAlphaMode mode) {
    Material *m;
    if (!registry || id < 0 || id >= registry->count ||
        mode < MAT_OPAQUE || mode > MAT_TRANSLUCENT) return 0;
    m = &registry->items[id];
    if (!m->valid) return 0;
    m->alpha_mode = mode;
    return 1;
}
