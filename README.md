 
Simon game, with MIDI input (to connect to a keyboard or other instrument) and MIDI output (to offload sound generation). As a standalone or audio plugin thanks to [DPF](https://github.com/DISTRHO/DPF/).

# Notes

During instructions defaults to first MIDI channel and full velocity, while playing keep those variables intact.

Only note on/off messages are passed through.

# TODO

- configurable "jokers", can have one error every xx note
- debounce for MIDI input?
- someday: a version with chords?

# Known issues

- upon closing the plugin notes if active will get stuck
