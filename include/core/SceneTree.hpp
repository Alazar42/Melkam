#pragma once

#include "core/Node.hpp"
#include "input.hpp"
#include "nodes/UI/Control.hpp"
#include <memory>

// Scene Tree manager (inspired by Godot's SceneTree) orchestrating scenes and dispatching frame cycles.
class SceneTree {
public:
  SceneTree() {
    m_root = std::make_shared<Node>("Root");
  }

  explicit SceneTree(std::shared_ptr<Node> rootNode) {
    m_root = rootNode ? std::move(rootNode) : std::make_shared<Node>("Root");
  }

  // Changes the active scene by replacing the scene root node.
  void changeScene(std::shared_ptr<Node> newRoot) {
    if (m_root) {
      m_root->onDestroy();
    }
    m_root = newRoot ? std::move(newRoot) : std::make_shared<Node>("Root");
    if (m_root) {
      m_root->onReady();
    }
  }

  // Processes per-frame updates across all nodes in the tree.
  void process(float delta) {
    if (m_root) {
      m_root->updateTree(delta);
    }
  }

  // Processes fixed-timestep physics updates across all nodes in the tree.
  void physicsProcess(float delta) {
    if (m_root) {
      m_root->physicsUpdateTree(delta);
    }
  }

  // Draws all visible nodes in the tree and deferred top-level UI popups/overlays.
  void draw() {
    if (m_root) {
      m_root->drawTree();
      Control::renderOverlays();
    }
  }

  // Dispatches an engine input event through top-level overlays then the scene tree.
  void input(const InputEvent &event) {
    if (Control::hasActiveOverlay()) {
      if (Control::processOverlayInput(event)) {
        const_cast<InputEvent &>(event).setHandled();
        return;
      }
    }
    if (m_root) {
      m_root->inputTree(event);
    }
  }

  // Dispatches an unhandled input event through the scene tree.
  void unhandledInput(const InputEvent &event) {
    if (m_root) {
      m_root->unhandledInputTree(event);
    }
  }

  // Returns the root node of the active scene tree.
  std::shared_ptr<Node> getRoot() const { return m_root; }

private:
  std::shared_ptr<Node> m_root;
};
