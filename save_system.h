#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include <stdint.h>

#define FD_SAVE_MAGIC 0x46524452UL /* "FRDR" */
#define FD_SAVE_VERSION 1
#define FD_SAVE_MAX_DISCOVERIES 32
#define FD_SAVE_MAX_COLLECTIBLES 24
#define FD_SAVE_MAX_PHASES 8

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t data_size;
    uint32_t checksum;
    uint8_t completed_phases[FD_SAVE_MAX_PHASES];
    uint8_t discovered[FD_SAVE_MAX_DISCOVERIES];
    uint8_t collected[FD_SAVE_MAX_COLLECTIBLES];
    uint32_t total_play_seconds;
    uint8_t reserved[32];
} FDSaveData;

typedef enum {
    FD_SAVE_OK = 0,
    FD_SAVE_INVALID_ARGUMENT,
    FD_SAVE_BAD_MAGIC,
    FD_SAVE_UNSUPPORTED_VERSION,
    FD_SAVE_BAD_SIZE,
    FD_SAVE_BAD_CHECKSUM
} FDSaveResult;

void FDSave_Init(FDSaveData *save);
uint32_t FDSave_CalculateChecksum(const FDSaveData *save);
void FDSave_UpdateChecksum(FDSaveData *save);
FDSaveResult FDSave_Validate(const FDSaveData *save);
void FDSave_MarkPhase(FDSaveData *save, uint8_t phase, uint8_t completed);
void FDSave_MarkDiscovery(FDSaveData *save, uint8_t index);
void FDSave_MarkCollectible(FDSaveData *save, uint8_t index);

/* Serialização binária em buffer; o backend Memory Card será integrado depois. */
int FDSave_Export(const FDSaveData *save, void *buffer, uint32_t buffer_size);
FDSaveResult FDSave_Import(FDSaveData *save, const void *buffer,
                           uint32_t buffer_size);

#endif
