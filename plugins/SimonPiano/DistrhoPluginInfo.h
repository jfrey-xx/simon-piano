
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
#define DISTRHO_UI_DEFAULT_HEIGHT 500
#define DISTRHO_UI_USER_RESIZABLE 1
#define DISTRHO_UI_URI DISTRHO_PLUGIN_URI "#UI"

enum Parameters {
    kStart,
    kRoot, 
    kNbNotes,
    // toggle for each note
    // NB: must be consecutive for some DSP trickery!
    kScaleC,
    kScaleCs,
    kScaleD,
    kScaleDs,
    kScaleE,
    kScaleF,
    kScaleFs,
    kScaleG,
    kScaleGs,
    kScaleA,
    kScaleAs,
    kScaleB,
    kShallNotPass,
    kRoundsForMiss,

    // output parameter from here
    
    // root and nb notes are not applied during game
    kEffectiveRoot,
    kEffectiveNbNotes,
    kStatus,
    kCurNote,
    kRound,
    kStep,
    // same for scale, even if very tedious
    // NB: here as well must be consecutive
    kEffectiveScaleC,
    kEffectiveScaleCs,
    kEffectiveScaleD,
    kEffectiveScaleDs,
    kEffectiveScaleE,
    kEffectiveScaleF,
    kEffectiveScaleFs,
    kEffectiveScaleG,
    kEffectiveScaleGs,
    kEffectiveScaleA,
    kEffectiveScaleAs,
    kEffectiveScaleB,
    // info about miss
    kNbMiss,
    kMaxMiss,
    // some misc info
    kMaxRound,

    kParameterCount
};

#endif
