

#ifndef RAYUI_HPP
#define RAYUI_HPP

#include "DistrhoUI.hpp"

// default FPS if not defined before including this file
#ifndef UI_REFRESH_RATE 
#define UI_REFRESH_RATE 30
#endif

class RayUI : public UI, public IdleCallback
{
public:
  RayUI();
  ~RayUI();
};

#endif // RAYUI_HPP
