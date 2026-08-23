#include "MelkamEngine.hpp"

// 1. Controllable Player Character (CharacterBody2D with CollisionShape2D,
// Cute Smiley Sprite & Camera2D)
class PlayerNode : public CharacterBody2D {
public:
  float speed = 350.0f;
  float jumpVelocity = -750.0f;
  float gravity = 1400.0f;
  Ref<Camera2D> camera = nullptr;

  void onReady() override {
    // 1. Attach CollisionShape2D child using addChild
    auto colShape = addChild<CollisionShape2D>(Vector2(48.0f, 48.0f));

    // 2. Attach Cute Smiley Box Sprite2D
    auto sprite = addChild<Sprite2D>("assets/sprites/player_smiley.png");
    sprite->size = {48.0f, 48.0f};

    // 3. Attach Camera2D child and make it current to follow player
    camera = addChild<Camera2D>();
    camera->makeCurrent();
  }

  void onPhysicsProcess(float delta) override {
    // 1. Apply Gravity if in the air
    if (!isOnFloor()) {
      velocity.y += gravity * delta;
    } else {
      if (velocity.y > 0.0f) {
        velocity.y = 0.0f;
      }
    }

    // 2. Handle Jump (Space or W)
    if ((Input::isKeyJustPressed(Key::Space) ||
         Input::isKeyJustPressed(Key::W)) &&
        isOnFloor()) {
      jump();
    }

    // 3. Horizontal Movement (A / D)
    float horizontal = Input::getAxis(Key::A, Key::D);
    velocity.x = horizontal * speed;

    // 4. Move and Slide with Box2D physics & collision resolution
    moveAndSlide();

    // Fall reset if falling off screen
    if (getPosition().y > 900.0f) {
      setPosition({300.0f, 300.0f});
      velocity = {0.0f, 0.0f};
    }
  }

  void jump() {
    velocity.y = jumpVelocity;
    setOnFloor(false);
  }
};

// Global HUD references
static int g_coinsCollected = 0;
static Ref<Label> g_coinLabel = nullptr;
static Ref<ProgressBar> g_progressBar = nullptr;

// 2. Collectible Coin Trigger (Area2D with Godot body_entered Signal & Shiny
// Coin Sprite)
class CoinNode : public Area2D {
public:
  void onReady() override {
    // 1. Circle collision shape child
    auto colShape = addChild<CollisionShape2D>(18.0f);

    // 2. Attach Coin Sprite2D
    auto sprite = colShape->addChild<Sprite2D>("assets/sprites/coin.png");
    sprite->size = {36.0f, 36.0f};

    // 3. Connect to Godot-style Signal body_entered
    body_entered.connect([this](Node2D *body) {
      if (body && body->name == "Player") {
        g_coinsCollected++;
        std::cout << "=== [SIGNAL FIRED] Coin collected! Total: "
                  << g_coinsCollected << " ===" << std::endl;

        if (g_coinLabel) {
          g_coinLabel->text =
              "Coins Collected: " + std::to_string(g_coinsCollected) + " / 3";
        }
        if (g_progressBar) {
          g_progressBar->setValue(g_coinsCollected * (100.0f / 3.0f));
        }

        visible = false;    // Coin collected!
        monitoring = false; // Stop monitoring
      }
    });
  }

  void onProcess(float delta) override {
    if (visible) {
      rotate(3.0f * delta);
    }
  }
};

int main() {
  // 1. Create Application with Godot Stretch Mode (CanvasItems) & Aspect (Keep
  // / Letterbox)
  Application app("MelkamEngine - Godot 2D/3D Canvas UI & HUD Sandbox", 1280,
                  720, true, StretchMode::CanvasItems, StretchAspect::Keep);

  // 2. Ground Platform (StaticBody2D with Floating Grass Platform Sprite)
  auto ground = app.addChild<StaticBody2D>("Ground");
  ground->setPosition({640.0f, 680.0f});
  auto groundShape =
      ground->addChild<CollisionShape2D>(Vector2(2560.0f, 50.0f));
  auto groundSprite =
      groundShape->addChild<Sprite2D>("assets/sprites/grass_platform.png");
  groundSprite->size = {2560.0f, 50.0f};

  // Floating Grass Platforms
  auto plat1 = app.addChild<StaticBody2D>("Platform1");
  plat1->setPosition({350.0f, 500.0f});
  auto plat1Shape = plat1->addChild<CollisionShape2D>(Vector2(260.0f, 40.0f));
  auto plat1Sprite =
      plat1->addChild<Sprite2D>("assets/sprites/grass_platform.png");
  plat1Sprite->size = {260.0f, 40.0f};

  auto plat2 = app.addChild<StaticBody2D>("Platform2");
  plat2->setPosition({850.0f, 380.0f});
  auto plat2Shape = plat2->addChild<CollisionShape2D>(Vector2(260.0f, 40.0f));
  auto plat2Sprite =
      plat2->addChild<Sprite2D>("assets/sprites/grass_platform.png");
  plat2Sprite->size = {260.0f, 40.0f};

  auto plat3 = app.addChild<StaticBody2D>("Platform3");
  plat3->setPosition({1400.0f, 480.0f});
  auto plat3Shape = plat3->addChild<CollisionShape2D>(Vector2(260.0f, 40.0f));
  auto plat3Sprite =
      plat3->addChild<Sprite2D>("assets/sprites/grass_platform.png");
  plat3Sprite->size = {260.0f, 40.0f};

  // 3. Collectible Coins (Area2D with Signal connection)
  auto coin1 = app.addChild<CoinNode>();
  coin1->setPosition({350.0f, 440.0f});

  auto coin2 = app.addChild<CoinNode>();
  coin2->setPosition({850.0f, 320.0f});

  auto coin3 = app.addChild<CoinNode>();
  coin3->setPosition({1400.0f, 420.0f});

  // 4. Dynamic Crates (RigidBody2D with Godot defaults: friction 1.0, bounce
  // 0.0)
  auto crate1 = app.addChild<RigidBody2D>("Crate1");
  crate1->lockRotation = true;
  crate1->setPosition({400.0f, 150.0f});
  auto crate1Shape = crate1->addChild<CollisionShape2D>(Vector2(45.0f, 45.0f));
  crate1Shape->addChild<MeshInstance2D>(Vector2(45.0f, 45.0f),
                                        Color::from_rgba8(205, 133, 63));

  auto crate2 = app.addChild<RigidBody2D>("Crate2");
  crate2->lockRotation = true;
  crate2->setPosition({820.0f, 100.0f});
  auto crate2Shape = crate2->addChild<CollisionShape2D>(Vector2(40.0f, 40.0f));
  crate2Shape->addChild<MeshInstance2D>(Vector2(40.0f, 40.0f),
                                        Color::from_rgba8(220, 160, 80));

  // 5. Player CharacterBody2D
  auto player = app.addChild<PlayerNode>();
  player->name = "Player";
  player->setPosition({300.0f, 300.0f});

  // =========================================================================
  // 6. Canvas UI / HUD Layer (9-Slice UI Textures & Subtree Theme Demo)
  // =========================================================================

  // Load UI Texture Assets
  auto panelTex = makeRef<Texture2D>("assets/UI/panel_frame.png");
  auto coinTex = makeRef<Texture2D>("assets/UI/coin_icon.png");
  auto btnTex = makeRef<Texture2D>("assets/UI/button_frame.png");

  // A. Create a Custom Futuristic Cyan HUD Theme for the HUD Panel Subtree
  auto hudTheme = makeRef<Theme>();
  hudTheme->setColor("font_color", "Label", Color::from_rgba8(130, 230, 255));
  hudTheme->setFontSize("font_size", "Label", 16);

  hudTheme->setColor("fill_color", "ProgressBar",
                     Color::from_rgba8(0, 230, 160));
  hudTheme->setColor("bg_color", "ProgressBar", Color::from_rgba8(12, 22, 35));
  hudTheme->setColor("border_color", "ProgressBar",
                     Color::from_rgba8(35, 110, 175));
  hudTheme->setColor("font_color", "ProgressBar", Color::WHITE);
  hudTheme->setConstant("corner_radius", "ProgressBar", 4);

  // Top-Left HUD Card Panel (Uses 9-Slice Textured Panel Frame + Custom Theme)
  auto hudPanel = app.addChild<Panel>();
  hudPanel->setPosition({24.0f, 24.0f});
  hudPanel->setSize({320.0f, 136.0f});
  hudPanel->texture = panelTex;
  hudPanel->patchMarginLeft = 20.0f;
  hudPanel->patchMarginTop = 20.0f;
  hudPanel->patchMarginRight = 20.0f;
  hudPanel->patchMarginBottom = 20.0f;
  hudPanel->theme = hudTheme; // Subtree theme inheritance!

  auto titleLabel = hudPanel->addChild<Label>("MELKAM ENGINE HUD", 18.0f,
                                              Color::from_rgba8(255, 215, 0));
  titleLabel->setPosition({20.0f, 16.0f});

  // Coin Icon & Counter
  auto coinIcon = hudPanel->addChild<TextureRect>(coinTex);
  coinIcon->setPosition({20.0f, 46.0f});
  coinIcon->setSize({24.0f, 24.0f});

  g_coinLabel = hudPanel->addChild<Label>(
      "Coins Collected: 0 / 3"); // Inherits cyan text from hudTheme
  g_coinLabel->setPosition({52.0f, 48.0f});

  g_progressBar =
      hudPanel
          ->addChild<ProgressBar>(); // Inherits neon emerald fill & cyan border
  g_progressBar->setPosition({20.0f, 86.0f});
  g_progressBar->setSize({278.0f, 24.0f});
  g_progressBar->setValue(0.0f);

  // B. Top-Right Game Controls Panel (Uses 9-slice panel texture + global
  // theme)
  auto controlsPanel = app.addChild<Panel>();
  controlsPanel->setPosition({924.0f, 24.0f});
  controlsPanel->setSize({332.0f, 280.0f});
  controlsPanel->texture = panelTex;
  controlsPanel->patchMarginLeft = 20.0f;
  controlsPanel->patchMarginTop = 20.0f;
  controlsPanel->patchMarginRight = 20.0f;
  controlsPanel->patchMarginBottom = 20.0f;

  auto controlsTitle =
      controlsPanel->addChild<Label>("GAME CONTROLS", 18.0f, Color::GOLD);
  controlsTitle->setPosition({20.0f, 14.0f});

  // 1. Move Speed Slider (HSlider)
  auto speedLabel = controlsPanel->addChild<Label>("Speed: 350", 16.0f);
  speedLabel->setPosition({20.0f, 42.0f});

  auto speedSlider = controlsPanel->addChild<HSlider>();
  speedSlider->setPosition({20.0f, 68.0f});
  speedSlider->setSize({290.0f, 20.0f});
  speedSlider->setMinValue(150.0f);
  speedSlider->setMaxValue(800.0f);
  speedSlider->setValue(350.0f);
  speedSlider->value_changed.connect([player, speedLabel](float newSpeed) {
    if (player)
      player->speed = newSpeed;
    if (speedLabel)
      speedLabel->text = "Speed: " + std::to_string(static_cast<int>(newSpeed));
  });

  // 2. Jump Power Selector (OptionButton)
  auto jumpOption = controlsPanel->addChild<OptionButton>();
  jumpOption->setPosition({20.0f, 96.0f});
  jumpOption->setSize({290.0f, 32.0f});
  jumpOption->fontSize = 16.0f;
  jumpOption->addItem("Normal Jump (-750)");
  jumpOption->addItem("Super Jump (-950)");
  jumpOption->addItem("Mega Jump (-1150)");
  jumpOption->item_selected.connect([player](int idx) {
    if (!player)
      return;
    if (idx == 0)
      player->jumpVelocity = -750.0f;
    else if (idx == 1)
      player->jumpVelocity = -950.0f;
    else if (idx == 2)
      player->jumpVelocity = -1150.0f;
    std::cout << "=== [OPTION SELECTED] Jump Velocity updated to "
              << player->jumpVelocity << " ===" << std::endl;
  });

  // 3. Camera Zoom Slider (HSlider)
  auto zoomLabel = controlsPanel->addChild<Label>("Camera Zoom: 1.0x", 16.0f);
  zoomLabel->setPosition({20.0f, 136.0f});

  auto zoomSlider = controlsPanel->addChild<HSlider>();
  zoomSlider->setPosition({20.0f, 160.0f});
  zoomSlider->setSize({290.0f, 20.0f});
  zoomSlider->setMinValue(0.5f);
  zoomSlider->setMaxValue(2.0f);
  zoomSlider->setValue(1.0f);
  zoomSlider->value_changed.connect([player, zoomLabel](float newZoom) {
    if (player && player->camera)
      player->camera->setZoom(newZoom);
    if (zoomLabel) {
      char buf[32];
      snprintf(buf, sizeof(buf), "Camera Zoom: %.1fx", newZoom);
      zoomLabel->text = buf;
    }
  });

  // 4. Jump Push Button with Icon & Textured Skin (Button)
  auto jumpBtn = controlsPanel->addChild<Button>("Jump Action");
  jumpBtn->setPosition({20.0f, 192.0f});
  jumpBtn->setSize({290.0f, 40.0f});
  jumpBtn->fontSize = 17.0f;
  jumpBtn->icon = coinTex;
  jumpBtn->iconSize = {20.0f, 20.0f};
  jumpBtn->textureNormal = btnTex;
  jumpBtn->patchMarginLeft = 12.0f;
  jumpBtn->patchMarginTop = 12.0f;
  jumpBtn->patchMarginRight = 12.0f;
  jumpBtn->patchMarginBottom = 12.0f;

  jumpBtn->pressed.connect([player]() {
    std::cout << "=== [UI BUTTON CLICKED] Jump Action Triggered! ==="
              << std::endl;
    if (player && player->isOnFloor()) {
      player->jump();
    }
  });

  // 7. Run the Game
  app.run();
  return 0;
}