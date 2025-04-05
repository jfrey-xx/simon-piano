
// how often no refresh on idle state, in Hz. 0 to disable animation during idle state
#define UI_REFRESH_RATE 30

#include "RayUI.hpp"
#include "SimonUtils.h"

START_NAMESPACE_DISTRHO

// codes for used sprites
enum KeyIdx 
{
  BLACK_KEY,
  WHITE_KEY,
  BLACK_KEY_DIMMED,
  WHITE_KEY_DIMMED,
  BLACK_KEY_INSTRUCTION,
  WHITE_KEY_INSTRUCTION,
  BLACK_KEY_CORRECT,
  WHITE_KEY_CORRECT,
  BLACK_KEY_INCORRECT,
  WHITE_KEY_INCORRECT,
  BLACK_KEY_DEBUG,
  WHITE_KEY_DEBUG
};

// return true if the key for this note is C, D, E, F, G, A, B
bool isKeyWhite(uint note) {
  // only twelve note
  note = note % 12;
  return (note == 0 or note == 2 or note == 4 or note == 5 or note == 7 or note == 9 or note == 11);
}

// return the number of white keys for that many notes starting from root
uint getNbWhiteKeys(uint rootKey, uint nbKeys) {
  // stick to one simple case when root is C
  if (rootKey % 12 == 0) {
    if (nbKeys % 12 <= 4) {
      return ceil((nbKeys % 12 ) / (float) 2) + nbKeys / 12 * 7;
    }
    return ceil((nbKeys % 12 + 1) / (float) 2) + nbKeys / 12 * 7;
  }
  // otherwise, compute compared to simple case -- for sure there is a more clever way but is eludes me today
  return getNbWhiteKeys(0, nbKeys + rootKey % 12) - getNbWhiteKeys(0, rootKey % 12);
}

class SimonPianoUI : public RayUI
{
public:
   /**
      UI class constructor.
      The UI should be initialized to a default state that matches the plugin side.
    */

  SimonPianoUI() : RayUI(UI_REFRESH_RATE)
    {
      String resourcesLocation = getResourcesLocation();
      d_stdout("resources location: %s", resourcesLocation.buffer());

      // load texture for piano keys
      piano = LoadTexture(resourcesLocation + "piano.png");
    }

  ~SimonPianoUI() {
    // Texture unloading
    UnloadTexture(piano);
  }

protected:
    // ----------------------------------------------------------------------------------------------------------------
    // DSP/Plugin Callbacks

   /**
      A parameter has changed on the *plugin side*.@n
      This is called by the host to inform the UI about parameter changes.
    */
    void parameterChanged(uint32_t index, float value) override {
      switch (index) {
      case kEffectiveRoot:
	root = value;
	break;
      // only taking into account what is currently used in the DSP for UI
      case kEffectiveNbNotes:
	nbNotes = value;
	break;
      case kStatus:
	status = value;
	break;
      case kCurNote:
	curNote = value;
	break;
      case kRound:
	round = value;
	break;
      case kStep:
	step = value;
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
	{
	  int numScale = index - kEffectiveScaleC;
	  if (numScale >= 0 && numScale < 12) {
	    scale[numScale] = value;
	  }
	}
	break;
      case kShallNotPass:
	shallNotPass = value;
	break;
      case kRoundsForMiss:
	roundsForMiss = value;
	break;
      case kNbMiss:
	nbMiss = value;
	break;
      case kMaxMiss:
	maxMiss = value;
	break;
      case kMaxRound:
	maxRound = value;
	break;

      default:
	break;
      }
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Widget Callbacks
  void onMainDisplay() override
  {
    ClearBackground(BLUE);
  }
  
   void onCanvasDisplay()
  {

    Vector2 anchor01 = { 110, 10 };
    
    bool ButtonScaleCPressed = false;
    bool ButtonScaleCsPressed = false;
    bool ButtonScaleDPressed = false;
    bool ButtonScaleDsPressed = false;
    bool ButtonScaleEPressed = false;
    bool ButtonScaleFPressed = false;
    bool ButtonScaleFsPressed = false;
    bool ButtonScaleGPressed = false;
    bool ButtonScaleGsPressed = false;
    bool ButtonScaleAPressed = false;
    bool ButtonScaleAsPressed = false;
    bool ButtonScaleBPressed = false;

    Rectangle layoutRecs[22] = {
        (Rectangle){ anchor01.x + 0, anchor01.y + 0, 128, 24 },
        (Rectangle){ anchor01.x + 0, anchor01.y + 32, 264, 24 },
        (Rectangle){ anchor01.x + 0, anchor01.y + 64, 120, 24 },
        (Rectangle){ anchor01.x + 0, anchor01.y + 96, 312, 24 },
        (Rectangle){ anchor01.x + 0, anchor01.y + 128, 312, 24 },
        (Rectangle){ anchor01.x + 0, anchor01.y + 160, 32, 24 },
        (Rectangle){ anchor01.x + 40, anchor01.y + 160, 32, 24 },
        (Rectangle){ anchor01.x + 80, anchor01.y + 160, 32, 24 },
        (Rectangle){ anchor01.x + 120, anchor01.y + 160, 32, 24 },
        (Rectangle){ anchor01.x + 160, anchor01.y + 160, 32, 24 },
        (Rectangle){ anchor01.x + 200, anchor01.y + 160, 32, 24 },
        (Rectangle){ anchor01.x + 240, anchor01.y + 160, 32, 24 },
        (Rectangle){ anchor01.x + 280, anchor01.y + 160, 32, 24 },
        (Rectangle){ anchor01.x + 320, anchor01.y + 160, 32, 24 },
        (Rectangle){ anchor01.x + 360, anchor01.y + 160, 32, 24 },
        (Rectangle){ anchor01.x + 400, anchor01.y + 160, 32, 24 },
        (Rectangle){ anchor01.x + 440, anchor01.y + 160, 32, 24 },
        (Rectangle){ anchor01.x + 0, anchor01.y + 192, 24, 24 },
        (Rectangle){ anchor01.x + -80, anchor01.y + 160, 72, 24 },
        (Rectangle){ anchor01.x + 0, anchor01.y + 224, 312, 24 },
        (Rectangle){ anchor01.x + 0, anchor01.y + 408, 120, 24 },
        (Rectangle){ anchor01.x + 0, anchor01.y + 440, 120, 24 },
    };

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); 
    
    switch (status) {
    case WAITING:
      GuiLabel(layoutRecs[0], "Welcome");
      GuiLabel(layoutRecs[1], "Press Start for a new game");
      break;
    case STARTING:
      GuiLabel(layoutRecs[0], TextFormat("Round %d", round));
      GuiLabel(layoutRecs[1], TextFormat("Pay attention..."));
      break;
    case INSTRUCTIONS:
      GuiLabel(layoutRecs[0], TextFormat("Round %d -- step %d", round, step));
      GuiLabel(layoutRecs[1], TextFormat("Play after me!"));
      break;
    case GAMEOVER:
      GuiLabel(layoutRecs[0], TextFormat("Game over during Round %d -- step %d", round, step));
      GuiLabel(layoutRecs[1], TextFormat("Try again!"));
      break;
    case PLAYING_WAIT:
    case PLAYING_CORRECT:
    case PLAYING_INCORRECT:
    case PLAYING_OVER:
      GuiLabel(layoutRecs[0], TextFormat("Round %d -- step %d", round, step));
      GuiLabel(layoutRecs[1], TextFormat("Your turn!"));
      break;
    default:
      GuiLabel(layoutRecs[0], TextFormat("Round %d", round));
      GuiLabel(layoutRecs[1], TextFormat("Play after me!"));
      break;
    }
    
    if (isRunning(status)) {
      // TODO, set disabled!
    }
    
    if(GuiButton(layoutRecs[2], "Start")) {
      // send false/true cylce to make sure to toggle
      setParameterValue(kStart, false);
      setParameterValue(kStart, true);
    }

    // sync root note
    float uiRoot = root;
    GuiSlider(layoutRecs[3], TextFormat("Root note: %d", (int)uiRoot), NULL, &uiRoot, params[kRoot].min, params[kRoot].max);
    // only send value if updated -- and might not be taken into account if game is running
    if ((int)uiRoot != root) {
      // note: output parameters will be fired back
      setParameterValue(kRoot, (int)uiRoot);
    }

    // same for number of notes/keys
    // TODO: the max number of notes will change depending on root, not that great for this UI (could reset or inscrease upon changing root)
    float uiNbNotes = nbNotes;
    GuiSliderBar(layoutRecs[4], TextFormat("Number of keys: %d", (int)uiNbNotes), NULL, &uiNbNotes, params[kNbNotes].min, params[kNbNotes].max);
    if ((int)uiNbNotes != nbNotes) {
      setParameterValue(kNbNotes, (int)uiNbNotes);
    }

    // this label shoul be aligned as with widgets legends, revert style temporarily
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiLabel(layoutRecs[18], "Scale");
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    ButtonScaleCPressed = GuiButton(layoutRecs[5], "C"); 
    ButtonScaleCsPressed = GuiButton(layoutRecs[6], "C#"); 
    ButtonScaleDPressed = GuiButton(layoutRecs[7], "D"); 
    ButtonScaleDsPressed = GuiButton(layoutRecs[8], "D#"); 
    ButtonScaleEPressed = GuiButton(layoutRecs[9], "E"); 
    ButtonScaleFPressed = GuiButton(layoutRecs[10], "F"); 
    ButtonScaleFsPressed = GuiButton(layoutRecs[11], "F#"); 
    ButtonScaleGPressed = GuiButton(layoutRecs[12], "G"); 
    ButtonScaleGsPressed = GuiButton(layoutRecs[13], "G#"); 
    ButtonScaleAPressed = GuiButton(layoutRecs[14], "A"); 
    ButtonScaleAsPressed = GuiButton(layoutRecs[15], "A#"); 
    ButtonScaleBPressed = GuiButton(layoutRecs[16], "B"); 

    // --- end disable part of the UI during game ---
    if (isRunning(status)) {
      // TODO
    }

    // option about letting notes through on not
    bool uiShallNotPass = shallNotPass;
    GuiCheckBox(layoutRecs[17], "Shall not pass", &uiShallNotPass);
    if (uiShallNotPass != shallNotPass) {
      // note: we have to sync in ui non-output parameters changed from ui, won't be fired back
      shallNotPass = uiShallNotPass;
      setParameterValue(kShallNotPass, uiShallNotPass);
    }

    // sync rounds for miss
    float uiRoundsForMiss = roundsForMiss;
    GuiSliderBar(layoutRecs[19], TextFormat("Rounds for miss: %d", (int)uiRoundsForMiss ), NULL, &uiRoundsForMiss, params[kRoundsForMiss].min, params[kRoundsForMiss].max);
    if ((int)uiRoundsForMiss != roundsForMiss) {
      roundsForMiss = (int)uiRoundsForMiss;
      setParameterValue(kRoundsForMiss, (int)uiRoundsForMiss);
    }

    GuiLabel(layoutRecs[20], TextFormat("Missed %d/%d", nbMiss, maxMiss));
    GuiLabel(layoutRecs[21], TextFormat("Current best: %d", maxRound));

    drawPiano({ getCanvasWidth() * 0.1f / 2, 300.0f }, { getCanvasWidth() * 0.9f, 100.0f }, root, nbNotes);

    DrawFPS(10, 10);
  }

    // ----------------------------------------------------------------------------------------------------------------

private:
  // texture for piano keys
  Texture2D piano;
  // location of current key in texture
  Rectangle keySpriteRec;
  // parameters sync with DSP
  int status = params[kStatus].def;
  int root = params[kRoot].def;
  int nbNotes = params[kNbNotes].def;
  int curNote = params[kCurNote].def;
  int round = params[kRound].def;
  int step = params[kStep].def;
  bool scale[12] = {0};
  int roundsForMiss = params[kRoundsForMiss].def;
  int nbMiss = params[kNbMiss].def;
  int maxMiss = params[kMaxMiss].def;
  int maxRound = params[kMaxRound].def;
  bool shallNotPass = params[kShallNotPass].def;

  // extract and draw sprite id a said location and size
  void drawKey(KeyIdx idx, Vector2 pos, Vector2 size) {
    // fixed size for all keys in the sprite sheet
    static const Rectangle spriteSize = {0, 0, 8, 32};
    // picking the right position
    int spriteShift = 0;
    switch(idx) {
    case BLACK_KEY:
      spriteShift = 2;
      break;
    case WHITE_KEY:
      spriteShift = 0;
      break;
    case BLACK_KEY_DIMMED:
      spriteShift = 6;
      break;
    case WHITE_KEY_DIMMED:
      spriteShift = 4;
      break;
    case BLACK_KEY_INSTRUCTION:
      spriteShift = 15;
      break;
    case WHITE_KEY_INSTRUCTION:
      spriteShift = 13;
      break;
    case BLACK_KEY_CORRECT:
      spriteShift = 19;
      break;
    case WHITE_KEY_CORRECT:
      spriteShift = 17;
      break;
    case BLACK_KEY_INCORRECT:
      spriteShift = 23;
      break;
    case WHITE_KEY_INCORRECT:
      spriteShift = 21;
      break;
    case BLACK_KEY_DEBUG:
      spriteShift = 10;
      break;
    default:
    case WHITE_KEY_DEBUG:
      spriteShift = 8;
      break;
    }
    
    // no rotation, no tint
    DrawTexturePro(piano, {spriteSize.x + spriteShift * spriteSize.width, spriteSize.y, spriteSize.width, spriteSize.height}, {pos.x, pos.y, size.x, size.y}, {0.0, 0.0}, 0, WHITE); 
  }

  // drawing a very simple keyboard using imgui, fetching drawList
  // pos: upper left corner of the widget
  // size: size of the widget
  void drawPiano(Vector2 pos, Vector2 size, uint rootKey, uint nbKeys) {
    // find number of white keys
    int nbWhiteKeys = getNbWhiteKeys(rootKey, nbKeys);
    // in case we start or end with black, leave some padding as half a white
    float uiNbWhiteKeys = nbWhiteKeys;
    if (!isKeyWhite(rootKey)) {
      uiNbWhiteKeys += 0.5;
    }
    if (!isKeyWhite(rootKey + nbKeys - 1)) {
      uiNbWhiteKeys += 0.5;
    }
    // around keyboard, between keys -- would be same ratio for 12 notes
    Vector2 margins = {size.x * 0.01f, size.y * 0.01f};
    // black keys over whites, width of a key will be conditioned by the former
    Vector2 keySize;
    keySize.x = (size.x  - margins.x * 2) / uiNbWhiteKeys;
    keySize.y = size.y - margins.y * 2;

    // draw the background
    // TODO: debug here
    DrawRectangle(pos.x, pos.y, size.x, size.y, BLUE);   

    // base position for current key
    Vector2 startPos = {pos.x + margins.x, pos.y + margins.y};
    // shift if we start with black key
    if (!isKeyWhite(rootKey)) {
      startPos.x += keySize.x * 0.5;
    }
    Vector2 curPos(startPos);
    KeyIdx sprite;

    // first draw white keys
    uint note = rootKey;
    for (uint i = 0; i < nbKeys; i++) {
      if(isKeyWhite(note)) {
	// dimm key if not in scale and option set
	if (shallNotPass && !scale[note % 12]) {
	  sprite = WHITE_KEY_DIMMED;
	}
	else {
	  sprite = WHITE_KEY;
	}
	// this note currently active, special color
	if ((int)note == curNote) {
	  switch(status) {
	  case INSTRUCTIONS:
	    sprite = WHITE_KEY_INSTRUCTION;
	    break;
	  case PLAYING_CORRECT:
	    sprite = WHITE_KEY_CORRECT;
	    break;
	  case PLAYING_INCORRECT:
	  case PLAYING_OVER:
	    sprite = WHITE_KEY_INCORRECT;
	    break;
	  default:
	    // debug
	    sprite = WHITE_KEY_DEBUG;
	    break;
	  }
	}
	drawKey(sprite, curPos, keySize);
	curPos.x += keySize.x;
      }
      note++;
    }
    // black on top
    curPos = startPos;
    note = rootKey;
    for (uint i = 0; i < nbKeys; i++) {
      // skip white keys, just advance
      if(isKeyWhite(note)) {
	curPos.x += keySize.x;
      }
      else {
	// dim key if not in scale and option set
	if (shallNotPass && !scale[note % 12]) {
	  sprite = BLACK_KEY_DIMMED;
	}
	else {
	  sprite = BLACK_KEY;
	}
	// this note currently active, special color
	if ((int)note == curNote) {
	  switch(status) {
	  case INSTRUCTIONS:
	    sprite = BLACK_KEY_INSTRUCTION;
	    break;
	  case PLAYING_CORRECT:
	    sprite = BLACK_KEY_CORRECT;
	    break;
	  case PLAYING_INCORRECT:
	  case PLAYING_OVER:
	    sprite = BLACK_KEY_INCORRECT;
	    break;
	  default:
	    // debug
	    sprite = BLACK_KEY_DEBUG;
	    break;
	  }
	}
	// for black key, shift half to right to center between two white keys
	drawKey(sprite, {curPos.x - keySize.x / 2, curPos.y}, keySize);
      }
      note++;
    }
  }
  
  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimonPianoUI)
};

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new SimonPianoUI();
}
 
// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
 
