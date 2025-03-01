
#include "DistrhoUI.hpp"
#include "SimonUtils.h"

START_NAMESPACE_DISTRHO

// we got to enable some extra functions in raylib for us to pass events
#define PLATFORM_DPF
#include "raylib.h"
#include "rlgl.h"

// how often no refresh on idle state, in Hz. 0 to disable animation during idle state
#define UI_REFRESH_RATE 30

// --------------------------------------------------------------------------------------------------------------------

class SimonPianoUI : public UI, public IdleCallback
{
public:
   /**
      UI class constructor.
      The UI should be initialized to a default state that matches the plugin side.
    */

    SimonPianoUI()
      : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT) 
    {
        // compute actual dimensions of the window
        double scaleFactor = getScaleFactor();
	if (scaleFactor <= 0.0) {
	  scaleFactor = 1.0;
	}
	const uint width = DISTRHO_UI_DEFAULT_WIDTH * scaleFactor;
	const uint height = DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor;
	setGeometryConstraints(width, height);

	// we have to resize window size if a scale factor is to be applied
        if (!d_isEqual(scaleFactor, 1.0))
        {
            setSize(width, height);
        }

        // init raylib -- unused title with DPF platform
	InitWindow(width,height, "");
	// always animate
	if (UI_REFRESH_RATE > 0) {
	  // method called every xx milliseconds
	  int refreshTime = 1000/UI_REFRESH_RATE;
	  addIdleCallback(this, refreshTime);
	}


	// Define the camera to look into our 3d world
	camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
	camera.fovy = 45.0f;                                // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
    }


  ~SimonPianoUI() {
    // cleanup
    if (UI_REFRESH_RATE > 0) {
      removeIdleCallback(this);
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
      case kEffectiveScaleC:
      case kEffectiveScaleCs:
      case kEffectiveScaleD:
      case kEffectiveScaleDs:
      case kEffectiveScaleE:
      case kEffectiveScaleF:
      case kEffectiveScaleFs:
      case kEffectiveScaleG:
      case kEffectiveScaleGs:
      case kEffectiveScaleA:
      case kEffectiveScaleAs:
      case kEffectiveScaleB:
	{
	  int numScale = index - kEffectiveScaleC;
	  if (numScale >= 0 && numScale < 12) {
	    scale[numScale] = value;
	  }
	}
	break;
      case kShallNotPass:
	shallNotPass = value;
	break;
      case kRoundsForMiss:
	roundsForMiss = value;
	break;
      case kNbMiss:
	nbMiss = value;
	break;
      case kMaxMiss:
	maxMiss = value;
	break;
      case kMaxRound:
	maxRound = value;
	break;

      default:
	break;
      }
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Widget Callbacks

  void idleCallback() override
  {
    // force display refresh
    repaint();
  }

   void onDisplay() override 
  {
    // dummy animation
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    Vector3 cubeSize = { 2.0f, 2.0f, 2.0f };

    Ray ray = { 0 };                    // Picking line ray

    d_stdout("loop mouse button down 0: %d, 1: %d, 2: %d", IsMouseButtonDown(0), IsMouseButtonDown(1), IsMouseButtonDown(2));
    d_stdout("loop mouse button pressed 0: %d, 1: %d, 2: %d", IsMouseButtonPressed(0), IsMouseButtonPressed(1), IsMouseButtonPressed(2));
    d_stdout("loop mouse button released 0: %d, 1: %d, 2: %d", IsMouseButtonReleased(0), IsMouseButtonReleased(1), IsMouseButtonReleased(2));

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      {
	if (!collision.hit)
	  {
	    ray = GetScreenToWorldRay(GetMousePosition(), camera);
	    
	    // Check collision between ray and box
	    collision = GetRayCollisionBox(ray,
					   (BoundingBox){(Vector3){ cubePosition.x - cubeSize.x/2, cubePosition.y - cubeSize.y/2, cubePosition.z - cubeSize.z/2 },
							 (Vector3){ cubePosition.x + cubeSize.x/2, cubePosition.y + cubeSize.y/2, cubePosition.z + cubeSize.z/2 }});
	  }
	else collision.hit = false;
      }
    
    
    BeginDrawing();
    
    ClearBackground(RAYWHITE);
    
    BeginMode3D(camera);
    // rotation animation
    rlPushMatrix();
    static int r = 0;
    r++;
    // rotate along <1,0,0> x-axis
    rlRotatef(r, 1, 0, 0);
    if (collision.hit) {
      DrawCube(cubePosition, cubeSize.x, cubeSize.y, cubeSize.z, BLUE);
    }
    else {
      DrawCube(cubePosition, cubeSize.x, cubeSize.y, cubeSize.z, RED);
    }
    rlPopMatrix();
    DrawCubeWires(cubePosition, cubeSize.x, cubeSize.y, cubeSize.z, MAROON);
    
    DrawGrid(10, 1.0f);
    
    EndMode3D();
     
    EndDrawing();

    d_stdout("raylib mouse button down 0: %d, 1: %d, 2: %d", IsMouseButtonDown(0), IsMouseButtonDown(1), IsMouseButtonDown(2));
    d_stdout("raylib mouse button pressed 0: %d, 1: %d, 2: %d", IsMouseButtonPressed(0), IsMouseButtonPressed(1), IsMouseButtonPressed(2));
    d_stdout("raylib mouse button released 0: %d, 1: %d, 2: %d", IsMouseButtonReleased(0), IsMouseButtonReleased(1), IsMouseButtonReleased(2));



    d_stdout("raylib mouse position: %d,%d", GetMouseX(), GetMouseY());

  }

  // mouse move
  bool onMotion(const MotionEvent& event) override
  {
    // unused: event.mod currently active keyboard modifier
    d_stdout("DPF motion event pos %f,%f, abspos %f,%f",  event.pos.getX(), event.pos.getY(), event.absolutePos.getX(), event.absolutePos.getY());
    SetMousePosition(event.pos.getX(), event.pos.getY());
    return false;
  }

  // mouse press
  bool onMouse(const MouseEvent& event) override
  {
    // unused: event.mod currently active keyboard modifier
    d_stdout("DPF mouse event button %d, press %d pos %f,%f, abspos %f,%f",  event.button, event.press, event.pos.getX(), event.pos.getY(), event.absolutePos.getX(), event.absolutePos.getY());
    // mouse button event should start from 1
    int button = event.button;
    if (button > 0) {
      SendMouseEvent(button-1, event.press, event.pos.getX(), event.pos.getY());
      return false;
    }
    // we do not catch a button 0 that should not happen...
    return true;
  }

    // ----------------------------------------------------------------------------------------------------------------

private:


  // ui
  Camera3D camera;
  // Ray collision hit info
  RayCollision collision = { 0 };     
  // parameters sync with DSP
  int status = params[kStatus].def;
  int root = params[kRoot].def;
  int nbNotes = params[kNbNotes].def;
  int curNote = params[kCurNote].def;
  int round = params[kRound].def;
  int step = params[kStep].def;
  bool scale[12] = {0};
  int roundsForMiss = params[kRoundsForMiss].def;
  int nbMiss = params[kNbMiss].def;
  int maxMiss = params[kMaxMiss].def;
  int maxRound = params[kMaxRound].def;
  bool shallNotPass = params[kShallNotPass].def;
  
  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimonPianoUI)
};

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new SimonPianoUI();
}
 
// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
 
