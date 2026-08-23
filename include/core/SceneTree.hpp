#pragma once

#include "core/Node.hpp"
#include "input.hpp"
#include "nodes/UI/Control.hpp"
#include <memory>
#include <utility>

// Scene Tree manager (inspired by Godot's SceneTree) orchestrating scenes and dispatching frame cycles.
class SceneTree {
public:
  inline static SceneTree *s_currentTree = nullptr;

  SceneTree() {
    s_currentTree = this;
    m_root = std::make_shared<Node>("Root");
  }

  explicit SceneTree(std::shared_ptr<Node> rootNode) {
    s_currentTree = this;
    m_root = rootNode ? std::move(rootNode) : std::make_shared<Node>("Root");
  }

  static SceneTree *getCurrent() { return s_currentTree; }

  // Changes the active scene by replacing the scene root node (deferred to frame boundary like Godot).
  void changeScene(std::shared_ptr<Node> newRoot) {
    m_pendingScene = std::move(newRoot);
    m_hasPendingScene = true;
  }

  void changeSceneToNode(std::shared_ptr<Node> newRoot) {
    changeScene(std::move(newRoot));
  }

  // Flushes any queued scene changes safely outside node iterations
  void flushPendingSceneChange() {
    if (m_hasPendingScene) {
      m_hasPendingScene = false;
      Control::clearAllOverlays();
      if (m_root) {
        m_root->onDestroy();
      }
      m_root = m_pendingScene ? std::move(m_pendingScene) : std::make_shared<Node>("Root");
      m_pendingScene = nullptr;
      if (m_root) {
        m_root->onReady();
      }
    }
  }


  void quit() {
    std::exit(0);
  }

  // Processes per-frame updates across all nodes in the tree.
  void process(float delta) {
    flushPendingSceneChange();
    if (m_root) {
      m_root->updateTree(delta);
    }
    flushPendingSceneChange();
  }

  // Processes fixed-timestep physics updates across all nodes in the tree.
  void physicsProcess(float delta) {
    flushPendingSceneChange();
    if (m_root) {
      m_root->physicsUpdateTree(delta);
    }
    flushPendingSceneChange();
  }

  // Draws all visible nodes in the tree and deferred top-level UI popups/overlays.
  void draw() {
    flushPendingSceneChange();
    if (m_root) {
      m_root->drawTree();
      Control::renderOverlays();
    }
  }

  // Dispatches an engine input event through top-level overlays then the scene tree.
  void input(const InputEvent &event) {
    flushPendingSceneChange();
    if (Control::hasActiveOverlay()) {
      if (Control::processOverlayInput(event)) {
        const_cast<InputEvent &>(event).setHandled();
        flushPendingSceneChange();
        return;
      }
    }
    if (m_root) {
      m_root->inputTree(event);
    }
    flushPendingSceneChange();
  }

  // Dispatches an unhandled input event through the scene tree.
  void unhandledInput(const InputEvent &event) {
    flushPendingSceneChange();
    if (m_root) {
      m_root->unhandledInputTree(event);
    }
    flushPendingSceneChange();
  }

  // Returns the root node of the active scene tree.
  std::shared_ptr<Node> getRoot() const { return m_root; }

private:
  std::shared_ptr<Node> m_root;
  std::shared_ptr<Node> m_pendingScene = nullptr;
  bool m_hasPendingScene = false;
};

inline SceneTree *Node::getTree() const {
  return SceneTree::getCurrent();
}
