
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

enum Parameters {
    kStart,
    kRoot, 
    kNbNotes,
    // root and nb notes are not applied during game
    kEffectiveRoot,
    kEffectiveNbNotes,
    kStatus,
    kCurNote,

    kParameterCount
};

enum Status {
                WAITING,
                STARTING,
                INSTRUCTIONS,
                TYPING_CORRECT,
                TYPING_INCORRECT,
                GAMEOVER,

                STATUS_COUNT
};

// sharing parameters info across DSP and UI.
const ParameterRanges params[kParameterCount] =
    {
     // default, min, max.
     ParameterRanges(0, 0, 1), // start
     ParameterRanges(60, 0, 127), // root
     ParameterRanges(12, 0, 127), // number of notes
     ParameterRanges(60, 0, 127), // effective root
     ParameterRanges(12, 0, 127), // effective number of notes
     ParameterRanges(WAITING, 0, STATUS_COUNT), // status
     ParameterRanges(-1, -1, 127), // current active note
    };

#endif
