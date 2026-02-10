#pragma once

#include <string_view>

namespace Melkam
{
    enum class Key
    {
        A,
        D,
        W,
        S,
        Left,
        Right,
        Up,
        Down,
        Space
    };

    enum class MouseButton
    {
        Left,
        Right,
        Middle,
        Side,
        Extra
    };

    enum class MouseMode
    {
        Visible,
        Hidden,
        Captured
    };

    struct Axis2D
    {
        float x = 0.0f;
        float y = 0.0f;
    };

    class Input
    {
    public:
        static void AddAction(std::string_view action);
        static void ClearAction(std::string_view action);
        static void BindKey(std::string_view action, Key key, float scale = 1.0f);

        static bool IsActionPressed(std::string_view action);
        static bool IsActionJustPressed(std::string_view action);
        static bool IsActionJustReleased(std::string_view action);
        static float GetActionStrength(std::string_view action);
        static Axis2D GetActionAxis2D(std::string_view negativeX, std::string_view positiveX,
                                      std::string_view negativeY, std::string_view positiveY);

        static bool IsKeyDown(Key key);
        static bool IsKeyPressed(Key key);
        static bool IsKeyReleased(Key key);

        static bool IsMouseButtonDown(MouseButton button);
        static bool IsMouseButtonPressed(MouseButton button);
        static bool IsMouseButtonReleased(MouseButton button);

        static Axis2D GetMousePosition();
        static Axis2D GetMouseDelta();
        static float GetMouseWheelMove();
        static void SetMousePosition(float x, float y);

        static void SetMouseMode(MouseMode mode);
        static MouseMode GetMouseMode();
    };
}
