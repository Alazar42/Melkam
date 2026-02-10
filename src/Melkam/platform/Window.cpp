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

        InitWindow(m_engine.config().width, m_engine.config().height, m_engine.config().title);
        if (!IsWindowReady())
        {
            Logger::Error("Raylib failed to create window.");
            return false;
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