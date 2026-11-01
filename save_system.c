#include "save_system.h"
#include <stddef.h>
#include <string.h>

void FDSave_Init(FDSaveData *save) {
    if (!save) return;
    memset(save, 0, sizeof(*save));
    save->magic = FD_SAVE_MAGIC;
    save->version = FD_SAVE_VERSION;
    save->data_size = (uint16_t)sizeof(FDSaveData);
    FDSave_UpdateChecksum(save);
}

uint32_t FDSave_CalculateChecksum(const FDSaveData *save) {
    const uint8_t *bytes;
    uint32_t crc = 0xFFFFFFFFUL;
    uint32_t i;
    uint32_t checksum_offset = (uint32_t)offsetof(FDSaveData, checksum);

    if (!save) return 0;
    bytes = (const uint8_t *)save;

    for (i = 0; i < sizeof(FDSaveData); ++i) {
        uint32_t bit;
        uint8_t value = (i >= checksum_offset &&
                         i < checksum_offset + sizeof(save->checksum))
                        ? 0 : bytes[i];
        crc ^= value;
        for (bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0xEDB88320UL & (0UL - (crc & 1UL)));
    }
    return ~crc;
}

void FDSave_UpdateChecksum(FDSaveData *save) {
    if (!save) return;
    save->magic = FD_SAVE_MAGIC;
    save->version = FD_SAVE_VERSION;
    save->data_size = (uint16_t)sizeof(FDSaveData);
    save->checksum = 0;
    save->checksum = FDSave_CalculateChecksum(save);
}

FDSaveResult FDSave_Validate(const FDSaveData *save) {
    if (!save) return FD_SAVE_INVALID_ARGUMENT;
    if (save->magic != FD_SAVE_MAGIC) return FD_SAVE_BAD_MAGIC;
    if (save->version != FD_SAVE_VERSION) return FD_SAVE_UNSUPPORTED_VERSION;
    if (save->data_size != sizeof(FDSaveData)) return FD_SAVE_BAD_SIZE;
    if (save->checksum != FDSave_CalculateChecksum(save))
        return FD_SAVE_BAD_CHECKSUM;
    return FD_SAVE_OK;
}

void FDSave_MarkPhase(FDSaveData *save, uint8_t phase, uint8_t completed) {
    if (!save || phase >= FD_SAVE_MAX_PHASES) return;
    save->completed_phases[phase] = completed ? 1 : 0;
    FDSave_UpdateChecksum(save);
}

void FDSave_MarkDiscovery(FDSaveData *save, uint8_t index) {
    if (!save || index >= FD_SAVE_MAX_DISCOVERIES) return;
    save->discovered[index] = 1;
    FDSave_UpdateChecksum(save);
}

void FDSave_MarkCollectible(FDSaveData *save, uint8_t index) {
    if (!save || index >= FD_SAVE_MAX_COLLECTIBLES) return;
    save->collected[index] = 1;
    FDSave_UpdateChecksum(save);
}

int FDSave_Export(const FDSaveData *save, void *buffer, uint32_t buffer_size) {
    FDSaveData copy;
    if (!save || !buffer || buffer_size < sizeof(FDSaveData)) return 0;
    copy = *save;
    FDSave_UpdateChecksum(&copy);
    memcpy(buffer, &copy, sizeof(FDSaveData));
    return (int)sizeof(FDSaveData);
}

FDSaveResult FDSave_Import(FDSaveData *save, const void *buffer,
                           uint32_t buffer_size) {
    FDSaveData candidate;
    FDSaveResult result;
    if (!save || !buffer) return FD_SAVE_INVALID_ARGUMENT;
    if (buffer_size != sizeof(FDSaveData)) return FD_SAVE_BAD_SIZE;
    memcpy(&candidate, buffer, sizeof(FDSaveData));
    result = FDSave_Validate(&candidate);
    if (result != FD_SAVE_OK) return result;
    *save = candidate;
    return FD_SAVE_OK;
}
