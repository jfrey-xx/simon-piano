
// how often no refresh on idle state, in Hz. 0 to disable animation during idle state
#define UI_REFRESH_RATE 30

#include "RayUI.hpp"
#include "SimonUtils.h"

START_NAMESPACE_DISTRHO

// codes for used sprites
enum KeyIdx 
{
  BLACK_KEY,
  WHITE_KEY,
  BLACK_KEY_DIMMED,
  WHITE_KEY_DIMMED,
  BLACK_KEY_INSTRUCTION,
  WHITE_KEY_INSTRUCTION,
  BLACK_KEY_CORRECT,
  WHITE_KEY_CORRECT,
  BLACK_KEY_INCORRECT,
  WHITE_KEY_INCORRECT,
  BLACK_KEY_DEBUG,
  WHITE_KEY_DEBUG
};

// return true if the key for this note is C, D, E, F, G, A, B
bool isKeyWhite(uint note) {
  // only twelve note
  note = note % 12;
  return (note == 0 or note == 2 or note == 4 or note == 5 or note == 7 or note == 9 or note == 11);
}

// return the number of white keys for that many notes starting from root
uint getNbWhiteKeys(uint rootKey, uint nbKeys) {
  // stick to one simple case when root is C
  if (rootKey % 12 == 0) {
    if (nbKeys % 12 <= 4) {
      return ceil((nbKeys % 12 ) / (float) 2) + nbKeys / 12 * 7;
    }
    return ceil((nbKeys % 12 + 1) / (float) 2) + nbKeys / 12 * 7;
  }
  // otherwise, compute compared to simple case -- for sure there is a more clever way but is eludes me today
  return getNbWhiteKeys(0, nbKeys + rootKey % 12) - getNbWhiteKeys(0, rootKey % 12);
}

class SimonPianoUI : public RayUI
{
public:
   /**
      UI class constructor.
      The UI should be initialized to a default state that matches the plugin side.
    */

  SimonPianoUI() : RayUI(UI_REFRESH_RATE)
    {
      String resourcesLocation = getResourcesLocation();
      d_stdout("resources location: %s", resourcesLocation.buffer());

      // load texture for piano keys
      piano = LoadTexture(resourcesLocation + "piano.png");
      
      // Define the camera to look into our 3d world
      camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
      camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
      camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
      camera.fovy = 45.0f;                                // Camera field-of-view Y
      camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
    }

  ~SimonPianoUI() {
    // Texture unloading
    UnloadTexture(piano);
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
  void onMainDisplay() override
  {
    posOrig = GetMousePosition();
    ClearBackground(BLUE);
  }
  
   void onCanvasDisplay()
  {

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


    drawPiano({ 10.0f, 10.0f }, { 300.0f, 300.0f }, root, nbNotes);
    DrawRectangleLines(10, 10, 300, 300, RED);   

    DrawText(TextFormat("Default Mouse: [%i , %i]", (int)posOrig.x, (int)posOrig.y), 300, 25, 20, GREEN);
    DrawText(TextFormat("Virtual Mouse: [%i , %i]", (int)posScaled.x, (int)posScaled.y), 300, 55, 20, BLUE);

    DrawFPS(10, 10);
  }

    // ----------------------------------------------------------------------------------------------------------------

private:

  // ui
  Camera3D camera;
  // Ray collision hit info
  RayCollision collision = { 0 };     
  // for debug
  Vector2 posOrig;
  // texture for piano keys
  Texture2D piano;
  // location of current key in texture
  Rectangle keySpriteRec;
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

  // extract and draw sprite id a said location and size
  void drawKey(KeyIdx idx, Vector2 pos, Vector2 size) {
    // fixed size for all keys in the sprite sheet
    static const Rectangle spriteSize = {0, 0, 8, 32};
    // picking the right position
    int spriteShift = 0;
    switch(idx) {
    case BLACK_KEY:
      spriteShift = 2;
      break;
    case WHITE_KEY:
      spriteShift = 0;
      break;
    case BLACK_KEY_DIMMED:
      spriteShift = 6;
      break;
    case WHITE_KEY_DIMMED:
      spriteShift = 4;
      break;
    case BLACK_KEY_INSTRUCTION:
      spriteShift = 15;
      break;
    case WHITE_KEY_INSTRUCTION:
      spriteShift = 13;
      break;
    case BLACK_KEY_CORRECT:
      spriteShift = 19;
      break;
    case WHITE_KEY_CORRECT:
      spriteShift = 17;
      break;
    case BLACK_KEY_INCORRECT:
      spriteShift = 23;
      break;
    case WHITE_KEY_INCORRECT:
      spriteShift = 21;
      break;
    case BLACK_KEY_DEBUG:
      spriteShift = 10;
      break;
    default:
    case WHITE_KEY_DEBUG:
      spriteShift = 8;
      break;
    }
    
    // no rotation, no tint
    DrawTexturePro(piano, {spriteSize.x + spriteShift * spriteSize.width, spriteSize.y, spriteSize.width, spriteSize.height}, {pos.x, pos.y, size.x, size.y}, {0.0, 0.0}, 0, WHITE); 
  }

  // drawing a very simple keyboard using imgui, fetching drawList
  // pos: upper left corner of the widget
  // size: size of the widget
  void drawPiano(Vector2 pos, Vector2 size, uint rootKey, uint nbKeys) {
    // find number of white keys
    int nbWhiteKeys = getNbWhiteKeys(rootKey, nbKeys);
    // in case we start or end with black, leave some padding as half a white
    float uiNbWhiteKeys = nbWhiteKeys;
    if (!isKeyWhite(rootKey)) {
      uiNbWhiteKeys += 0.5;
    }
    if (!isKeyWhite(rootKey + nbKeys - 1)) {
      uiNbWhiteKeys += 0.5;
    }
    // around keyboard, between keys -- would be same ratio for 12 notes
    Vector2 margins = {size.x * 0.01f, size.y * 0.01f};
    // black keys over whites, width of a key will be conditioned by the former
    Vector2 keySize;
    keySize.x = (size.x  - margins.x * 2) / uiNbWhiteKeys;
    keySize.y = size.y - margins.y * 2;

    // draw the background
    // TODO: debug here
    DrawRectangle(pos.x, pos.y, size.x, size.y, BLUE);   

    // base position for current key
    Vector2 startPos = {pos.x + margins.x, pos.y + margins.y};
    // shift if we start with black key
    if (!isKeyWhite(rootKey)) {
      startPos.x += keySize.x * 0.5;
    }
    Vector2 curPos(startPos);
    KeyIdx sprite;

    // first draw white keys
    uint note = rootKey;
    for (uint i = 0; i < nbKeys; i++) {
      if(isKeyWhite(note)) {
	// dimm key if not in scale and option set
	if (shallNotPass && !scale[note % 12]) {
	  sprite = WHITE_KEY_DIMMED;
	}
	else {
	  sprite = WHITE_KEY;
	}
	// this note currently active, special color
	if ((int)note == curNote) {
	  switch(status) {
	  case INSTRUCTIONS:
	    sprite = WHITE_KEY_INSTRUCTION;
	    break;
	  case PLAYING_CORRECT:
	    sprite = WHITE_KEY_CORRECT;
	    break;
	  case PLAYING_INCORRECT:
	  case PLAYING_OVER:
	    sprite = WHITE_KEY_INCORRECT;
	    break;
	  default:
	    // debug
	    sprite = WHITE_KEY_DEBUG;
	    break;
	  }
	}
	drawKey(sprite, curPos, keySize);
	curPos.x += keySize.x;
      }
      note++;
    }
    // black on top
    curPos = startPos;
    note = rootKey;
    for (uint i = 0; i < nbKeys; i++) {
      // skip white keys, just advance
      if(isKeyWhite(note)) {
	curPos.x += keySize.x;
      }
      else {
	// dim key if not in scale and option set
	if (shallNotPass && !scale[note % 12]) {
	  sprite = BLACK_KEY_DIMMED;
	}
	else {
	  sprite = BLACK_KEY;
	}
	// this note currently active, special color
	if ((int)note == curNote) {
	  switch(status) {
	  case INSTRUCTIONS:
	    sprite = BLACK_KEY_INSTRUCTION;
	    break;
	  case PLAYING_CORRECT:
	    sprite = BLACK_KEY_CORRECT;
	    break;
	  case PLAYING_INCORRECT:
	  case PLAYING_OVER:
	    sprite = BLACK_KEY_INCORRECT;
	    break;
	  default:
	    // debug
	    sprite = BLACK_KEY_DEBUG;
	    break;
	  }
	}
	// for black key, shift half to right to center between two white keys
	drawKey(sprite, {curPos.x - keySize.x / 2, curPos.y}, keySize);
      }
      note++;
    }
  }
  
  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimonPianoUI)
};

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new SimonPianoUI();
}
 
// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
 
