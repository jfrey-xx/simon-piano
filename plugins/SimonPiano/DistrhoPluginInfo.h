
#ifndef DISTRHO_PLUGIN_INFO_H_INCLUDED
#define DISTRHO_PLUGIN_INFO_H_INCLUDED

#define DISTRHO_PLUGIN_NAME  "SimonPiano"
#define DISTRHO_PLUGIN_URI   "http://jfrey.info/simon-piano"
#define DISTRHO_PLUGIN_CLAP_ID "simon-piano"

#define DISTRHO_PLUGIN_NUM_INPUTS   0
#define DISTRHO_PLUGIN_NUM_OUTPUTS  0
#define DISTRHO_PLUGIN_WANT_MIDI_INPUT 1
#define DISTRHO_PLUGIN_WANT_MIDI_OUTPUT 1
#define DISTRHO_PLUGIN_IS_RT_SAFE   1

#define DISTRHO_PLUGIN_HAS_UI 1
#define DISTRHO_UI_USE_CUSTOM 1
#define DISTRHO_UI_CUSTOM_INCLUDE_PATH "DearImGui.hpp"
#define DISTRHO_UI_CUSTOM_WIDGET_TYPE DGL_NAMESPACE::ImGuiTopLevelWidget
#define DISTRHO_UI_DEFAULT_WIDTH 600
#define DISTRHO_UI_DEFAULT_HEIGHT 400
#define DISTRHO_UI_USER_RESIZABLE 1
#define DISTRHO_UI_URI DISTRHO_PLUGIN_URI "#UI"

// does not seem feasible to go there
#define MAX_ROUND 128

enum Parameters {
    kStart,
    kRoot, 
    kNbNotes,
    // root and nb notes are not applied during game
    kEffectiveRoot,
    kEffectiveNbNotes,
    kStatus,
    kCurNote,
    kRound,

    kParameterCount
};

enum Status {
                WAITING,
                STARTING,
                INSTRUCTIONS,
                PLAYING_WAIT,
                PLAYING_CORRECT,
                PLAYING_INCORRECT,
                GAMEOVER,

                STATUS_COUNT
};

// is a game currently playing -- quick and ugly placing it here to share between DSP and UI
inline bool isRunning(int status) {
    return !(status == WAITING || status == GAMEOVER);
}
// player's turn
inline bool isPlaying(int status) {
    return (status == PLAYING_WAIT || status == PLAYING_CORRECT || status == PLAYING_INCORRECT);
}

// check if a played note is similar to one of interest, considering the root note and the interval (number of notes). Tries to be as smart as possible, if the input note is out of range consider that the position is just shifted (user might not have the correct octave configured)
inline  bool isCorrespondingNote(int note, int target, int root, int nbNotes) {
    // round number of notes to next octave
  int interval = ceil(nbNotes / (float) 12) * 12;
    d_stdout("note: %d, target: %d, interval: %d", note, target, interval);
    d_stdout("note shift: %d, target shift: %d", note + root % 12, target + root % 12);
    d_stdout("note shift mod: %d, target shift mod: %d", (note + root % 12) % interval, (target + root % 12) % interval);
    return (note + root % 12) % interval  == (target + root % 12) % interval;
  }

// sharing parameters info across DSP and UI.
const ParameterRanges params[kParameterCount] =
    {
     // default, min, max.
     ParameterRanges(0, 0, 1), // start
     ParameterRanges(60, 0, 127), // root
     ParameterRanges(12, 1, 128), // number of notes
     ParameterRanges(60, 0, 127), // effective root
     ParameterRanges(12, 1, 128), // effective number of notes
     ParameterRanges(WAITING, 0, STATUS_COUNT), // status
     ParameterRanges(-1, -1, 127), // current active note
     ParameterRanges(0, 0, MAX_ROUND), // current active note
    };

#endif
