
#ifndef SIMON_UTILS_H
#define SIMON_UTILS_H

// sync parameters' list the declared number of parameters
#include "DistrhoPluginInfo.h"

// does not seem feasible to go there
#define MAX_ROUND 128

enum Status {
                WAITING,
                STARTING,
                INSTRUCTIONS,
                PLAYING_WAIT, // waiting for the player
                PLAYING_CORRECT,
                PLAYING_INCORRECT,
                FEEDBACK_INCORRECT, // after an error we give feedback
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

// sharing parameters info across DSP and UI.
const ParameterRanges params[kParameterCount] =
    {
     // default, min, max.
     ParameterRanges(0, 0, 1), // start
     ParameterRanges(60, 0, 127), // root
     ParameterRanges(12, 1, 128), // number of notes
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(0, 0, 1), // shall not pass
     ParameterRanges(1, 0, MAX_ROUND), // rounds for miss
     ParameterRanges(60, 0, 127), // effective root
     ParameterRanges(12, 1, 128), // effective number of notes
     ParameterRanges(WAITING, 0, STATUS_COUNT), // status
     ParameterRanges(-1, -1, 127), // current active note
     ParameterRanges(0, 0, MAX_ROUND), // current round
     ParameterRanges(0, 0, MAX_ROUND), // current step number
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(1, 0, 1), // scale
     ParameterRanges(0, 0, MAX_ROUND), // current nb miss
     ParameterRanges(0, 0, MAX_ROUND), // max miss possible
     ParameterRanges(0, 0, MAX_ROUND), // max round completed
    };

// names for notes
static const char* const scaleNotes[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
// tune for illegal symbols
static const char* const scaleNotesSymbols[12] = {"C", "Cs", "D", "Ds", "E", "F", "Fs", "G", "Gs", "A", "As", "B"};

// implementing our own rand/srand following basic example from their man page, so as to have a separate sequence
class Rando {

  // we will not generate numbers beyond that
  const int RANDO_MAX = 32767; 

 public:
  /* RAND_MAX assumed to be 32767 */
  int rand(void) {
    next = next * 1103515245 + 12345;
    return((unsigned)(next/65536) % 32768);
  }

  // c
  void srand(unsigned int seed) {
    next = seed;
  }

 private:
  unsigned long next = 1;
  
};

// dealing with pseudo-presets in the ui
// negative values would mean that the parameter is not dealt with for this preset
struct Preset {
  // name limited by space on-screen anyhow
  char name[20];
  int root;
  int nbNotes;
  int scale[12];
};


#define NB_PRESETS 8

// note: two consecutive presets should not differ by only the presence of "-1", otherwise with current code it will not cycle
const Preset presets[NB_PRESETS] = {
  {"One Octave", 60, 12, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {"D major",    60, 12, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1},
  // aka All-Blacks
  {"F# Maj pentatonic", 60, 12, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0},
  {"25 keys", 60-12, 25, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {"49 keys", 60-24, 49, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {"61 keys", 60-24, 61, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {"88 keys", 60-39, 88, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {"Custom", -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

// return -1 if the preset cannot be found
inline int getPresetIdx(int root, int nbNotes, bool scale[12]) {
  int i;
  for (i=0; i < NB_PRESETS; i++) {
    if ((presets[i].root >= 0 && root != presets[i].root) ||
	(presets[i].nbNotes >= 0 && nbNotes != presets[i].nbNotes)) {
      continue;
    }
    int j;
    for (j=0; j < 12; j++) {
      if (presets[i].scale[j] >=0 && scale[j] != presets[i].scale[j]) {
	break;
      }
    }
    // stopped during scale verification, no good
    if (j < 12) {
      continue;
    }
    // if we arrive here we got our preset
    return i;
  }
  // nothing found
  return -1;
}

// build a list of presets' names, to be consumed by a GuiComboBox
static String getPresetsNames() {
  String names;
  for (int i=0; i < NB_PRESETS - 1; i++) {
    names = names + presets[i].name;
    names = names + ";";
  }
  // we don't want to end by a semi-column
  if(NB_PRESETS > 0) {
    names = names + presets[NB_PRESETS-1].name;
  }
  return names;
}

const String presetsNames = getPresetsNames();

#endif /* SIMON_UTILS_H */
