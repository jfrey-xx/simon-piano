
#include "RayUI.hpp"
#include "DistrhoPluginUtils.hpp"

// put here raygui implementation
START_NAMESPACE_DISTRHO
#include "raygui.h"
#include "style_qualya.h"

#define MIN(a, b) ((a)<(b)? (a) : (b))

 RayUI::RayUI(uint newFPS, TextureFilter filter)
  : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT) 
{
  fps = newFPS;

  // determine resource location. If the plugin is not bundled, i.e. with jack, we will try a location relative to binary
  // HOTFIX, for clap (due to host upon folder passed upon init?) bundle path is (or might?) be the path to the binary, we need to target the folder instead
  if (getBundlePath() == nullptr or String(getPluginFormatName()) == "CLAP")
    {
      resourcesLocation = getBinaryFilename();
      resourcesLocation.truncate(resourcesLocation.rfind(DISTRHO_OS_SEP));
      // on Mac the binary when MACOS_APP_BUNDLE is in Contents/MacOS, should detect and go back twice more
      // TODO check that this is the same on Mac with jack -- MACOS_APP_BUNDLE appears to be set with UI files, at least for our opengl
#ifdef DISTRHO_OS_MAC
      resourcesLocation.truncate(resourcesLocation.rfind(DISTRHO_OS_SEP));
      resourcesLocation.truncate(resourcesLocation.rfind(DISTRHO_OS_SEP));
#endif // DISTRHO_OS_MAC
      resourcesLocation += getResourcePath("");
    }
  else
    {
      resourcesLocation = getResourcePath(getBundlePath());
    }
  resourcesLocation += String(DISTRHO_OS_SEP);

  // compute actual dimensions of the window
  double scaleFactor = getScaleFactor();
  if (scaleFactor <= 0.0) {
    scaleFactor = 1.0;
  }

  // to get a consistent display the canvas size is set in stone to the default size. Only the window will be scaled
  canvasWidth = DISTRHO_UI_DEFAULT_WIDTH;
  canvasHeight = DISTRHO_UI_DEFAULT_HEIGHT;
  setGeometryConstraints(canvasWidth * scaleFactor, canvasHeight * scaleFactor);
  
  // we have to resize window size if a scale factor is to be applied
  if (!d_isEqual(scaleFactor, 1.0))
    {
      setSize(canvasWidth * scaleFactor, canvasHeight * scaleFactor);
    }
  
  // init raylib -- unused title with DPF platform
  InitWindow(canvasWidth * scaleFactor, canvasHeight * scaleFactor, "");
  
  // always animate if option set
  if (fps > 0) {
    // method called every xx milliseconds
    int refreshTime = 1000/fps;
    addIdleCallback(this, refreshTime);
  }

  // load style
  GuiLoadStyleQualya();

  // init rendering texture
  canvas = LoadRenderTexture(canvasWidth, canvasHeight);
  SetTextureFilter(canvas.texture, filter);
}

RayUI::~RayUI() {
  // cleanup
  if (fps > 0) {
    removeIdleCallback(this);
  }
  UnloadRenderTexture(canvas);
}

void RayUI::idleCallback()
{
    // force display refresh
    repaint();
}

void RayUI::onMainDisplay()
{
  // black around main screen
  ClearBackground(BLACK);
}

void RayUI::onDisplay()
{
  // TODO: scale window if scale factor change?

  // update scale for drawing area
  double scaleWidth = GetScreenWidth() / (float) canvasWidth;
  double scaleHeight = GetScreenHeight() / (float) canvasHeight;
  float scale = MIN(scaleWidth, scaleHeight);

  BeginDrawing();
  
  // original mouse position
  SetMouseOffset(0,0);
  SetMouseScale(1,1);
  // let child class draw to mian screen first
  onMainDisplay();

  // Apply the same transformation as the virtual mouse to the real mouse (i.e. to work with raygui)
  SetMouseOffset(-(GetScreenWidth() - (canvasWidth*scale))*0.5f, -(GetScreenHeight() - (canvasHeight*scale))*0.5f);
  SetMouseScale(1/scale, 1/scale);

  // using texture to render scaled UI
  BeginTextureMode(canvas);
  onCanvasDisplay();
  EndTextureMode();
  
  // Draw render texture to screen, properly scaled
  DrawTexturePro(
		 canvas.texture,
		 (Rectangle){ 0.0f, 0.0f, (float)canvas.texture.width, (float)-canvas.texture.height },
		 (Rectangle){ (GetScreenWidth() - ((float)canvasWidth*scale))*0.5f, (GetScreenHeight() - ((float)canvasHeight*scale))*0.5f,
			      (float)canvasWidth*scale, (float)canvasHeight*scale },
		 (Vector2){ 0, 0 }, 0.0f, WHITE);
  
  // back to global mouse coordinates for the last pass
  SetMouseOffset(0,0);
  SetMouseScale(1,1);
  // let child class draw to mian screen first
  onMainDisplayLast();

  EndDrawing();
  
}

// mouse move
bool RayUI::onMotion(const MotionEvent& event)
{
  // unused: event.mod currently active keyboard modifier
#if defined(PLATFORM_DPF)
  SetMousePosition(event.pos.getX(), event.pos.getY());
#endif
  return false;
}

// mouse press
bool RayUI::onMouse(const MouseEvent& event)
{
  // unused: event.mod currently active keyboard modifier
  // mouse button event should start from 1
#if defined(PLATFORM_DPF)
  int button = event.button;
  if (button > 0) {
    SendMouseEvent(button-1, event.press, event.pos.getX(), event.pos.getY());
    return false;
  }
#endif
  // we do not catch a button 0 that should not happen...
  return true;
}

// resize
void RayUI::onResize(const ResizeEvent& event)
{
  // tell that to raylib
  SetWindowSize(event.size.getWidth(), event.size.getHeight());
}


String RayUI::getResourcesLocation()
{
  return String(resourcesLocation);
}

END_NAMESPACE_DISTRHO
