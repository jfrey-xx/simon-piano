
// how often no refresh on idle state, in Hz. 0 to disable animation during idle state
#define UI_REFRESH_RATE 30
// how many frames per seconds for animations
#define ANIM_FRAME_RATE 30

#include "RayUI.hpp"
#include "SimonUtils.h"
// for requestMIDI for web
#if defined(DISTRHO_OS_WASM)
#include "DistrhoStandaloneUtils.hpp"
#endif

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
  BLACK_KEY_PLAY,
  WHITE_KEY_PLAY,
  BLACK_KEY_DEBUG,
  WHITE_KEY_DEBUG,
  BACKGROUND_LEFT,
  BACKGROUND_RIGHT,
  BACKGROUND_FELT,
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

  SimonPianoUI() : RayUI(UI_REFRESH_RATE, TEXTURE_FILTER_POINT)
    {
      String resourcesLocation = getResourcesLocation();
      d_stdout("resources location: %s", resourcesLocation.buffer());

      // load texture for piano keys
      piano = LoadTexture(resourcesLocation + "piano.png");

      // camera above origin, high enough to fit model with fov and have full model in view
      camera.position = (Vector3){ 0.0, 5.1, 0.0 };
      // toward origin
      camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
      // camera pointing down
      camera.up = (Vector3){ 0.0, 0.0, -1.0};
      // field-of-view Y, we want to avoid deformations too extreme with perspective
      camera.fovy = 90.0f/8;
      // camera projection type
      camera.projection = CAMERA_PERSPECTIVE;

      // load model and animation
      model = LoadModel(resourcesLocation + "piano.gltf");
      d_stdout("model loaded. material count: %d, mesh count: %d", model.materialCount, model.meshCount);
      // expect first animation key press, second wrong key
	// TODO: detect anim mapping depending on name?
      modelAnimations = LoadModelAnimations(resourcesLocation + "piano.gltf", &animsCount);
      d_stdout("anim loaded, count %d", animsCount);

      // init texture used to draw piano, wide as a hack to tune margins ratio on the shape on-screen
      texturePiano = LoadRenderTexture(1024, 128);
      // for 3D scene, mesh is supposed to be a square, will help with filtering on Y during key press animation and subsequent rotation
      canvasPiano = LoadRenderTexture(1024, 1024);
      // to deal with lots of keys we enable filtering for both
      SetTextureFilter(texturePiano.texture, TEXTURE_FILTER_BILINEAR);
      SetTextureFilter(canvasPiano.texture, TEXTURE_FILTER_BILINEAR);

      // last texture should be the one we target for the surface of the piano
      if (model.materialCount > 0) {
	// first unload texture that will not be used
	rlUnloadTexture(model.materials[model.materialCount-1].maps[MATERIAL_MAP_DIFFUSE].texture.id);
	// replace model texture with this one
	SetMaterialTexture(&(model.materials[model.materialCount-1]), MATERIAL_MAP_DIFFUSE, texturePiano.texture);
      }
    }

  ~SimonPianoUI() {
    // Texture unloading
    UnloadTexture(piano);
    UnloadRenderTexture(texturePiano);
    // unload model (including meshes) and animations
    UnloadModelAnimations(modelAnimations, animsCount);
    UnloadModel(model);
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
    ClearBackground(BLUE);

    // render piano to texture -- on main display rather than canvas because cannot nest texture rendering
    BeginTextureMode(texturePiano);
    // note: since rendered to another canvas afterward no need to flip
    drawPiano({0, 0}, {(float)texturePiano.texture.width, (float)texturePiano.texture.height}, root, nbNotes, true);
    EndTextureMode();

    // Select current animation
    if (animsCount > 0) {
      bool  newAnim = false;
      // we are playing a new note, anim key press
      if (animNote != curNote && curNote >= 0) {
	animIndex = 0;
	newAnim = true;
      }
      // anim wrong upon said feedback
      else if (animStatus != status && status == FEEDBACK_INCORRECT) {
	animIndex = 1;
	newAnim = true;
      }
      // nothing new to animate, just stay still
      else if (curNote < 0 && status != FEEDBACK_INCORRECT && animIndex >= 0 && animCurrentTime >= animDuration) {
	animIndex = -1;
	// first frame of first animation for still
	UpdateModelAnimation(model, modelAnimations[0], 0);
      }
      // we have an animation to (re)set
      if (newAnim && animIndex >= 0 && animIndex < animsCount) {
	animCurrentTime = 0;
	animDuration = modelAnimations[animIndex].frameCount / (float) ANIM_FRAME_RATE;
	// set to first frame
	UpdateModelAnimation(model, modelAnimations[animIndex], 0);
      }
      // since animation is already set to first frame, start on next framees
      else {
	// Update model animation, if any
	if (animIndex >= 0 && animIndex < animsCount) {
	  ModelAnimation anim = modelAnimations[animIndex];
	  if (anim.frameCount > 0 && animDuration > 0 && animCurrentTime < animDuration) {
	    animCurrentTime += GetFrameTime();
	    int animCurrentFrame =  anim.frameCount * animCurrentTime / animDuration;
	    // play animation once
	    if (animCurrentFrame >= anim.frameCount) {
	      animCurrentFrame = anim.frameCount - 1;
	    }
	    UpdateModelAnimation(model, anim, animCurrentFrame);
	  }
	}
      }

      animNote = curNote;
      animStatus = status;
      newAnim = false;
    }

    // draw piano mesh to virtual scene
    BeginTextureMode(canvasPiano);
    // we have to clear background for anything to display
    ClearBackground(Color({0,0,0,0}));
    BeginMode3D(camera);
    // Draw animated model, original scale, no tint
    DrawModel(model, position, 1.0f, WHITE);
    EndMode3D();

    EndTextureMode();
  }
  
  void onCanvasDisplay() override
  {

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); 
    GuiSetState(STATE_NORMAL);
    
    switch (status) {
    case WAITING:
      GuiLabel(layoutRecs[0], "Welcome");
      GuiLabel(layoutRecs[1], "Press Start for a new game");
      break;
    case STARTING:
      GuiLabel(layoutRecs[0], TextFormat("Round %d", round));
      GuiLabel(layoutRecs[1], TextFormat("Pay attention..."));
      break;
    case INSTRUCTIONS:
      GuiLabel(layoutRecs[0], TextFormat("Round %d - step %d", round, step));
      GuiLabel(layoutRecs[1], TextFormat("Play after me!"));
      break;
    case GAMEOVER:
      GuiLabel(layoutRecs[0], TextFormat("Game over during Round %d - step %d", round, step));
      GuiLabel(layoutRecs[1], TextFormat("Try again!"));
      break;
    case PLAYING_WAIT:
    case PLAYING_CORRECT:
    case PLAYING_INCORRECT:
    case FEEDBACK_INCORRECT:
      GuiLabel(layoutRecs[0], TextFormat("Round %d - step %d", round, step));
      GuiLabel(layoutRecs[1], TextFormat("Your turn!"));
      break;
    default:
      GuiLabel(layoutRecs[0], TextFormat("Round %d", round));
      GuiLabel(layoutRecs[1], TextFormat("Play after me!"));
      break;
    }

    // same button for start/stop
    if (isRunning(status)) {
      // highlight button for abort
      GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, GuiGetStyle(DEFAULT, BASE_COLOR_PRESSED));
      GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED));
      GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, GuiGetStyle(DEFAULT, TEXT_COLOR_FOCUSED));
      GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, GuiGetStyle(DEFAULT, BASE_COLOR_FOCUSED));
      if(GuiButton(layoutRecs[2], "Abort")) {
	// send false/true cylce to make sure to toggle
	setParameterValue(kStart, true);
	setParameterValue(kStart, false);
      }
      GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL));
      GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));
      GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, GuiGetStyle(DEFAULT, BASE_COLOR_FOCUSED));
      GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, GuiGetStyle(DEFAULT, TEXT_COLOR_FOCUSED));
    } else {
      if(GuiButton(layoutRecs[2], "Start")) {
	// send false/true cylce to make sure to toggle
	setParameterValue(kStart, false);
	setParameterValue(kStart, true);
      }
    }

    // disable part of the UI during play
    if (isRunning(status)) {
      GuiSetState(STATE_DISABLED);
    }

    int preset = getPresetIdx(root, nbNotes, scale);
    if (preset < 0) {
      // failsafe, last should be custom
      preset = NB_PRESETS - 1;
    }
    // somehow presets, a tad tedious
    int uiPreset = preset;

    GuiComboBox(layoutRecs[3], presetsNames, &uiPreset);
    if (uiPreset != preset) {
      // special case for last preset, custom, we force cycle because nothing would change upon selection
      if (uiPreset >= NB_PRESETS - 1) {
	uiPreset = 0;
      }
      if (presets[uiPreset].root >= 0 && presets[uiPreset].root != root) {
	setParameterValue(kRoot, presets[uiPreset].root);
      }
      if (presets[uiPreset].nbNotes >= 0 && presets[uiPreset].nbNotes != nbNotes) {
	setParameterValue(kNbNotes, presets[uiPreset].nbNotes);
      }
      for (int i=0; i < 12; i++) {
	if (presets[uiPreset].scale[i] >= 0 && presets[uiPreset].scale[i] != scale[i]) {
	  setParameterValue(kScaleC + i, !scale[i]);
	}
      }
    }
    
    // An additional button (aligned with Start) on the web to ask for webmidi permission
#if defined(DISTRHO_OS_WASM)
    if (supportsMIDI() && !isMIDIEnabled() && GuiButton(layoutRecs[4], "Enable WebMIDI")) {
      requestMIDI();
    }
#endif

    // sync root note
    float uiRoot = root;
    GuiSlider(layoutRecs[5], TextFormat("Root note: %d", (int)uiRoot), NULL, &uiRoot, params[kRoot].min, params[kRoot].max);
    // only send value if updated -- and might not be taken into account if game is running
    if ((int)uiRoot != root) {
      // note: output parameters will be fired back
      setParameterValue(kRoot, (int)uiRoot);
    }

    // same for number of notes/keys
    // TODO: the max number of notes will change depending on root, not that great for this UI (could reset or increase upon changing root)
    float uiNbNotes = nbNotes;
    GuiSliderBar(layoutRecs[6], TextFormat("Number of keys: %d", (int)uiNbNotes), NULL, &uiNbNotes, params[kNbNotes].min, params[kNbNotes].max);
    if ((int)uiNbNotes != nbNotes) {
      setParameterValue(kNbNotes, (int)uiNbNotes);
    }

    // this label shoul be aligned as with widgets legends, revert style temporarily
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiLabel(layoutRecs[7], "Scale");
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    // first white keys
    GuiSetStyle(TOGGLE, BORDER_COLOR_NORMAL, (int)0x000000ff);
    GuiSetStyle(TOGGLE, BASE_COLOR_NORMAL, (int)0xffffffff);
    GuiSetStyle(TOGGLE, TEXT_COLOR_NORMAL, (int)0x000000ff);
    GuiSetStyle(TOGGLE, BORDER_COLOR_FOCUSED, (int)0x000000ff);
    GuiSetStyle(TOGGLE, BASE_COLOR_FOCUSED, (int)0x515151ff);
    GuiSetStyle(TOGGLE, TEXT_COLOR_FOCUSED, (int)0x000000ff);
    GuiSetStyle(TOGGLE, BORDER_COLOR_PRESSED, (int)0x000000ff);
    GuiSetStyle(TOGGLE, BASE_COLOR_PRESSED, (int)0xd9d9d9ff);
    GuiSetStyle(TOGGLE, TEXT_COLOR_PRESSED, (int)0x000000ff);
    GuiSetStyle(TOGGLE, BORDER_COLOR_DISABLED, (int)0xd9d9d9ff);
    GuiSetStyle(TOGGLE, BASE_COLOR_DISABLED, (int)0x515151ff);
    GuiSetStyle(TOGGLE, TEXT_COLOR_DISABLED, (int)0xd9d9d9ff);
    for (int i=0; i < 12; i++) {
      if(isKeyWhite(i)) {
	// toggles are inverted, we activate to disable note
	bool scaleToggle = !scale[i];
	GuiToggle(layoutRecs[8+i], scaleNotes[i], &scaleToggle);
	if (scaleToggle != !scale[i]) {
	  setParameterValue(kScaleC + i, !scale[i]);
	}
      }
    }

    // then black keys
    GuiSetStyle(TOGGLE, BORDER_COLOR_NORMAL, (int)0xffffffff);
    GuiSetStyle(TOGGLE, BASE_COLOR_NORMAL, (int)0x000000ff);
    GuiSetStyle(TOGGLE, TEXT_COLOR_NORMAL, (int)0xffffffff);
    GuiSetStyle(TOGGLE, BORDER_COLOR_FOCUSED, (int)0xffffffff);
    GuiSetStyle(TOGGLE, BASE_COLOR_FOCUSED, (int)0x515151ff);
    GuiSetStyle(TOGGLE, TEXT_COLOR_FOCUSED, (int)0xffffffff);
    GuiSetStyle(TOGGLE, BORDER_COLOR_PRESSED, (int)0xffffffff);
    GuiSetStyle(TOGGLE, BASE_COLOR_PRESSED, (int)0x323232ff);
    GuiSetStyle(TOGGLE, TEXT_COLOR_PRESSED, (int)0xffffffff);
    GuiSetStyle(TOGGLE, BORDER_COLOR_DISABLED, (int)0xd9d9d9ff);
    GuiSetStyle(TOGGLE, BASE_COLOR_DISABLED, (int)0x323232ff);
    GuiSetStyle(TOGGLE, TEXT_COLOR_DISABLED, (int)0xd9d9d9ff);
    for (int i=0; i < 12; i++) {
      if(!isKeyWhite(i)) {
	bool scaleToggle = !scale[i];
	GuiToggle(layoutRecs[8+i], scaleNotes[i], &scaleToggle);
	if (scaleToggle != !scale[i]) {
	  setParameterValue(kScaleC + i, !scale[i]);
	}
      }
    }

    // NOTE: no need to go back to default style for toggle as it is no used (yet?) elsewhere

    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); 

    
    // --- end disable part of the UI during game ---
    GuiSetState(STATE_NORMAL);

    // option about letting notes through on not
    bool uiShallNotPass = shallNotPass;
    GuiCheckBox(layoutRecs[20], "Shall not pass", &uiShallNotPass);
    if (uiShallNotPass != shallNotPass) {
      // note: we have to sync in ui non-output parameters changed from ui, won't be fired back
      shallNotPass = uiShallNotPass;
      setParameterValue(kShallNotPass, uiShallNotPass);
    }

    // sync rounds for miss
    float uiRoundsForMiss = roundsForMiss;
    GuiSliderBar(layoutRecs[21], TextFormat("Rounds for miss: %d", (int)uiRoundsForMiss ), NULL, &uiRoundsForMiss, params[kRoundsForMiss].min, params[kRoundsForMiss].max);
    if ((int)uiRoundsForMiss != roundsForMiss) {
      roundsForMiss = (int)uiRoundsForMiss;
      setParameterValue(kRoundsForMiss, (int)uiRoundsForMiss);
    }

    // render the 3D scene
    DrawTexturePro(
		   canvasPiano.texture,
		   // flip Y so we get the right texture
		   (Rectangle){ 0.0f, 0.0f , (float)canvasPiano.texture.width, -(float)canvasPiano.texture.height },
		   (Rectangle){layoutRecs[22].x, layoutRecs[22].y, layoutRecs[22].width, layoutRecs[22].height },
		   (Vector2){ 0, 0 }, 0.0f, WHITE);

    GuiLabel(layoutRecs[23], TextFormat("Missed %d/%d", nbMiss, maxMiss));
    GuiLabel(layoutRecs[24], TextFormat("Current best: %d", maxRound));

    DrawFPS(10, 10);
  }

    // -------------------------------------------------------------------------------------------------- --------------

private:
  // texture for piano keys
  Texture2D piano;
  // upper left reference point for UI
  static constexpr Vector2 anchor = { 15, 10 };
  // layout of the GUI
  const Rectangle layoutRecs[25] = {
    (Rectangle){ anchor.x + 200, anchor.y + 0, 568, 32 },
    (Rectangle){ anchor.x + 200, anchor.y + 40, 568, 32 },
    (Rectangle){ anchor.x + 200, anchor.y + 80, 120, 32 },
    (Rectangle){ anchor.x + 336, anchor.y + 80, 224, 32 },
    (Rectangle){ anchor.x + 576, anchor.y + 80, 192, 32 },
    (Rectangle){ anchor.x + 200, anchor.y + 120, 568, 32 },
    (Rectangle){ anchor.x + 200, anchor.y + 160, 568, 32 },
    (Rectangle){ anchor.x + 48, anchor.y + 200, 144, 32 },
    (Rectangle){ anchor.x + 200, anchor.y + 200, 40, 32 },
    (Rectangle){ anchor.x + 248, anchor.y + 200, 40, 32 },
    (Rectangle){ anchor.x + 296, anchor.y + 200, 40, 32 },
    (Rectangle){ anchor.x + 344, anchor.y + 200, 40, 32 },
    (Rectangle){ anchor.x + 392, anchor.y + 200, 40, 32 },
    (Rectangle){ anchor.x + 440, anchor.y + 200, 40, 32 },
    (Rectangle){ anchor.x + 488, anchor.y + 200, 40, 32 },
    (Rectangle){ anchor.x + 536, anchor.y + 200, 40, 32 },
    (Rectangle){ anchor.x + 584, anchor.y + 200, 40, 32 },
    (Rectangle){ anchor.x + 632, anchor.y + 200, 40, 32 },
    (Rectangle){ anchor.x + 680, anchor.y + 200, 40, 32 },
    (Rectangle){ anchor.x + 728, anchor.y + 200, 40, 32 },
    (Rectangle){ anchor.x + 200, anchor.y + 240, 32, 32 },
    (Rectangle){ anchor.x + 200, anchor.y + 280, 568, 32 },
    (Rectangle){ anchor.x + 0, anchor.y + 320, 768, 136 },
    (Rectangle){ anchor.x + 200, anchor.y + 464, 568, 32 },
    (Rectangle){ anchor.x + 200, anchor.y + 504, 568, 32 },
  };

  // used for 3D rendering
  Camera camera;
  // model and its position
  Model model;
  Vector3 position = { 0.0f, 0.0f, 0.0f };
  // Load gltf model animations
  int animsCount = -1;
  // array of animations contained in the model
  ModelAnimation *modelAnimations;
  // selected animation, < 0: disable animation
  int animIndex = -1;
  // duration in seconds for current anim
  float animDuration = 0;
  // how long in current anim we are
  float animCurrentTime = 0;
  // status for animation
  int animNote = params[kCurNote].def;
  int animStatus = params[kStatus].def;
  // render texture to mesh
  RenderTexture2D texturePiano;
  // render 3D scene to canvas
  RenderTexture2D canvasPiano;


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
  // also background, hacking slightly
  // flip: Y flip for sprite
  void drawKey(KeyIdx idx, Vector2 pos, Vector2 size, bool flip = false) {
    // fixed size for all keys in the sprite sheet
    static const Rectangle spriteSize = {0, 0, 16, 64};
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
    case BLACK_KEY_PLAY:
      spriteShift = 11;
      break;
    case WHITE_KEY_PLAY:
      spriteShift = 9;
      break;
    case BLACK_KEY_DEBUG:
      spriteShift = 3;
      break;
    case BACKGROUND_LEFT:
      spriteShift = 24;
      break;
    case BACKGROUND_RIGHT:
      spriteShift = 25;
      break;
    case BACKGROUND_FELT:
      spriteShift = 26;
      break;
    default:
    case WHITE_KEY_DEBUG:
      spriteShift = 1;
      break;
    }
    
    // flip sprite if set
    float spriteHeight = spriteSize.height;
    if (flip) {
      spriteHeight *= -1;
    }
    // no rotation, no tint
    DrawTexturePro(piano, {spriteSize.x + spriteShift * spriteSize.width, spriteSize.y, spriteSize.width, spriteHeight}, {pos.x, pos.y, size.x, size.y}, {0.0, 0.0}, 0, WHITE);
  }

  // drawing a very simple keyboard using imgui, fetching drawList
  // pos: upper left corner of the widget
  // size: size of the widget
  // flip: will flip sprites in Y-axis, e.g. used for textures and different origin for OpenGL
  void drawPiano(Vector2 pos, Vector2 size, uint rootKey, uint nbKeys, bool flip = false) {
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
    // around keyboard, between keys
    // keep ratio of sprite -- FIXME: actually depends on the actual displayed ratio
    Vector2 margins = {size.y * 0.25f, 0.0f};
    // black keys over whites, width of a key will be conditioned by the former
    Vector2 keySize;
    keySize.x = (size.x  - margins.x * 2) / uiNbWhiteKeys;
    keySize.y = size.y - margins.y * 2;

    // draw the background, sides
    drawKey(BACKGROUND_LEFT, {pos.x, pos.y+margins.y}, {margins.x, size.y - 2*margins.y}, flip);
    drawKey(BACKGROUND_RIGHT, {pos.x + size. x -margins.x, pos.y+margins.y}, {margins.x, size.y - 2*margins.y}, flip);
    // background
    drawKey(BACKGROUND_FELT, {pos.x + margins.x, pos.y + margins.y}, {size.x - 2 * margins.x, size.y - 2*margins.y}, flip);

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
	    sprite = WHITE_KEY_INCORRECT;
	    break;
	    // notes outside game or during feedback
	  default:
	    sprite = WHITE_KEY_PLAY;
	    break;
	  }
	}
	drawKey(sprite, curPos, keySize, flip);
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
	    sprite = BLACK_KEY_INCORRECT;
	    break;
	    // notes outside game or during feedback
	  default:
	    sprite = BLACK_KEY_PLAY;
	    break;
	  }
	}
	// for black key, shift half to right to center between two white keys
	drawKey(sprite, {curPos.x - keySize.x / 2, curPos.y}, keySize, flip);
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
 
