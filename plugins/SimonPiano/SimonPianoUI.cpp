
#include "DistrhoUI.hpp"
#include "SimonUtils.h"

//#define RLGL_IMPLEMENTATION

START_NAMESPACE_DISTRHO
//#include "pugl/gl.h"
#define RAYLIB_IMPLEMENTATION
//#define RLGL_IMPLEMENTATION
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h" 

#define RED        (Color){ 230, 41, 55, 255 }     // Red
#define RAYWHITE   (Color){ 245, 245, 245, 255 }   // My own White (raylib logo)
#define DARKGRAY   (Color){ 80, 80, 80, 255 }      // Dark Gray


// Color, 4 components, R8G8B8A8 (32bit)
typedef struct ColorB {
    unsigned char r;        // Color red value
    unsigned char g;        // Color green value
    unsigned char b;        // Color blue value
    unsigned char a;        // Color alpha value
} ColorB;

// Camera type, defines a camera position/orientation in 3d space
typedef struct CameraB {
    Vector3 position;       // Camera position
    Vector3 target;         // Camera target it looks-at
    Vector3 up;             // Camera up vector (rotation over its axis)
    float fovy;             // Camera field-of-view apperture in Y (degrees) in perspective, used as near plane width in orthographic
    int projection;         // Camera projection: CAMERA_PERSPECTIVE or CAMERA_ORTHOGRAPHIC
} CameraB;

// Matrix, 4x4 components, column major, OpenGL style, right-handed
/*typedef struct MatrixR {
    float m0, m4, m8, m12;  // Matrix first row (4 components)
    float m1, m5, m9, m13;  // Matrix second row (4 components)
    float m2, m6, m10, m14; // Matrix third row (4 components)
    float m3, m7, m11, m15; // Matrix fourth row (4 components)
    } MatrixR; */


// Draw cube wires
static void DrawCubeWiresB(Vector3 position, float width, float height, float length, Color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    rlPushMatrix();

        rlTranslatef(position.x, position.y, position.z);
        //rlRotatef(45, 0, 1, 0);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            // Front Face -----------------------------------------------------
            // Bottom Line
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right

            // Left Line
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right

            // Top Line
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left

            // Right Line
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left

            // Back Face ------------------------------------------------------
            // Bottom Line
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right

            // Left Line
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right

            // Top Line
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left

            // Right Line
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left

            // Top Face -------------------------------------------------------
            // Left Line
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left Front
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left Back

            // Right Line
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right Front
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right Back

            // Bottom Face  ---------------------------------------------------
            // Left Line
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Top Left Front
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left Back

            // Right Line
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Top Right Front
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Top Right Back
        rlEnd();
    rlPopMatrix();
}

static void DrawRectangleVB(Vector2 position, Vector2 size, Color color)
{
    rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        rlVertex2f(position.x, position.y);
        rlVertex2f(position.x, position.y + size.y);
        rlVertex2f(position.x + size.x, position.y + size.y);

        rlVertex2f(position.x, position.y);
        rlVertex2f(position.x + size.x, position.y + size.y);
        rlVertex2f(position.x + size.x, position.y);
    rlEnd();
}

// Draw a grid centered at (0, 0, 0)
static void DrawGridB(int slices, float spacing)
{
    int halfSlices = slices / 2;

    rlBegin(RL_LINES);
        for (int i = -halfSlices; i <= halfSlices; i++)
        {
            if (i == 0)
            {
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
            }

            rlVertex3f((float)i*spacing, 0.0f, (float)-halfSlices*spacing);
            rlVertex3f((float)i*spacing, 0.0f, (float)halfSlices*spacing);

            rlVertex3f((float)-halfSlices*spacing, 0.0f, (float)i*spacing);
            rlVertex3f((float)halfSlices*spacing, 0.0f, (float)i*spacing);
        }
    rlEnd();
}

// Draw cube
// NOTE: Cube position is the center position
static void DrawCubeB(Vector3 position, float width, float height, float length, Color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    rlPushMatrix();

        // NOTE: Be careful! Function order matters (rotate -> scale -> translate)
        rlTranslatef(position.x, position.y, position.z);
        //rlScalef(2.0f, 2.0f, 2.0f);
        //rlRotatef(45, 0, 1, 0);

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            // Front Face -----------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left

            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right

            // Back Face ------------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right

            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left

            // Top Face -------------------------------------------------------
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Bottom Right

            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Bottom Right

            // Bottom Face ----------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left

            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Top Right
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left

            // Right face -----------------------------------------------------
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Left

            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Left

            // Left Face ------------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Right

            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Right
        rlEnd();
    rlPopMatrix();
}

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

        InitWindow(800, 450, "raylib [core] example - basic window");

            
	    //rlLoadExtensions((void*)puglGetProcAddress);
            const uint width = DISTRHO_UI_DEFAULT_WIDTH * scaleFactor;
            const uint height = DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor;

            // rlglInit(width, height);
            /*
    // Initialize viewport and internal projection/modelview matrices
	    rlViewport(0, 0, width, height);
	    rlMatrixMode(RL_PROJECTION);                        // Switch to PROJECTION matrix
	    rlLoadIdentity();                                   // Reset current matrix (PROJECTION)
	    rlOrtho(0, width, height, 0, 0.0f, 1.0f); // Orthographic projection with top-left corner at (0,0)
	    rlMatrixMode(RL_MODELVIEW);                         // Switch back to MODELVIEW matrix
	    rlLoadIdentity();                                   // Reset current matrix (MODELVIEW)

	    rlClearColor(245, 245, 245, 255);                   // Define clear color
	    rlEnableDepthTest();     

            */
    // Define the camera to look into our 3d world
    camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    //    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    cubePosition = { 0.0f, 0.0f, 0.0f };



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
   void onDisplay() override
    {

       //----------------------------------------------------------------------------------

        BeginDrawing();

        ClearBackground(WHITE);
            DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
            //        DrawRectangleVB((Vector2){ 10.0f, 10.0f }, (Vector2){ 780.0f, 20.0f }, RED);

        EndDrawing();


    }
    /*
   void onDisplaytoto()
  //  void onImGuiDisplay() override
    {
        rlClearScreenBuffers();             // Clear current framebuffer
	Matrix matProj = MatrixPerspective((double)(camera.fovy*DEG2RAD), (double)getWidth()/(double)getHeight(), 0.01, 1000.0);
            Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

            rlSetMatrixModelview(matView);    // Set internal modelview matrix (default shader)
            rlSetMatrixProjection(matProj);   // Set internal projection matrix (default shader)

	    
      DrawCubeB(cubePosition, 2.0f, 2.0f, 2.0f, RED);
         DrawCubeWiresB(cubePosition, 2.0f, 2.0f, 2.0f, RAYWHITE);
            DrawGridB(10, 1.0f);


        rlDrawRenderBatchActive();

	matProj = MatrixOrtho(0.0, getWidth(), getHeight(), 0.0, 0.0, 1.0);
            matView = MatrixIdentity();

            rlSetMatrixModelview(matView);    // Set internal modelview matrix (default shader)
                       rlSetMatrixProjection(matProj);   // Set internal projection matrix (default shader)

	   DrawRectangleVB((Vector2){ 10.0f, 10.0f }, (Vector2){ 780.0f, 20.0f }, RED);

            // Draw internal render batch buffers (2D data)
            rlDrawRenderBatchActive();
            //-----------------------------------------------


      return;

      
    }

    */
    // ----------------------------------------------------------------------------------------------------------------

private:
  CameraB camera;
  Vector3 cubePosition;


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
