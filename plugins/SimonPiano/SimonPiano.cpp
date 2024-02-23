
#include "ExtendedPlugin.hpp"

START_NAMESPACE_DISTRHO

// time in seconds between notes for instructions (and before first instruction)
#define NOTE_INTERVAL 1
// how long each note is held during instruction
#define NOTE_DURATION 1

class SimonPiano : public ExtendedPlugin {
public:
  // Note: do not care with default values since we will sent all parameters upon init
  SimonPiano() : ExtendedPlugin(kParameterCount, 0, 0) {
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
      parameter.unit = "";
      parameter.ranges.def = params[kRound].def;
      parameter.ranges.min = params[kRound].min;
      parameter.ranges.max = params[kRound].max;
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
    case kEffectiveRoot:
      return effectiveRoot;
    case kEffectiveNbNotes:
      return effectiveNbNotes;
    case kStatus:
      return status;
    case kCurNote:
      return curNote;

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

  // user playing notes, keep channel and velocity for user
  void noteOn(uint8_t note, uint8_t velocity, uint8_t channel, uint32_t frame) {
    d_stdout("NoteOn %d, channel %d, frame %d", note, channel, frame);
    // only pass through during playing, keep all info
    if (isPlaying(status)) {
      d_stdout("pass it");
      // disable any currently playing note -- i.e. monophonic
      if (curNote >= 0) {
        d_stdout("kill previous note %d", curNote);
        sendNoteOff(curNote, channel, frame);
      }
      sendNoteOn(note, velocity, channel, frame);
      curNote = note;
      if (playN < MAX_ROUND && curNote == sequence[playN]) {
        d_stdout("correct");
        status = PLAYING_CORRECT;
      }
      else {
        d_stdout("incorrect");
        status = PLAYING_INCORRECT;
      }
      playN++;
      if ((int) playN >= round) {
        d_stdout("playing over");
        status = PLAYING_OVER;
      }
    }      
  }

  void noteOff(uint8_t note, uint8_t channel, uint32_t frame) {
    d_stdout("NoteOff %d, channel %d, frame %d", note, channel, frame);
    if (isPlaying(status) || status == PLAYING_OVER) {
      d_stdout("pass it");
      sendNoteOff(note, channel, frame);
      curNote = -1;
      if (isPlaying(status)) {
        status = PLAYING_WAIT;
      }
      // PLAYING_OVER, time for new round
      else {
        newRound();
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
    instructionN = 0;
    playN = 0;
    // draw another note
    addNote();
    // reset counters
    lastTime = curTime;
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
      // TODO
      sequence[round] = effectiveRoot + round;
    }
  }

  // playing next note in the sequence
  // frame: frame of the event in the buffer
  void nextNote(uint32_t frame=0) {
    d_stdout("instruction %d", instructionN);
    // reached end of sequence, user's turn 
    if((int) instructionN >= round) {
      d_stdout("instruction reached sequence");
      status = PLAYING_WAIT;
    }
    else if (instructionN < MAX_ROUND) {
      curNote = sequence[instructionN];
      d_stdout("next note %d", curNote);
      if (curNote >= 0) {
        // full velocity and first channel by default
        sendNoteOn(curNote, 127, 0, frame);
        instructionN++;
      }
    }
  }

  // turning off current instruction note
  // frame: frame of the event in the buffer
  void endNote(uint32_t frame=0) {
    d_stdout("end note %d", curNote);
    if (curNote >= 0) {
      // first channel by default
      sendNoteOff(curNote, 0, frame);
      curNote = -1;
    }
  }

  void process(uint32_t nbSamples, uint32_t frame) {
    // in-between games, sync state
    if (!isRunning(status)) {
      if (effectiveRoot != root) {
        setParameterValue(kEffectiveRoot, root);
      }
      // limit number of notes depending on root position
      int maxNotes = params[kNbNotes].max - effectiveRoot;
      if (nbNotes > maxNotes) {
        setParameterValue(kEffectiveNbNotes, maxNotes);
      }
      else {
        setParameterValue(kEffectiveNbNotes, nbNotes);
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
            endNote(frame + i);
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
    instructionN = 0;
    playN = 0;
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
  // current round number
  int round = params[kRound].def;
  // for computing time based on frame count
  double curTime = 0;
  double lastTime = 0;
  // sequence of notes for this round
  int sequence[MAX_ROUND];
  // where in the sequence the instruction is at
  uint instructionN = 0;
  // where in the sequence the player is at
  uint playN = 0;


  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimonPiano);
};

Plugin *createPlugin() { return new SimonPiano(); }

END_NAMESPACE_DISTRHO
