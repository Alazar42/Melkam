#include <Melkam/platform/Input.hpp>

#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>

namespace Melkam
{
    namespace
    {
        struct Binding
        {
            Key key;
            float scale;
        };

        std::unordered_map<std::string, std::vector<Binding>> s_actions;
        MouseMode s_mouseMode = MouseMode::Visible;

        int toRaylibKey(Key key)
        {
            switch (key)
            {
            case Key::A:
                return KEY_A;
            case Key::D:
                return KEY_D;
            case Key::W:
                return KEY_W;
            case Key::S:
                return KEY_S;
            case Key::Left:
                return KEY_LEFT;
            case Key::Right:
                return KEY_RIGHT;
            case Key::Up:
                return KEY_UP;
            case Key::Down:
                return KEY_DOWN;
            case Key::Space:
                return KEY_SPACE;
            }
            return KEY_NULL;
        }

        int toRaylibMouseButton(MouseButton button)
        {
            switch (button)
            {
            case MouseButton::Left:
                return MOUSE_BUTTON_LEFT;
            case MouseButton::Right:
                return MOUSE_BUTTON_RIGHT;
            case MouseButton::Middle:
                return MOUSE_BUTTON_MIDDLE;
            case MouseButton::Side:
                return MOUSE_BUTTON_SIDE;
            case MouseButton::Extra:
                return MOUSE_BUTTON_EXTRA;
            }
            return MOUSE_BUTTON_LEFT;
        }

        float clampAxis(float value)
        {
            return std::max(-1.0f, std::min(1.0f, value));
        }
    }

    void Input::AddAction(std::string_view action)
    {
        s_actions.emplace(std::string(action), std::vector<Binding>{});
    }

    void Input::ClearAction(std::string_view action)
    {
        auto it = s_actions.find(std::string(action));
        if (it != s_actions.end())
        {
            it->second.clear();
        }
    }

    void Input::BindKey(std::string_view action, Key key, float scale)
    {
        auto &bindings = s_actions[std::string(action)];
        bindings.push_back({key, scale});
    }

    bool Input::IsActionPressed(std::string_view action)
    {
        auto it = s_actions.find(std::string(action));
        if (it == s_actions.end())
        {
            return false;
        }

        for (const auto &binding : it->second)
        {
            if (IsKeyDown(binding.key))
            {
                return true;
            }
        }
        return false;
    }

    bool Input::IsActionJustPressed(std::string_view action)
    {
        auto it = s_actions.find(std::string(action));
        if (it == s_actions.end())
        {
            return false;
        }

        for (const auto &binding : it->second)
        {
            if (IsKeyPressed(binding.key))
            {
                return true;
            }
        }
        return false;
    }

    bool Input::IsActionJustReleased(std::string_view action)
    {
        auto it = s_actions.find(std::string(action));
        if (it == s_actions.end())
        {
            return false;
        }

        for (const auto &binding : it->second)
        {
            if (IsKeyReleased(binding.key))
            {
                return true;
            }
        }
        return false;
    }

    float Input::GetActionStrength(std::string_view action)
    {
        auto it = s_actions.find(std::string(action));
        if (it == s_actions.end())
        {
            return 0.0f;
        }

        float value = 0.0f;
        for (const auto &binding : it->second)
        {
            if (IsKeyDown(binding.key))
            {
                value += binding.scale;
            }
        }
        return clampAxis(value);
    }

    Axis2D Input::GetActionAxis2D(std::string_view negativeX, std::string_view positiveX,
                                  std::string_view negativeY, std::string_view positiveY)
    {
        Axis2D axis;
        axis.x = GetActionStrength(positiveX) - GetActionStrength(negativeX);
        axis.y = GetActionStrength(positiveY) - GetActionStrength(negativeY);
        const float length = std::sqrt(axis.x * axis.x + axis.y * axis.y);
        if (length > 1.0f)
        {
            axis.x /= length;
            axis.y /= length;
        }
        return axis;
    }

    bool Input::IsKeyDown(Key key)
    {
        return ::IsKeyDown(toRaylibKey(key));
    }

    bool Input::IsKeyPressed(Key key)
    {
        return ::IsKeyPressed(toRaylibKey(key));
    }

    bool Input::IsKeyReleased(Key key)
    {
        return ::IsKeyReleased(toRaylibKey(key));
    }

    bool Input::IsMouseButtonDown(MouseButton button)
    {
        return ::IsMouseButtonDown(toRaylibMouseButton(button));
    }

    bool Input::IsMouseButtonPressed(MouseButton button)
    {
        return ::IsMouseButtonPressed(toRaylibMouseButton(button));
    }

    bool Input::IsMouseButtonReleased(MouseButton button)
    {
        return ::IsMouseButtonReleased(toRaylibMouseButton(button));
    }

    Axis2D Input::GetMousePosition()
    {
        const Vector2 pos = ::GetMousePosition();
        return Axis2D{pos.x, pos.y};
    }

    Axis2D Input::GetMouseDelta()
    {
        const Vector2 delta = ::GetMouseDelta();
        return Axis2D{delta.x, delta.y};
    }

    float Input::GetMouseWheelMove()
    {
        return ::GetMouseWheelMove();
    }

    void Input::SetMousePosition(float x, float y)
    {
        ::SetMousePosition(static_cast<int>(x), static_cast<int>(y));
    }

    void Input::SetMouseMode(MouseMode mode)
    {
        if (s_mouseMode == mode)
        {
            return;
        }

        switch (mode)
        {
        case MouseMode::Visible:
            ::EnableCursor();
            ::ShowCursor();
            break;
        case MouseMode::Hidden:
            ::EnableCursor();
            ::HideCursor();
            break;
        case MouseMode::Captured:
            ::DisableCursor();
            ::HideCursor();
            break;
        }

        s_mouseMode = mode;
    }

    MouseMode Input::GetMouseMode()
    {
        return s_mouseMode;
    }
}
