#pragma once

#include "core/Memory.hpp"
#include "core/Signal.hpp"
#include "helper/vectors/Vector2.hpp"
#include "input.hpp"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

class SceneTree;
class Window;

// Base Node class for the entire MelkamEngine Scene Tree (inspired by Godot Node architecture).
class Node : public std::enable_shared_from_this<Node> {
public:
  std::string name = "Node";
  bool visible = true;
  bool active = true;

  // Built-in Godot standard signals
  Signal<> ready;
  Signal<float> process;
  Signal<float> physics_process;
  Signal<Node *> child_entered_tree;
  Signal<Node *> child_exiting_tree;

  Node() = default;
  explicit Node(std::string nodeName) : name(std::move(nodeName)) {}
  virtual ~Node() = default;

  // Copy constructor (preserves independent tree hierarchy)
  Node(const Node &other)
      : name(other.name), visible(other.visible), active(other.active) {}

  // Copy assignment (preserves existing parent and children)
  Node &operator=(const Node &other) {
    if (this != &other) {
      name = other.name;
      visible = other.visible;
      active = other.active;
    }
    return *this;
  }

  // Checks if this node and all of its ancestors are visible
  bool isGlobalVisible() const {
    if (!visible) return false;
    const Node *curr = m_parent;
    while (curr) {
      if (!curr->visible) return false;
      curr = curr->m_parent;
    }
    return true;
  }

  // Returns active window viewport dimensions (defined after Window class)
  Vector2 getViewportSize() const;
  Vector2 getViewportCenter() const;

  // Returns active SceneTree (defined after SceneTree class)
  SceneTree *getTree() const;


  virtual void onReady() {}
  virtual void onProcess(float) {}
  virtual void onPhysicsProcess(float) {}
  virtual void onInput(const InputEvent &) {}
  virtual void onUnhandledInput(const InputEvent &) {}
  virtual void onDraw() {}
  virtual void onDestroy() {}

  // Attaches an existing child node to this node.
  std::shared_ptr<Node> addChild(std::shared_ptr<Node> child) {
    if (!child || child.get() == this) return nullptr;
    child->m_parent = this;
    m_children.push_back(child);
    child->onReady();
    return child;
  }

  // Instantiates and attaches a child node in a single line.
  template <typename T, typename... Args>
  std::shared_ptr<T> addChild(Args &&...args) {
    auto child = std::make_shared<T>(std::forward<Args>(args)...);
    addChild(std::static_pointer_cast<Node>(child));
    return child;
  }

  // Backwards-compatibility alias
  template <typename T, typename... Args>
  std::shared_ptr<T> spawnChild(Args &&...args) {
    return addChild<T>(std::forward<Args>(args)...);
  }

  // Removes a child node from this node.
  void removeChild(const std::shared_ptr<Node> &child) {
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
      (*it)->onDestroy();
      (*it)->m_parent = nullptr;
      m_children.erase(it);
    }
  }

  // Removes all child nodes from this node.
  void removeAllChildren() {
    for (auto &child : m_children) {
      if (child) {
        child->onDestroy();
        child->m_parent = nullptr;
      }
    }
    m_children.clear();
  }


  // Finds a child node by its name (shallow search).
  std::shared_ptr<Node> findChild(const std::string &childName) const {
    for (const auto &child : m_children) {
      if (child && child->name == childName) {
        return child;
      }
    }
    return nullptr;
  }

  // Recursively processes this node and all active children.
  void updateTree(float delta) {
    if (!active) return;
    onProcess(delta);
    for (auto &child : m_children) {
      if (child) {
        child->updateTree(delta);
      }
    }
  }

  // Recursively processes fixed physics updates on this node and all active children.
  void physicsUpdateTree(float delta) {
    if (!active) return;
    onPhysicsProcess(delta);
    for (auto &child : m_children) {
      if (child) {
        child->physicsUpdateTree(delta);
      }
    }
  }

  // Recursively saves physics transform state across the node hierarchy prior to fixed physics simulation step
  virtual void savePhysicsTransformState() {
    for (auto &child : m_children) {
      if (child) {
        child->savePhysicsTransformState();
      }
    }
  }

  // Recursively computes interpolated render transforms between previous and current physics states
  virtual void interpolatePhysicsTransforms(float alpha) {
    for (auto &child : m_children) {
      if (child) {
        child->interpolatePhysicsTransforms(alpha);
      }
    }
  }

  // Recursively draws this node and all visible children.
  void drawTree() {
    if (!visible || !active) return;
    onDraw();
    for (auto &child : m_children) {
      if (child) {
        child->drawTree();
      }
    }
  }

  // Recursively propagates input events to children in reverse order (front-to-back), then self.
  void inputTree(const InputEvent &event) {
    if (!active || !visible) return;

    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
      if (*it) {
        (*it)->inputTree(event);
        if (event.isHandled()) return;
      }
    }

    if (!event.isHandled()) {
      onInput(event);
    }
  }

  // Recursively propagates unhandled input events to children in reverse order (front-to-back), then self.
  void unhandledInputTree(const InputEvent &event) {
    if (!active || !visible) return;

    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
      if (*it) {
        (*it)->unhandledInputTree(event);
        if (event.isHandled()) return;
      }
    }

    if (!event.isHandled()) {
      onUnhandledInput(event);
    }
  }


  // Returns pointer to parent node (nullptr if root).
  Node *getParent() const { return m_parent; }

  // Returns list of children.
  const std::vector<std::shared_ptr<Node>> &getChildren() const {
    return m_children;
  }

private:
  Node *m_parent = nullptr;
  std::vector<std::shared_ptr<Node>> m_children;
};
