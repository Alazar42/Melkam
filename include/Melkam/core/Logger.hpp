#pragma once
#include <string_view>

namespace Melkam
{
    class Logger
    {
    public:
        enum class Level
        {
            Info,
            Warning,
            Error
        };

        static void Info(std::string_view message);
        static void Warn(std::string_view message);
        static void Error(std::string_view message);
        static void Log(Level level, std::string_view message);
    };
}
