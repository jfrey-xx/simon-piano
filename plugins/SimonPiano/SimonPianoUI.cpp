
// how often no refresh on idle state, in Hz. 0 to disable animation during idle state
#define UI_REFRESH_RATE 30

#include "RayUI.hpp"
#include "SimonUtils.h"

START_NAMESPACE_DISTRHO

 #define MIN(a, b) ((a)<(b)? (a) : (b))

class SimonPianoUI : public RayUI, public IdleCallback
{
public:
   /**
      UI class constructor.
      The UI should be initialized to a default state that matches the plugin side.
    */

    SimonPianoUI()
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

	// init rendering texture
	target = LoadRenderTexture(width, height);
	SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

	// Define the camera to look into our 3d world
	camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
	camera.fovy = 45.0f;                                // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
    }


  ~SimonPianoUI() {
    d_stdout("SimonPianoUI destructor");
    // cleanup
    UnloadRenderTexture(target);
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
    
    // take into account resize of window
    double scaleFactor = getScaleFactor();
    if (scaleFactor <= 0.0) {
      scaleFactor = 1.0;
    }
    uint width = DISTRHO_UI_DEFAULT_WIDTH * scaleFactor;
    uint height = DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor;

    double scaleWidth = GetScreenWidth() / (float) width;
    double scaleHeight = GetScreenHeight() / (float) height;
    float scale = MIN(scaleWidth, scaleHeight);

    // original mouse position
    SetMouseOffset(0,0);
    SetMouseScale(1,1);
    Vector2 posOrig = GetMousePosition();

    // Apply the same transformation as the virtual mouse to the real mouse (i.e. to work with raygui)
    SetMouseOffset(-(GetScreenWidth() - (width*scale))*0.5f, -(GetScreenHeight() - (height*scale))*0.5f);
    SetMouseScale(1/scale, 1/scale);
    Vector2 posScaled = GetMousePosition();

    // dummy animation
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    Vector3 cubeSize = { 2.0f, 2.0f, 2.0f };

    Ray ray = { 0 };                    // Picking line ray
    
    BeginTextureMode(target);
    
    ClearBackground(RAYWHITE);

    // action either via GUI or directly over the cube
    // NOTE: click on the GUI will also de-select cube. for testing only...
    bool butPress = GuiButton( (Rectangle){ 20, 40, 200, 20 }, "Press me!");
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      {
	if (!collision.hit)
	  {
	    ray = GetScreenToWorldRayEx(GetMousePosition(), camera, width, height);
	    
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


    DrawText(TextFormat("Default Mouse: [%i , %i]", (int)posOrig.x, (int)posOrig.y), 350, 25, 20, GREEN);
    DrawText(TextFormat("Virtual Mouse: [%i , %i]", (int)posScaled.x, (int)posScaled.y), 350, 55, 20, BLUE);

    EndTextureMode();
     
    BeginDrawing();

    // black around main screen
    ClearBackground(BLACK);    

    // Draw render texture to screen, properly scaled
    DrawTexturePro(
		   target.texture,
		   (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
		   (Rectangle){ (GetScreenWidth() - ((float)width*scale))*0.5f, (GetScreenHeight() - ((float)height*scale))*0.5f,
				(float)width*scale, (float)height*scale },
		   (Vector2){ 0, 0 }, 0.0f, WHITE);

    EndDrawing();
  }

  // mouse move
  bool onMotion(const MotionEvent& event) override
  {
    // unused: event.mod currently active keyboard modifier
    //d_stdout("DPF motion event pos %f,%f, abspos %f,%f",  event.pos.getX(), event.pos.getY(), event.absolutePos.getX(), event.absolutePos.getY());
    SetMousePosition(event.pos.getX(), event.pos.getY());
    return false;
  }

  // mouse press
  bool onMouse(const MouseEvent& event) override
  {
    // unused: event.mod currently active keyboard modifier
    //d_stdout("DPF mouse event button %d, press %d pos %f,%f, abspos %f,%f",  event.button, event.press, event.pos.getX(), event.pos.getY(), event.absolutePos.getX(), event.absolutePos.getY());
    // mouse button event should start from 1
    int button = event.button;
    if (button > 0) {
      SendMouseEvent(button-1, event.press, event.pos.getX(), event.pos.getY());
      return false;
    }
    // we do not catch a button 0 that should not happen...
    return true;
  }

  // resize
  void onResize(const ResizeEvent& event)
{
  // tell that to raylib
  SetWindowSize(event.size.getWidth(), event.size.getHeight());
  //d_stdout("resize mouse position: %d,%d", GetMouseX(), GetMouseY());
}


    // ----------------------------------------------------------------------------------------------------------------

private:

  // everything will be rendered to texture
  RenderTexture2D target;
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
 
