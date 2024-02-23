
#include "ExtendedPlugin.hpp"

START_NAMESPACE_DISTRHO

class SimonPiano : public ExtendedPlugin {
public:
  // Note: do not care with default values since we will sent all parameters upon init
  SimonPiano() : ExtendedPlugin(kParameterCount, 0, 0) {
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
        if (status == WAITING || status == GAMEOVER) {
          d_stdout("new game");
          status = STARTING;
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

  // callbacks for processing MIDI
  void noteOn(uint8_t note, uint8_t, uint8_t channel, uint32_t frame) {
    d_stdout("NoteOn %d, channel %d, frame %d", note, channel, frame);
  }

  void noteOff(uint8_t note, uint8_t channel, uint32_t frame) {
    d_stdout("NoteOff %d, channel %d, frame %d", note, channel, frame);
  }

  void drawNote() {
  }

  void process(uint32_t nbSamples, uint32_t /*frame*/) {
    // sync state
    if (status == WAITING || status == GAMEOVER) {
      if (effectiveNbNotes != nbNotes) {
        setParameterValue(kEffectiveNbNotes, nbNotes);
      }
      if (effectiveRoot != root) {
        setParameterValue(kEffectiveRoot, root);
      }
    }

      for (uint32_t i = 0; i < nbSamples; i++) {
       curTime +=  1./ getSampleRate();
        // ping every second
        if (curTime - lastTime >= 1.0) {
          d_stdout("tick, time: %lf", curTime);
          lastTime += 1.0;
        }
      }

  };

private:
  // parameters
  bool start = false;
  int status = params[kStatus].def;
  int root = params[kRoot].def;
  int nbNotes = params[kNbNotes].def;
  int effectiveRoot =  params[kEffectiveRoot].def;
  int effectiveNbNotes =  params[kEffectiveNbNotes].def;
  int curNote = params[kCurNote].def;
  // for computing time based on frame count
  double curTime = 0;
  double lastTime = 0;

  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimonPiano);
};

Plugin *createPlugin() { return new SimonPiano(); }

END_NAMESPACE_DISTRHO
