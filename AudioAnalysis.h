#pragma once

// AudioAnalysis.h
// Handles analyzing music and generating a note chart for CubeBeatz.

#include <vector>

// Identifies the type of rhythm note.
enum class NoteType
{
    TAP,HOLD
};

// Represents one detected note in the song.
struct Note
{
    double hitTime;                 // Scheduled time to hit the note.
    int lane;                       // Lane where the note appears.

    NoteType type = NoteType::TAP;  // Type of note.
    double duration = 0.0;          // Length of a hold note in seconds.
};

// Analyzes a music file and generates notes from detected beats.
bool AnalyzeMusic(
    const wchar_t* filename,
    std::vector<Note>& generatedNotes
);

