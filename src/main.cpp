#include "MelkamEngine.hpp"

// 1. Custom Player Node (pure Godot style - visual nodes are children)
class PlayerNode : public Node2D {
public:
  float speed = 400.0f;

  void onReady() override {
    // Attach visual mesh as a child node
    spawnChild<MeshInstance2D>(Vector2(50.0f, 50.0f), Color::GOLD);
  }

  void onProcess(float delta) override {
    Vector2 dir = Input::getVector(Key::A, Key::D, Key::W, Key::S);
    translate(dir * speed * delta);
  }
};

// 2. Custom Spinner Node
class SpinnerNode : public Node2D {
public:
  void onReady() override {
    // Attach triangle mesh as a child node
    auto mesh = spawnChild<MeshInstance2D>();
    *mesh = MeshInstance2D::createTriangle(
        {-30.0f, 30.0f}, {30.0f, 30.0f}, {0.0f, -30.0f}, Color::CORAL);
  }

  void onProcess(float delta) override {
    rotate(2.0f * delta);
  }
};

int main() {
  // 1. Create Application (with maximized = true)
  Application app("MelkamEngine - Game Sandbox", 1280, 720, true);

  // 2. Add nodes directly to the Application
  auto player = app.spawn<PlayerNode>();
  player->setPosition({640.0f, 360.0f});

  auto spinner = app.spawn<SpinnerNode>();
  spinner->setPosition({320.0f, 240.0f});

  // 3. Run the Game
  app.run();
  return 0;
}