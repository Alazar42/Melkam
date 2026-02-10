#include <Melkam/core/Logger.hpp>

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Melkam
{
    namespace
    {
#ifdef _WIN32
        void setColor(Logger::Level level, bool toStdErr)
        {
            HANDLE handle = GetStdHandle(toStdErr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
            if (handle == INVALID_HANDLE_VALUE)
            {
                return;
            }

            WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            switch (level)
            {
            case Logger::Level::Info:
                color = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
                break;
            case Logger::Level::Warning:
                color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                break;
            case Logger::Level::Error:
                color = FOREGROUND_RED | FOREGROUND_INTENSITY;
                break;
            }

            SetConsoleTextAttribute(handle, color);
        }

        void resetColor(bool toStdErr)
        {
            HANDLE handle = GetStdHandle(toStdErr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
            if (handle == INVALID_HANDLE_VALUE)
            {
                return;
            }

            SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
#else
        const char *colorCode(Logger::Level level)
        {
            switch (level)
            {
            case Logger::Level::Info:
                return "\033[36m";
            case Logger::Level::Warning:
                return "\033[33m";
            case Logger::Level::Error:
                return "\033[31m";
            }
            return "\033[0m";
        }
#endif

        const char *prefix(Logger::Level level)
        {
            switch (level)
            {
            case Logger::Level::Info:
                return "[Info] ";
            case Logger::Level::Warning:
                return "[Warn] ";
            case Logger::Level::Error:
                return "[Error] ";
            }
            return "";
        }
    }

    void Logger::Info(std::string_view message)
    {
        Log(Level::Info, message);
    }

    void Logger::Warn(std::string_view message)
    {
        Log(Level::Warning, message);
    }

    void Logger::Error(std::string_view message)
    {
        Log(Level::Error, message);
    }

    void Logger::Log(Level level, std::string_view message)
    {
        const bool toStdErr = level == Level::Error;
        std::ostream &out = toStdErr ? std::cerr : std::cout;

#ifdef _WIN32
        setColor(level, toStdErr);
        out << prefix(level) << message << "\n";
        resetColor(toStdErr);
#else
        out << colorCode(level) << prefix(level) << message << "\033[0m\n";
#endif
    }
}
