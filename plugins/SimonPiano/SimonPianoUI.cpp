
#include "DistrhoUI.hpp"
#include "SimonUtils.h"
#include <iostream>

START_NAMESPACE_DISTRHO
//#include "pugl/gl.h"

#include "raylib.h"
//#include "raymath.h"                // Vector2, Vector3, Quaternion and Matrix  functionality
//#define RLGL_IMPLEMENTATION
//#include "rlgl.h"
 //#include "raymath.h" 
 

// --------------------------------------------------------------------------------------------------------------------

class SimonPianoUI : public UI
{
public:
   /**
      UI class constructor.
      The UI should be initialized to a default state that matches the plugin side.
    */

    SimonPianoUI()
        : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT)
    {
        const double scaleFactor = getScaleFactor();

        if (d_isEqual(scaleFactor, 1.0))
        {
            setGeometryConstraints(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT);
        }
        else
        {
            const uint width = DISTRHO_UI_DEFAULT_WIDTH * scaleFactor;
            const uint height = DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor;
            setGeometryConstraints(width, height);
            setSize(width, height);
        }


            
	    //rlLoadExtensions((void*)puglGetProcAddress);
            const uint width = DISTRHO_UI_DEFAULT_WIDTH * scaleFactor;
            const uint height = DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor;

            //             rlglInit(50, 50);
            InitWindow(width,height, "raylib [core] example - basic window");
            
    // Initialize viewport and internal projection/modelview matrices
             //rlViewport(0, 0, width, height);
	    //rlMatrixMode(RL_PROJECTION);                        // Switch to PROJECTION matrix
	    //rlLoadIdentity();                                   // Reset current matrix (PROJECTION)
	    //rlOrtho(0, width, height, 0, 0.0f, 1.0f); // Orthographic projection with top-left corner at (0,0)
	    //rlMatrixMode(RL_MODELVIEW);                         // Switch back to MODELVIEW matrix
	    //rlLoadIdentity();                                   // Reset current matrix (MODELVIEW)

             // rlClearColor(245, 245, 245, 255);                   // Define clear color
                    //	    rlEnableDepthTest();     





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
  //void onTrueDisplay() override
   void onDisplay() 
    {

       //----------------------------------------------------------------------------------
   // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
      UpdateCamera(&camera, CAMERA_FREE);

     BeginDrawing();

                 ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);

                DrawGrid(10, 1.0f);

            EndMode3D();


        EndDrawing();



    }
   void onnDisplay()
  //  void onImGuiDisplay() override
    {
        d_stdout("on display \n");
               BeginDrawing();
                ClearBackground(WHITE);
                EndDrawing();
                       static int toto = 0;
                       static bool tata = false;
                       toto++;
                       if (toto > 20) {
                           toto = 0;
                       }
                       else if (toto > 10) {
                           ClearBackground(RED);
                           //                                 rlClearColor(245, 245, 245, 255);                   // Define clear color
                       }
                       else if (toto > 20) {
                            toto = 0;
                       }
                       else {
                ClearBackground(WHITE);
                           //             rlClearColor(245, 0, 245, 255);                   // Define clear color
                       }

            // Draw internal render batch buffers (2D data)
            //-----------------------------------------------


      return;

      
    }

    // ----------------------------------------------------------------------------------------------------------------

private:


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
 
