
#include "ExtendedPlugin.hpp"
#include "SimonUtils.h"
#include <time.h> 

START_NAMESPACE_DISTRHO


// time in seconds between notes for instructions (and before first instruction)
#define NOTE_INTERVAL 0.25
// how long each note is held during instruction
#define NOTE_DURATION 0.5

class SimonPiano : public ExtendedPlugin {
public:
  // Note: do not care with default values since we will sent all parameters upon init
  SimonPiano() : ExtendedPlugin(kParameterCount, 0, 0) {
    // randomize seed for our number generator
    ran.srand(time(NULL));
    // make sure to init all variables
    reset();
  }

protected:
  // metadata
  const char *getLabel() const override { return "SimonPiano"; }
  const char *getDescription() const override {
    return "Follow the lead, play after me";
  }
  const char *getMaker() const override { return "jfrey"; }
  uint32_t getVersion() const override { return d_version(1,0,0); }
  int64_t getUniqueId() const override { 
    return d_cconst('S','I','P','I'); 
  }
  
  // params
  void initParameter (uint32_t index, Parameter& parameter) override {

    switch (index) {
    case kStart:
      // must go from false to true during WAITING state to take effect
      parameter.hints = kParameterIsAutomatable|kParameterIsBoolean;
      parameter.name = "Start new round";
      parameter.shortName = "start";
      parameter.symbol = "start";
      parameter.unit = "";
      parameter.ranges.def = params[kStart].def;
      parameter.ranges.min = params[kStart].min;
      parameter.ranges.max = params[kStart].max;
      break;
    case kRoot:
      parameter.hints = kParameterIsInteger | kParameterIsAutomatable;
      parameter.name = "Output root note";
      parameter.shortName = "root";
      parameter.symbol = "rootnote";
      parameter.unit = "";
      parameter.ranges.def = params[kRoot].def;
      parameter.ranges.min = params[kRoot].min;
      parameter.ranges.max = params[kRoot].max;
      break;
    case kNbNotes:
      parameter.hints = kParameterIsInteger | kParameterIsAutomatable;
      parameter.name = "Number of notes";
      parameter.shortName = "nb notes";
      parameter.symbol = "nbnotes";
      parameter.unit = "notes";
      parameter.ranges.def = params[kNbNotes].def;
      parameter.ranges.min = params[kNbNotes].min;
      parameter.ranges.max = params[kNbNotes].max;
      break;
    case kScaleC:
    case kScaleCs:
    case kScaleD:
    case kScaleDs:
    case kScaleE:
    case kScaleF:
    case kScaleFs:
    case kScaleG:
    case kScaleGs:
    case kScaleA:
    case kScaleAs:
    case kScaleB:
      parameter.hints = kParameterIsAutomatable|kParameterIsBoolean;
      {
        // we will do our bet to make that a tiny bit less tedious
        int numScale = index - kScaleC;
        if (numScale >= 0 && numScale < 12) {
          parameter.name = scaleNotes[numScale];
          parameter.shortName = scaleNotes[numScale];
          parameter.symbol = scaleNotes[numScale];
        }
      }
      parameter.unit = "";
      parameter.ranges.def = params[index].def;
      parameter.ranges.min = params[index].min;
      parameter.ranges.max = params[index].max;
      break;
    case kRoundsForMiss:
      // how many rounds to grant a new miss. 0 to disable.
      parameter.hints = kParameterIsInteger | kParameterIsAutomatable;
      parameter.name = "Rounds to grant a miss";
      parameter.shortName = "rnd f mis";
      parameter.symbol = "roundsformiss";
      parameter.unit = "rounds";
      parameter.ranges.def = params[index].def;
      parameter.ranges.min = params[index].min;
      parameter.ranges.max = params[index].max;
      break;
    case kEffectiveRoot:
      parameter.hints = kParameterIsInteger | kParameterIsOutput;
      parameter.name = "Effective root";
      parameter.shortName = "eff root";
      parameter.symbol = "effectiveroot";
      parameter.unit = "";
      parameter.ranges.def = params[kEffectiveRoot].def;
      parameter.ranges.min = params[kEffectiveRoot].min;
      parameter.ranges.max = params[kEffectiveRoot].max;
      break;
    case kEffectiveNbNotes:
      parameter.hints = kParameterIsInteger | kParameterIsOutput;
      parameter.name = "Effective number of notes";
      parameter.shortName = "eff nbnotes";
      parameter.symbol = "effectivenbroot";
      parameter.unit = "notes";
      parameter.ranges.def = params[kEffectiveNbNotes].def;
      parameter.ranges.min = params[kEffectiveNbNotes].min;
      parameter.ranges.max = params[kEffectiveNbNotes].max;
      break;
    case kStatus:
      // used to pass to UI what general status and current note is about
      parameter.hints = kParameterIsInteger | kParameterIsOutput;
      parameter.name = "Status";
      parameter.shortName = "stat";
      parameter.symbol = "status";
      parameter.unit = "";
      parameter.ranges.def = params[kStatus].def;
      parameter.ranges.min = params[kStatus].min;
      parameter.ranges.max = params[kStatus].max;
      break;
    case kCurNote:
      // used to pass to UI current active note
      parameter.hints = kParameterIsInteger | kParameterIsOutput;
      parameter.name = "Current active note";
      parameter.shortName = "note";
      parameter.symbol = "curnote";
      parameter.unit = "";
      parameter.ranges.def = params[kCurNote].def;
      parameter.ranges.min = params[kCurNote].min;
      parameter.ranges.max = params[kCurNote].max;
      break;
    case kRound:
      // what round we are at
      parameter.hints = kParameterIsInteger | kParameterIsOutput;
      parameter.name = "Round number";
      parameter.shortName = "round";
      parameter.symbol = "round";
      parameter.unit = "round";
      parameter.ranges.def = params[kRound].def;
      parameter.ranges.min = params[kRound].min;
      parameter.ranges.max = params[kRound].max;
      break;
    case kStep:
      // what step we are at
      parameter.hints = kParameterIsInteger | kParameterIsOutput;
      parameter.name = "Step number";
      parameter.shortName = "step";
      parameter.symbol = "step";
      parameter.unit = "step";
      parameter.ranges.def = params[kStep].def;
      parameter.ranges.min = params[kStep].min;
      parameter.ranges.max = params[kStep].max;
      break;
    case kEffectiveScaleC:
    case kEffectiveScaleCs:
    case kEffectiveScaleD:
    case kEffectiveScaleDs:
    case kEffectiveScaleE:
    case kEffectiveScaleF:
    case kEffectiveScaleFs:
    case kEffectiveScaleG:
    case kEffectiveScaleGs:
    case kEffectiveScaleA:
    case kEffectiveScaleAs:
    case kEffectiveScaleB:
      parameter.hints = kParameterIsBoolean | kParameterIsOutput;
      {
          // we will do our bet to make that a tiny bit less tedious
        int numScale = index - kEffectiveScaleC;
        if (numScale >= 0 && numScale < 12) {
          parameter.name = String("Effective ") + scaleNotes[numScale];
          parameter.shortName = String("eff ") + scaleNotes[numScale];
          parameter.symbol = String("effective") + scaleNotes[numScale];
          }
      }
      parameter.unit = "";
      parameter.ranges.def = params[index].def;
      parameter.ranges.min = params[index].min;
      parameter.ranges.max = params[index].max;
      break;
    case kNbMiss:
      parameter.hints = kParameterIsInteger | kParameterIsOutput;
      parameter.name = "Number of miss";
      parameter.shortName = "nb miss";
      parameter.symbol = "nbmiss";
      parameter.unit = "miss";
      parameter.ranges.def = params[index].def;
      parameter.ranges.min = params[index].min;
      parameter.ranges.max = params[index].max;
      break;
    case kMaxMiss:
      parameter.hints = kParameterIsInteger | kParameterIsOutput;
      parameter.name = "Max number of miss";
      parameter.shortName = "max miss";
      parameter.symbol = "maxmiss";
      parameter.unit = "miss";
      parameter.ranges.def = params[index].def;
      parameter.ranges.min = params[index].min;
      parameter.ranges.max = params[index].max;
      break;
    case kMaxRound:
      parameter.hints = kParameterIsInteger | kParameterIsOutput;
      parameter.name = "Max round reached";
      parameter.shortName = "max round";
      parameter.symbol = "maxround";
      parameter.unit = "rounds";
      parameter.ranges.def = params[index].def;
      parameter.ranges.min = params[index].min;
      parameter.ranges.max = params[index].max;
      break;

    default:
      break;
    }
    
    // effectively set parameter
    setParameterValue(index, parameter.ranges.def);
  }

  float getParameterValue(uint32_t index) const override {
    switch (index) {

    case kStart:
      return start;
    case kRoot:
      return root;
    case kNbNotes:
      return nbNotes;
    case kScaleC:
    case kScaleCs:
    case kScaleD:
    case kScaleDs:
    case kScaleE:
    case kScaleF:
    case kScaleFs:
    case kScaleG:
    case kScaleGs:
    case kScaleA:
    case kScaleAs:
    case kScaleB:
      {
        int numScale = index - kScaleC;
        if (numScale >= 0 && numScale < 12) {
          return scale[index];
        }
      }
      return 0.9;
    case kRoundsForMiss:
      return roundsForMiss;
    case kEffectiveRoot:
      return effectiveRoot;
    case kEffectiveNbNotes:
      return effectiveNbNotes;
    case kStatus:
      return status;
    case kCurNote:
      return curNote;
    case kRound:
      return round;
    case kStep:
      return stepN;
    case kEffectiveScaleC:
    case kEffectiveScaleCs:
    case kEffectiveScaleD:
    case kEffectiveScaleDs:
    case kEffectiveScaleE:
    case kEffectiveScaleF:
    case kEffectiveScaleFs:
    case kEffectiveScaleG:
    case kEffectiveScaleGs:
    case kEffectiveScaleA:
    case kEffectiveScaleAs:
    case kEffectiveScaleB:
      {
        int numScale = index - kEffectiveScaleC;
        if (numScale >= 0 && numScale < 12) {
          return effectiveScale[numScale];
        }
      }
      return 0.9;
    case kNbMiss:
      return nbMiss;
    case kMaxMiss:
      return maxMiss;
    case kMaxRound:
      return maxRound;

    default:
      return 0.0;
    }
  }
  
  void setParameterValue(uint32_t index, float value) override {
    switch (index) {
    case kStart:
      // only effectively start if waiting for it
      if (start == false && value != start) {
        d_stdout("new start");
        if (!isRunning(status)) {
          newGame();
        }
      }
      start = value;
      break;
    case kRoot:
      root = value;
      break;
    case kNbNotes:
      nbNotes = value;
      break;
    case kScaleC:
    case kScaleCs:
    case kScaleD:
    case kScaleDs:
    case kScaleE:
    case kScaleF:
    case kScaleFs:
    case kScaleG:
    case kScaleGs:
    case kScaleA:
    case kScaleAs:
    case kScaleB:
      {
        int numScale = index - kScaleC;
        if (numScale >= 0 && numScale < 12) {
          scale[numScale] = value;
        }
      }
      break;
    case kRoundsForMiss:
      roundsForMiss = value;
      break;
    case kEffectiveRoot:
      effectiveRoot = value;
      break;
    case kEffectiveNbNotes:
      effectiveNbNotes = value;
      break;
    case kCurNote:
      curNote = value;
      break;

    default:
      break;
    }
  }

  // turning off current note
  // frame: frame of the event in the buffer
  void abortCurrentNote(uint32_t frame=0) {
    if (curNote >= 0) {
      d_stdout("kill previous note %d at channel %d", curNote, curChannel);
      sendNoteOff(curNote, curChannel, frame);
      curNote = -1;
    }
  }

  // shift input, considering the current root note and the interval (number of notes) on interest
  int shiftNote(int note) {
    // round number of notes to next octave
    int interval = ceil(effectiveNbNotes / (float) 12) * 12;
    // we seek position in interval
    note = (note - effectiveRoot) % interval;
    // % is remainder in C, not modulo, adapt
    if (note < 0) {
      note += interval;
    }
    d_stdout("shiftnote: %d", note + effectiveRoot);
    // shift back compared to root
    return note + effectiveRoot;
  }

  // user playing notes, shifting note to interval of interest.  keep channel and velocity for user during pass-through
  void noteOn(uint8_t note, uint8_t velocity, uint8_t channel, uint32_t frame) {
    d_stdout("NoteOn %d, channel %d, frame %d", note, channel, frame);
    // Tries to be as smart as possible, if the input note is out of range consider that the position is just shifted (user might not have the correct octave configured)
    note = shiftNote(note); 
    
    // while the game is not running we can hit notes to try-out
    if (!isRunning(status)) {
      d_stdout("not running, pass it");
      // disable any currently playing note -- i.e. monophonic
      abortCurrentNote(frame);
      sendNoteOn(note, velocity, channel, frame);
      curNote = note;
      curChannel = channel;
    }
    // only pass through during playing until last note, keep all info
    else if (isPlaying(status) && (int) stepN < round) {
      d_stdout("pass it");
      // disable any currently playing note -- i.e. monophonic
      if (curNote >= 0) {
        d_stdout("kill previous note %d", curNote);
        sendNoteOff(curNote, channel, frame);
      }
      sendNoteOn(note, velocity, channel, frame);
      curNote = note;
      curChannel = channel;
      if (stepN < MAX_ROUND && curNote == sequence[stepN]) {
        d_stdout("correct");
        status = PLAYING_CORRECT;
        stepN++;
      }
      else {
        d_stdout("incorrect. current miss: %d / %d", nbMiss, maxMiss);
        status = PLAYING_INCORRECT;
        nbMiss++;
        if (nbMiss > maxMiss) {
          d_stdout("game over!");
          status = PLAYING_OVER;
        }
      }
    }      
  }

  // will detect new round upon note off of last playing
  void noteOff(uint8_t note, uint8_t channel, uint32_t frame) {
    d_stdout("NoteOff %d, channel %d, frame %d", note, channel, frame);
    // shift here is well
    note = shiftNote(note);

    // do not change status in-between games, just pass-through note off events
    if (!isRunning(status)) {
      d_stdout("not running, pass it");
      curNote = -1;
      sendNoteOff(note, channel, frame);
    }
    // while playing check if round is over
    else if (isPlaying(status) || status == PLAYING_OVER) {
      d_stdout("pass it");
      sendNoteOff(note, channel, frame);
      // take into account for play only if we turn off current note (we might also release part of a chord)
      if (note == curNote) {
        // sequence is terminated
        if (status == PLAYING_OVER) {
          status = GAMEOVER;
          // level up!
          if (round > 0 && round - 1 > maxRound) {
            maxRound = round - 1;
          }
        }
        // still in play, either next or new round
        else {
          status = PLAYING_WAIT;
          // time for new round
          if ((int) stepN >= round) {
            d_stdout("playing over");
            newRound();
          }
        }
        curNote = -1;
      }
    }
  }

  void newGame() {
    d_stdout("new game");
    reset();
    status = STARTING;
    // reset counters
    lastTime = curTime;
    round = 0;
  }

  // starting new instructions
  void newRound() {
    d_stdout("new round");
    status = INSTRUCTIONS;
    stepN = 0;
    // draw another note
    addNote();
    // reset counters
    lastTime = curTime;
    // check if we should grant a new miss
    if (roundsForMiss > 0 && round - lastRFM >= roundsForMiss) {
      lastRFM = round;
      maxMiss++;
      d_stdout("grant a new miss!. current miss: %d / %d", nbMiss, maxMiss);
    }
    // increase round last, sequence < round
    round++;
  }

  // draw another note to the sequence
  void addNote() {
    if (round >= MAX_ROUND) {
      d_stdout("max round reached!");
      status = GAMEOVER;
    }
    else if (round >= 0) {
      // brute-force the possible notes considering root, number of notes, scale
      int activeNotes[128] = {0};
      int nbActives = 0;
      for (int i = 0; i < 128 && i < effectiveNbNotes; i++) {
        int note = effectiveRoot + i;
        // make extra-sure we won't go out of bounds
        if ((note % 12) >= 0 and effectiveScale[note % 12]) {
          activeNotes[nbActives] = note;
          nbActives++;
        }
      }
      if (nbActives > 0) {
        sequence[round] = activeNotes[ran.rand() % nbActives];
      }
      // fallback: root
      else {
        sequence[round] = effectiveRoot;
      }
    }
  }

  // playing next note in the sequence
  // frame: frame of the event in the buffer
  void nextNote(uint32_t frame=0) {
    d_stdout("instruction %d", stepN);
    // reached end of sequence, user's turn
    if((int) stepN >= round) {
      d_stdout("instruction reached sequence");
      status = PLAYING_WAIT;
      // reset counter for user
      stepN = 0;
    }
    else if (stepN < MAX_ROUND) {
      curNote = sequence[stepN];
      d_stdout("next note %d", curNote);
      if (curNote >= 0) {
        // first channel and full velocity by default
        curChannel = 0;
        sendNoteOn(curNote, 127, curChannel, frame);
        stepN++;
      }
    }
  }

  void process(uint32_t nbSamples, uint32_t frame) {
    // in-between games, sync state
    // Note: upon change on root or nbNotes we will kill any pending notes, to avoid stuck keys upon shifting
    if (!isRunning(status)) {
      if (effectiveRoot != root) {
        abortCurrentNote(frame);
        effectiveRoot = root;
      }
      // limit number of notes depending on root position
      int maxNotes = params[kNbNotes].max - effectiveRoot;
      if (nbNotes > maxNotes && effectiveNbNotes != maxNotes) {
        abortCurrentNote(frame);
        effectiveNbNotes = maxNotes;
      }
      else if (nbNotes <= maxNotes && effectiveNbNotes != nbNotes) {
        abortCurrentNote(frame);
        effectiveNbNotes = nbNotes;
      }
      // sync scale now
      for (int i = 0; i < 12; i++) {
        effectiveScale[i] = scale[i];
      }
    }

    // update time
    for (uint32_t i = 0; i < nbSamples; i++) {
      curTime +=  1./ getSampleRate();
      double elapsedTime = curTime - lastTime;

      switch(status) {
      // pause before real start
      case STARTING:
        if (elapsedTime >= NOTE_INTERVAL) {
          newRound();
        }
        break;
      // displaying example
      case INSTRUCTIONS:
        // displaying note, wait to turn it off
        if (curNote >= 0) {
          if (elapsedTime >= NOTE_DURATION) {
            d_stdout("turn off note");
            lastTime = curTime;
            abortCurrentNote(frame + i);
          }
        }
        // test if we should start new note
        else {
          if (elapsedTime >= NOTE_INTERVAL) {
            d_stdout("go note");
            lastTime = curTime;
            nextNote(frame + i);
          }
        }
          break;
      default:
        break;
      }
    }

  };

  // reset inner state upon new game
  void reset() {
    d_stdout("reset");
    // init array
    for (int i = 0; i < MAX_ROUND; i++) {
      sequence[i] = -1;
    }
    setParameterValue(kCurNote, -1);
    round = 0;
    stepN = 0;
    nbMiss = 0;
    maxMiss = 0;
    lastRFM = 0;
  }

private:
  // parameters
  bool start = false;
  int status = params[kStatus].def;
  int root = params[kRoot].def;
  int nbNotes = params[kNbNotes].def;
  int effectiveRoot =  params[kEffectiveRoot].def;
  int effectiveNbNotes =  params[kEffectiveNbNotes].def;
  int curNote = params[kCurNote].def;
  bool scale[12];
  bool effectiveScale[12];
  // how many notes are missed in this round
  int nbMiss = 0;
  // max miss before game over
  int maxMiss = 0;
  // increase number of possible miss every xx round
  int roundsForMiss = 1;
  // max round completed
  int maxRound = 0;
  // associated channel, to abort note
  int curChannel = 0;
  // current round number
  int round = params[kRound].def;
  // for computing time based on frame count
  double curTime = 0;
  double lastTime = 0;
  // sequence of notes for this round
  int sequence[MAX_ROUND];
  // where in the sequence the instruction or the player is at
  uint stepN = 0;
  // our number generator
  Rando ran;
  // last time a miss was granted
  int lastRFM = 0;

  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimonPiano);
};

Plugin *createPlugin() { return new SimonPiano(); }

END_NAMESPACE_DISTRHO
