// AudioAnalysis.cpp
// Handles analyzing music and generating a note chart for CubeBeatz.

#include "AudioAnalysis.h"
#include "./miniaudio.h"

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

    // Audio decoding succeeded.
    // Beat detection will be implemented next.

    return true;
}
