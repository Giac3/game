#pragma once

#include <stdbool.h>
#include "miniaudio.h"
#include "../types.h"
#include "../util/util.h"

typedef struct {
    ma_engine engine;
} AudioContext;

typedef struct {
    ma_sound sound;
} AudioSound;

typedef AudioSound AudioMusic;  // Alias for clarity

void audio_init(void);
void audio_uninit(void);

void audio_sound_load(AudioSound *audio_sound, const char *path);
void audio_sound_play(AudioSound *audio_sound);
void audio_sound_unload(AudioSound *audio_sound);

void audio_music_load(AudioMusic *audio_music, const char *path);
void audio_music_play(AudioMusic *audio_music);
void audio_music_unload(AudioMusic *audio_music);