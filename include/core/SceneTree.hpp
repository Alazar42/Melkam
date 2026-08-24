#pragma once

#include "animation/Tween.hpp"
#include "core/Node.hpp"
#include "core/Task.hpp"
#include "input.hpp"
#include "nodes/2D/Camera2D.hpp"
#include "nodes/UI/Control.hpp"
#include "renderers/Renderer2D.hpp"

#include <memory>
#include <utility>
#include <vector>

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

  // Creates and registers a new Tween bound to the scene tree (like Godot 4 create_tween())
  std::shared_ptr<Tween> createTween() {
    auto tween = std::make_shared<Tween>();
    m_tweens.push_back(tween);
    return tween;
  }

  // Creates and registers a new Godot-style SceneTreeTimer (like Godot 4 get_tree().create_timer(1.5))
  // Can be awaited directly: 'await getTree()->createTimer(1.5f);'
  std::shared_ptr<SceneTreeTimer> createTimer(float seconds) {
    return std::make_shared<SceneTreeTimer>(seconds);
  }

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
      Camera2D::clearCurrentCamera();
      m_tweens.clear();
      CoroutineScheduler::get().clear();

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

  // Processes per-frame updates across all nodes in the tree, active tweens, and coroutines.
  void process(float delta) {
    flushPendingSceneChange();

    // 1. Process active coroutine timers and awaiters
    CoroutineScheduler::get().process(delta);

    // 2. Process active tweens safely without iterator invalidation
    if (!m_tweens.empty()) {
      auto activeTweens = std::move(m_tweens);
      m_tweens.clear();
      std::vector<std::shared_ptr<Tween>> remainingTweens;
      remainingTweens.reserve(activeTweens.size());

      for (auto &t : activeTweens) {
        if (t && !t->process(delta) && !t->isKilled()) {
          remainingTweens.push_back(std::move(t));
        }
      }

      if (!m_tweens.empty()) {
        remainingTweens.insert(remainingTweens.end(),
                               std::make_move_iterator(m_tweens.begin()),
                               std::make_move_iterator(m_tweens.end()));
      }
      m_tweens = std::move(remainingTweens);
    }

    // 3. Process active node tree
    if (m_root) {
      m_root->updateTree(delta);
    }
    flushPendingSceneChange();
  }


  // Saves physics transform state across all nodes before a fixed physics simulation step
  void savePhysicsTransformState() {
    flushPendingSceneChange();
    if (m_root) {
      m_root->savePhysicsTransformState();
    }
  }

  // Interpolates physics transforms between simulation steps
  void interpolatePhysicsTransforms(float alpha) {
    if (m_root) {
      m_root->interpolatePhysicsTransforms(alpha);
    }
  }

  // Processes fixed-timestep physics updates across all nodes in the tree and physics awaiters.
  void physicsProcess(float delta) {
    flushPendingSceneChange();

    // 1. Process active physics tick awaiters
    CoroutineScheduler::get().physicsProcess(delta);

    // 2. Process active node physics
    if (m_root) {
      m_root->physicsUpdateTree(delta);
    }
    flushPendingSceneChange();
  }

  // Draws all visible nodes in the tree with render transform interpolation and deferred UI overlays
  void draw(float alpha = 1.0f) {
    flushPendingSceneChange();
    if (m_root) {
      Node2D::s_inRenderPass = true;
      m_root->interpolatePhysicsTransforms(alpha);
      m_root->drawTree();
      Control::renderOverlays();
      Node2D::s_inRenderPass = false;
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
  std::vector<std::shared_ptr<Tween>> m_tweens;
};

inline SceneTree *Node::getTree() const {
  return SceneTree::getCurrent();
}

