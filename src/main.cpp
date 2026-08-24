#include "MelkamEngine.hpp"
#include <iostream>
#include <memory>
#include <string>

// =============================================================================
// 1. SCENE DECLARATIONS
// =============================================================================

class MainMenuScene : public Control {
public:
  MainMenuScene() : Control("MainMenuScene") {}
  void onReady() override;
};

class GameScene : public Node {
public:
  GameScene() : Node("GameScene") {}
  void onReady() override;
};

class UISimulationScene : public Control {
public:
  UISimulationScene() : Control("UISimulationScene") {}
  void onReady() override;
};

class Showcase3DScene : public Node {
public:
  Showcase3DScene() : Node("Showcase3DScene") {}
  ~Showcase3DScene() override {
    PhysicsServer3D::get().clear();
    Input::setMouseMode(MouseMode::Visible);
  }
  void onReady() override;
  void onProcess(float delta) override;
  void onDestroy() override {
    PhysicsServer3D::get().clear();
    Input::setMouseMode(MouseMode::Visible);
  }

private:
  Ref<WorldEnvironment> m_environment = nullptr;
  Ref<CharacterBody3D> m_player = nullptr;
  Ref<MeshInstance3D> m_playerMesh = nullptr;
  Ref<SpringArm3D> m_springArm = nullptr;
  Ref<Camera3D> m_camera = nullptr;
  Ref<DirectionalLight3D> m_sunLight = nullptr;
  Ref<OmniLight3D> m_omniLight = nullptr;
  Ref<StaticBody3D> m_floorBody = nullptr;
  Ref<MeshInstance3D> m_floorMesh = nullptr;
  Ref<MeshInstance3D> m_sphere = nullptr;
  float m_orbitAngle = 0.0f;

  // Diagnostic HUD Labels (powered by Time & Physics subsystems)
  Ref<Label> m_fpsLabel = nullptr;
  Ref<Label> m_playerPosLabel = nullptr;
  Ref<Label> m_physicsStatusLabel = nullptr;
  Ref<Label> m_cameraInfoLabel = nullptr;
  Ref<Label> m_mouseModeLabel = nullptr;
  Ref<Label> m_coinCountLabel = nullptr;

  // 3D Glowing Coins
  struct Coin3D {
    Ref<MeshInstance3D> meshNode;
    Vector3 basePos;
    bool collected = false;
    float bobOffset = 0.0f;
  };
  std::vector<Coin3D> m_coins;
  int m_collectedCoinCount = 0;
  float m_coinAngle = 0.0f;
};

// =============================================================================
// 2. GAMEPLAY ENTITIES (Player, Coins, Physics)
// =============================================================================

static int g_coinsCollected = 0;
static Ref<Label> g_coinLabel = nullptr;
static Ref<ProgressBar> g_progressBar = nullptr;

class PlayerNode : public CharacterBody2D {
public:
  float speed = 350.0f;
  float jumpVelocity = -750.0f;
  float gravity = 1400.0f;
  Ref<Camera2D> camera = nullptr;
  Ref<PointLight2D> lanternLight = nullptr;
  Ref<RayCast2D> floorRay = nullptr;

  void onReady() override {
    // 1. Collision shape
    addChild<CollisionShape2D>(Vector2(48.0f, 48.0f));

    // 2. Player Sprite
    auto sprite = addChild<Sprite2D>("assets/sprites/player_smiley.png");
    sprite->size = {48.0f, 48.0f};

    // 3. Soft Atmospheric Lantern Light (PointLight2D)
    lanternLight = addChild<PointLight2D>(150.0f, Color::from_rgba8(255, 230, 160, 160), 0.45f);
    lanternLight->attenuation = 1.6f;

    // 4. Ground Probe Sensor (RayCast2D)
    floorRay = addChild<RayCast2D>(Vector2(0.0f, 50.0f));

    // 5. Camera2D Follow

    camera = addChild<Camera2D>();
    camera->makeCurrent();
  }


  void onPhysicsProcess(float delta) override {
    if (!isOnFloor()) {
      velocity.y += gravity * delta;
    } else if (velocity.y > 0.0f) {
      velocity.y = 0.0f;
    }

    if ((Input::isKeyJustPressed(Key::Space) || Input::isKeyJustPressed(Key::W)) && isOnFloor()) {
      jump();
    }

    float horizontal = Input::getAxis(Key::A, Key::D);
    velocity.x = horizontal * speed;

    moveAndSlide();

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

class CoinNode : public Area2D {
public:
  Task<void> playCollectAnimation() {
    monitoring = false;
    auto tween = getTree()->createTween();
    tween->tweenProperty<Vector2>([this]() { return transform.scale; },
                                 [this](Vector2 s) { transform.scale = s; },
                                 Vector2(1.6f, 1.6f), 0.18f);
    await tween;
    visible = false;
  }

  void onReady() override {
    auto colShape = addChild<CollisionShape2D>(18.0f);
    auto sprite = colShape->addChild<Sprite2D>("assets/sprites/coin.png");
    sprite->size = {36.0f, 36.0f};

    body_entered.connect([this](Node2D *body) {
      if (body && body->name == "Player" && monitoring) {
        g_coinsCollected++;
        std::cout << "=== [SIGNAL FIRED] Coin collected! Total: " << g_coinsCollected << " ===" << std::endl;

        if (g_coinLabel) {
          g_coinLabel->text = "Coins Collected: " + std::to_string(g_coinsCollected) + " / 3";
        }
        if (g_progressBar) {
          g_progressBar->setValue(g_coinsCollected * (100.0f / 3.0f));
        }

        startCoroutine(playCollectAnimation());
      }
    });
  }

  void onProcess(float delta) override {
    if (!m_initialized) {
      m_baseY = transform.position.y;
      m_initialized = true;
    }

    if (visible) {
      m_bobTime += delta * 3.5f;
      transform.position.y = m_baseY + std::sin(m_bobTime) * 8.0f;
      rotate(3.0f * delta);
    }
  }

private:
  float m_baseY = 0.0f;
  float m_bobTime = 0.0f;
  bool m_initialized = false;
};


// =============================================================================
// Coroutine sequence demonstrating sequential awaits without blocking the main engine loop
static Task<void> playMenuCoroutineSequence(Ref<Label> statusLabel) {
  statusLabel->text = "[Coroutine] Initializing C++20 await...";
  await WaitForSeconds(1.2f);
  statusLabel->text = "[Coroutine] Timer elapsed! Awaiting 2.0s SceneTreeTimer...";
  await SceneTree::getCurrent()->createTimer(2.0f);
  statusLabel->text = "[Coroutine] NextPhysicsTick sync...";
  await NextPhysicsTick{};
  statusLabel->text = "[Coroutine] C++20 'await' System Fully Active!";
}

// 3. MAIN MENU SCENE IMPLEMENTATION
// =============================================================================

void MainMenuScene::onReady() {
  Vector2 vp = Window::getViewportSize();
  setSize(vp);

  // Background Gradient / Backdrop
  auto bg = addChild<ColorRect>(Color::from_rgba8(16, 18, 26));
  bg->setSize(vp);

  // Center Container for Menu Card
  auto center = addChild<CenterContainer>();
  center->setSize(vp);

  // Main Menu Panel Card
  auto card = center->addChild<Panel>();
  card->customMinimumSize = {460.0f, 500.0f};
  card->backgroundColor = Color::from_rgba8(25, 29, 42, 245);
  card->borderColor = Color::from_rgba8(65, 80, 120);
  card->borderWidth = 1.5f;
  card->cornerRadius = 10.0f;

  // Card Layout Container
  auto vbox = card->addChild<VBoxContainer>(12.0f);
  vbox->setPosition({30.0f, 24.0f});
  vbox->setSize({400.0f, 450.0f});

  // Engine Title
  auto titleLabel = vbox->addChild<Label>("MELKAM ENGINE", 28.0f, Color::GOLD);
  titleLabel->horizontalAlignment = HorizontalAlignment::Center;

  auto subLabel = vbox->addChild<Label>("Godot-Inspired 2D/3D Game Engine in C++", 14.0f, Color::from_rgba8(140, 160, 200));
  subLabel->horizontalAlignment = HorizontalAlignment::Center;

  auto coroutineLabel = vbox->addChild<Label>("[Coroutine] Active", 12.0f, Color::from_rgba8(100, 230, 160));
  coroutineLabel->horizontalAlignment = HorizontalAlignment::Center;
  startCoroutine(playMenuCoroutineSequence(coroutineLabel));

  vbox->addChild<HSeparator>();

  // 1. Play Button
  auto playBtn = vbox->addChild<Button>(IconType::Play, "Play Platformer Game");
  playBtn->customMinimumSize = {400.0f, 42.0f};
  playBtn->fontSize = 16.0f;
  playBtn->pressed.connect([this]() {
    std::cout << "=== Switching to Game Scene ===" << std::endl;
    getTree()->changeScene(makeRef<GameScene>());
  });

  // 2. 3D Vulkan Engine Showcase Button
  auto showcase3DBtn = vbox->addChild<Button>(IconType::Play, "3D Vulkan Engine Showcase");
  showcase3DBtn->customMinimumSize = {400.0f, 42.0f};
  showcase3DBtn->fontSize = 16.0f;
  showcase3DBtn->pressed.connect([this]() {
    std::cout << "=== Switching to 3D Vulkan Showcase Scene ===" << std::endl;
    getTree()->changeScene(makeRef<Showcase3DScene>());
  });

  // 3. UI Simulation Button
  auto uiBtn = vbox->addChild<Button>(IconType::Gear, "UI Simulation & Showcase");
  uiBtn->customMinimumSize = {400.0f, 42.0f};
  uiBtn->fontSize = 16.0f;
  uiBtn->pressed.connect([this]() {
    std::cout << "=== Switching to UI Simulation Scene ===" << std::endl;
    getTree()->changeScene(makeRef<UISimulationScene>());
  });

  // 4. About Dialog & Button
  auto aboutDialog = addChild<AcceptDialog>(
      "MelkamEngine v1.0\n\n"
      "• Scene Tree architecture with Node, Node2D & Node3D hierarchy\n"
      "• 3D Engine Architecture: Camera3D, MeshInstance3D & Vulkan Pipeline\n"
      "• C++20 Coroutine & 'await' Async Scripting (Timers, Signals, Tweens)\n"
      "• Complete Godot-standard Canvas UI suite & StyleBoxes\n"
      "• Box2D 2D Physics Server & CharacterBody2D moveAndSlide\n"
      "• Flexible Signal / Slot reactive event system\n"
      "• Hardware-accelerated 2D Batching with SDL3",
      "About MelkamEngine");

  auto aboutBtn = vbox->addChild<Button>(IconType::Info, "About MelkamEngine");
  aboutBtn->customMinimumSize = {400.0f, 38.0f};
  aboutBtn->fontSize = 14.0f;
  aboutBtn->pressed.connect([aboutDialog]() {
    if (aboutDialog) aboutDialog->popupCentered({520.0f, 300.0f});
  });

  // 5. Quit Button & Confirmation
  auto quitDialog = addChild<ConfirmationDialog>("Are you sure you want to quit MelkamEngine?", "Confirm Quit");
  quitDialog->confirmed.connect([this]() {
    std::cout << "=== Quitting Application ===" << std::endl;
    getTree()->quit();
  });

  auto quitBtn = vbox->addChild<Button>(IconType::Close, "Quit");
  quitBtn->customMinimumSize = {400.0f, 38.0f};
  quitBtn->fontSize = 14.0f;
  quitBtn->pressed.connect([quitDialog]() {
    if (quitDialog) quitDialog->popupCentered({380.0f, 180.0f});
  });

  vbox->addChild<HSeparator>();

  // Link Button
  auto docLink = vbox->addChild<LinkButton>("Learn more on GitHub Documentation", "https://github.com/Alazar42/MelkamEngine");
  docLink->fontColor = Color::from_rgba8(80, 160, 255);
}

// =============================================================================
// 4. 3D VULKAN SHOWCASE SCENE IMPLEMENTATION
// =============================================================================

void Showcase3DScene::onReady() {
  std::cout << "=== [3D Third-Person Showcase] Scene Ready ===" << std::endl;

  // 1. World Environment (Sky & ACES Tonemapping & Bloom Glow)
  m_environment = addChild<WorldEnvironment>();
  m_environment->environment->backgroundMode = EnvironmentBGMode::Sky;
  m_environment->environment->ambientLightColor = Color::from_rgba8(70, 85, 120);
  m_environment->environment->ambientLightEnergy = 1.0f;
  m_environment->environment->tonemapMode = TonemapMode3D::ACES;
  m_environment->environment->glowEnabled = true;
  m_environment->environment->glowIntensity = 1.0f;
  m_environment->environment->glowThreshold = 0.65f;
  m_environment->environment->glowBloom = 0.85f;

  // 2. Directional Sun Light & Omni Point Light
  m_sunLight = addChild<DirectionalLight3D>();
  m_sunLight->lightColor = Color::from_rgba8(255, 245, 220);
  m_sunLight->lightEnergy = 1.3f;

  m_omniLight = addChild<OmniLight3D>();
  m_omniLight->setPosition(Vector3(0.0f, 4.0f, 0.0f));
  m_omniLight->lightColor = Color::CYAN;
  m_omniLight->omniRange = 25.0f;

  // Golden ambient glow light for coins
  auto coinGlowLight = addChild<OmniLight3D>();
  coinGlowLight->setPosition(Vector3(0.0f, 1.8f, 0.0f));
  coinGlowLight->lightColor = Color::from_rgba8(255, 215, 70);
  coinGlowLight->lightEnergy = 1.4f;
  coinGlowLight->omniRange = 14.0f;

  // 3. Static Physics Ground Floor (StaticBody3D + CollisionShape3D + PlaneMesh)
  m_floorBody = addChild<StaticBody3D>();
  m_floorBody->setPosition(Vector3(0.0f, -0.1f, 0.0f));

  auto floorCol = m_floorBody->addChild<CollisionShape3D>(std::make_shared<BoxShape3D>(Vector3(18.0f, 0.2f, 18.0f)));
  (void)floorCol;

  auto floorMat = StandardMaterial3D::create(Color::from_rgba8(35, 42, 60));
  floorMat->cullMode = CullMode3D::Disabled;

  m_floorMesh = m_floorBody->addChild<MeshInstance3D>(PlaneMesh::create(Vector2(18.0f, 18.0f)));
  m_floorMesh->setMaterial(floorMat);

  // 4. Third-Person CharacterBody3D (Player)
  m_player = addChild<CharacterBody3D>();
  m_player->setPosition(Vector3(0.0f, 2.0f, 0.0f));

  auto playerCol = m_player->addChild<CollisionShape3D>(std::make_shared<BoxShape3D>(Vector3(0.8f, 1.8f, 0.8f)));
  (void)playerCol;

  auto heroMat = StandardMaterial3D::create(Color::from_rgba8(40, 140, 255));
  heroMat->metallic = 0.5f;
  heroMat->roughness = 0.3f;

  m_playerMesh = m_player->addChild<MeshInstance3D>(BoxMesh::create(Vector3(0.8f, 1.8f, 0.8f)));
  m_playerMesh->setMaterial(heroMat);

  // 5. SpringArm3D Attached to Player at Head/Shoulder Height
  m_springArm = m_player->addChild<SpringArm3D>();
  m_springArm->setPosition(Vector3(0.0f, 1.4f, 0.0f));
  m_springArm->springLength = 6.0f;
  m_springArm->pitch = -0.3f;
  m_springArm->updateRotation();

  // 6. Camera3D Attached to the end of SpringArm3D Boom
  m_camera = m_springArm->addChild<Camera3D>();
  m_camera->fov = 70.0f;
  m_camera->makeCurrent();

  // 7. Step-Up Obstacle Crate (StaticBody3D + CollisionShape3D)
  auto crateMat = StandardMaterial3D::create(Color::from_rgba8(175, 130, 75));
  crateMat->metallic = 0.0f;
  crateMat->roughness = 0.85f;

  auto crate = addChild<StaticBody3D>();
  crate->setPosition(Vector3(3.0f, 0.6f, -2.0f));
  crate->addChild<CollisionShape3D>(std::make_shared<BoxShape3D>(Vector3(1.8f, 1.2f, 1.8f)));
  auto crateMesh = crate->addChild<MeshInstance3D>(BoxMesh::create(Vector3(1.8f, 1.2f, 1.8f)));
  crateMesh->setMaterial(crateMat);

  // 8. Orbiting Decorative UV Sphere
  auto aquaMat = StandardMaterial3D::create(Color::from_rgba8(80, 200, 255));
  aquaMat->metallic = 0.4f;
  aquaMat->roughness = 0.1f;

  m_sphere = addChild<MeshInstance3D>(SphereMesh::create(0.5f));
  m_sphere->setPosition(Vector3(-3.0f, 1.5f, -2.0f));
  m_sphere->setMaterial(aquaMat);

  // 8.5. 3D Glowing Golden Collectible Coins
  auto goldCoinMat = StandardMaterial3D::create(Color::from_rgba8(255, 215, 0));
  goldCoinMat->roughness = 0.15f;
  goldCoinMat->metallic = 0.95f;
  goldCoinMat->emissionColor = Color::from_rgba8(255, 200, 30);
  goldCoinMat->emissionEnergy = 1.4f;
  goldCoinMat->cullMode = CullMode3D::Back;

  std::vector<Vector3> coinPositions = {
      Vector3(0.0f, 1.0f, -4.5f),
      Vector3(-4.5f, 1.0f, 0.0f),
      Vector3(4.5f, 1.0f, 0.0f),
      Vector3(0.0f, 1.0f, 4.5f),
      Vector3(3.0f, 1.8f, -2.0f), // on top of step crate
      Vector3(-3.0f, 1.0f, 3.0f),
  };

  m_coins.clear();
  m_collectedCoinCount = 0;
  for (size_t i = 0; i < coinPositions.size(); ++i) {
    auto coinMesh = addChild<MeshInstance3D>(CylinderMesh::create(0.35f, 0.08f));
    coinMesh->setPosition(coinPositions[i]);
    coinMesh->setMaterial(goldCoinMat);
    m_coins.push_back({coinMesh, coinPositions[i], false, static_cast<float>(i) * 1.05f});
  }

  // 9. UI HUD Overlay
  Vector2 vp = Window::getViewportSize();
  auto overlay = addChild<Control>("UIOverlay");
  overlay->setSize(vp);

  // 10. Default Mouse Capture (Godot-Style Locked Mode)
  Input::setMouseMode(MouseMode::Captured);

  // Left Panel: Controls & Navigation
  auto panel = overlay->addChild<Panel>();
  panel->setPosition({24.0f, 24.0f});
  panel->setSize({380.0f, 260.0f});
  panel->backgroundColor = Color::from_rgba8(18, 22, 34, 235);
  panel->borderColor = Color::from_rgba8(70, 90, 140);
  panel->borderWidth = 1.5f;
  panel->cornerRadius = 8.0f;

  auto vbox = panel->addChild<VBoxContainer>(5.0f);
  vbox->setPosition({14.0f, 12.0f});
  vbox->setSize({352.0f, 236.0f});

  vbox->addChild<Label>("3D THIRD-PERSON CONTROLLER", 15.0f, Color::GOLD);
  vbox->addChild<Label>("• Walk: W/A/S/D or Arrow Keys", 11.5f, Color::from_rgba8(100, 230, 160));
  vbox->addChild<Label>("• Jump: SPACE (Bullet 3 Kinematic Step)", 11.5f, Color::from_rgba8(120, 220, 255));
  vbox->addChild<Label>("• Look Around: Mouse (Locked by Default)", 11.5f, Color::from_rgba8(255, 215, 80));
  vbox->addChild<Label>("• Unlock / Lock Mouse: ESC Key or Click", 11.5f, Color::from_rgba8(255, 140, 100));
  vbox->addChild<Label>("• Collect: 3D Glowing Gold Coins", 11.5f, Color::from_rgba8(255, 230, 80));
  vbox->addChild<Label>("• Zoom Boom: Mouse Scroll Wheel", 11.5f, Color::from_rgba8(255, 215, 80));
  vbox->addChild<Label>("• Nodes: CharacterBody3D + SpringArm3D", 11.5f, Color::from_rgba8(170, 190, 230));

  vbox->addChild<HSeparator>();

  auto backBtn = vbox->addChild<Button>(IconType::ChevronLeft, "Return to Main Menu");
  backBtn->customMinimumSize = {352.0f, 28.0f};
  backBtn->fontSize = 12.0f;
  backBtn->pressed.connect([this]() {
    std::cout << "=== Returning to Main Menu ===" << std::endl;
    Input::setMouseMode(MouseMode::Visible);
    getTree()->changeScene(makeRef<MainMenuScene>());
  });

  // Right Panel: Live Engine, Time & Physics Diagnostics
  auto diagPanel = overlay->addChild<Panel>();
  diagPanel->setPosition({std::max(24.0f, vp.x - 450.0f), 24.0f});
  diagPanel->setSize({426.0f, 265.0f});
  diagPanel->backgroundColor = Color::from_rgba8(18, 22, 34, 235);
  diagPanel->borderColor = Color::from_rgba8(70, 90, 140);
  diagPanel->borderWidth = 1.5f;
  diagPanel->cornerRadius = 8.0f;

  auto diagVbox = diagPanel->addChild<VBoxContainer>(5.0f);
  diagVbox->setPosition({14.0f, 12.0f});
  diagVbox->setSize({398.0f, 241.0f});

  diagVbox->addChild<Label>("REAL-TIME ENGINE DIAGNOSTICS", 15.0f, Color::CYAN);
  m_fpsLabel = diagVbox->addChild<Label>("Performance: Calculating...", 12.0f, Color::from_rgba8(80, 240, 140));
  m_physicsStatusLabel = diagVbox->addChild<Label>("Bullet 3 Physics: [INITIALIZING]", 11.5f, Color::from_rgba8(120, 220, 255));
  m_playerPosLabel = diagVbox->addChild<Label>("Player: Pos (0.0, 0.0, 0.0) | Vel (0.0, 0.0, 0.0)", 11.5f, Color::from_rgba8(220, 230, 245));
  m_cameraInfoLabel = diagVbox->addChild<Label>("Camera Boom: 6.0m | Pitch: -17° | Yaw: 0°", 11.5f, Color::from_rgba8(255, 215, 80));
  m_mouseModeLabel = diagVbox->addChild<Label>("Mouse Mode: [LOCKED / CAPTURED]", 11.5f, Color::from_rgba8(80, 240, 140));
  m_coinCountLabel = diagVbox->addChild<Label>("Golden Coins: 0 / 6 Collected", 12.0f, Color::from_rgba8(255, 220, 50));
  
  diagVbox->addChild<HSeparator>();
  diagVbox->addChild<Label>("Renderer: 3D Vulkan PBR Software Rasterizer (60+ FPS)", 11.0f, Color::from_rgba8(150, 170, 200));
}

void Showcase3DScene::onProcess(float delta) {
  // 0. Mouse Capture Toggle via ESC
  if (Input::isKeyJustPressed(Key::Escape)) {
    if (Input::isMouseCaptured()) {
      Input::setMouseMode(MouseMode::Visible);
    } else {
      Input::setMouseMode(MouseMode::Captured);
    }
  }

  // 1. Third-Person Player Kinematic Movement with SpringArm3D Heading
  if (m_player) {
    Vector3 inputDir(0.0f, 0.0f, 0.0f);
    if (Input::isKeyPressed(Key::W) || Input::isKeyPressed(Key::Up)) inputDir.z -= 1.0f;
    if (Input::isKeyPressed(Key::S) || Input::isKeyPressed(Key::Down)) inputDir.z += 1.0f;
    if (Input::isKeyPressed(Key::A) || Input::isKeyPressed(Key::Left)) inputDir.x -= 1.0f;
    if (Input::isKeyPressed(Key::D) || Input::isKeyPressed(Key::Right)) inputDir.x += 1.0f;

    float yaw = m_springArm ? m_springArm->yaw : 0.0f;
    Vector3 forward(-std::sin(yaw), 0.0f, -std::cos(yaw));
    Vector3 right(std::cos(yaw), 0.0f, -std::sin(yaw));
    Vector3 moveVec = (forward * (-inputDir.z) + right * inputDir.x);

    float speed = 7.0f;
    if (moveVec.length_squared() > 0.001f) {
      moveVec = moveVec.normalized();
      m_player->velocity.x = moveVec.x * speed;
      m_player->velocity.z = moveVec.z * speed;

      float targetAngle = std::atan2(moveVec.x, moveVec.z);
      if (m_playerMesh) {
        m_playerMesh->setRotation(Vector3(0.0f, targetAngle, 0.0f));
      }
    } else {
      m_player->velocity.x = 0.0f;
      m_player->velocity.z = 0.0f;
    }

    // Apply gravity only in air
    if (!m_player->isOnFloor()) {
      m_player->velocity.y -= 24.0f * delta;
    } else {
      m_player->velocity.y = -0.1f;
    }

    // Jump
    if (Input::isKeyPressed(Key::Space) && m_player->isOnFloor()) {
      m_player->velocity.y = 9.0f;
    }

    m_player->move_and_slide(delta);

    // If fallen off platform into the void, respawn
    if (m_player->getPosition().y < -30.0f) {
      m_player->setPosition(Vector3(0.0f, 3.0f, 0.0f));
      m_player->velocity = Vector3(0.0f, 0.0f, 0.0f);
    }
  }

  // 2. Real-Time HUD Metrics Update (using Time subsystem)
  if (m_fpsLabel) {
    float fps = Time::getFPS();
    float dtMs = Time::getDeltaTimeMs();
    uint64_t frameCount = Time::getFrameCount();
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Performance: %.1f FPS  (%.2f ms) | Frame #%llu", fps, dtMs, static_cast<unsigned long long>(frameCount));
    m_fpsLabel->setText(buf);
    if (fps >= 50.0f) m_fpsLabel->fontColor = Color::from_rgba8(80, 240, 140);
    else if (fps >= 30.0f) m_fpsLabel->fontColor = Color::from_rgba8(255, 215, 80);
    else m_fpsLabel->fontColor = Color::from_rgba8(255, 90, 90);
  }

  if (m_physicsStatusLabel && m_player) {
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Bullet 3 Physics: %s %s",
                  m_player->isOnFloor() ? "[GROUNDED]" : "[AIRBORNE / FALLING]",
                  m_player->isOnWall() ? "[WALL CONTACT]" : "");
    m_physicsStatusLabel->setText(buf);
    m_physicsStatusLabel->fontColor = m_player->isOnFloor() ? Color::from_rgba8(120, 220, 255) : Color::from_rgba8(255, 140, 80);
  }

  if (m_playerPosLabel && m_player) {
    Vector3 pos = m_player->getPosition();
    Vector3 vel = m_player->velocity;
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Player: Pos (%.2f, %.2f, %.2f) | Vel (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z, vel.x, vel.y, vel.z);
    m_playerPosLabel->setText(buf);
  }

  if (m_cameraInfoLabel && m_springArm) {
    float pitchDeg = m_springArm->pitch * 180.0f / 3.14159265f;
    float yawDeg = m_springArm->yaw * 180.0f / 3.14159265f;
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Camera Boom: %.2f m / %.1f m | Pitch: %.1f° | Yaw: %.1f°",
                  m_springArm->getHitLength(), m_springArm->springLength, pitchDeg, yawDeg);
    m_cameraInfoLabel->setText(buf);
  }

  if (m_mouseModeLabel) {
    if (Input::isMouseCaptured()) {
      m_mouseModeLabel->setText("Mouse Mode: [LOCKED / CAPTURED] (ESC to Unlock)");
      m_mouseModeLabel->fontColor = Color::from_rgba8(80, 240, 140);
    } else {
      m_mouseModeLabel->setText("Mouse Mode: [VISIBLE / UNLOCKED] (Click to Lock)");
      m_mouseModeLabel->fontColor = Color::from_rgba8(255, 215, 80);
    }
  }

  // 3. Update 3D Glowing Golden Coins & Collection
  m_coinAngle += delta * 3.5f;
  for (size_t i = 0; i < m_coins.size(); ++i) {
    auto &coin = m_coins[i];
    if (coin.collected || !coin.meshNode) continue;

    float yBob = std::sin(m_coinAngle + coin.bobOffset) * 0.15f;
    coin.meshNode->setPosition(Vector3(coin.basePos.x, coin.basePos.y + yBob, coin.basePos.z));
    coin.meshNode->setRotation(Vector3(0.2f, m_coinAngle + coin.bobOffset, 0.0f));

    if (m_player) {
      Vector3 ppos = m_player->getPosition();
      Vector3 diff = ppos - coin.basePos;
      if (std::sqrt(diff.x * diff.x + diff.z * diff.z) < 1.3f && std::abs(diff.y) < 1.5f) {
        coin.collected = true;
        coin.meshNode->setVisible(false);
        m_collectedCoinCount++;
        std::cout << "=== [3D SHOWCASE] Golden Coin Collected! (" << m_collectedCoinCount << " / " << m_coins.size() << ") ===" << std::endl;
      }
    }
  }

  if (m_coinCountLabel) {
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Golden Coins: %d / %zu %s",
                  m_collectedCoinCount, m_coins.size(),
                  (m_collectedCoinCount == static_cast<int>(m_coins.size())) ? "★ ALL COLLECTED!" : "Collected");
    m_coinCountLabel->setText(buf);
    if (m_collectedCoinCount == static_cast<int>(m_coins.size())) {
      m_coinCountLabel->fontColor = Color::from_rgba8(100, 255, 140);
    }
  }

  // 3. Decorative rotating sphere
  if (m_sphere) {
    m_orbitAngle += delta * 1.5f;
    float radius = 3.0f;
    m_sphere->setPosition(Vector3(
        -3.0f + std::cos(m_orbitAngle) * radius,
        1.5f + std::sin(m_orbitAngle * 2.0f) * 0.5f,
        -2.0f + std::sin(m_orbitAngle) * radius));
    m_sphere->rotateY(2.0f * delta);
  }
}

// =============================================================================
// 4. GAMEPLAY SCENE IMPLEMENTATION
// =============================================================================

void GameScene::onReady() {
  g_coinsCollected = 0;

  // Load Textures
  auto panelTex = makeRef<Texture2D>("assets/UI/panel_frame.png");
  auto coinTex = makeRef<Texture2D>("assets/UI/coin_icon.png");
  auto btnTex = makeRef<Texture2D>("assets/UI/button_frame.png");

  // Ground Platform
  auto ground = addChild<StaticBody2D>("Ground");
  ground->setPosition({640.0f, 680.0f});
  auto groundShape = ground->addChild<CollisionShape2D>(Vector2(2560.0f, 50.0f));
  auto groundSprite = groundShape->addChild<Sprite2D>("assets/sprites/grass_platform.png");
  groundSprite->size = {2560.0f, 50.0f};

  // Floating Platforms
  auto plat1 = addChild<StaticBody2D>("Platform1");
  plat1->setPosition({350.0f, 500.0f});
  auto plat1Shape = plat1->addChild<CollisionShape2D>(Vector2(260.0f, 40.0f));
  auto plat1Sprite = plat1Shape->addChild<Sprite2D>("assets/sprites/grass_platform.png");
  plat1Sprite->size = {260.0f, 40.0f};

  auto plat2 = addChild<StaticBody2D>("Platform2");
  plat2->setPosition({850.0f, 380.0f});
  auto plat2Shape = plat2->addChild<CollisionShape2D>(Vector2(260.0f, 40.0f));
  auto plat2Sprite = plat2Shape->addChild<Sprite2D>("assets/sprites/grass_platform.png");
  plat2Sprite->size = {260.0f, 40.0f};

  auto plat3 = addChild<StaticBody2D>("Platform3");
  plat3->setPosition({1400.0f, 480.0f});
  auto plat3Shape = plat3->addChild<CollisionShape2D>(Vector2(260.0f, 40.0f));
  auto plat3Sprite = plat3Shape->addChild<Sprite2D>("assets/sprites/grass_platform.png");
  plat3Sprite->size = {260.0f, 40.0f};

  // 2D Spline Path with Moving Platform (Path2D + PathFollow2D)
  auto movingPath = addChild<Path2D>();
  movingPath->curve->addPoint({500.0f, 260.0f}, {0.0f, 0.0f}, {80.0f, -40.0f});
  movingPath->curve->addPoint({750.0f, 210.0f}, {-60.0f, 30.0f}, {60.0f, 30.0f});
  movingPath->curve->addPoint({1000.0f, 260.0f}, {-80.0f, -40.0f}, {0.0f, 0.0f});

  auto moverFollow = movingPath->addChild<PathFollow2D>(120.0f, true); // Moves back and forth along curve (ping-pong)
  moverFollow->rotates = false;
  moverFollow->pingPong = true;

  auto moverPlat = moverFollow->addChild<StaticBody2D>("MovingPlatform");
  auto moverShape = moverPlat->addChild<CollisionShape2D>(Vector2(200.0f, 36.0f));
  auto moverSprite = moverShape->addChild<Sprite2D>("assets/sprites/grass_platform.png");
  moverSprite->size = {200.0f, 36.0f};


  // Spawn Point Marker (Marker2D)
  auto spawnMarker = addChild<Marker2D>();
  spawnMarker->setPosition({300.0f, 300.0f});




  // Collectible Coins with PointLight2D & Particle Sparkles
  auto coin1 = addChild<CoinNode>();
  coin1->setPosition({350.0f, 440.0f});
  auto coinLight1 = coin1->addChild<PointLight2D>(90.0f, Color::from_rgba8(255, 215, 0, 140), 0.45f);
  auto sparkles1 = coin1->addChild<CPUParticles2D>(12);
  sparkles1->color = Color::GOLD;
  sparkles1->scaleMin = 2.0f;
  sparkles1->scaleMax = 5.0f;
  sparkles1->spreadDegrees = 360.0f;
  sparkles1->gravity = {0.0f, -30.0f};

  auto coin2 = addChild<CoinNode>();
  coin2->setPosition({850.0f, 320.0f});
  auto coinLight2 = coin2->addChild<PointLight2D>(90.0f, Color::from_rgba8(255, 215, 0, 140), 0.45f);
  auto sparkles2 = coin2->addChild<CPUParticles2D>(12);
  sparkles2->color = Color::GOLD;
  sparkles2->scaleMin = 2.0f;
  sparkles2->scaleMax = 5.0f;
  sparkles2->spreadDegrees = 360.0f;
  sparkles2->gravity = {0.0f, -30.0f};

  auto coin3 = addChild<CoinNode>();
  coin3->setPosition({1400.0f, 420.0f});
  auto coinLight3 = coin3->addChild<PointLight2D>(90.0f, Color::from_rgba8(255, 215, 0, 140), 0.45f);
  auto sparkles3 = coin3->addChild<CPUParticles2D>(12);
  sparkles3->color = Color::GOLD;
  sparkles3->scaleMin = 2.0f;
  sparkles3->scaleMax = 5.0f;
  sparkles3->spreadDegrees = 360.0f;
  sparkles3->gravity = {0.0f, -30.0f};

  // Dynamic Crates
  auto crate1 = addChild<RigidBody2D>("Crate1");
  crate1->lockRotation = true;
  crate1->setPosition({400.0f, 150.0f});
  auto crate1Shape = crate1->addChild<CollisionShape2D>(Vector2(45.0f, 45.0f));
  crate1Shape->addChild<MeshInstance2D>(Vector2(45.0f, 45.0f), Color::from_rgba8(205, 133, 63));

  auto crate2 = addChild<RigidBody2D>("Crate2");
  crate2->lockRotation = true;
  crate2->setPosition({820.0f, 100.0f});
  auto crate2Shape = crate2->addChild<CollisionShape2D>(Vector2(40.0f, 40.0f));
  crate2Shape->addChild<MeshInstance2D>(Vector2(40.0f, 40.0f), Color::from_rgba8(220, 160, 80));

  // Player (Lantern PointLight2D, RayCast2D, and Camera2D created in PlayerNode::onReady)
  auto player = addChild<PlayerNode>();
  player->name = "Player";
  player->setPosition({300.0f, 300.0f});



  // HUD Theme
  auto hudTheme = makeRef<Theme>();
  hudTheme->setColor("font_color", "Label", Color::from_rgba8(130, 230, 255));
  hudTheme->setFontSize("font_size", "Label", 16);
  hudTheme->setColor("fill_color", "ProgressBar", Color::from_rgba8(0, 230, 160));
  hudTheme->setColor("bg_color", "ProgressBar", Color::from_rgba8(12, 22, 35));
  hudTheme->setColor("border_color", "ProgressBar", Color::from_rgba8(35, 110, 175));

  // Top Navigation / Back Button
  auto backBtn = addChild<Button>(IconType::ArrowBack, "Back to Menu");
  backBtn->setPosition({24.0f, 16.0f});
  backBtn->setSize({160.0f, 34.0f});
  backBtn->fontSize = 15.0f;
  backBtn->pressed.connect([this]() {
    getTree()->changeScene(makeRef<MainMenuScene>());
  });

  // HUD Card Panel
  auto hudPanel = addChild<Panel>();
  hudPanel->setPosition({24.0f, 60.0f});
  hudPanel->setSize({320.0f, 136.0f});
  hudPanel->texture = panelTex;
  hudPanel->patchMarginLeft = 20.0f;
  hudPanel->patchMarginTop = 20.0f;
  hudPanel->patchMarginRight = 20.0f;
  hudPanel->patchMarginBottom = 20.0f;
  hudPanel->theme = hudTheme;

  auto titleLabel = hudPanel->addChild<Label>("MELKAM ENGINE HUD", 18.0f, Color::from_rgba8(255, 215, 0));
  titleLabel->setPosition({20.0f, 16.0f});

  auto coinIcon = hudPanel->addChild<TextureRect>(coinTex);
  coinIcon->setPosition({20.0f, 46.0f});
  coinIcon->setSize({24.0f, 24.0f});

  g_coinLabel = hudPanel->addChild<Label>("Coins Collected: 0 / 3");
  g_coinLabel->setPosition({52.0f, 48.0f});

  g_progressBar = hudPanel->addChild<ProgressBar>();
  g_progressBar->setPosition({20.0f, 86.0f});
  g_progressBar->setSize({278.0f, 24.0f});
  g_progressBar->setValue(0.0f);

  // Right Controls Panel
  auto controlsPanel = addChild<Panel>();
  controlsPanel->setPosition({924.0f, 16.0f});
  controlsPanel->setSize({332.0f, 280.0f});
  controlsPanel->texture = panelTex;
  controlsPanel->patchMarginLeft = 20.0f;
  controlsPanel->patchMarginTop = 20.0f;
  controlsPanel->patchMarginRight = 20.0f;
  controlsPanel->patchMarginBottom = 20.0f;

  auto controlsTitle = controlsPanel->addChild<Label>("GAME CONTROLS", 18.0f, Color::GOLD);
  controlsTitle->setPosition({20.0f, 14.0f});

  auto speedLabel = controlsPanel->addChild<Label>("Speed: 350", 16.0f);
  speedLabel->setPosition({20.0f, 42.0f});

  auto speedSlider = controlsPanel->addChild<HSlider>();
  speedSlider->setPosition({20.0f, 68.0f});
  speedSlider->setSize({290.0f, 20.0f});
  speedSlider->setMinValue(150.0f);
  speedSlider->setMaxValue(800.0f);
  speedSlider->setValue(350.0f);
  speedSlider->value_changed.connect([player, speedLabel](float newSpeed) {
    if (player) player->speed = newSpeed;
    if (speedLabel) speedLabel->text = "Speed: " + std::to_string(static_cast<int>(newSpeed));
  });

  auto jumpOption = controlsPanel->addChild<OptionButton>();
  jumpOption->setPosition({20.0f, 96.0f});
  jumpOption->setSize({290.0f, 32.0f});
  jumpOption->fontSize = 16.0f;
  jumpOption->addItem("Normal Jump (-750)");
  jumpOption->addItem("Super Jump (-950)");
  jumpOption->addItem("Mega Jump (-1150)");
  jumpOption->item_selected.connect([player](int idx) {
    if (!player) return;
    if (idx == 0) player->jumpVelocity = -750.0f;
    else if (idx == 1) player->jumpVelocity = -950.0f;
    else if (idx == 2) player->jumpVelocity = -1150.0f;
  });

  auto zoomLabel = controlsPanel->addChild<Label>("Camera Zoom: 1.0x", 16.0f);
  zoomLabel->setPosition({20.0f, 136.0f});

  auto zoomSlider = controlsPanel->addChild<HSlider>();
  zoomSlider->setPosition({20.0f, 160.0f});
  zoomSlider->setSize({290.0f, 20.0f});
  zoomSlider->setMinValue(0.5f);
  zoomSlider->setMaxValue(2.0f);
  zoomSlider->setValue(1.0f);
  zoomSlider->value_changed.connect([player, zoomLabel](float newZoom) {
    if (player && player->camera) player->camera->setZoom(newZoom);
    if (zoomLabel) {
      char buf[32];
      snprintf(buf, sizeof(buf), "Camera Zoom: %.1fx", newZoom);
      zoomLabel->text = buf;
    }
  });

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
    if (player && player->isOnFloor()) {
      player->jump();
    }
  });
}



// =============================================================================
// 5. UI SIMULATION & COMPREHENSIVE SHOWCASE IMPLEMENTATION
// =============================================================================

void UISimulationScene::onReady() {
  Vector2 vp = Window::getViewportSize();
  setSize(vp);

  // Background
  auto bg = addChild<ColorRect>(Color::from_rgba8(18, 20, 30));
  bg->setSize(vp);

  // Top Navigation Bar
  auto topNav = addChild<Panel>();
  topNav->setPosition({0.0f, 0.0f});
  topNav->setSize({vp.x, 48.0f});
  topNav->backgroundColor = Color::from_rgba8(28, 32, 48);
  topNav->borderColor = Color::from_rgba8(60, 70, 95);
  topNav->borderWidth = 1.0f;

  auto backBtn = topNav->addChild<Button>(IconType::ArrowBack, "Back to Main Menu");
  backBtn->setPosition({16.0f, 8.0f});
  backBtn->setSize({180.0f, 32.0f});
  backBtn->fontSize = 15.0f;
  backBtn->pressed.connect([this]() {
    getTree()->changeScene(makeRef<MainMenuScene>());
  });

  auto navTitle = topNav->addChild<Label>("CANVAS UI SIMULATION & COMPREHENSIVE SHOWCASE", 17.0f, Color::GOLD);
  navTitle->setPosition({210.0f, 14.0f});

  // Tab Container for all UI categories
  auto tabs = addChild<TabContainer>();
  tabs->setPosition({16.0f, 60.0f});
  tabs->setSize({vp.x - 32.0f, vp.y - 76.0f});

  // ---------------------------------------------------------------------------
  // Tab 1: Buttons & Inputs
  // ---------------------------------------------------------------------------
  auto tab1 = tabs->addTab("Buttons & Inputs");
  auto vbox1 = tab1->addChild<VBoxContainer>(12.0f);
  vbox1->setPosition({20.0f, 20.0f});
  vbox1->setSize({tabs->getSize().x - 40.0f, tabs->getSize().y - 40.0f});

  vbox1->addChild<Label>("Interactive Buttons & Selectors", 18.0f, Color::from_rgba8(130, 230, 255));

  auto hbox1 = vbox1->addChild<HBoxContainer>(16.0f);

  auto btnStandard = hbox1->addChild<Button>("Standard Button");
  btnStandard->customMinimumSize = {180.0f, 38.0f};

  auto btnFlat = hbox1->addChild<Button>("Flat Button");
  btnFlat->flat = true;
  btnFlat->customMinimumSize = {140.0f, 38.0f};

  auto linkBtn = hbox1->addChild<LinkButton>("Hyperlink Button", "https://github.com/Alazar42/MelkamEngine");
  linkBtn->fontColor = Color::from_rgba8(90, 180, 255);

  vbox1->addChild<HSeparator>();
  vbox1->addChild<Label>("Dropdown Selectors & Popup Menus", 16.0f, Color::WHITE);

  auto hboxMenu = vbox1->addChild<HBoxContainer>(16.0f);
  auto menuBtn = hboxMenu->addChild<MenuButton>("Select Action ▼");
  menuBtn->customMinimumSize = {180.0f, 36.0f};
  auto popup = menuBtn->getPopup();
  popup->addItem("New Game", 1);
  popup->addItem("Save Game", 2);
  popup->addSeparator();
  popup->addItem("Engine Settings", 3);

  auto popupStatus = hboxMenu->addChild<Label>("Menu Selection: (none)", 15.0f);
  popup->id_pressed.connect([popupStatus](int id) {
    if (!popupStatus) return;
    if (id == 1) popupStatus->text = "Menu Selection: New Game (ID: 1)";
    else if (id == 2) popupStatus->text = "Menu Selection: Save Game (ID: 2)";
    else if (id == 3) popupStatus->text = "Menu Selection: Engine Settings (ID: 3)";
  });

  auto optBtn = hboxMenu->addChild<OptionButton>();
  optBtn->customMinimumSize = {200.0f, 36.0f};
  optBtn->addItem("Option A: Low Quality");
  optBtn->addItem("Option B: Medium Quality");
  optBtn->addItem("Option C: Ultra Quality");

  vbox1->addChild<HSeparator>();
  vbox1->addChild<Label>("Toggle Switches & Radio Button Groups", 16.0f, Color::WHITE);

  auto hbox2 = vbox1->addChild<HBoxContainer>(20.0f);
  auto toggleSwitch = hbox2->addChild<CheckButton>("Sound Effects");
  toggleSwitch->customMinimumSize = {160.0f, 32.0f};
  toggleSwitch->setButtonPressed(true);

  auto btnGroup = makeRef<ButtonGroup>();
  auto radio1 = hbox2->addChild<CheckBox>("Easy Mode");
  radio1->setButtonGroup(btnGroup);
  radio1->setButtonPressed(true);

  auto radio2 = hbox2->addChild<CheckBox>("Normal Mode");
  radio2->setButtonGroup(btnGroup);

  auto radio3 = hbox2->addChild<CheckBox>("Hard Mode");
  radio3->setButtonGroup(btnGroup);

  vbox1->addChild<HSeparator>();
  vbox1->addChild<Label>("Text Input Fields & Steppers", 16.0f, Color::WHITE);

  auto hbox3 = vbox1->addChild<HBoxContainer>(16.0f);
  auto lineEdit = hbox3->addChild<LineEdit>("PlayerOne");
  lineEdit->placeholderText = "Enter character name...";
  lineEdit->customMinimumSize = {260.0f, 36.0f};
  lineEdit->clearButtonEnabled = true;

  auto spinBox = hbox3->addChild<SpinBox>(1.0f, 100.0f, 25.0f, 1.0f);
  spinBox->suffix = " HP";
  spinBox->customMinimumSize = {160.0f, 36.0f};


  // ---------------------------------------------------------------------------
  // Tab 2: Text & BBCode Editor
  // ---------------------------------------------------------------------------
  auto tab2 = tabs->addTab("Text & BBCode");
  auto vbox2 = tab2->addChild<VBoxContainer>(12.0f);
  vbox2->setPosition({20.0f, 20.0f});
  vbox2->setSize({tabs->getSize().x - 40.0f, tabs->getSize().y - 40.0f});

  vbox2->addChild<Label>("RichTextLabel BBCode Parser Demonstration", 18.0f, Color::GOLD);

  auto richLabel = vbox2->addChild<RichTextLabel>();
  richLabel->customMinimumSize = {tabs->getSize().x - 40.0f, 90.0f};
  richLabel->setBbcode(
      "[b]Bold Heading[/b] with [i]Italic Accent[/i] and [u]Underlined Emphasis[/u].\n"
      "Supports [color=#00e6a0]Emerald[/color], [color=#ff4060]Crimson[/color], [color=#ffd700]Golden Highlights[/color], "
      "and [url=https://melkamengine.org]Interactive Clickable Links[/url]!");

  vbox2->addChild<HSeparator>();
  vbox2->addChild<Label>("TextEdit Multi-Line Source Code Editor", 18.0f, Color::from_rgba8(130, 230, 255));

  auto textEdit = vbox2->addChild<TextEdit>();
  textEdit->customMinimumSize = {tabs->getSize().x - 40.0f, 180.0f};
  textEdit->setText(
      "// MelkamEngine C++ Example Script\n"
      "#include \"MelkamEngine.hpp\"\n\n"
      "void setupScene(SceneTree* tree) {\n"
      "    auto player = tree->getRoot()->addChild<PlayerNode>();\n"
      "    player->speed = 400.0f;\n"
      "    std::cout << \"Player spawned!\" << std::endl;\n"
      "}");

  // ---------------------------------------------------------------------------
  // Tab 3: Sliders & Progress
  // ---------------------------------------------------------------------------
  auto tab3 = tabs->addTab("Sliders & Meters");
  auto vbox3 = tab3->addChild<VBoxContainer>(14.0f);
  vbox3->setPosition({20.0f, 20.0f});
  vbox3->setSize({tabs->getSize().x - 40.0f, tabs->getSize().y - 40.0f});

  vbox3->addChild<Label>("Progress Indicators & Range Meters", 18.0f, Color::GOLD);

  auto progBarH = vbox3->addChild<ProgressBar>();
  progBarH->customMinimumSize = {420.0f, 26.0f};
  progBarH->setValue(65.0f);

  auto sliderValLabel = vbox3->addChild<Label>("Interactive Slider Value: 65%", 16.0f);

  auto hSlider = vbox3->addChild<HSlider>();
  hSlider->customMinimumSize = {420.0f, 24.0f};
  hSlider->setMinValue(0.0f);
  hSlider->setMaxValue(100.0f);
  hSlider->setValue(65.0f);
  hSlider->value_changed.connect([progBarH, sliderValLabel](float val) {
    if (progBarH) progBarH->setValue(val);
    if (sliderValLabel) sliderValLabel->text = "Interactive Slider Value: " + std::to_string(static_cast<int>(val)) + "%";
  });

  vbox3->addChild<HSeparator>();
  vbox3->addChild<Label>("ScrollBars (Proportional Thumbs)", 16.0f, Color::WHITE);

  auto hScrollBar = vbox3->addChild<HScrollBar>();
  hScrollBar->customMinimumSize = {420.0f, 18.0f};
  hScrollBar->setValue(30.0f);

  // ---------------------------------------------------------------------------
  // Tab 4: Containers & Layouts
  // ---------------------------------------------------------------------------
  auto tab4 = tabs->addTab("Containers & Layouts");
  auto vbox4 = tab4->addChild<VBoxContainer>(12.0f);
  vbox4->setPosition({20.0f, 20.0f});
  vbox4->setSize({tabs->getSize().x - 40.0f, tabs->getSize().y - 40.0f});

  vbox4->addChild<Label>("Draggable SplitContainer (Move divider to resize)", 17.0f, Color::GOLD);

  auto split = vbox4->addChild<HSplitContainer>();
  split->customMinimumSize = {tabs->getSize().x - 40.0f, 120.0f};
  split->splitOffset = 280.0f;

  auto leftPane = split->addChild<Panel>();
  leftPane->backgroundColor = Color::from_rgba8(35, 45, 65);
  auto leftLabel = leftPane->addChild<Label>("Left Panel (Resizable)");
  leftLabel->setPosition({14.0f, 14.0f});

  auto rightPane = split->addChild<Panel>();
  rightPane->backgroundColor = Color::from_rgba8(45, 35, 55);
  auto rightLabel = rightPane->addChild<Label>("Right Panel (Resizable)");
  rightLabel->setPosition({14.0f, 14.0f});

  vbox4->addChild<HSeparator>();
  vbox4->addChild<Label>("FlowContainer (Dynamic Word-Wrap Tags)", 17.0f, Color::from_rgba8(130, 230, 255));

  auto flow = vbox4->addChild<HFlowContainer>(10.0f, 10.0f);
  flow->customMinimumSize = {tabs->getSize().x - 40.0f, 80.0f};

  const char *tags[] = {"#GameDev", "#Cpp20", "#GodotParity", "#Box2D", "#Renderer2D", "#CanvasItem", "#SignalSlots", "#SceneTree"};
  for (const char *tag : tags) {
    auto chip = flow->addChild<Button>(tag);
    chip->customMinimumSize = {120.0f, 32.0f};
  }

  // ---------------------------------------------------------------------------
  // Tab 5: Dialogs & Floating Windows
  // ---------------------------------------------------------------------------
  auto tab5 = tabs->addTab("Modals & Windows");
  auto vbox5 = tab5->addChild<VBoxContainer>(14.0f);
  vbox5->setPosition({20.0f, 20.0f});
  vbox5->setSize({tabs->getSize().x - 40.0f, tabs->getSize().y - 40.0f});

  vbox5->addChild<Label>("Modal Dialogs & Draggable UI Windows", 18.0f, Color::GOLD);

  // Modal Alerts
  auto demoAccept = addChild<AcceptDialog>("This is a modal AcceptDialog alert!\nClick OK to close.", "Alert Modal");
  auto demoConfirm = addChild<ConfirmationDialog>("Do you want to proceed with this operation?", "Confirm Modal");

  // Draggable Floating Window
  auto floatingWin = addChild<UIWindow>("Floating Toolbox Window");
  floatingWin->customMinimumSize = {320.0f, 200.0f};
  auto winContent = floatingWin->addChild<VBoxContainer>(10.0f);
  winContent->setPosition({16.0f, 44.0f});
  winContent->setSize({288.0f, 140.0f});
  winContent->addChild<Label>("Drag my title bar around!", 15.0f, Color::from_rgba8(130, 230, 255));
  winContent->addChild<Button>("Toolbox Button 1");
  winContent->addChild<Button>("Toolbox Button 2");

  auto hbox5 = vbox5->addChild<HBoxContainer>(16.0f);

  auto btnOpenAlert = hbox5->addChild<Button>("Open AcceptDialog");
  btnOpenAlert->customMinimumSize = {180.0f, 40.0f};
  btnOpenAlert->pressed.connect([demoAccept]() {
    if (demoAccept) demoAccept->popupCentered({400.0f, 200.0f});
  });

  auto btnOpenConfirm = hbox5->addChild<Button>("Open ConfirmationDialog");
  btnOpenConfirm->customMinimumSize = {200.0f, 40.0f};
  btnOpenConfirm->pressed.connect([demoConfirm]() {
    if (demoConfirm) demoConfirm->popupCentered({420.0f, 200.0f});
  });

  auto btnOpenWindow = hbox5->addChild<Button>("Open Floating Window");
  btnOpenWindow->customMinimumSize = {180.0f, 40.0f};
  btnOpenWindow->pressed.connect([floatingWin]() {
    if (floatingWin) floatingWin->popupCentered({340.0f, 220.0f});
  });

  vbox5->addChild<HSeparator>();
  auto statusLabel = vbox5->addChild<Label>("Click the buttons above to test modal dialogs and draggable floating windows.", 15.0f, Color::from_rgba8(160, 175, 205));

  demoConfirm->confirmed.connect([statusLabel]() {
    if (statusLabel) statusLabel->text = "Status: User CONFIRMED the dialog!";
  });
  demoConfirm->canceled.connect([statusLabel]() {
    if (statusLabel) statusLabel->text = "Status: User CANCELED the dialog!";
  });

  // ---------------------------------------------------------------------------
  // Tab 6: 2D FX, Lighting & Splines
  // ---------------------------------------------------------------------------
  auto tab6 = tabs->addTab("2D FX & Lighting");
  auto vbox6 = tab6->addChild<VBoxContainer>(14.0f);
  vbox6->setPosition({20.0f, 20.0f});
  vbox6->setSize({tabs->getSize().x - 40.0f, tabs->getSize().y - 40.0f});

  vbox6->addChild<Label>("Procedural 2D Nodes, Particles & Lighting", 18.0f, Color::GOLD);

  auto hbox6 = vbox6->addChild<HBoxContainer>(20.0f);

  // Left card: Particle emitter & Light
  auto fxCard = hbox6->addChild<Panel>();
  fxCard->customMinimumSize = {360.0f, 240.0f};
  fxCard->backgroundColor = Color::from_rgba8(20, 24, 35);
  fxCard->borderColor = Color::from_rgba8(60, 75, 110);
  auto fxCardLabel = fxCard->addChild<Label>("CPUParticles2D & PointLight2D", 15.0f, Color::from_rgba8(130, 230, 255));
  fxCardLabel->setPosition({14.0f, 10.0f});

  auto particles = fxCard->addChild<CPUParticles2D>(32);
  particles->setPosition({180.0f, 160.0f});
  particles->color = Color::from_rgba8(255, 140, 50);
  particles->colorEnd = Color::from_rgba8(255, 50, 50, 0);
  particles->scaleMin = 4.0f;
  particles->scaleMax = 10.0f;
  particles->initialVelocityMin = 60.0f;
  particles->initialVelocityMax = 120.0f;
  particles->spreadDegrees = 360.0f;
  particles->gravity = {0.0f, -40.0f};

  auto light = fxCard->addChild<PointLight2D>(120.0f, Color::from_rgba8(255, 180, 50, 180), 0.9f);
  light->setPosition({180.0f, 160.0f});

  // Middle card: Line2D and Polygon2D
  auto polyCard = hbox6->addChild<Panel>();
  polyCard->customMinimumSize = {360.0f, 240.0f};
  polyCard->backgroundColor = Color::from_rgba8(20, 24, 35);
  polyCard->borderColor = Color::from_rgba8(60, 75, 110);
  auto polyCardLabel = polyCard->addChild<Label>("Line2D Polyline & Polygon2D", 15.0f, Color::from_rgba8(130, 230, 255));
  polyCardLabel->setPosition({14.0f, 10.0f});

  auto demoLine = polyCard->addChild<Line2D>();
  demoLine->setPosition({30.0f, 60.0f});
  demoLine->width = 6.0f;
  demoLine->gradient = {Color::CYAN, Color::MAGENTA, Color::GOLD};
  demoLine->addPoint({0.0f, 40.0f});
  demoLine->addPoint({70.0f, 10.0f});
  demoLine->addPoint({140.0f, 50.0f});
  demoLine->addPoint({210.0f, 20.0f});
  demoLine->addPoint({280.0f, 60.0f});

  auto demoPoly = polyCard->addChild<Polygon2D>();
  demoPoly->setPosition({180.0f, 165.0f});
  demoPoly->color = Color::from_rgba8(0, 230, 180);
  demoPoly->setPolygon({
    {0.0f, -35.0f}, {10.0f, -10.0f}, {35.0f, -10.0f},
    {15.0f, 6.0f}, {22.0f, 32.0f}, {0.0f, 16.0f},
    {-22.0f, 32.0f}, {-15.0f, 6.0f}, {-35.0f, -10.0f},
    {-10.0f, -10.0f}
  });

  // Right card: Controls & Stats
  auto infoCard = hbox6->addChild<Panel>();
  infoCard->customMinimumSize = {360.0f, 240.0f};
  infoCard->backgroundColor = Color::from_rgba8(20, 24, 35);
  infoCard->borderColor = Color::from_rgba8(60, 75, 110);
  auto infoCardLabel = infoCard->addChild<Label>("2D Physics & Spatial Controls", 15.0f, Color::from_rgba8(130, 230, 255));
  infoCardLabel->setPosition({14.0f, 10.0f});

  auto infoContent = infoCard->addChild<VBoxContainer>(10.0f);
  infoContent->setPosition({14.0f, 40.0f});
  infoContent->setSize({330.0f, 180.0f});

  auto burstBtn = infoContent->addChild<Button>("✦ Burst Particle Explosion");
  burstBtn->customMinimumSize = {320.0f, 36.0f};
  burstBtn->pressed.connect([particles]() {
    if (particles) {
      particles->explosiveness = 1.0f;
      particles->restart();
    }
  });

  auto emitToggle = infoContent->addChild<CheckButton>("Continuous Particles");
  emitToggle->customMinimumSize = {240.0f, 32.0f};
  emitToggle->setButtonPressed(true);
  emitToggle->toggled.connect([particles](bool on) {
    if (particles) particles->emitting = on;
  });

  auto lightToggle = infoContent->addChild<CheckButton>("PointLight2D Glow");
  lightToggle->customMinimumSize = {240.0f, 32.0f};
  lightToggle->setButtonPressed(true);
  lightToggle->toggled.connect([light](bool on) {
    if (light) light->enabled = on;
  });
}


// =============================================================================
// 6. MAIN ENTRY POINT
// =============================================================================

int main() {
  Application app("MelkamEngine - Godot-Style 2D Engine & Canvas UI Framework", 1280, 720, true,
                  StretchMode::CanvasItems, StretchAspect::Keep);

  // Set initial scene to Main Menu
  app.getTree()->changeScene(makeRef<MainMenuScene>());

  // Run Main Engine Loop
  app.run();
  return 0;
}