#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <stdint.h>
#include <stddef.h>
#include "renderer3d.h"

#define MODEL_FILE_MAGIC 0x334D4446u /* "FDM3" em little-endian */
#define MODEL_FILE_VERSION 1u
#define MODEL_MAX_VERTICES 2048
#define MODEL_MAX_INDICES 4096

typedef struct {
    RenderVertex *vertices;
    uint16_t vertex_capacity;
    uint16_t vertex_count;
    uint16_t *indices;
    uint16_t index_capacity;
    uint16_t index_count;
} ModelBuffer;

typedef enum {
    MODEL_LOAD_OK = 0,
    MODEL_LOAD_BAD_ARGUMENT,
    MODEL_LOAD_BAD_HEADER,
    MODEL_LOAD_UNSUPPORTED_VERSION,
    MODEL_LOAD_CAPACITY,
    MODEL_LOAD_TRUNCATED,
    MODEL_LOAD_BAD_INDEX
} ModelLoadResult;

/* Lê o formato binário FDM3 v1 de um buffer já carregado em memória.
   O chamador fornece os arrays de vértices e índices; nenhuma alocação ocorre. */
ModelLoadResult Model_LoadFDM3(const uint8_t *data, size_t data_size,
                               ModelBuffer *out_model);

#endif
