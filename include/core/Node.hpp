#pragma once

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

  // Returns active window viewport dimensions (defined after Window class)
  Vector2 getViewportSize() const;
  Vector2 getViewportCenter() const;

  virtual void onReady() {}
  virtual void onProcess(float) {}
  virtual void onPhysicsProcess(float) {}
  virtual void onInput(const InputEvent &) {}
  virtual void onUnhandledInput(const InputEvent &) {}
  virtual void onDraw() {}
  virtual void onDestroy() {}

  // Attaches a child node to this node.
  void addChild(std::shared_ptr<Node> child) {
    if (!child || child.get() == this) return;
    child->m_parent = this;
    m_children.push_back(child);
    child->onReady();
  }

  // Instantiates and attaches a child node in a single line.
  template <typename T, typename... Args>
  std::shared_ptr<T> spawnChild(Args &&...args) {
    auto child = std::make_shared<T>(std::forward<Args>(args)...);
    addChild(child);
    return child;
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

  // Recursively draws this node and all visible children.
  void drawTree() {
    if (!visible) return;
    onDraw();
    for (auto &child : m_children) {
      if (child) {
        child->drawTree();
      }
    }
  }

  // Recursively propagates input events to this node and all active children.
  void inputTree(const InputEvent &event) {
    if (!active) return;
    onInput(event);
    for (auto &child : m_children) {
      if (child) {
        child->inputTree(event);
      }
    }
  }

  // Recursively propagates unhandled input events to this node and all active children.
  void unhandledInputTree(const InputEvent &event) {
    if (!active) return;
    onUnhandledInput(event);
    for (auto &child : m_children) {
      if (child) {
        child->unhandledInputTree(event);
      }
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
