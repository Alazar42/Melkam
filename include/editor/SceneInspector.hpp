#pragma once

#include "core/Node.hpp"
#include "core/SceneTree.hpp"
#include "helper/color/Color.hpp"
#include "helper/Rect2.hpp"
#include "helper/vectors/Vector2.hpp"
#include "input.hpp"
#include "nodes/2D/Camera2D.hpp"
#include "nodes/2D/CPUParticles2D.hpp"
#include "nodes/2D/Node2D.hpp"
#include "nodes/2D/PointLight2D.hpp"
#include "nodes/2D/physics/CharacterBody2D.hpp"
#include "nodes/2D/physics/CollisionObject2D.hpp"
#include "nodes/2D/tilemap/TileMapLayer.hpp"
#include "physics/2D/PhysicsServer2D.hpp"
#include "renderers/Renderer2D.hpp"
#include "time.hpp"
#include "window.hpp"
#include <algorithm>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

// Professional In-Engine Runtime Scene Inspector & Live Debug Suite (Godot 4 Parity)
class SceneInspector {
public:
  static inline bool s_visible = false;
  static inline bool s_drawPhysicsWireframe = false;
  static inline std::shared_ptr<Node> s_selectedNode = nullptr;
  static inline int s_activeTab = 0; // 0: Inspector, 1: Stats & Profiler, 2: Physics Debug
  static inline float s_scrollOffsetY = 0.0f;
  static inline std::unordered_set<Node *> s_collapsedNodes;

  static void toggle() {
    s_visible = !s_visible;
  }

  static void setVisible(bool vis) {
    s_visible = vis;
  }

  static bool isVisible() {
    return s_visible;
  }

  static void handleInput(const InputEvent &event) {
    // 1. Hotkey toggles (F12 or Grave/Tilde for Inspector, F3 for Physics Wireframe)
    if (event.isKeyPressed(Key::F12) || event.isKeyPressed(Key::Grave)) {
      toggle();
      const_cast<InputEvent &>(event).setHandled();
      return;
    }

    if (event.isKeyPressed(Key::F3)) {
      s_drawPhysicsWireframe = !s_drawPhysicsWireframe;
      const_cast<InputEvent &>(event).setHandled();
      return;
    }

    if (!s_visible) return;

    Vector2 mpos = Input::getMousePosition();

    // 2. Mouse Wheel Scroll on Hierarchy panel
    if (event.type == InputEventType::MouseWheel) {
      s_scrollOffsetY -= event.mouseScroll.y * 30.0f;
      s_scrollOffsetY = std::max(0.0f, s_scrollOffsetY);
      const_cast<InputEvent &>(event).setHandled();
      return;
    }

    // 3. Mouse clicks inside Inspector UI
    if (event.isMouseButton(MouseButton::Left) && event.isPressed()) {
      if (processClick(mpos)) {
        const_cast<InputEvent &>(event).setHandled();
      }
    }
  }

  static void render() {
    // 1. Render World Space Physics Wireframe Overlay (F3 Mode)
    if (s_drawPhysicsWireframe) {
      renderPhysicsWireframes();
    }

    // 2. Render World Space Selection Gizmo around active Node2D
    if (s_selectedNode) {
      if (auto *n2d = dynamic_cast<Node2D *>(s_selectedNode.get())) {
        Transform2D trans = n2d->getGlobalTransform();
        Vector2 pos = trans.position;
        Vector2 sz{48.0f, 48.0f};

        if (auto *colObj = dynamic_cast<CollisionObject2D *>(n2d)) {
          sz = colObj->getHalfExtents() * 2.0f;
        }

        // Selection bounding rect
        Renderer2D::drawRect(pos - sz * 0.5f, sz, Color::from_rgba8(255, 215, 0, 220), false);
        // Gizmo crosshair
        Renderer2D::drawLine(pos - Vector2(10.0f, 0.0f), pos + Vector2(10.0f, 0.0f), Color::from_rgba8(255, 230, 80, 255), 2.0f);
        Renderer2D::drawLine(pos - Vector2(0.0f, 10.0f), pos + Vector2(0.0f, 10.0f), Color::from_rgba8(255, 230, 80, 255), 2.0f);
      }
    }

    if (!s_visible) return;

    // 3. Screen-Space Inspector Window Geometry
    Vector2 vp = Window::getViewportSize();
    float winWidth = std::min(840.0f, vp.x - 40.0f);
    float winHeight = std::min(600.0f, vp.y - 40.0f);
    Vector2 winPos{20.0f, 20.0f};

    s_clickBoxes.clear();

    // Main window background (sleek dark glass with neon cyan border)
    Renderer2D::drawRoundedRectScreen(winPos, Vector2(winWidth, winHeight), 10.0f,
                                      Color::from_rgba8(16, 20, 30, 245),
                                      Color::from_rgba8(60, 150, 255, 200), 2.0f);

    // Title & Header Bar
    Renderer2D::drawRoundedRectScreen(winPos, Vector2(winWidth, 42.0f), 10.0f,
                                      Color::from_rgba8(26, 34, 52, 255));
    Renderer2D::drawText("MELKAM ENGINE - LIVE SCENE INSPECTOR", winPos + Vector2(16.0f, 12.0f),
                         Color::from_rgba8(240, 245, 255, 255), 15.0f);

    // Close button [X]
    Rect2 closeRect(winPos.x + winWidth - 36.0f, winPos.y + 8.0f, 26.0f, 26.0f);
    Renderer2D::drawRoundedRectScreen(closeRect.position, closeRect.size, 4.0f, Color::from_rgba8(220, 50, 60, 200));
    Renderer2D::drawText("X", closeRect.position + Vector2(8.0f, 4.0f), Color::WHITE, 14.0f);
    s_clickBoxes.push_back({closeRect, []() { setVisible(false); }});

    // Top Status & Tabs Toolbar (Y: winPos.y + 48)
    Vector2 tabPos = winPos + Vector2(16.0f, 48.0f);
    drawTabButton(tabPos, "Scene & Properties", 0);
    drawTabButton(tabPos + Vector2(170.0f, 0.0f), "Stats & Profiler", 1);
    drawTabButton(tabPos + Vector2(320.0f, 0.0f), "Physics Debug", 2);

    // Wireframe Quick Toggle on right of toolbar
    Rect2 wfRect(winPos.x + winWidth - 190.0f, winPos.y + 48.0f, 174.0f, 26.0f);
    Color wfBg = s_drawPhysicsWireframe ? Color::from_rgba8(40, 160, 80, 220) : Color::from_rgba8(45, 52, 70, 200);
    Renderer2D::drawRoundedRectScreen(wfRect.position, wfRect.size, 4.0f, wfBg);
    Renderer2D::drawText(s_drawPhysicsWireframe ? "[F3] Wireframe: ON" : "[F3] Wireframe: OFF",
                         wfRect.position + Vector2(12.0f, 5.0f), Color::WHITE, 12.0f);
    s_clickBoxes.push_back({wfRect, []() { s_drawPhysicsWireframe = !s_drawPhysicsWireframe; }});

    float contentY = winPos.y + 82.0f;
    float contentHeight = winHeight - 94.0f;

    if (s_activeTab == 0) {
      // TAB 0: Hierarchy (Left) + Property Inspector (Right)
      float treeWidth = winWidth * 0.46f;
      float propWidth = winWidth - treeWidth - 44.0f;

      renderHierarchyPanel(winPos + Vector2(16.0f, 62.0f), Vector2(treeWidth, contentHeight));
      renderPropertyPanel(winPos + Vector2(treeWidth + 28.0f, 62.0f), Vector2(propWidth, contentHeight));
    } else if (s_activeTab == 1) {
      // TAB 1: Stats & Profiler
      renderStatsPanel(winPos + Vector2(16.0f, 62.0f), Vector2(winWidth - 32.0f, contentHeight));
    } else if (s_activeTab == 2) {
      // TAB 2: Physics Debug
      renderPhysicsPanel(winPos + Vector2(16.0f, 62.0f), Vector2(winWidth - 32.0f, contentHeight));
    }
  }

private:
  struct ActionClickBox {
    Rect2 rect;
    std::function<void()> callback;
  };

  static inline std::vector<ActionClickBox> s_clickBoxes;

  static void drawTabButton(const Vector2 &pos, const std::string &title, int tabIndex) {
    Rect2 rect(pos.x, pos.y, 140.0f, 26.0f);
    bool isActive = (s_activeTab == tabIndex);
    Color bg = isActive ? Color::from_rgba8(50, 130, 240, 240) : Color::from_rgba8(32, 40, 58, 200);
    Renderer2D::drawRoundedRectScreen(rect.position, rect.size, 4.0f, bg);
    Renderer2D::drawText(title, rect.position + Vector2(10.0f, 5.0f), isActive ? Color::WHITE : Color::from_rgba8(180, 200, 230), 12.0f);

    s_clickBoxes.push_back({rect, [tabIndex]() { s_activeTab = tabIndex; }});
  }

  static void renderHierarchyPanel(const Vector2 &pos, const Vector2 &size) {
    // Panel background container
    Renderer2D::drawRoundedRectScreen(pos, size, 6.0f, Color::from_rgba8(12, 15, 22, 235),
                                      Color::from_rgba8(45, 55, 78, 180), 1.0f);
    Renderer2D::drawText("SCENE HIERARCHY", pos + Vector2(12.0f, 8.0f),
                         Color::from_rgba8(110, 180, 255), 13.0f);

    float viewY = pos.y + 30.0f;
    float viewHeight = size.y - 36.0f;

    float drawY = viewY - s_scrollOffsetY;
    if (SceneTree::getCurrent() && SceneTree::getCurrent()->getRoot()) {
      drawNodeTreeRecursive(SceneTree::getCurrent()->getRoot(), pos.x + 8.0f, drawY, 0, size.x - 20.0f, viewY, viewHeight);
    }

    // Scrollbar track & thumb
    float totalContentHeight = std::max(viewHeight, (drawY + s_scrollOffsetY) - viewY);
    if (totalContentHeight > viewHeight) {
      float thumbHeight = std::max(24.0f, (viewHeight / totalContentHeight) * viewHeight);
      float thumbY = viewY + (s_scrollOffsetY / (totalContentHeight - viewHeight)) * (viewHeight - thumbHeight);
      Renderer2D::drawRoundedRectScreen(Vector2(pos.x + size.x - 8.0f, thumbY), Vector2(5.0f, thumbHeight), 2.0f,
                                        Color::from_rgba8(80, 140, 240, 180));
    }
  }

  static void drawNodeTreeRecursive(std::shared_ptr<Node> node, float x, float &y, int depth, float maxWidth, float viewY, float viewHeight) {
    if (!node) return;

    bool isSelected = (s_selectedNode == node);
    bool isCollapsed = (s_collapsedNodes.find(node.get()) != s_collapsedNodes.end());
    bool hasChildren = !node->getChildren().empty();

    Rect2 itemRect(x, y, maxWidth, 20.0f);

    // Only render and register clicks if within scroll viewport
    if (y + 20.0f >= viewY && y <= viewY + viewHeight) {
      if (isSelected) {
        Renderer2D::drawRoundedRectScreen(itemRect.position, itemRect.size, 4.0f,
                                          Color::from_rgba8(35, 95, 190, 220));
      }

      // Foldout arrow
      if (hasChildren) {
        std::string foldout = isCollapsed ? ">" : "v";
        Renderer2D::drawText(foldout, Vector2(x + depth * 14.0f, y + 2.0f), Color::from_rgba8(140, 170, 210), 12.0f);
      }

      // Node Name
      std::string text = node->name;
      Color nameCol = isSelected ? Color::WHITE : Color::from_rgba8(210, 220, 240);
      Renderer2D::drawText(text, Vector2(x + (depth * 14.0f) + 14.0f, y + 2.0f), nameCol, 12.0f);

      // Node Type Badge on right
      std::string badge = getNodeTypeBadge(node.get());
      Renderer2D::drawText(badge, Vector2(x + maxWidth - 60.0f, y + 2.0f), Color::from_rgba8(100, 200, 160, 200), 11.0f);

      // Click callback
      s_clickBoxes.push_back({itemRect, [node, hasChildren]() {
        s_selectedNode = node;
      }});
    }

    y += 22.0f;

    if (!isCollapsed) {
      for (const auto &child : node->getChildren()) {
        drawNodeTreeRecursive(child, x, y, depth + 1, maxWidth, viewY, viewHeight);
      }
    }
  }

  static std::string getNodeTypeBadge(Node *node) {
    if (dynamic_cast<CharacterBody2D *>(node)) return "[Char]";
    if (dynamic_cast<StaticBody2D *>(node)) return "[Stat]";
    if (dynamic_cast<CollisionShape2D *>(node)) return "[Col]";
    if (dynamic_cast<PointLight2D *>(node)) return "[Light]";
    if (dynamic_cast<CPUParticles2D *>(node)) return "[FX]";
    if (dynamic_cast<TileMapLayer *>(node)) return "[Tile]";
    if (dynamic_cast<Camera2D *>(node)) return "[Cam]";
    if (dynamic_cast<Node2D *>(node)) return "[2D]";
    return "[Node]";
  }

  static void renderPropertyPanel(const Vector2 &pos, const Vector2 &size) {
    Renderer2D::drawRoundedRectScreen(pos, size, 6.0f, Color::from_rgba8(12, 15, 22, 235),
                                      Color::from_rgba8(45, 55, 78, 180), 1.0f);
    Renderer2D::drawText("PROPERTY INSPECTOR", pos + Vector2(12.0f, 8.0f),
                         Color::from_rgba8(110, 180, 255), 13.0f);

    if (!s_selectedNode) {
      Renderer2D::drawText("Select any node in the hierarchy", pos + Vector2(16.0f, 40.0f),
                           Color::from_rgba8(140, 150, 170), 13.0f);
      return;
    }

    float curY = pos.y + 32.0f;
    float contentWidth = size.x - 24.0f;
    float startX = pos.x + 12.0f;

    // Node Name & Type
    Renderer2D::drawText("Name: " + s_selectedNode->name, Vector2(startX, curY), Color::WHITE, 14.0f);
    curY += 20.0f;

    std::string typeName = typeid(*s_selectedNode).name();
    if (typeName.rfind("class ", 0) == 0) typeName = typeName.substr(6);
    Renderer2D::drawText("Type: " + typeName, Vector2(startX, curY), Color::from_rgba8(130, 180, 255), 12.0f);
    curY += 22.0f;

    // Visible Toggle Switch
    Rect2 visRect(startX, curY, 120.0f, 22.0f);
    Color visBg = s_selectedNode->visible ? Color::from_rgba8(35, 140, 70, 220) : Color::from_rgba8(70, 75, 90, 200);
    Renderer2D::drawRoundedRectScreen(visRect.position, visRect.size, 4.0f, visBg);
    Renderer2D::drawText(s_selectedNode->visible ? "Visible: ON" : "Visible: OFF", visRect.position + Vector2(10.0f, 4.0f), Color::WHITE, 12.0f);
    s_clickBoxes.push_back({visRect, []() {
      if (s_selectedNode) s_selectedNode->visible = !s_selectedNode->visible;
    }});
    curY += 30.0f;

    // Node2D Spatial Transform Properties
    if (auto *n2d = dynamic_cast<Node2D *>(s_selectedNode.get())) {
      Renderer2D::drawText("Transform2D", Vector2(startX, curY), Color::from_rgba8(255, 215, 80), 13.0f);
      curY += 18.0f;

      // Position Controls (X and Y with [-] [+] nudge buttons)
      drawNudgeControl(startX, curY, "Pos X", n2d->transform.position.x, 10.0f, [n2d](float v) { n2d->transform.position.x = v; });
      curY += 24.0f;
      drawNudgeControl(startX, curY, "Pos Y", n2d->transform.position.y, 10.0f, [n2d](float v) { n2d->transform.position.y = v; });
      curY += 24.0f;
      drawNudgeControl(startX, curY, "Rotation", n2d->transform.rotation * 57.2958f, 5.0f, [n2d](float deg) { n2d->setRotationDegrees(deg); });
      curY += 24.0f;
      drawNudgeControl(startX, curY, "Scale X", n2d->transform.scale.x, 0.1f, [n2d](float v) { n2d->transform.scale.x = v; });
      curY += 28.0f;
    }

    // PointLight2D Tweaker
    if (auto *light = dynamic_cast<PointLight2D *>(s_selectedNode.get())) {
      Renderer2D::drawText("PointLight2D Properties", Vector2(startX, curY), Color::from_rgba8(255, 215, 80), 13.0f);
      curY += 18.0f;

      drawNudgeControl(startX, curY, "Energy", light->energy, 0.1f, [light](float v) { light->energy = std::max(0.0f, v); });
      curY += 24.0f;
      drawNudgeControl(startX, curY, "Radius", light->radius, 20.0f, [light](float v) { light->radius = std::max(10.0f, v); });
      curY += 24.0f;
      drawNudgeControl(startX, curY, "Attenuation", light->attenuation, 0.2f, [light](float v) { light->attenuation = std::max(0.1f, v); });
      curY += 28.0f;
    }

    // CPUParticles2D Tweaker
    if (auto *parts = dynamic_cast<CPUParticles2D *>(s_selectedNode.get())) {
      Renderer2D::drawText("CPUParticles2D Properties", Vector2(startX, curY), Color::from_rgba8(120, 240, 200), 13.0f);
      curY += 18.0f;

      Rect2 emitRect(startX, curY, 130.0f, 22.0f);
      Renderer2D::drawRoundedRectScreen(emitRect.position, emitRect.size, 4.0f,
                                        parts->emitting ? Color::from_rgba8(35, 140, 70, 220) : Color::from_rgba8(70, 75, 90, 200));
      Renderer2D::drawText(parts->emitting ? "Emitting: YES" : "Emitting: NO", emitRect.position + Vector2(10.0f, 4.0f), Color::WHITE, 12.0f);
      s_clickBoxes.push_back({emitRect, [parts]() { parts->emitting = !parts->emitting; }});
      curY += 26.0f;

      drawNudgeControl(startX, curY, "SpeedScale", parts->speedScale, 0.2f, [parts](float v) { parts->speedScale = std::max(0.1f, v); });
      curY += 28.0f;
    }

    // CharacterBody2D Physics Status
    if (auto *body = dynamic_cast<CharacterBody2D *>(s_selectedNode.get())) {
      Renderer2D::drawText("CharacterBody2D Physics", Vector2(startX, curY), Color::from_rgba8(255, 140, 140), 13.0f);
      curY += 18.0f;

      std::string status = "Floor: " + std::string(body->isOnFloor() ? "YES" : "NO") +
                           "  |  Wall: " + std::string(body->isOnWall() ? "YES" : "NO");
      Renderer2D::drawText(status, Vector2(startX, curY), Color::from_rgba8(220, 230, 245), 12.0f);
      curY += 18.0f;

      std::ostringstream ssVel;
      ssVel << std::fixed << std::setprecision(1) << "Velocity: (" << body->velocity.x << ", " << body->velocity.y << ")";
      Renderer2D::drawText(ssVel.str(), Vector2(startX, curY), Color::from_rgba8(220, 230, 245), 12.0f);
      curY += 28.0f;
    }

    // TileMapLayer Info
    if (auto *tileMap = dynamic_cast<TileMapLayer *>(s_selectedNode.get())) {
      Renderer2D::drawText("TileMapLayer Properties", Vector2(startX, curY), Color::from_rgba8(130, 210, 255), 13.0f);
      curY += 18.0f;

      Renderer2D::drawText("Active Solid Cells: " + std::to_string(tileMap->getCellCount()), Vector2(startX, curY), Color::WHITE, 12.0f);
      curY += 22.0f;

      Rect2 colToggleRect(startX, curY, 160.0f, 22.0f);
      Color colBg = tileMap->collisionEnabled ? Color::from_rgba8(35, 140, 70, 220) : Color::from_rgba8(70, 75, 90, 200);
      Renderer2D::drawRoundedRectScreen(colToggleRect.position, colToggleRect.size, 4.0f, colBg);
      Renderer2D::drawText(tileMap->collisionEnabled ? "Colliders: ENABLED" : "Colliders: DISABLED",
                           colToggleRect.position + Vector2(10.0f, 4.0f), Color::WHITE, 12.0f);
      s_clickBoxes.push_back({colToggleRect, [tileMap]() {
        tileMap->collisionEnabled = !tileMap->collisionEnabled;
        tileMap->updateColliders();
      }});
    }
  }

  static void drawNudgeControl(float x, float y, const std::string &label, float value, float step, std::function<void(float)> setter) {
    Renderer2D::drawText(label + ":", Vector2(x, y + 2.0f), Color::from_rgba8(190, 205, 230), 12.0f);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << value;
    Renderer2D::drawText(ss.str(), Vector2(x + 90.0f, y + 2.0f), Color::WHITE, 12.0f);

    // [-] Button
    Rect2 minusRect(x + 160.0f, y, 22.0f, 20.0f);
    Renderer2D::drawRoundedRectScreen(minusRect.position, minusRect.size, 3.0f, Color::from_rgba8(45, 55, 80, 220));
    Renderer2D::drawText("-", minusRect.position + Vector2(7.0f, 2.0f), Color::WHITE, 13.0f);
    s_clickBoxes.push_back({minusRect, [value, step, setter]() { setter(value - step); }});

    // [+] Button
    Rect2 plusRect(x + 188.0f, y, 22.0f, 20.0f);
    Renderer2D::drawRoundedRectScreen(plusRect.position, plusRect.size, 3.0f, Color::from_rgba8(45, 55, 80, 220));
    Renderer2D::drawText("+", plusRect.position + Vector2(5.0f, 2.0f), Color::WHITE, 13.0f);
    s_clickBoxes.push_back({plusRect, [value, step, setter]() { setter(value + step); }});
  }

  static void renderStatsPanel(const Vector2 &pos, const Vector2 &size) {
    Renderer2D::drawRoundedRectScreen(pos, size, 6.0f, Color::from_rgba8(12, 15, 22, 235),
                                      Color::from_rgba8(45, 55, 78, 180), 1.0f);
    Renderer2D::drawText("ENGINE STATS & PROFILER", pos + Vector2(16.0f, 12.0f),
                         Color::from_rgba8(110, 180, 255), 14.0f);

    float curY = pos.y + 40.0f;
    float startX = pos.x + 16.0f;

    int fps = static_cast<int>(std::round(1.0f / std::max(0.0001f, Time::getDeltaTime())));
    float frameTimeMs = Time::getDeltaTime() * 1000.0f;

    Renderer2D::drawText("Framerate (FPS): " + std::to_string(fps), Vector2(startX, curY), Color::from_rgba8(100, 240, 160), 14.0f);
    curY += 22.0f;

    std::ostringstream ssFt;
    ssFt << std::fixed << std::setprecision(2) << "Frame Time: " << frameTimeMs << " ms";
    Renderer2D::drawText(ssFt.str(), Vector2(startX, curY), Color::WHITE, 13.0f);
    curY += 22.0f;

    int totalNodes = countNodes(SceneTree::getCurrent() ? SceneTree::getCurrent()->getRoot().get() : nullptr);
    Renderer2D::drawText("Total Active Nodes in SceneTree: " + std::to_string(totalNodes), Vector2(startX, curY), Color::WHITE, 13.0f);
    curY += 22.0f;

    int physObjects = static_cast<int>(PhysicsServer2D::getRegisteredObjects().size());
    Renderer2D::drawText("Registered 2D Physics Bodies: " + std::to_string(physObjects), Vector2(startX, curY), Color::WHITE, 13.0f);
    curY += 22.0f;

    Vector2 vp = Window::getViewportSize();
    Renderer2D::drawText("Viewport Resolution: " + std::to_string(static_cast<int>(vp.x)) + " x " + std::to_string(static_cast<int>(vp.y)),
                         Vector2(startX, curY), Color::WHITE, 13.0f);
  }

  static void renderPhysicsPanel(const Vector2 &pos, const Vector2 &size) {
    Renderer2D::drawRoundedRectScreen(pos, size, 6.0f, Color::from_rgba8(12, 15, 22, 235),
                                      Color::from_rgba8(45, 55, 78, 180), 1.0f);
    Renderer2D::drawText("PHYSICS SERVER 2D DEBUG", pos + Vector2(16.0f, 12.0f),
                         Color::from_rgba8(110, 180, 255), 14.0f);

    float curY = pos.y + 40.0f;
    float startX = pos.x + 16.0f;

    Rect2 wfToggle(startX, curY, 200.0f, 26.0f);
    Color wfBg = s_drawPhysicsWireframe ? Color::from_rgba8(40, 160, 80, 220) : Color::from_rgba8(45, 52, 70, 200);
    Renderer2D::drawRoundedRectScreen(wfToggle.position, wfToggle.size, 4.0f, wfBg);
    Renderer2D::drawText(s_drawPhysicsWireframe ? "Physics Wireframes: ON" : "Physics Wireframes: OFF",
                         wfToggle.position + Vector2(12.0f, 5.0f), Color::WHITE, 12.0f);
    s_clickBoxes.push_back({wfToggle, []() { s_drawPhysicsWireframe = !s_drawPhysicsWireframe; }});
    curY += 36.0f;

    const auto &objects = PhysicsServer2D::getRegisteredObjects();
    Renderer2D::drawText("Active Colliders (" + std::to_string(objects.size()) + "):", Vector2(startX, curY), Color::from_rgba8(255, 215, 80), 13.0f);
    curY += 20.0f;

    for (size_t i = 0; i < objects.size() && i < 12; ++i) {
      if (auto *obj = objects[i]) {
        Vector2 p = obj->getGlobalPhysicsPosition();
        Vector2 h = obj->getHalfExtents();
        std::ostringstream ss;
        ss << "#" << (i + 1) << " " << obj->name << " at (" << static_cast<int>(p.x) << ", " << static_cast<int>(p.y)
           << ") size (" << static_cast<int>(h.x * 2.0f) << "x" << static_cast<int>(h.y * 2.0f) << ")";
        Renderer2D::drawText(ss.str(), Vector2(startX + 8.0f, curY), Color::from_rgba8(200, 215, 235), 12.0f);
        curY += 18.0f;
      }
    }
  }

  static void renderPhysicsWireframes() {
    const auto &objects = PhysicsServer2D::getRegisteredObjects();
    for (CollisionObject2D *obj : objects) {
      if (!obj || !obj->isGlobalVisible()) continue;

      Vector2 pos = obj->getGlobalPhysicsPosition();
      Vector2 half = obj->getHalfExtents();
      Vector2 sz = half * 2.0f;
      Vector2 topLeft = pos - half;

      Color wireCol = obj->isSensorBody() ? Color::from_rgba8(255, 140, 0, 200) : Color::from_rgba8(0, 255, 128, 220);
      Renderer2D::drawRect(topLeft, sz, wireCol, false);
    }
  }

  static int countNodes(Node *node) {
    if (!node) return 0;
    int count = 1;
    for (const auto &child : node->getChildren()) {
      count += countNodes(child.get());
    }
    return count;
  }

  static bool processClick(const Vector2 &mpos) {
    for (const auto &box : s_clickBoxes) {
      if (box.rect.hasPoint(mpos)) {
        if (box.callback) box.callback();
        return true;
      }
    }
    return false;
  }
};
