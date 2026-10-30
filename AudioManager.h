
#pragma once

// AudioManager.h
// Handles music playback for CubeBeatz.

#include "./miniaudio.h"

// Initializes the audio engine.
bool InitializeAudio();

// Loads an MP3 or WAV music file.
bool LoadMusic(const wchar_t* filename);

// Starts music playback.
bool PlayMusic();

// Pauses music while remembering its position.
void PauseMusic();

// Resumes music from its paused position.
void ResumeMusic();

// Stops music and resets its position.
void StopMusic();

// Returns the current playback position in seconds.
double GetMusicPosition();

// Releases audio resources.
void CleanupAudio();
