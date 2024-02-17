
#include "DistrhoUI.hpp"

START_NAMESPACE_DISTRHO

// return true if the key for this note is C, D, E, F, G, A, B
bool isKeyWhite(uint note) {
  // only twelve note
  note = note % 12;
  return (note == 0 or note == 2 or note == 4 or note == 5 or note == 7 or note == 9 or note == 11);
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
      case kRoot:
	root = value;
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
      d_stdout("w: %d, h: %d, scale: %lf, scale live: %lf", getWidth(), getHeight(), getScaleFactor(), scaleFactor);

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
      
      ImGui::TextWrapped("Play after me!");

      // sync root note
      int uiRoot = root;
      ImGui::SliderInt("Root note", &uiRoot, params[kRoot].min, params[kRoot].max);
      // only send value if updated
      if (uiRoot != root) {
	root = uiRoot;
	setParameterValue(kRoot, uiRoot);
      }

      ImGui::Spacing();

      // draw next to current position
      const ImVec2 p = ImGui::GetCursorScreenPos(); 
      d_stdout("px: %f, py: %f", p.x, p.y);
      const ImVec2 keyboardSize(400 * scaleFactor, 200 * scaleFactor);
      drawPiano(p, keyboardSize, root);
      // move along
      ImGui::SetCursorScreenPos(p + keyboardSize) ;
      ImGui::Spacing();
      ImGui::TextWrapped("Lalala");

      ImGui::End();
      
    }

    // ----------------------------------------------------------------------------------------------------------------

private:
  // parameters sync with DSP
  int root = params[kRoot].def;

  // drawing a very simple keyboard using imgui, fetching drawList
  // pos: upper left corner of the widget
  // size: size of the widget
  void drawPiano(ImVec2 pos, ImVec2 size, uint rootKey) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList(); 
    int nbKeys = 12;
    float nbWhiteKeys = 7;
    // in case we start or end with black, leave some padding as half a white
    if (!isKeyWhite(rootKey) or !isKeyWhite(rootKey + nbKeys - 1)) {
      nbWhiteKeys += 0.5;
    } 
    if (!isKeyWhite(rootKey + nbKeys - 1)) {
      nbWhiteKeys += 0.5;
    }
    int nbBlackKeys = 5;
    // around keyboard, between keys -- would be same ratio for 12 notes
    ImVec2 spacing;
    // TODO: check if only one key
    spacing.x = size.x / (nbWhiteKeys + 1) * 0.1f;
    spacing.y = size.y / 7 * 0.1f;
    // TODO: check if no white key
    ImVec2 whiteKeySize;
    whiteKeySize.x = (size.x  - spacing.x * (nbWhiteKeys + 1)) / nbWhiteKeys;
    // whole height except margin
    whiteKeySize.y = size.y - spacing.y * 2;
    ImVec2 blackKeySize;
    blackKeySize.x = (whiteKeySize.x - 2 * spacing.x) * 0.4;
    blackKeySize.y = whiteKeySize.y * 0.4;

    ImU32 colBackground = ImColor(ImVec4(0.2f, 0.2f, 0.2f, 0.5f)); 
    // temp info for current key
    ImU32 colKey;
    ImVec2 sizeKey;
    int note;
    // base position for current key
    ImVec2 curPos = pos + spacing;
    if (!isKeyWhite(rootKey)) {
      curPos.x += whiteKeySize.x * 0.5;
    }

    // draw the background
    //draw_list->AddRectFilled(pos, pos + size, colBackground);
    draw_list->AddRectFilled(pos, pos + size, ImColor(ImVec4(0.0f, 1.0f, 0.0f, 0.5f))); 

    for (int i = 0; i < nbKeys; i++) { 
      note = i + rootKey;
      if (isKeyWhite(note)) {
	colKey = ImColor(ImVec4((float) i / nbKeys, 1.0f, 1.0f, 0.5f)); 
	draw_list->AddRectFilled(curPos, curPos + whiteKeySize, colKey);
	// only advance between white keys, we should not have consecutive black keys
	curPos.x += whiteKeySize.x + spacing.x;
      }
      else {
	colKey = ImColor(ImVec4(1.0f, 0.0f, 0.0f, 0.5f)); 
	// for black key, first we have to draw the background for spacing

	ImVec2 tmpPos = curPos;
	tmpPos.x = curPos.x - spacing.x / 2 - blackKeySize.x / 2 - spacing.x;
	ImVec2 tmpSize(blackKeySize.x + 2 * spacing.x, blackKeySize.y + spacing.y);
	draw_list->AddRectFilled(tmpPos, tmpPos + tmpSize , colBackground);
	tmpPos.x += spacing.x;
	draw_list->AddRectFilled(tmpPos, tmpPos + blackKeySize, colKey);
      }
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
