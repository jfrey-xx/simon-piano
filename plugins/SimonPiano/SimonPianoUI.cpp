
#include "DistrhoUI.hpp"

START_NAMESPACE_DISTRHO

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
      const uint width = DISTRHO_UI_DEFAULT_WIDTH * scaleFactor;
      const uint height = DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor;

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

      // center window with fixed ration defined by default width and height
      ImGui::SetNextWindowPos(ImVec2((getWidth() - DISTRHO_UI_DEFAULT_WIDTH * scaleFactor) /2, (getHeight() - DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor) /2));
      ImGui::SetNextWindowSize(ImVec2(DISTRHO_UI_DEFAULT_WIDTH * scaleFactor, DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor));

      // alter background color to check position
      ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
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
	break;
      case PLAYING_WAIT:
      case PLAYING_CORRECT:
      case PLAYING_INCORRECT:
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
      ImGui::Spacing();

      // same for number of notes/keys
      int uiNbNotes = nbNotes;
      ImGui::SliderInt("Number of keys", &uiNbNotes, params[kNbNotes].min, params[kNbNotes].max);
      if (uiNbNotes != nbNotes) {
	setParameterValue(kNbNotes, uiNbNotes);
      }
      // --- end disable part of the UI during game ---
      if (isRunning(status)) {
	ImGui::EndDisabled();
      }

      // draw next to current position
      const ImVec2 p = ImGui::GetCursorScreenPos(); 
      const ImVec2 keyboardSize(400 * scaleFactor, 200 * scaleFactor);
      drawPiano(p, keyboardSize, root, nbNotes);
      // move along
      ImGui::SetCursorScreenPos(p + keyboardSize) ;
      ImGui::Spacing();
      ImGui::TextWrapped("Current best: ?");

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

    ImU32 colBackground = ImColor(ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); 
    ImU32 colBlackKey = ImColor(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); 
    ImU32 colWhiteKey = ImColor(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); 
    ImU32 colInstructionKey = ImColor(ImVec4(0.0f, 0.0f, 1.0f, 1.0f)); 
    ImU32 colCorrectKey = ImColor(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); 
    ImU32 colIncorrectKey = ImColor(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); 

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

  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimonPianoUI)
};

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new SimonPianoUI();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
