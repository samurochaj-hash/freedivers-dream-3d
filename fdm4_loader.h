#ifndef FDM4_LOADER_H
#define FDM4_LOADER_H

#include <stdint.h>
#include "animation_system.h"
#include "skinning_system.h"
#include "skeleton_system.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FDM4_MAGIC 0x344D4446u /* bytes: F D M 4 */
#define FDM4_VERSION 1u
#define FDM4_MAX_VERTICES 8192u
#define FDM4_MAX_INDICES 24576u
#define FDM4_MAX_JOINTS 32u
#define FDM4_MAX_CLIPS 16u
#define FDM4_MAX_KEYS_PER_TRACK 64u

typedef struct {
    SkinSourceVertex *vertices;
    SkinVertexWeights *weights;
    uint32_t *indices;
    AnimJoint *joints;
    SkinMatrix4 *inverse_bind;
    AnimJointTrack *tracks;
    AnimKeyframe *keys;
    AnimClip *clips;
} FDM4Buffers;

typedef struct {
    uint32_t vertex_count;
    uint32_t index_count;
    uint16_t joint_count;
    uint16_t clip_count;
    uint16_t track_count;
    uint32_t key_count;
} FDM4Counts;

typedef enum {
    FDM4_OK = 0,
    FDM4_ERR_ARGUMENT = -1,
    FDM4_ERR_HEADER = -2,
    FDM4_ERR_VERSION = -3,
    FDM4_ERR_LIMIT = -4,
    FDM4_ERR_BUFFER = -5,
    FDM4_ERR_TRUNCATED = -6,
    FDM4_ERR_DATA = -7
} FDM4Result;

/*
 * Loads a little-endian FDM4 v1 blob into caller-owned buffers.
 * The file is a compact runtime interchange format, not a general FBX/OBJ reader.
 * Output pointers in FDM4Buffers must have capacity for the declared counts.
 */
FDM4Result fdm4_load_memory(const uint8_t *data, uint32_t data_size,
                            FDM4Buffers *buffers, FDM4Counts *out_counts);

#ifdef __cplusplus
}
#endif
#endif
