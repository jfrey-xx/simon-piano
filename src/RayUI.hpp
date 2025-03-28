

#ifndef RAYUI_HPP
#define RAYUI_HPP

#include "DistrhoUI.hpp"

// encapsulate raylib in the DISTHRO namespace to avoid clashes with other functions. Child classes must then also use raylib within this namespace
START_NAMESPACE_DISTRHO
// we got to enable some extra functions in raylib for us to pass events
#include "raylib.h"
#include "rlgl.h"
#include "raygui.h"

// default FPS if not defined before including this file
#ifndef UI_REFRESH_RATE 
#define UI_REFRESH_RATE 30
#endif

// even though there is an option with PUGL to scale the window we will scale UI through raylib, better/looking with texture filtering and more flexible (possible to use margins around the canvas)
class RayUI : public UI, private IdleCallback
{
public:
  RayUI();
  ~RayUI();
protected:
  // used for constant refresh rate. might override if client do not want that
  void idleCallback() override;
  // all area of the window, called before onCanvasDisplay
  // Will be enclosed in BeginDrawing() .. EndDrawing(). Mouse will be set to whole window
  void onMainDisplay();
  // also in main display, but called after canvas is rendered
  void onMainDisplayLast() {}
  // part that will be put to scale by rendering to a texture. At least that is to be implemented
  // mouse will be offset to canvas
  // Note: to do raycasting with mouse, remember to specify canvas size, e.g. 
  //   ray = GetScreenToWorldRayEx(GetMousePosition(), camera, getCanvasWidth(), getCanvasHeight());
  virtual void onCanvasDisplay() = 0;

  uint getCanvasWidth() { return canvasWidth; }
  uint getCanvasHeight() { return canvasHeight; }

private:
  // callback from UI class, will be split between main and canvas
  void onDisplay() override final;

  // we also take full control of events
  bool onMotion(const MotionEvent& event) override final;
  bool onMouse(const MouseEvent& event) override final; 
  void onResize(const ResizeEvent& event) override final;
  // FIXME: implement keyboard

  // everything will be rendered to texture
  RenderTexture2D canvas;
  // its size
  uint canvasWidth;
  uint canvasHeight;
};

END_NAMESPACE_DISTRHO

#endif // RAYUI_HPP

