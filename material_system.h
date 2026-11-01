#ifndef MATERIAL_SYSTEM_H
#define MATERIAL_SYSTEM_H

#include <stdint.h>

#define MATERIAL_MAX_COUNT 64
#define MATERIAL_NAME_LENGTH 24

typedef enum {
    TEX_FORMAT_RGBA32 = 0,
    TEX_FORMAT_RGBA16,
    TEX_FORMAT_INDEX8,
    TEX_FORMAT_INDEX4
} TextureFormat;

typedef enum {
    TEX_WRAP_CLAMP = 0,
    TEX_WRAP_REPEAT
} TextureWrap;

typedef enum {
    TEX_FILTER_NEAREST = 0,
    TEX_FILTER_LINEAR
} TextureFilter;

typedef enum {
    MAT_OPAQUE = 0,
    MAT_CUTOUT,
    MAT_TRANSLUCENT
} MaterialAlphaMode;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t buffer_width;
    TextureFormat format;
    TextureWrap wrap_u;
    TextureWrap wrap_v;
    TextureFilter filter;
    uint32_t vram_address;
    uint32_t palette_address;
    uint8_t has_palette;
    uint8_t resident;
} TextureDescriptor;

typedef struct {
    char name[MATERIAL_NAME_LENGTH];
    TextureDescriptor texture;
    float color_r, color_g, color_b, color_a;
    float uv_scale_u, uv_scale_v;
    MaterialAlphaMode alpha_mode;
    uint8_t lighting_enabled;
    uint8_t double_sided;
    uint8_t valid;
} Material;

typedef struct {
    Material items[MATERIAL_MAX_COUNT];
    uint16_t count;
} MaterialRegistry;

void MaterialRegistry_Init(MaterialRegistry *registry);
int Material_Create(MaterialRegistry *registry, const char *name,
                    const TextureDescriptor *texture);
int Material_SetColor(MaterialRegistry *registry, int id,
                      float r, float g, float b, float a);
int Material_SetUVScale(MaterialRegistry *registry, int id,
                        float u, float v);
int Material_SetAlphaMode(MaterialRegistry *registry, int id,
                          MaterialAlphaMode mode);
uint32_t Texture_EstimateBytes(const TextureDescriptor *texture);
int Texture_ValidateDescriptor(const TextureDescriptor *texture);

#endif
