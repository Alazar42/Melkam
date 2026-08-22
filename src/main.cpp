#include "renderers/Renderer2D.hpp"
#include "window.hpp"

int main() {
  WindowProps props;
  props.title = "MelkamEngine - 2D Sandbox";
  props.width = 1280;
  props.height = 720;
  props.maximized = true;
  props.vsync = true;
  props.clearColor = Color::from_rgba8(20, 20, 25);

  Window window(props);
  Renderer2D::init(window);

  Vector2 playerPos(640.0f, 360.0f);
  float playerSpeed = 350.0f;
  float rotationAngle = 0.0f;

  while (window.isOpen()) {
    window.pollEvents();

    if (Input::isKeyJustPressed(Key::Escape)) {
      window.close();
    }

    // Smooth FPS & Metrics in Title
    if (Time::getFrameCount() % 30 == 0) {
      window.setTitle(
          "MelkamEngine | FPS: " +
          std::to_string(static_cast<int>(Time::getFPS())) + " | dt: " +
          std::to_string(Time::getDeltaTimeMs()).substr(0, 4) + "ms");
    }

    // Movement logic (frame-rate independent)
    float dt = Time::getDeltaTime();
    Vector2 moveDir = Input::getVector(Key::A, Key::D, Key::W, Key::S);
    playerPos += moveDir * playerSpeed * dt;

    rotationAngle += 1.5f * dt;

    // Render Frame
    window.clear();

    Renderer2D::begin();

    // 1. Draw Player
    Renderer2D::drawRect(playerPos - Vector2(25.0f, 25.0f), {50.0f, 50.0f},
                         Color::GOLD);

    // 2. Draw Rotating Square
    Renderer2D::drawRectRotated({300.0f, 300.0f}, {80.0f, 80.0f}, rotationAngle,
                                Color::CORAL);

    // 3. Draw Crosshair at Mouse position
    Vector2 mousePos = Input::getMousePosition();
    Renderer2D::drawCircle(mousePos, 18.0f, Color::CYAN, false);

    // 4. Draw laser pointer between player and mouse
    Renderer2D::drawLine(playerPos, mousePos, Color::RED, 2.0f);

    // 5. Draw decorative shapes
    Renderer2D::drawTriangle({150.0f, 150.0f}, {220.0f, 150.0f},
                             {185.0f, 220.0f}, Color::AQUAMARINE);

    Renderer2D::end();

    window.present();
  }

  return 0;
}