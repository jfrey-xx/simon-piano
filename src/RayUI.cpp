
#include "RayUI.hpp"

// put here raygui implementation
START_NAMESPACE_DISTRHO
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

RayUI::RayUI()
  : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT) 
{
  d_stdout("RayUI constructor");
  
  // always animate
  if (UI_REFRESH_RATE > 0) {
    // method called every xx milliseconds
    int refreshTime = 1000/UI_REFRESH_RATE;
    addIdleCallback(this, refreshTime);
  }
}

RayUI::~RayUI() {
  d_stdout("RayUI destructor");
  // cleanup
  if (UI_REFRESH_RATE > 0) {
    removeIdleCallback(this);
  }
}

void RayUI::idleCallback()
{
    // force display refresh
    repaint();
}

END_NAMESPACE_DISTRHO
