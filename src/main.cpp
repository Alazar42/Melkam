#include "MelkamEngine.hpp"

// 1. Controllable Player Character (CharacterBody2D with CollisionShape2D &
// MeshInstance2D children)
class PlayerNode : public CharacterBody2D {
public:
  float speed = 350.0f;
  float jumpVelocity = -700.0f;
  float gravity = 1400.0f;

  void onReady() override {
    // 1. Attach CollisionShape2D child using addChild
    auto colShape = addChild<CollisionShape2D>(Vector2(40.0f, 50.0f));

    // 2. Attach MeshInstance2D as a child of CollisionShape2D
    colShape->addChild<MeshInstance2D>(Vector2(40.0f, 50.0f), Color::GOLD);
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
      velocity.y = jumpVelocity;
      setOnFloor(false);
    }

    // 3. Horizontal Movement (A / D)
    float horizontal = Input::getAxis(Key::A, Key::D);
    velocity.x = horizontal * speed;

    // 4. Move and Slide with Box2D physics & collision resolution
    moveAndSlide();

    // Fall reset if falling off screen
    if (getPosition().y > 900.0f) {
      setPosition({200.0f, 300.0f});
      velocity = {0.0f, 0.0f};
    }
  }
};

// 2. Collectible Coin Trigger (Area2D with Godot body_entered Signal)
class CoinNode : public Area2D {
public:
  void onReady() override {
    // 1. Circle collision shape child
    auto colShape = addChild<CollisionShape2D>(15.0f);

    // 2. Visual mesh attached as a child of the collision shape
    colShape->addChild<MeshInstance2D>(15.0f,
                                       Color::from_rgba8(255, 215, 0, 220));

    // 3. Connect to Godot-style Signal body_entered
    body_entered.connect([this](Node2D *body) {
      if (body && body->name == "Player") {
        SDL_Log("=== [SIGNAL FIRED] Coin collected by %s! ===", body->name.c_str());
        std::cout << "=== [SIGNAL FIRED] Coin collected by " << body->name << "! ===" << std::endl;
        std::flush(std::cout);
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
  // 1. Create Application (maximized)
  Application app("MelkamEngine - Godot Signal & Scene Sandbox", 1280, 720,
                  true);

  // 2. Ground Platform (StaticBody2D with CollisionShape2D + MeshInstance2D
  // children)
  auto ground = app.addChild<StaticBody2D>("Ground");
  ground->setPosition({640.0f, 680.0f});
  auto groundShape =
      ground->addChild<CollisionShape2D>(Vector2(1280.0f, 40.0f));
  groundShape->addChild<MeshInstance2D>(Vector2(1280.0f, 40.0f),
                                        Color::from_rgba8(60, 60, 75));

  // Floating Platforms
  auto plat1 = app.addChild<StaticBody2D>("Platform1");
  plat1->setPosition({350.0f, 500.0f});
  auto plat1Shape = plat1->addChild<CollisionShape2D>(Vector2(250.0f, 30.0f));
  plat1Shape->addChild<MeshInstance2D>(Vector2(250.0f, 30.0f),
                                       Color::from_rgba8(90, 90, 115));

  auto plat2 = app.addChild<StaticBody2D>("Platform2");
  plat2->setPosition({850.0f, 380.0f});
  auto plat2Shape = plat2->addChild<CollisionShape2D>(Vector2(250.0f, 30.0f));
  plat2Shape->addChild<MeshInstance2D>(Vector2(250.0f, 30.0f),
                                       Color::from_rgba8(90, 90, 115));

  // 3. Collectible Coins (Area2D with Signal connection)
  auto coin1 = app.addChild<CoinNode>();
  coin1->setPosition({350.0f, 440.0f});

  auto coin2 = app.addChild<CoinNode>();
  coin2->setPosition({850.0f, 320.0f});

  // 4. Dynamic Crates (RigidBody2D with CollisionShape2D + MeshInstance2D children)
  auto crate1 = app.addChild<RigidBody2D>("Crate1");
  crate1->setPosition({400.0f, 150.0f});
  crate1->restitution = 0.05f;
  crate1->friction = 0.6f;
  auto crate1Shape = crate1->addChild<CollisionShape2D>(Vector2(45.0f, 45.0f));
  crate1Shape->addChild<MeshInstance2D>(Vector2(45.0f, 45.0f),
                                        Color::from_rgba8(205, 133, 63));

  auto crate2 = app.addChild<RigidBody2D>("Crate2");
  crate2->setPosition({820.0f, 100.0f});
  crate2->restitution = 0.05f;
  crate2->friction = 0.6f;
  auto crate2Shape = crate2->addChild<CollisionShape2D>(Vector2(40.0f, 40.0f));
  crate2Shape->addChild<MeshInstance2D>(Vector2(40.0f, 40.0f),
                                        Color::from_rgba8(220, 160, 80));

  // 5. Player CharacterBody2D
  auto player = app.addChild<PlayerNode>();
  player->name = "Player";
  player->setPosition({200.0f, 400.0f});

  // 6. Run the Game
  app.run();
  return 0;
}