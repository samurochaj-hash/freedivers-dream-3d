#ifndef SKINNING_SYSTEM_H
#define SKINNING_SYSTEM_H
#include <stdint.h>
#define SKIN_MAX_INFLUENCES 4
typedef struct { float x,y,z; } SkinVec3;
typedef struct { uint16_t joint[4]; float weight[4]; } SkinVertexWeights;
typedef struct { SkinVec3 position,normal; } SkinSourceVertex;
typedef struct { SkinVec3 position,normal; } SkinOutputVertex;
typedef struct { float m[16]; } SkinMatrix4;
typedef enum { SKIN_OK=0, SKIN_ERROR_NULL=-1, SKIN_ERROR_CAPACITY=-2, SKIN_ERROR_JOINT_INDEX=-3 } SkinResult;
void skin_matrix_identity(SkinMatrix4 *out);
SkinVec3 skin_transform_point(const SkinMatrix4 *m,SkinVec3 p);
SkinVec3 skin_transform_vector(const SkinMatrix4 *m,SkinVec3 v);
SkinResult skin_deform_vertices(const SkinSourceVertex *source,const SkinVertexWeights *weights,uint32_t vertex_count,const SkinMatrix4 *palette,uint16_t palette_count,SkinOutputVertex *destination);
#endif
