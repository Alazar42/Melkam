#pragma once

#include <functional>

#include <Melkam/scene/Scene.hpp>

namespace Melkam
{
    using UiButtonCallback = std::function<void(Entity button)>;

    void UpdateUi(Scene &scene, int screenWidth, int screenHeight);
    void DrawUi(Scene &scene, int screenWidth, int screenHeight);
    void ConnectButtonPressed(Entity button, UiButtonCallback callback);
    void RegisterUiSystems(Scene &scene);
    void SetUiThemeStyle(const std::string &stylePath);
    void SetUiThemeMelkam();
}