

#ifndef RAYUI_HPP
#define RAYUI_HPP

#include "DistrhoUI.hpp"

// encapsulate raylib in the DISTHRO namespace to avoid clashes with other functions. Child classes must then also use raylib within this namespace
START_NAMESPACE_DISTRHO
// we got to enable some extra functions in raylib for us to pass events
#include "raylib.h"
#include "rlgl.h"
#include "raygui.h"
END_NAMESPACE_DISTRHO

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
