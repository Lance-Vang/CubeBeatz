#pragma once

// AudioAnalysis.h
// Handles analyzing music and generating a note chart for CubeBeatz.

#include <vector>

// Represents one detected note in the song.
struct Note
{
    double hitTime;
    int lane;
};

// Analyzes a music file and generates notes from detected beats.
bool AnalyzeMusic(
    const wchar_t* filename,
    std::vector<Note>& generatedNotes
);

