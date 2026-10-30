#define MINIAUDIO_IMPLEMENTATION
#include "./miniaudio.h"
#include "AudioManager.h"

#include <windows.h>
// Audio engine.
static ma_engine audioEngine;

// Sound object representing the loaded music.
static ma_sound musicSound;

// Tracks whether the audio engine and music are ready.
static bool audioInitialized = false;
static bool musicLoaded = false;

// Stores the music position when paused.
static ma_uint64 pausedPosition = 0;

// Initializes miniaudio.
bool InitializeAudio()
{
    ma_result result = ma_engine_init(NULL, &audioEngine);

    if (result != MA_SUCCESS)
    {
        MessageBoxA(
            NULL,
            "Could not initialize audio.",
            "CubeBeatz Error",
            MB_OK | MB_ICONERROR
        );

        return false;
    }

    audioInitialized = true;
    return true;
}

// Loads a music file.
bool LoadMusic(const wchar_t* filename)
{
    if (!audioInitialized)
        return false;

    if (musicLoaded)
    {
        ma_sound_uninit(&musicSound);
        musicLoaded = false;
    }

    ma_result result = ma_sound_init_from_file_w(
        &audioEngine,
        filename,
        0,
        NULL,
        NULL,
        &musicSound
    );

    if (result != MA_SUCCESS)
    {
        MessageBoxA(
            NULL,
            "Could not load the music file.",
            "CubeBeatz Error",
            MB_OK | MB_ICONERROR
        );

        return false;
    }

    musicLoaded = true;
    return true;
}

// Starts music playback.
bool PlayMusic()
{
    if (!musicLoaded)
        return false;

    return ma_sound_start(&musicSound) == MA_SUCCESS;
}

// Pauses music and remembers its current position.
void PauseMusic()
{
    if (musicLoaded)
    {
        ma_sound_get_cursor_in_pcm_frames(
            &musicSound,
            &pausedPosition
        );

        ma_sound_stop(&musicSound);
    }
}

// Resumes music from its previous position.
void ResumeMusic()
{
    if (musicLoaded)
    {
        ma_sound_seek_to_pcm_frame(
            &musicSound,
            pausedPosition
        );

        ma_sound_start(&musicSound);
    }
}
// Stops music and resets its playback position.
void StopMusic()
{
    if (musicLoaded)
    {
        ma_sound_stop(&musicSound);
        ma_sound_seek_to_pcm_frame(&musicSound, 0);

        pausedPosition = 0;
    }
}

// Gets current playback position in seconds.
double GetMusicPosition()
{
    if (!musicLoaded)
        return 0.0;

    ma_uint64 framePosition = 0;

    if (ma_sound_get_cursor_in_pcm_frames(
        &musicSound,
        &framePosition
    ) != MA_SUCCESS)
    {
        return 0.0;
    }

    ma_uint32 sampleRate =
        ma_engine_get_sample_rate(&audioEngine);

    if (sampleRate == 0)
        return 0.0;

    return static_cast<double>(framePosition) / sampleRate;
}

// Releases audio resources.
void CleanupAudio()
{
    if (musicLoaded)
    {
        ma_sound_uninit(&musicSound);
        musicLoaded = false;
    }

    if (audioInitialized)
    {
        ma_engine_uninit(&audioEngine);
        audioInitialized = false;
    }
}
