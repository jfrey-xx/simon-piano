
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

    default:
      break;
    }
    
    // effectively set parameter
    setParameterValue(index, parameter.ranges.def);
  }

  float getParameterValue(uint32_t index) const override {
    switch (index) {

    case kRoot:
      return root;

    default:
      return 0.0;
    }
  }
  
  void setParameterValue(uint32_t index, float value) override {
    // FIXME: check up to which point function is repeatedly called from host even when value does not change
    switch (index) {
    case kRoot:
      root = value;
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

private:
  // parameters
  int root;

  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimonPiano);
};

Plugin *createPlugin() { return new SimonPiano(); }

END_NAMESPACE_DISTRHO
