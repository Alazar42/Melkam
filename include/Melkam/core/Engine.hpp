#pragma once
#include <memory>
#include <string>
#include "../platform/Window.hpp"

namespace Melkam
{
    class Scene;
}

namespace Melkam
{
    enum class EngineState
    {
        Uninitialized,
        Running,
        ShuttingDown
    };

    struct EngineConfig
    {
        int width = 1280;
        int height = 720;
        const char* title = "Melkam Engine";
        bool resizable = true;
        bool maximized = false;
        bool fullscreen = false;
        bool borderless = false;
        bool vsync = false;
        bool highDpi = false;
    };

    class Engine
    {
    public:
        Engine(const EngineConfig& config);
        ~Engine();

        // Delete copy to avoid unique_ptr issues
        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        // Allow move if needed
        Engine(Engine&&) = default;
        Engine& operator=(Engine&&) = default;

        void run();
        void shutdown();

        std::shared_ptr<Scene> createScene(const std::string& name);
        void setActiveScene(const std::shared_ptr<Scene>& scene);
        std::shared_ptr<Scene> activeScene() const;

        void requestSceneChange(const std::shared_ptr<Scene>& scene);
        void requestSceneReload();

        EngineState state() const;
        const EngineConfig& config() const;

    private:
        void init();
        void cleanup();
        
        EngineConfig m_config;
        EngineState m_state;

        std::unique_ptr<Window> m_window;
        std::shared_ptr<Scene> m_activeScene;
        std::shared_ptr<Scene> m_pendingScene;
        bool m_reloadRequested = false;
    };
}
