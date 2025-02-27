
#include "DistrhoUI.hpp"
#include "SimonUtils.h"

START_NAMESPACE_DISTRHO

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

   /**
      ImGui specific onDisplay function.
    */

  void idleCallback() override
  {
    draw();
    // force display refresh
    repaint();
  }

  //void onTrueDisplay() override
   void onDisplay() override 
    {
      draw();
    }

  // actual drawing
  void draw() {
    
    // dummy animation
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    
    BeginDrawing();
    
    ClearBackground(RAYWHITE);
    
    BeginMode3D(camera);
    // rotation animation
    rlPushMatrix();
    static int r = 0;
    r++;
    // rotate along <1,0,0> x-axis
    rlRotatef(r, 1, 0, 0);
    DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
    rlPopMatrix();
    DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
    
    DrawGrid(10, 1.0f);
    
    EndMode3D();
     
    EndDrawing();

  }

    // ----------------------------------------------------------------------------------------------------------------

private:


  // ui
  Camera3D camera;
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
 
