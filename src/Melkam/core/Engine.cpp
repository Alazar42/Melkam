#include <Melkam/core/Engine.hpp>
#include <Melkam/core/Application.hpp>
#include <Melkam/scene/Scene.hpp>
#include <chrono>

namespace Melkam
{
    Engine::Engine(const EngineConfig &config)
        : m_config(config), m_state(EngineState::Uninitialized)
    {
        init();
    }

    Engine::~Engine()
    {
        cleanup();
    }

    void Engine::init()
    {
        m_window = std::make_unique<Window>(*this);
    }

    void Engine::cleanup()
    {
        m_window.reset();
    }

    void Engine::shutdown()
    {
        m_state = EngineState::ShuttingDown;
        cleanup();
    }

    void Engine::run()
    {
        m_state = EngineState::Running;
        Application app;
        app.Run(*this); // Pass Engine by reference

        if (!m_window || !m_window->open())
        {
            m_state = EngineState::ShuttingDown;
            return;
        }

        auto lastTick = std::chrono::steady_clock::now();
        while (m_state == EngineState::Running && !m_window->shouldClose())
        {
            auto now = std::chrono::steady_clock::now();
            const float dt = std::chrono::duration<float>(now - lastTick).count();
            lastTick = now;

            m_window->pollEvents();

            if (m_activeScene)
            {
                m_activeScene->update(dt);
            }

            if (m_pendingScene)
            {
                m_activeScene = m_pendingScene;
                m_pendingScene.reset();
                m_reloadRequested = false;
            }
            else if (m_reloadRequested && m_activeScene)
            {
                m_activeScene->rebuild();
                m_reloadRequested = false;
            }

            m_window->swapBuffers();
        }

        shutdown();
    }

    std::shared_ptr<Scene> Engine::createScene(const std::string &name)
    {
        auto scene = std::make_shared<Scene>(name);
        if (!m_activeScene)
        {
            m_activeScene = scene;
        }
        return scene;
    }

    void Engine::setActiveScene(const std::shared_ptr<Scene> &scene)
    {
        m_activeScene = scene;
    }

    std::shared_ptr<Scene> Engine::activeScene() const
    {
        return m_activeScene;
    }

    void Engine::requestSceneChange(const std::shared_ptr<Scene> &scene)
    {
        m_pendingScene = scene;
    }

    void Engine::requestSceneReload()
    {
        m_reloadRequested = true;
    }

    EngineState Engine::state() const
    {
        return m_state;
    }

    const EngineConfig &Engine::config() const
    {
        return m_config;
    }
}
