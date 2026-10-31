// AudioAnalysis.cpp
// Handles analyzing music and generating a note chart for CubeBeatz.

#include "AudioAnalysis.h"
#include "./miniaudio.h"

#define NOMINMAX
#include <windows.h>
#include <vector>
#include <cmath>
#include <algorithm>

// Analyzes a music file and generates notes from detected beats.
bool AnalyzeMusic(
    const wchar_t* filename,
    std::vector<Note>& generatedNotes
)
{
    // Clear any previous notes.
    generatedNotes.clear();

    // Check that a filename was provided.
    if (filename == nullptr || filename[0] == L'\0')
        return false;

    // Create an audio decoder.
    ma_decoder decoder;

    // Initialize the decoder using the music file.
    ma_result result = ma_decoder_init_file_w(
        filename,
        NULL,
        &decoder
    );

    if (result != MA_SUCCESS)
    {
        MessageBoxA(
            NULL,
            "Could not decode the music file.",
            "CubeBeatz Error",
            MB_OK | MB_ICONERROR
        );

        return false;
    }

    // Get information about the decoded audio.
    ma_uint64 totalFrames = 0;

    result = ma_decoder_get_length_in_pcm_frames(
        &decoder,
        &totalFrames
    );

    if (result != MA_SUCCESS || totalFrames == 0)
    {
        ma_decoder_uninit(&decoder);
        return false;
    }

    ma_uint32 channels = decoder.outputChannels;
    ma_uint32 sampleRate = decoder.outputSampleRate;

    if (channels == 0 || sampleRate == 0)
    {
        ma_decoder_uninit(&decoder);
        return false;
    }

    // Store the decoded audio samples.
    std::vector<float> audioSamples(
        static_cast<size_t>(totalFrames) * channels
    );

    ma_uint64 framesRead = 0;

    // Read the audio into memory.
    result = ma_decoder_read_pcm_frames(
        &decoder,
        audioSamples.data(),
        totalFrames,
        &framesRead
    );

    // Release the decoder.
    ma_decoder_uninit(&decoder);

    if (result != MA_SUCCESS || framesRead == 0)
        return false;

    // Each window contains 1024 audio frames.
    const ma_uint64 windowSize = 1024;

    // Store the energy measurement for each window.
    std::vector<float> audioEnergy;

    // Process the decoded audio in small windows.
    for (ma_uint64 startFrame = 0;
        startFrame < framesRead;
        startFrame += windowSize)
    {
        ma_uint64 endFrame = std::min(
            startFrame + windowSize,
            framesRead
        );

        double totalEnergy = 0.0;
        ma_uint64 sampleCount = 0;

        // Examine every audio sample in this window.
        for (ma_uint64 frame = startFrame;
            frame < endFrame;
            frame++)
        {
            for (ma_uint32 channel = 0;
                channel < channels;
                channel++)
            {
                size_t sampleIndex =
                    static_cast<size_t>(frame) * channels + channel;

                float sample = audioSamples[sampleIndex];

                // Square the sample to measure its energy.
                totalEnergy += sample * sample;
                sampleCount++;
            }
        }

        // Calculate the average energy of this window.
        float averageEnergy = 0.0f;

        if (sampleCount > 0)
        {
            averageEnergy = static_cast<float>(
                totalEnergy / sampleCount
                );
        }

        audioEnergy.push_back(averageEnergy);
    }

    // Audio energy measurements are now available.
    // Begin detecting sudden increases in audio energy.
   
    // Minimum time between detected notes.
    const double minimumNoteSpacing = 0.20;

    // Remember the time of the previous detected note.
    double lastNoteTime = -minimumNoteSpacing;

    // Tracks which of the six lanes receives the next note.
    int nextLane = 0;

    // Examine each energy window.
    for (size_t i = 1; i + 1 < audioEnergy.size(); i++)
    {
        // Get the energy of the current window.
        float currentEnergy = audioEnergy[i];

        // Get the energy of the previous window.
        float previousEnergy = audioEnergy[i - 1];

        // Get the energy of the next window.
        float nextEnergy = audioEnergy[i + 1];

        // Calculate the increase in energy.
        float energyIncrease = currentEnergy - previousEnergy;

        // Check whether this window is a local energy peak.
        bool isPeak =
            currentEnergy > previousEnergy &&
            currentEnergy >= nextEnergy;

        // Ignore small increases in energy.
        const float minimumIncrease = 0.0005f;

        if (isPeak && energyIncrease > minimumIncrease)
        {
            // Convert the audio window position into seconds.
            double noteTime =
                static_cast<double>(i * windowSize) / sampleRate;

            // Only add a note if enough time has passed.
            if (noteTime - lastNoteTime >= minimumNoteSpacing)
            {
                // Add the detected note to the current lane.
                generatedNotes.push_back({ noteTime, nextLane });

                // Move to the next lane.
                nextLane = (nextLane + 1) % 6;

                // Remember when the last note was generated.
                lastNoteTime = noteTime;
            }
        }
    }

    return true;
}

