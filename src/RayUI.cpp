
#include "RayUI.hpp"

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
