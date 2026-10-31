// AudioAnalysis.cpp
// Handles analyzing music and generating a note chart for CubeBeatz.

#include "AudioAnalysis.h"
#include "./miniaudio.h"

#define NOMINMAX
#include <windows.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <complex>

// Performs a Fast Fourier Transform on audio samples.
void PerformFFT(std::vector<std::complex<double>>& samples)
{
    // Get the number of samples.
    size_t count = samples.size();

    // FFT requires a power-of-two sample count.
    if (count == 0 || (count & (count - 1)) != 0)
        return;

    // Reorder samples using bit-reversed indices.
    size_t j = 0;

    for (size_t i = 1; i < count; i++)
    {
        size_t bit = count >> 1;

        while (j & bit)
        {
            j ^= bit;
            bit >>= 1;
        }

        j ^= bit;

        if (i < j)
        {
            std::swap(samples[i], samples[j]);
        }
    }

    // Process increasingly larger groups of samples.
    for (size_t length = 2; length <= count; length *= 2)
    {
        // Calculate the rotation angle for this group.
        // PI constant used for calculating FFT rotation angles.
        const double PI = 3.14159265358979323846;

        double angle = -2.0 * PI /
            static_cast<double>(length);

        std::complex<double> root(
            std::cos(angle),
            std::sin(angle)
        );

        // Process each group.
        for (size_t start = 0; start < count; start += length)
        {
            std::complex<double> rotation(1.0, 0.0);

            for (size_t k = 0; k < length / 2; k++)
            {
                // Get the two samples being combined.
                auto even = samples[start + k];
                auto odd = samples[start + k + length / 2]
                    * rotation;

                // Combine the samples.
                samples[start + k] = even + odd;

                samples[start + k + length / 2] =
                    even - odd;

                // Update the rotation factor.
                rotation *= root;
            }
        }
    }
}


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

    // Stores frequency magnitudes for every audio window.
    std::vector<std::vector<float>> spectralMagnitudes;

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


        // Prepare audio samples for the FFT.
        std::vector<std::complex<double>> fftSamples(
            static_cast<size_t>(windowSize),
            std::complex<double>(0.0, 0.0)
        );

        // Convert the audio window into mono FFT samples.
        for (ma_uint64 frame = startFrame;
            frame < endFrame;
            frame++)
        {
            double monoSample = 0.0;

            // Combine audio channels into one sample.
            for (ma_uint32 channel = 0;
                channel < channels;
                channel++)
            {
                size_t sampleIndex =
                    static_cast<size_t>(frame) * channels + channel;

                monoSample += audioSamples[sampleIndex];
            }

            monoSample /= channels;

            // Calculate the position inside this FFT window.
            size_t windowIndex =
                static_cast<size_t>(frame - startFrame);

            // Apply the Hann window.
            const double PI = 3.14159265358979323846;

            double hannWindow =
                0.5 * (1.0 - std::cos(
                    2.0 * PI * windowIndex / (windowSize - 1)
                ));

            // Store the windowed sample as a complex number.
            fftSamples[windowIndex] =
                std::complex<double>(monoSample * hannWindow, 0.0);
        }

        // Perform the Fast Fourier Transform.
        PerformFFT(fftSamples);

        // Store the magnitude of each frequency bin.
        std::vector<float> magnitudes;

        for (size_t bin = 0; bin <= windowSize / 2; bin++)
        {
            double realPart = fftSamples[bin].real();
            double imaginaryPart = fftSamples[bin].imag();

            double magnitude = std::sqrt(
                realPart * realPart +
                imaginaryPart * imaginaryPart
            );

            magnitudes.push_back(static_cast<float>(magnitude));
        }

        // Save the frequency information for this window.
        spectralMagnitudes.push_back(magnitudes);

    }

    // Stores spectral flux values for each audio window.
    std::vector<float> spectralFlux;

    // Compare consecutive audio windows to detect frequency changes.
    for (size_t i = 1; i < spectralMagnitudes.size(); i++)
    {
        // Get the frequency magnitudes from both windows.
        const std::vector<float>& previousMagnitudes =
            spectralMagnitudes[i - 1];

        const std::vector<float>& currentMagnitudes =
            spectralMagnitudes[i];

        // Store the total increase in frequency energy.
        double flux = 0.0;

        // Compare each frequency bin.
        for (size_t bin = 1; bin < currentMagnitudes.size(); bin++)
        {
            // Calculate the change in this frequency bin.
            float difference =
                currentMagnitudes[bin] - previousMagnitudes[bin];

            // Only count increases in frequency energy.
            if (difference > 0.0f)
            {
                flux += difference;
            }
        }

        // Calculate the average positive frequency change.
        float averageFlux = static_cast<float>(
            flux / (currentMagnitudes.size() - 1)
            );

        // Store the spectral flux for this audio window.
        spectralFlux.push_back(averageFlux);
    }


    // Spectral flux measurements are now available.
    // Begin detecting sudden increases in frequency activity.

    // Minimum time between detected notes.
    const double minimumNoteSpacing = 0.20;

    // Number of previous windows used to calculate background flux.
    const size_t analysisWindow = 20;

    // Remember the time of the previous detected note.
    double lastNoteTime = -minimumNoteSpacing;

    // Random number generator for selecting note lanes.
    std::random_device randomDevice;
    std::mt19937 randomGenerator(randomDevice());
    // Selects one of the two lanes within a frequency range.
    std::uniform_int_distribution<int> lanePairDistribution(0, 1);

    // Remembers the previous lane.
    int previousLane = -1;

    // Examine spectral flux values for local peaks.
    for (size_t i = 1; i + 1 < spectralFlux.size(); i++)
    {
        // Get the current spectral flux value.
        float currentFlux = spectralFlux[i];

        // Get the previous spectral flux value.
        float previousFlux = spectralFlux[i - 1];

        // Get the next spectral flux value.
        float nextFlux = spectralFlux[i + 1];

        // Calculate the average flux from recent previous windows.
        double recentFlux = 0.0;
        size_t samplesUsed = 0;

        size_t start = (i > analysisWindow)
            ? i - analysisWindow
            : 0;

        for (size_t j = start; j < i; j++)
        {
            recentFlux += spectralFlux[j];
            samplesUsed++;
        }

        float averageRecentFlux = 0.0f;

        if (samplesUsed > 0)
        {
            averageRecentFlux =
                static_cast<float>(recentFlux / samplesUsed);
        }

        // Check whether the current flux is a local peak.
        bool isPeak =
            currentFlux > previousFlux &&
            currentFlux >= nextFlux;

        // Require the peak to stand out from recent activity.
        bool isStrongPeak =
            currentFlux > averageRecentFlux * 1.5f;

        // Only generate notes for strong spectral flux peaks.
        if (isPeak && isStrongPeak && currentFlux > 0.0f)
        {
            // Flux index i represents audio window i + 1.
            double noteTime =
                static_cast<double>((i + 1) * windowSize)
                / sampleRate;

            // Only add a note if enough time has passed.
            if (noteTime - lastNoteTime >= minimumNoteSpacing)
            {
                // Get the magnitude data from the two windows
 // associated with this spectral flux peak.
                const std::vector<float>& previousMagnitudes = spectralMagnitudes[i];
                const std::vector<float>& currentMagnitudes = spectralMagnitudes[i + 1];

                // Store the positive frequency changes for each range.
                float bassChange = 0.0f;
                float midChange = 0.0f;
                float highChange = 0.0f;

                // Compare the frequency changes between the two windows.
                for (size_t bin = 1; bin < currentMagnitudes.size(); bin++)
                {
                    // Convert the FFT bin into its frequency in Hz.
                    float frequencyHz =
                        static_cast<float>(bin) * sampleRate / windowSize;

                    // Measure only increases in magnitude.
                    float change = currentMagnitudes[bin] - previousMagnitudes[bin];

                    if (change < 0.0f)
                        change = 0.0f;

                    // Add the change to its corresponding frequency range.
                    if (frequencyHz >= 20.0f && frequencyHz < 250.0f)
                    {
                        bassChange += change;
                    }
                    else if (frequencyHz >= 250.0f && frequencyHz < 2000.0f)
                    {
                        midChange += change;
                    }
                    else if (frequencyHz >= 2000.0f && frequencyHz <= 8000.0f)
                    {
                        highChange += change;
                    }
                }

                // Determine which frequency range changed the most.
                int frequencyRange = 0;

                if (midChange > bassChange && midChange >= highChange)
                {
                    frequencyRange = 1;
                }
                else if (highChange > bassChange && highChange > midChange)
                {
                    frequencyRange = 2;
                }

                // Select one of the two lanes belonging to that range.
                int selectedLane =
                    (frequencyRange * 2) + lanePairDistribution(randomGenerator);

                // Prevent consecutive notes from using the same lane.
                while (selectedLane == previousLane)
                {
                    selectedLane =
                        (frequencyRange * 2) + lanePairDistribution(randomGenerator);
                }

                // Add the note to the selected lane.
                generatedNotes.push_back({ noteTime, selectedLane });

                // Remember the selected lane.
                previousLane = selectedLane;

                // Remember when the last note was generated.
                lastNoteTime = noteTime;
            }
        }
    }


    return true;

}

