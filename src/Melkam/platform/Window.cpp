#include <Melkam/core/Engine.hpp>
#include <Melkam/core/Logger.hpp>
#include <raylib.h>
#include <cmath>

namespace Melkam
{
    Window::Window(Engine &engine) : m_engine(engine)
    {
        // Constructor implementation
    }

    Window::~Window()
    {
        close();
    }

    void Window::pollEvents()
    {
        if (!m_isOpen)
        {
            return;
        }

        if (WindowShouldClose())
        {
            m_shouldClose = true;
        }
    }

    void Window::swapBuffers()
    {
        if (!m_isOpen)
        {
            return;
        }
    }

    void Window::close()
    {
        if (m_isOpen)
        {
            CloseWindow();
            m_isOpen = false;
        }
    }

    bool Window::open()
    {
        if (m_isOpen)
        {
            return true;
        }

        int configFlags = 0;
        if (m_engine.config().resizable)
        {
            configFlags |= FLAG_WINDOW_RESIZABLE;
        }
        if (m_engine.config().borderless)
        {
            configFlags |= FLAG_WINDOW_UNDECORATED;
        }
        if (m_engine.config().highDpi)
        {
            configFlags |= FLAG_WINDOW_HIGHDPI;
        }
        if (m_engine.config().vsync)
        {
            configFlags |= FLAG_VSYNC_HINT;
        }
        if (configFlags != 0)
        {
            SetConfigFlags(configFlags);
        }

        InitWindow(m_engine.config().width, m_engine.config().height, m_engine.config().title);
        if (!IsWindowReady())
        {
            Logger::Error("Raylib failed to create window.");
            return false;
        }

        if (m_engine.config().maximized)
        {
            MaximizeWindow();
        }

        if (m_engine.config().fullscreen)
        {
            ToggleFullscreen();
        }

        SetTargetFPS(60);

        m_isOpen = true;
        m_shouldClose = false;
        return true;
    }

    bool Window::isOpen() const
    {
        return m_isOpen;
    }

    bool Window::shouldClose() const
    {
        return m_shouldClose;
    }
}