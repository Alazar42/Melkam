#pragma once

namespace Melkam
{
    class Engine;

    class Window
    {
    private:
        Engine &m_engine;
        bool m_isOpen = false;
        bool m_shouldClose = false;

    public:
        Window(Engine &engine);
        ~Window();

        void pollEvents();
        void swapBuffers();
        void close();
        bool open();
        bool isOpen() const;
        bool shouldClose() const;
    };
}
