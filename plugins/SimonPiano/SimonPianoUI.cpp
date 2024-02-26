
#include "DistrhoUI.hpp"
#include "SimonUtils.h"

START_NAMESPACE_DISTRHO

// some shared color
const ImU32 colBackground = ImColor(ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); 
const ImU32 colBlackKey = ImColor(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); 
const ImU32 colWhiteKey = ImColor(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); 
const ImU32 colBlackKeyDimmed = ImColor(ImVec4(0.33f, 0.33f, 0.33f, 1.0f)); 
const ImU32 colWhiteKeyDimmed = ImColor(ImVec4(0.66f, 0.66f, 0.66f, 1.0f)); 
const ImU32 colInstructionKey = ImColor(ImVec4(0.0f, 0.0f, 1.0f, 1.0f)); 
const ImU32 colCorrectKey = ImColor(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); 
const ImU32 colIncorrectKey = ImColor(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); 

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

// --------------------------------------------------------------------------------------------------------------------

class SimonPianoUI : public UI
{
public:
   /**
      UI class constructor.
      The UI should be initialized to a default state that matches the plugin side.
    */
    SimonPianoUI()
        : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT)
    {
        const double scaleFactor = getScaleFactor();

        if (d_isEqual(scaleFactor, 1.0))
        {
            setGeometryConstraints(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT);
        }
        else
        {
            const uint width = DISTRHO_UI_DEFAULT_WIDTH * scaleFactor;
            const uint height = DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor;
            setGeometryConstraints(width, height);
            setSize(width, height);
        }
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

   /**
      ImGui specific onDisplay function.
    */
    void onImGuiDisplay() override
    {

      // compute base width/height
      double scaleFactor = getScaleFactor();
      uint width = DISTRHO_UI_DEFAULT_WIDTH * scaleFactor;
      uint height = DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor;

      // take into account resize of window
      double scaleWidth = getWidth() / (float) width;
      double scaleHeight = getHeight() / (float) height;
      if (scaleWidth < scaleHeight) {
	scaleFactor = scaleFactor * scaleWidth;
      }
      else {
	scaleFactor = scaleFactor * scaleHeight;
	// larger than ratio, center
	ImGui::SetNextWindowPos(ImVec2(0, 0));
      }

      // padding is a fraction of the area
      ImVec2 winPadding(DISTRHO_UI_DEFAULT_WIDTH * 0.01 * scaleFactor, DISTRHO_UI_DEFAULT_HEIGHT * 0.01 * scaleFactor);

      // center window with fixed ration defined by default width and height
      ImGui::SetNextWindowPos(ImVec2((getWidth() - DISTRHO_UI_DEFAULT_WIDTH * scaleFactor) /2, (getHeight() - DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor) /2));
      ImGui::SetNextWindowSize(ImVec2(DISTRHO_UI_DEFAULT_WIDTH * scaleFactor, DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor));

      // alter background color to check position
      ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      // specify our own padding to better scale and know it
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, winPadding);
      // we only use one window, will take all space and hide controls
      ImGui::Begin("Demo window", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
      // scale UI
      ImGui::SetWindowFontScale(scaleFactor);

      switch (status) {
      case WAITING:
	ImGui::TextWrapped("Welcome");
	ImGui::TextWrapped("Press start for a new game");
	break;
      case STARTING:
	ImGui::TextWrapped("Round %d", round);
	ImGui::TextWrapped("Pay attention...");
	break;
      case INSTRUCTIONS:
	ImGui::TextWrapped("Round %d -- step %d", round, step);
	ImGui::TextWrapped("Play after me!");
	break;
      case GAMEOVER:
	ImGui::TextWrapped("Game over during Round %d -- step %d", round, step);
	ImGui::TextWrapped("Try again!");
	break;
      case PLAYING_WAIT:
      case PLAYING_CORRECT:
      case PLAYING_INCORRECT:
      case PLAYING_OVER:
	ImGui::TextWrapped("Round %d -- step %d", round, step);
	ImGui::TextWrapped("Your turn!");
	break;
      default:
	ImGui::TextWrapped("Round %d", round);
	ImGui::TextWrapped("Play after me!");
	break;
      }

      // --- disable part of the UI during game ---
      if (isRunning(status)) {
	ImGui::BeginDisabled();
      }
      if (ImGui::Button("Start")) {
	d_stdout("button click");
	// send false/true cylce to make sure to toggle
	setParameterValue(kStart, false);
	setParameterValue(kStart, true);
      }

      // sync root note
      int uiRoot = root;
      ImGui::SliderInt("Root note", &uiRoot, params[kRoot].min, params[kRoot].max);
      // only send value if updated -- and might not be taken into account if game is running
      if (uiRoot != root) {
	setParameterValue(kRoot, uiRoot);
      }

      // same for number of notes/keys
      int uiNbNotes = nbNotes;
      ImGui::SliderInt("Number of keys", &uiNbNotes, params[kNbNotes].min, params[kNbNotes].max);
      if (uiNbNotes != nbNotes) {
	setParameterValue(kNbNotes, uiNbNotes);
      }

      drawScale(scaleFactor);

      // --- end disable part of the UI during game ---
      if (isRunning(status)) {
	ImGui::EndDisabled();
      }
      ImGui::Spacing();

      // sync rounds for miss
      int uiRoundsForMiss = roundsForMiss;
      ImGui::SliderInt("Rounds for miss", &uiRoundsForMiss, params[kRoundsForMiss].min, params[kRoundsForMiss].max);
      if (uiRoundsForMiss != roundsForMiss) {
	roundsForMiss = uiRoundsForMiss;
	setParameterValue(kRoundsForMiss, roundsForMiss);
      }

      ImGui::Spacing();

      // draw next to current position
      const ImVec2 p = ImGui::GetCursorScreenPos(); 
      const ImVec2 keyboardSize(ImGui::GetWindowSize().x - winPadding.x * 2, 200 * scaleFactor);
      drawPiano(p, keyboardSize, root, nbNotes);
      // move along
      ImGui::SetCursorScreenPos(p + keyboardSize) ;
      ImGui::Spacing();
      ImGui::TextWrapped("Missed %d/%d", nbMiss, maxMiss);
      ImGui::TextWrapped("Current best: %d", maxRound);

      ImGui::End();
      
    }

    // ----------------------------------------------------------------------------------------------------------------

private:
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
  
  // drawing a very simple keyboard using imgui, fetching drawList
  // pos: upper left corner of the widget
  // size: size of the widget
  void drawPiano(ImVec2 pos, ImVec2 size, uint rootKey, uint nbKeys) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList(); 

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
    ImVec2 spacing;
    spacing.x = size.x / (uiNbWhiteKeys + 1) * 0.1f;
    spacing.y = size.y / 7 * 0.1f;
    ImVec2 whiteKeySize;
    whiteKeySize.x = (size.x  - spacing.x * (uiNbWhiteKeys + 1)) / uiNbWhiteKeys;
    // whole height except margin
    whiteKeySize.y = size.y - spacing.y * 2;
    ImVec2 blackKeySize;
    blackKeySize.x = (whiteKeySize.x - 2 * spacing.x) * 0.4;
    blackKeySize.y = whiteKeySize.y * 0.4;
    // background area for black keys
    ImVec2 blackBackgroundSize(blackKeySize.x + 2 * spacing.x, blackKeySize.y + spacing.y);

    // draw the background
    draw_list->AddRectFilled(pos, pos + size, colBackground);

    // base position for current key
    ImVec2 startPos = pos + spacing;
    // shift if we start with black key
    if (!isKeyWhite(rootKey)) {
      startPos.x += whiteKeySize.x * 0.5;
    }
    ImVec2 curPos(startPos);
    ImU32 colCurKey;

    // first draw white keys
    uint note = rootKey;
    for (uint i = 0; i < nbKeys; i++) { 
      if(isKeyWhite(note)) {
	colCurKey = colWhiteKey;
	// this note currently active, special color
	if ((int)note == curNote) {
	  switch(status) {
	  case INSTRUCTIONS:
	    colCurKey = colInstructionKey;
	    break;
	  case PLAYING_CORRECT:
	    colCurKey = colCorrectKey;
	    break;
	  case PLAYING_INCORRECT:
	  case PLAYING_OVER:
	    colCurKey = colIncorrectKey;
	    break;
	  default:
	    // debug
	    colCurKey = ImColor(ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	    break;
	  }
	}
	draw_list->AddRectFilled(curPos, curPos + whiteKeySize, colCurKey);
	// only advance between white keys, we should not have consecutive black keys
	curPos.x += whiteKeySize.x + spacing.x;
      }
      note++;
    }
    // black on top
    curPos = startPos;
    note = rootKey;
    for (uint i = 0; i < nbKeys; i++) { 
      // skip white keys, just advance
      if(isKeyWhite(note)) {
	curPos.x += whiteKeySize.x + spacing.x;
      }
      else {
	colCurKey = colBlackKey;
	// this note currently active, special color
	if ((int)note == curNote) {
	  switch(status) {
	  case INSTRUCTIONS:
	    colCurKey = colInstructionKey;
	    break;
	  case PLAYING_CORRECT:
	    colCurKey = colCorrectKey;
	    break;
	  case PLAYING_INCORRECT:
	  case PLAYING_OVER:
	    colCurKey = colIncorrectKey;
	    break;
	  default:
	    // debug
	    colCurKey = ImColor(ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	    break;
	  }
	}
	// for black key, first we have to draw the background for spacing
	ImVec2 tmpPos = curPos;
	tmpPos.x = curPos.x - spacing.x / 2 - blackKeySize.x / 2 - spacing.x;
	ImVec2 tmpSize(blackKeySize.x + 2 * spacing.x, blackKeySize.y + spacing.y);
	draw_list->AddRectFilled(tmpPos, tmpPos + blackBackgroundSize , colBackground);
	tmpPos.x += spacing.x;
	draw_list->AddRectFilled(tmpPos, tmpPos + blackKeySize, colCurKey);
      }
      note++;
    }
  }

  // toggles for scale
  void drawScale(float scaleFactor) {
    // FIXME: here manually adjusted values, compute compard to actual height
    float spaceX = 5.0f * scaleFactor;
    float whiteX = 38.0f * scaleFactor;
    float blackX = 32.0f * scaleFactor;
    float keyX;
    // manually shift element to scale properly
    float posX = spaceX;
    // temp variables for colors
    ImU32 colKey;
    ImU32 colActive;
    ImU32 colText;
    for (int i = 0; i < 12; i++) {
      ImGui::PushID(i);
      if (i > 0) {
	ImGui::SameLine(posX);
      }
      // as with a piano keyboard, here as well mimic black/white keys
      if(isKeyWhite(i)) {
	if (scale[i]) {
	  colKey = colWhiteKey;
	  colActive = colWhiteKeyDimmed;
	}
	else {
	  colKey = colWhiteKeyDimmed;
	  colActive = colWhiteKey;
	}
	colText = colBlackKey;
	keyX = whiteX;
      }
      else {
	if (scale[i]) {
	  colKey = colBlackKey;
	  colActive = colBlackKeyDimmed;
	}
	else {
	  colKey = colBlackKeyDimmed;
	  colActive = colBlackKey;
	}
	colText = colWhiteKey;
	keyX = blackX;
      }
      ImGui::PushStyleColor(ImGuiCol_Button, colKey);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colKey);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, colActive);
      ImGui::PushStyleColor(ImGuiCol_Text, colText);

      if (ImGui::Button(scaleNotes[i], ImVec2(keyX, 0))) {
	setParameterValue(kScaleC + i, !scale[i]);
      }

      posX += keyX + spaceX;
      ImGui::PopStyleColor(4);
      ImGui::PopID();
    }
    ImGui::SameLine();
    ImGui::Text("Scale");
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
