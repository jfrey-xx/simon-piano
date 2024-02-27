
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
                PLAYING_OVER, // lost the round, wait before ending it for good (i.e. for key release)
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

#endif /* SIMON_UTILS_H */
