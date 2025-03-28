
// how often no refresh on idle state, in Hz. 0 to disable animation during idle state
#define UI_REFRESH_RATE 30

#include "RayUI.hpp"
#include "SimonUtils.h"

START_NAMESPACE_DISTRHO


class SimonPianoUI : public RayUI
{
public:
   /**
      UI class constructor.
      The UI should be initialized to a default state that matches the plugin side.
    */

  SimonPianoUI() : RayUI(UI_REFRESH_RATE)
    {
	// Define the camera to look into our 3d world
	camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
	camera.fovy = 45.0f;                                // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
    }


  ~SimonPianoUI() {
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

   void onCanvasDisplay()
  {
    

    //Vector2 posOrig = GetMousePosition();
    Vector2 posScaled = GetMousePosition();

    // dummy animation
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    Vector3 cubeSize = { 2.0f, 2.0f, 2.0f };

    Ray ray = { 0 };                    // Picking line ray
    
    ClearBackground(RAYWHITE);

    // action either via GUI or directly over the cube
    // NOTE: click on the GUI will also de-select cube. for testing only...
    bool butPress = GuiButton( (Rectangle){ 20, 40, 200, 20 }, "Press me!");
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      {
	if (!collision.hit)
	  {
	    ray = GetScreenToWorldRayEx(GetMousePosition(), camera, getCanvasWidth(), getCanvasHeight());
	    
	    // Check collision between ray and box
	    collision = GetRayCollisionBox(ray,
					   (BoundingBox){(Vector3){ cubePosition.x - cubeSize.x/2, cubePosition.y - cubeSize.y/2, cubePosition.z - cubeSize.z/2 },
							 (Vector3){ cubePosition.x + cubeSize.x/2, cubePosition.y + cubeSize.y/2, cubePosition.z + cubeSize.z/2 }});
	  }
	else collision.hit = false;
      }
    
    else if (butPress)
      {
	if (!collision.hit)
	  {
	    collision.hit = true;
	  }
	else collision.hit = false;
      }

    
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

    DrawFPS(10, 10);


    //DrawText(TextFormat("Default Mouse: [%i , %i]", (int)posOrig.x, (int)posOrig.y), 350, 25, 20, GREEN);
    DrawText(TextFormat("Virtual Mouse: [%i , %i]", (int)posScaled.x, (int)posScaled.y), 350, 55, 20, BLUE);
     
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
 
