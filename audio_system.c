#include "audio_system.h"

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void FDAudio_Init(FDAudioSystem *audio) {
    uint32_t i;
    if (!audio) return;
    audio->bus_volume[FD_AUDIO_BUS_MUSIC] = 0.65f;
    audio->bus_volume[FD_AUDIO_BUS_AMBIENCE] = 0.8f;
    audio->bus_volume[FD_AUDIO_BUS_SFX] = 0.9f;
    audio->master_muted = 0;
    for (i = 0; i < FD_AUDIO_MAX_CHANNELS; ++i) {
        audio->channels[i].active = 0;
        audio->channels[i].loop = 0;
        audio->channels[i].bus = FD_AUDIO_BUS_SFX;
        audio->channels[i].sound_id = 0;
        audio->channels[i].volume = 1.0f;
        audio->channels[i].pan = 0.0f;
        audio->channels[i].pitch = 1.0f;
        audio->channels[i].sample_position = 0;
    }
}

void FDAudio_SetMasterMute(FDAudioSystem *audio, int muted) {
    if (!audio) return;
    audio->master_muted = muted ? 1 : 0;
}

void FDAudio_SetBusVolume(FDAudioSystem *audio, FDAudioBus bus, float volume) {
    if (!audio || bus >= FD_AUDIO_BUS_COUNT) return;
    audio->bus_volume[bus] = clampf(volume, 0.0f, 1.0f);
}

int FDAudio_Play(FDAudioSystem *audio, FDAudioSoundId sound,
                 FDAudioBus bus, int loop, float volume, float pan) {
    uint32_t i;
    FDAudioChannel *ch = 0;
    if (!audio || sound >= FD_SFX_COUNT || bus >= FD_AUDIO_BUS_COUNT)
        return -1;

    for (i = 0; i < FD_AUDIO_MAX_CHANNELS; ++i) {
        if (!audio->channels[i].active) {
            ch = &audio->channels[i];
            break;
        }
    }
    if (!ch) return -1;

    ch->active = 1;
    ch->loop = loop ? 1 : 0;
    ch->bus = (uint8_t)bus;
    ch->sound_id = (uint8_t)sound;
    ch->volume = clampf(volume, 0.0f, 1.0f);
    ch->pan = clampf(pan, -1.0f, 1.0f);
    ch->pitch = 1.0f;
    ch->sample_position = 0;
    return (int)i;
}

void FDAudio_StopAll(FDAudioSystem *audio) {
    uint32_t i;
    if (!audio) return;
    for (i = 0; i < FD_AUDIO_MAX_CHANNELS; ++i)
        audio->channels[i].active = 0;
}

void FDAudio_StopSound(FDAudioSystem *audio, FDAudioSoundId sound) {
    uint32_t i;
    if (!audio) return;
    for (i = 0; i < FD_AUDIO_MAX_CHANNELS; ++i) {
        if (audio->channels[i].active &&
            audio->channels[i].sound_id == (uint8_t)sound)
            audio->channels[i].active = 0;
    }
}

void FDAudio_Update(FDAudioSystem *audio, float dt) {
    uint32_t i;
    if (!audio || dt <= 0.0f) return;
    /* O avanço real dos samples será controlado pelo backend de áudio. */
    for (i = 0; i < FD_AUDIO_MAX_CHANNELS; ++i) {
        if (audio->channels[i].active) {
            audio->channels[i].volume =
                clampf(audio->channels[i].volume, 0.0f, 1.0f);
        }
    }
}

float FDAudio_GetEffectiveVolume(const FDAudioSystem *audio,
                                 const FDAudioChannel *channel) {
    if (!audio || !channel || !channel->active || audio->master_muted)
        return 0.0f;
    if (channel->bus >= FD_AUDIO_BUS_COUNT) return 0.0f;
    return clampf(channel->volume *
                  audio->bus_volume[channel->bus], 0.0f, 1.0f);
}
