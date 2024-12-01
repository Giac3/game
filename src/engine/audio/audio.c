#include "audio.h"
#include "../types.h"
#include "../util/util.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

static AudioContext g_audioContext;

void audio_init(void) {
    ma_context_config contextConfig = ma_context_config_init();
    
    ma_result result = ma_engine_init(NULL, &g_audioContext.engine);
    if (result != MA_SUCCESS) {
        ERROR_EXIT("Failed to initialize audio engine.");
    }
}

void audio_uninit(void) {
    ma_engine_uninit(&g_audioContext.engine);
}

void audio_sound_load(AudioSound *audio_sound, const char *path) {
    ma_result result = ma_sound_init_from_file(
        &g_audioContext.engine, path,
        MA_SOUND_FLAG_DECODE,
        NULL, NULL, &audio_sound->sound
    );
    if (result != MA_SUCCESS) {
        ERROR_EXIT("Failed to load sound effect: %s", path);
    }
}

void audio_sound_play(AudioSound *audio_sound) {
    ma_sound_stop(&audio_sound->sound);
    ma_sound_seek_to_pcm_frame(&audio_sound->sound, 0);
    ma_sound_start(&audio_sound->sound);
}

void audio_sound_unload(AudioSound *audio_sound) {
    ma_sound_uninit(&audio_sound->sound);
}

void audio_music_load(AudioMusic *audio_music, const char *path) {
    ma_result result = ma_sound_init_from_file(
        &g_audioContext.engine, path,
        MA_SOUND_FLAG_STREAM,
        NULL, NULL, &audio_music->sound
    );
    if (result != MA_SUCCESS) {
        ERROR_EXIT("Failed to load music: %s", path);
    }
    ma_sound_set_looping(&audio_music->sound, MA_TRUE);
}

void audio_music_play(AudioMusic *audio_music) {
    ma_sound_start(&audio_music->sound);
}

void audio_music_unload(AudioMusic *audio_music) {
    ma_sound_uninit(&audio_music->sound);
}