
#ifndef RAYUI_HPP
#define RAYUI_HPP

#include "DistrhoUI.hpp"

// encapsulate raylib in the DISTHRO namespace to avoid clashes with other functions. Child classes must then also use raylib within this namespace
START_NAMESPACE_DISTRHO
// we got to enable some extra functions in raylib for us to pass events
#include "raylib.h"
#include "rlgl.h"
#include "raygui.h"

// default FPS if not set
#define UI_DEFAULT_REFRESH_RATE 30

// Wrapper for geting raylib within a DPF plugin
// Even though there is an option with PUGL to scale the window we will scale UI through raylib, better/looking with texture filtering and more flexible (possible to use margins around the canvas)
// TODO: deal with scale change e.g. through onScaleFactorChanged or by polling scaleFactor if I get that to change in some situations

class RayUI : public UI, private IdleCallback
{
public:
  // newFPS: set desired FPS. 0 to disable animation during idle state
  // filter: define the TextureFilter used for canvas, e.g. TEXTURE_FILTER_POINT for pixel approximation
  RayUI(uint newFPS=UI_DEFAULT_REFRESH_RATE, TextureFilter filter=TEXTURE_FILTER_BILINEAR);
  ~RayUI();
protected:
  // used for constant refresh rate. might override if client do not want that
  void idleCallback() override;
  // all area of the window, called before onCanvasDisplay
  // Will be enclosed in BeginDrawing() .. EndDrawing(). Mouse will be set to whole window
  virtual void onMainDisplay();
  // also in main display, but called after canvas is rendered
  virtual void onMainDisplayLast() {}
  // part that will be put to scale by rendering to a texture. At least that is to be implemented
  // mouse will be offset to canvas
  // Note: to do raycasting with mouse, remember to specify canvas size, e.g. 
  //   ray = GetScreenToWorldRayEx(GetMousePosition(), camera, getCanvasWidth(), getCanvasHeight());
  virtual void onCanvasDisplay() = 0;

  // return path to desired resource, terminated by os separator -- not ensuring if said location exists. 
  String getResourcesLocation();

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
  // configured fps
  uint fps;
  // (supposed) full path to resources
  String resourcesLocation;
};

END_NAMESPACE_DISTRHO

#endif // RAYUI_HPP

