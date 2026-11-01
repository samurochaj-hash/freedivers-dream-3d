#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <stdint.h>

#define FD_AUDIO_MAX_CHANNELS 8

typedef enum {
    FD_SFX_BUBBLES = 0,
    FD_SFX_BREATHING,
    FD_SFX_FISH_SWIM,
    FD_SFX_DOLPHIN_CALL,
    FD_SFX_CAVE_ECHO,
    FD_SFX_COLLECT,
    FD_SFX_DISCOVERY,
    FD_SFX_COUNT
} FDAudioSoundId;

typedef enum {
    FD_AUDIO_BUS_MUSIC = 0,
    FD_AUDIO_BUS_AMBIENCE,
    FD_AUDIO_BUS_SFX,
    FD_AUDIO_BUS_COUNT
} FDAudioBus;

typedef struct {
    uint8_t active;
    uint8_t loop;
    uint8_t bus;
    uint8_t sound_id;
    float volume;
    float pan;
    float pitch;
    uint32_t sample_position;
} FDAudioChannel;

typedef struct {
    float bus_volume[FD_AUDIO_BUS_COUNT];
    FDAudioChannel channels[FD_AUDIO_MAX_CHANNELS];
    uint8_t master_muted;
} FDAudioSystem;

void FDAudio_Init(FDAudioSystem *audio);
void FDAudio_SetMasterMute(FDAudioSystem *audio, int muted);
void FDAudio_SetBusVolume(FDAudioSystem *audio, FDAudioBus bus, float volume);
int FDAudio_Play(FDAudioSystem *audio, FDAudioSoundId sound,
                 FDAudioBus bus, int loop, float volume, float pan);
void FDAudio_StopAll(FDAudioSystem *audio);
void FDAudio_StopSound(FDAudioSystem *audio, FDAudioSoundId sound);
void FDAudio_Update(FDAudioSystem *audio, float dt);

/* Ponto de integração: implementação real deverá encaminhar os comandos ao backend SPU2. */
float FDAudio_GetEffectiveVolume(const FDAudioSystem *audio,
                                 const FDAudioChannel *channel);

#endif
