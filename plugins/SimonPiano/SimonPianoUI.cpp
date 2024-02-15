
#include "DistrhoUI.hpp"

START_NAMESPACE_DISTRHO

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
      A parameter has changed on the plugin side.@n
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
      static int uiRoot;
      uiRoot = root;
      ImGui::SliderInt("Root note", &uiRoot, params[kRoot].min, params[kRoot].max);
      // only send value if updated
      if (uiRoot != root) {
	setParameterValue(kRoot, uiRoot);
      }

      ImGui::End();
      
    }

    // ----------------------------------------------------------------------------------------------------------------

private:
  // parameters sync with DSP
  int root = params[kRoot].def;

  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimonPianoUI)
};

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new SimonPianoUI();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
