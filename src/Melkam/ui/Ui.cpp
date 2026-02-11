#include <Melkam/ui/Ui.hpp>

#include <Melkam/scene/Components.hpp>
#include <Melkam/scene/Entity.hpp>
#include <Melkam/scene/System.hpp>

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "raylib/raygui.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Melkam
{
    namespace
    {
        struct UiRect
        {
            float x;
            float y;
            float w;
            float h;
        };

        struct UiDrawItem
        {
            Entity entity;
            UiRect rect;
            int layer;
            int depth;
        };

        struct UiButtonItem
        {
            Entity entity;
            UiRect rect;
            int layer;
            int depth;
        };

        struct UiTextEditItem
        {
            Entity entity;
            UiRect rect;
            int layer;
            int depth;
        };

        std::unordered_map<EntityId, std::vector<UiButtonCallback>> s_buttonCallbacks;
        EntityId s_focusedTextEdit = InvalidEntity;
        std::string s_globalStylePath;
        std::string s_currentStylePath;
        bool s_pendingMelkamTheme = false;
        Font s_uiFont = {};
        bool s_uiFontLoaded = false;

        Color toColor(const unsigned char color[4])
        {
            return {color[0], color[1], color[2], color[3]};
        }

        bool hasFlag(std::uint32_t flags, UiSizeFlags flag)
        {
            return (flags & static_cast<std::uint32_t>(flag)) != 0u;
        }

        int packColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
        {
            return (static_cast<int>(r) << 24) | (static_cast<int>(g) << 16) |
                   (static_cast<int>(b) << 8) | static_cast<int>(a);
        }

        std::string resolveMelkamFontPath()
        {
            std::filesystem::path base(GetApplicationDirectory());
            std::filesystem::path fontPath = base / ".." / ".." / "fonts" / "SNPro" / "SNPro-Regular.ttf";
            fontPath = fontPath.lexically_normal();
            return fontPath.string();
        }

        void applyMelkamFont()
        {
            if (s_uiFontLoaded)
            {
                GuiSetFont(s_uiFont);
                return;
            }

            const std::string fontPath = resolveMelkamFontPath();
            if (!FileExists(fontPath.c_str()))
            {
                return;
            }

            s_uiFont = LoadFontEx(fontPath.c_str(), 24, nullptr, 0);
            s_uiFontLoaded = s_uiFont.texture.id != 0;
            if (s_uiFontLoaded)
            {
                GenTextureMipmaps(&s_uiFont.texture);
                SetTextureFilter(s_uiFont.texture, TEXTURE_FILTER_BILINEAR);
                GuiSetFont(s_uiFont);
            }
        }

        void applyStylePath(const std::string &stylePath, bool useDefault)
        {
            if (useDefault || stylePath == "default")
            {
                if (s_currentStylePath != "default")
                {
                    GuiLoadStyleDefault();
                    s_currentStylePath = "default";
                }
                return;
            }

            if (stylePath.empty() || stylePath == s_currentStylePath)
            {
                return;
            }

            GuiLoadStyle(stylePath.c_str());
            s_currentStylePath = stylePath;
        }

        void applyStyleFor(Entity entity)
        {
            std::string stylePath;
            bool useDefault = false;

            Entity current = entity;
            while (current.isValid())
            {
                if (auto *style = current.tryGetComponent<UiStyleComponent>())
                {
                    stylePath = style->stylePath;
                    useDefault = style->useDefault;
                    break;
                }
                current = current.parent();
            }

            if (stylePath.empty() && !useDefault)
            {
                stylePath = s_globalStylePath;
            }

            applyStylePath(stylePath, useDefault);
        }

        void applyPendingMelkamTheme()
        {
            if (!s_pendingMelkamTheme)
            {
                return;
            }

            if (!IsWindowReady())
            {
                return;
            }

            s_pendingMelkamTheme = false;
            SetUiThemeMelkam();
        }

        Entity findScrollContainer(Entity entity)
        {
            Entity current = entity;
            while (current.isValid())
            {
                if (current.hasComponent<ScrollContainerComponent>())
                {
                    return current;
                }
                current = current.parent();
            }
            return Entity();
        }

        UiRect resolveRect(Scene &scene, Entity entity, const UiRect &viewport,
                   std::unordered_map<EntityId, UiRect> &cache,
                   std::unordered_map<EntityId, UiRect> &containerCache)
        {
            auto cacheIt = cache.find(entity.id());
            if (cacheIt != cache.end())
            {
                return cacheIt->second;
            }

            UiRect parentRect = viewport;
            Entity parent = entity.parent();
            if (parent.isValid())
            {
                if (parent.hasComponent<ControlComponent>())
                {
                    parentRect = resolveRect(scene, parent, viewport, cache, containerCache);
                }

                if (parent.hasComponent<VBoxContainerComponent>() || parent.hasComponent<HBoxContainerComponent>())
                {
                    auto it = containerCache.find(entity.id());
                    if (it == containerCache.end())
                    {
                        const bool isVBox = parent.hasComponent<VBoxContainerComponent>();
                        Entity scrollEntity = findScrollContainer(parent);
                        auto *scrollContainer = scrollEntity.tryGetComponent<ScrollContainerComponent>();
                        const float padding = isVBox ? parent.tryGetComponent<VBoxContainerComponent>()->padding
                                                     : parent.tryGetComponent<HBoxContainerComponent>()->padding;
                        const float spacing = isVBox ? parent.tryGetComponent<VBoxContainerComponent>()->spacing
                                                     : parent.tryGetComponent<HBoxContainerComponent>()->spacing;

                        float cursorX = parentRect.x + padding;
                        float cursorY = parentRect.y + padding;
                        const float availableW = std::max(0.0f, parentRect.w - padding * 2.0f);
                        const float availableH = std::max(0.0f, parentRect.h - padding * 2.0f);

                        std::vector<Entity> layoutChildren;
                        layoutChildren.reserve(parent.children().size());
                        for (auto &child : parent.children())
                        {
                            if (child.hasComponent<ControlComponent>())
                            {
                                layoutChildren.push_back(child);
                            }
                        }

                        float totalFixed = 0.0f;
                        int expandCount = 0;
                        for (auto &child : layoutChildren)
                        {
                            auto *childControl = child.tryGetComponent<ControlComponent>();
                            if (!childControl)
                            {
                                continue;
                            }

                            if (isVBox)
                            {
                                const float baseH = std::max(childControl->minSize[1], 0.0f);
                                totalFixed += baseH;
                                if (hasFlag(childControl->sizeFlagsV, UiSizeFlags::Expand))
                                {
                                    ++expandCount;
                                }
                            }
                            else
                            {
                                const float baseW = std::max(childControl->minSize[0], 0.0f);
                                totalFixed += baseW;
                                if (hasFlag(childControl->sizeFlagsH, UiSizeFlags::Expand))
                                {
                                    ++expandCount;
                                }
                            }
                        }

                        if (!layoutChildren.empty())
                        {
                            totalFixed += spacing * static_cast<float>(layoutChildren.size() - 1);
                        }

                        const float availableMain = isVBox ? availableH : availableW;
                        const float extra = std::max(0.0f, availableMain - totalFixed);
                        const float extraPer = (expandCount > 0) ? (extra / static_cast<float>(expandCount)) : 0.0f;

                        float maxRight = parentRect.x + padding;
                        float maxBottom = parentRect.y + padding;

                        for (auto &child : layoutChildren)
                        {
                            auto *childControl = child.tryGetComponent<ControlComponent>();
                            if (!childControl)
                            {
                                continue;
                            }

                            UiRect childRect = parentRect;
                            if (isVBox)
                            {
                                float height = std::max(childControl->minSize[1], 0.0f);
                                if (hasFlag(childControl->sizeFlagsV, UiSizeFlags::Expand))
                                {
                                    height += extraPer;
                                }

                                float width = availableW;
                                if (!hasFlag(childControl->sizeFlagsH, UiSizeFlags::Fill) &&
                                    !hasFlag(childControl->sizeFlagsH, UiSizeFlags::Expand))
                                {
                                    width = std::max(0.0f, childControl->minSize[0]);
                                }

                                childRect.x = parentRect.x + padding;
                                childRect.y = cursorY;
                                childRect.w = width > 0.0f ? width : availableW;
                                childRect.h = height > 0.0f ? height : 0.0f;
                                cursorY += childRect.h + spacing;
                                maxRight = std::max(maxRight, childRect.x + childRect.w);
                                maxBottom = std::max(maxBottom, childRect.y + childRect.h);
                            }
                            else
                            {
                                float width = std::max(childControl->minSize[0], 0.0f);
                                if (hasFlag(childControl->sizeFlagsH, UiSizeFlags::Expand))
                                {
                                    width += extraPer;
                                }

                                float height = availableH;
                                if (!hasFlag(childControl->sizeFlagsV, UiSizeFlags::Fill) &&
                                    !hasFlag(childControl->sizeFlagsV, UiSizeFlags::Expand))
                                {
                                    height = std::max(0.0f, childControl->minSize[1]);
                                }

                                childRect.x = cursorX;
                                childRect.y = parentRect.y + padding;
                                childRect.w = width > 0.0f ? width : 0.0f;
                                childRect.h = height > 0.0f ? height : availableH;
                                cursorX += childRect.w + spacing;
                                maxRight = std::max(maxRight, childRect.x + childRect.w);
                                maxBottom = std::max(maxBottom, childRect.y + childRect.h);
                            }

                            if (scrollContainer)
                            {
                                childRect.x -= scrollContainer->scrollX;
                                childRect.y -= scrollContainer->scrollY;
                            }

                            containerCache.emplace(child.id(), childRect);
                        }

                        if (scrollContainer)
                        {
                            scrollContainer->contentWidth = std::max(0.0f, maxRight - parentRect.x);
                            scrollContainer->contentHeight = std::max(0.0f, maxBottom - parentRect.y);
                        }

                        it = containerCache.find(entity.id());
                    }

                    if (it != containerCache.end())
                    {
                        auto *control = entity.tryGetComponent<ControlComponent>();
                        UiRect rect = it->second;
                        if (control)
                        {
                            control->rectX = rect.x;
                            control->rectY = rect.y;
                            control->rectW = rect.w;
                            control->rectH = rect.h;
                        }
                        cache.emplace(entity.id(), rect);
                        return rect;
                    }
                }
            }

            auto *control = entity.tryGetComponent<ControlComponent>();
            UiRect rect = parentRect;
            if (control)
            {
                rect.x = parentRect.x + parentRect.w * control->anchorLeft + control->offsetLeft;
                rect.y = parentRect.y + parentRect.h * control->anchorTop + control->offsetTop;
                rect.w = parentRect.w * (control->anchorRight - control->anchorLeft) + (control->offsetRight - control->offsetLeft);
                rect.h = parentRect.h * (control->anchorBottom - control->anchorTop) + (control->offsetBottom - control->offsetTop);

                rect.w = std::max(rect.w, control->minSize[0]);
                rect.h = std::max(rect.h, control->minSize[1]);

                // Clamp to parent bounds so controls never overflow the window/parent.
                rect.x = std::max(rect.x, parentRect.x);
                rect.y = std::max(rect.y, parentRect.y);
                rect.w = std::min(rect.w, parentRect.w - (rect.x - parentRect.x));
                rect.h = std::min(rect.h, parentRect.h - (rect.y - parentRect.y));
                rect.w = std::max(0.0f, rect.w);
                rect.h = std::max(0.0f, rect.h);

                control->rectX = rect.x;
                control->rectY = rect.y;
                control->rectW = rect.w;
                control->rectH = rect.h;
            }

            cache.emplace(entity.id(), rect);
            return rect;
        }

        bool contains(const UiRect &rect, float x, float y)
        {
            return x >= rect.x && y >= rect.y && x <= rect.x + rect.w && y <= rect.y + rect.h;
        }

        int resolveLayer(Entity entity)
        {
            Entity current = entity;
            while (current.isValid())
            {
                if (auto *layer = current.tryGetComponent<CanvasLayerComponent>())
                {
                    return layer->layer;
                }
                current = current.parent();
            }
            return 0;
        }

        int resolveDepth(Entity entity)
        {
            int depth = 0;
            Entity current = entity.parent();
            while (current.isValid())
            {
                ++depth;
                current = current.parent();
            }
            return depth;
        }

        void drawTextInRect(const std::string &text, const UiRect &rect, int fontSize, Color color)
        {
            const float x = rect.x + 6.0f;
            const float y = rect.y + 4.0f;
            DrawText(text.c_str(), static_cast<int>(x), static_cast<int>(y), fontSize, color);
        }

        void drawTextureRect(const Texture2D &texture, const UiRect &rect, bool keepAspect, Color tint)
        {
            if (texture.id == 0)
            {
                return;
            }

            Rectangle dest = {rect.x, rect.y, rect.w, rect.h};
            Rectangle src = {0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height)};
            Vector2 origin = {0.0f, 0.0f};

            if (keepAspect)
            {
                const float texAspect = static_cast<float>(texture.width) / static_cast<float>(texture.height);
                const float rectAspect = rect.w / rect.h;
                float drawW = rect.w;
                float drawH = rect.h;
                if (texAspect > rectAspect)
                {
                    drawH = rect.w / texAspect;
                }
                else
                {
                    drawW = rect.h * texAspect;
                }
                dest.x = rect.x + (rect.w - drawW) * 0.5f;
                dest.y = rect.y + (rect.h - drawH) * 0.5f;
                dest.width = drawW;
                dest.height = drawH;
            }

            DrawTexturePro(texture, src, dest, origin, 0.0f, tint);
        }
    }

    void UpdateUi(Scene &scene, int screenWidth, int screenHeight)
    {
        UiRect viewport = {0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight)};
        std::unordered_map<EntityId, UiRect> rectCache;
        std::unordered_map<EntityId, UiRect> containerCache;
        std::vector<UiTextEditItem> textEdits;

        applyPendingMelkamTheme();
        std::unordered_map<EntityId, UiRect> scrollRects;
        for (auto &entity : scene.view<ControlComponent>())
        {
            auto *control = entity.tryGetComponent<ControlComponent>();
            if (!control || !control->visible)
            {
                continue;
            }

            UiRect rect = resolveRect(scene, entity, viewport, rectCache, containerCache);
            if (entity.hasComponent<ScrollContainerComponent>())
            {
                scrollRects.emplace(entity.id(), rect);
            }
        }

        for (auto &entity : scene.view<ControlComponent, TextEditComponent>())
        {
            auto *control = entity.tryGetComponent<ControlComponent>();
            auto *textEdit = entity.tryGetComponent<TextEditComponent>();
            if (!control || !textEdit || !control->visible)
            {
                continue;
            }

            UiRect rect = resolveRect(scene, entity, viewport, rectCache, containerCache);
            UiTextEditItem item{entity, rect, resolveLayer(entity), resolveDepth(entity)};
            textEdits.push_back(item);
        }

        std::sort(textEdits.begin(), textEdits.end(), [](const UiTextEditItem &a, const UiTextEditItem &b)
                  {
                      if (a.layer != b.layer)
                      {
                          return a.layer < b.layer;
                      }
                      return a.depth < b.depth;
                  });

        const Vector2 mouse = GetMousePosition();
        const bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        if (mousePressed)
        {
            EntityId focused = InvalidEntity;
            for (const auto &item : textEdits)
            {
                if (contains(item.rect, mouse.x, mouse.y))
                {
                    focused = item.entity.id();
                }
            }
            s_focusedTextEdit = focused;
        }

        const float wheel = GetMouseWheelMove();
        if (std::abs(wheel) > 0.001f)
        {
            for (auto &pair : scrollRects)
            {
                if (!contains(pair.second, mouse.x, mouse.y))
                {
                    continue;
                }

                Entity scrollEntity(&scene, pair.first);
                auto *scroll = scrollEntity.tryGetComponent<ScrollContainerComponent>();
                if (!scroll)
                {
                    continue;
                }

                scroll->scrollY -= wheel * scroll->wheelSpeed;
                const float maxScrollY = std::max(0.0f, scroll->contentHeight - pair.second.h);
                scroll->scrollY = std::max(0.0f, std::min(scroll->scrollY, maxScrollY));
            }
        }

        (void)mouse;
    }

    void DrawUi(Scene &scene, int screenWidth, int screenHeight)
    {
        UiRect viewport = {0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight)};
        std::unordered_map<EntityId, UiRect> rectCache;
        std::unordered_map<EntityId, UiRect> containerCache;
        std::vector<UiDrawItem> drawItems;
        std::unordered_map<EntityId, UiRect> scrollRects;

        applyPendingMelkamTheme();
        for (auto &entity : scene.view<ControlComponent>())
        {
            auto *control = entity.tryGetComponent<ControlComponent>();
            if (!control || !control->visible)
            {
                continue;
            }

            UiRect rect = resolveRect(scene, entity, viewport, rectCache, containerCache);
            UiDrawItem item{entity, rect, resolveLayer(entity), resolveDepth(entity)};
            drawItems.push_back(item);

            if (entity.hasComponent<ScrollContainerComponent>())
            {
                scrollRects.emplace(entity.id(), rect);
            }
        }

        std::sort(drawItems.begin(), drawItems.end(), [](const UiDrawItem &a, const UiDrawItem &b)
                  {
                      if (a.layer != b.layer)
                      {
                          return a.layer < b.layer;
                      }
                      return a.depth < b.depth;
                  });

        static std::unordered_map<std::string, Texture2D> textureCache;

        for (const auto &item : drawItems)
        {
            auto *control = item.entity.tryGetComponent<ControlComponent>();
            if (!control || !control->visible)
            {
                continue;
            }
        }

        auto beginClip = [&](const Entity &entity)
        {
            Entity scrollParent = findScrollContainer(entity);
            if (!scrollParent.isValid())
            {
                return false;
            }

            auto it = scrollRects.find(scrollParent.id());
            if (it == scrollRects.end())
            {
                return false;
            }

            const UiRect &clip = it->second;
            BeginScissorMode(static_cast<int>(clip.x), static_cast<int>(clip.y),
                             static_cast<int>(clip.w), static_cast<int>(clip.h));
            return true;
        };

        auto endClip = [&](bool clipped)
        {
            if (clipped)
            {
                EndScissorMode();
            }
        };

        for (const auto &item : drawItems)
        {
            bool clipped = beginClip(item.entity);

            if (auto *colorRect = item.entity.tryGetComponent<ColorRectComponent>())
            {
                DrawRectangle(static_cast<int>(item.rect.x), static_cast<int>(item.rect.y),
                              static_cast<int>(item.rect.w), static_cast<int>(item.rect.h),
                              toColor(colorRect->color));
            }

            endClip(clipped);
        }

        for (const auto &item : drawItems)
        {
            bool clipped = beginClip(item.entity);

            if (auto *textureRect = item.entity.tryGetComponent<TextureRectComponent>())
            {
                Texture2D texture = {};
                if (!textureRect->texturePath.empty())
                {
                    auto it = textureCache.find(textureRect->texturePath);
                    if (it == textureCache.end())
                    {
                        texture = LoadTexture(textureRect->texturePath.c_str());
                        textureCache.emplace(textureRect->texturePath, texture);
                    }
                    else
                    {
                        texture = it->second;
                    }
                }
                drawTextureRect(texture, item.rect, textureRect->keepAspect, toColor(textureRect->tint));
            }

            endClip(clipped);
        }

        for (auto &item : drawItems)
        {
            bool clipped = beginClip(item.entity);
            if (auto *windowBox = item.entity.tryGetComponent<WindowBoxComponent>())
            {
                if (!windowBox->open)
                {
                    continue;
                }
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                if (GuiWindowBox(rect, windowBox->title.c_str()))
                {
                    windowBox->open = false;
                }
            }

                if (auto *panel = item.entity.tryGetComponent<PanelComponent>())
            {
                (void)panel;
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiPanel(rect, "");
            }

            if (auto *groupBox = item.entity.tryGetComponent<GroupBoxComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiGroupBox(rect, groupBox->text.c_str());
            }

            if (auto *line = item.entity.tryGetComponent<LineComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiLine(rect, line->text.c_str());
            }

            if (auto *status = item.entity.tryGetComponent<StatusBarComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiStatusBar(rect, status->text.c_str());
            }

            if (auto *dummy = item.entity.tryGetComponent<DummyRecComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiDummyRec(rect, dummy->text.c_str());
            }

            if (auto *grid = item.entity.tryGetComponent<GridComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                Vector2 cell = {0.0f, 0.0f};
                GuiGrid(rect, "", grid->spacing, grid->subdivs, &cell);
                grid->mouseCell[0] = cell.x;
                grid->mouseCell[1] = cell.y;
            }

            if (auto *scrollPanel = item.entity.tryGetComponent<ScrollPanelComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                Rectangle content = {0.0f, 0.0f, scrollPanel->contentWidth, scrollPanel->contentHeight};
                Vector2 scroll = {scrollPanel->scrollX, scrollPanel->scrollY};
                Rectangle view = {};
                GuiScrollPanel(rect, "", content, &scroll, &view);
                scrollPanel->scrollX = scroll.x;
                scrollPanel->scrollY = scroll.y;
                scrollPanel->viewX = view.x;
                scrollPanel->viewY = view.y;
                scrollPanel->viewW = view.width;
                scrollPanel->viewH = view.height;
            }

            if (auto *tabBar = item.entity.tryGetComponent<TabBarComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                std::vector<std::string> parts;
                std::vector<const char *> labels;
                std::string current;
                for (char ch : tabBar->items)
                {
                    if (ch == ';')
                    {
                        parts.push_back(current);
                        current.clear();
                    }
                    else
                    {
                        current.push_back(ch);
                    }
                }
                if (!current.empty() || tabBar->items.find(';') != std::string::npos)
                {
                    parts.push_back(current);
                }

                labels.reserve(parts.size());
                for (const auto &part : parts)
                {
                    labels.push_back(part.c_str());
                }

                if (!labels.empty())
                {
                    GuiTabBar(rect, labels.data(), static_cast<int>(labels.size()), &tabBar->active);
                }
            }

            endClip(clipped);
        }

        for (auto &item : drawItems)
        {
            bool clipped = beginClip(item.entity);
            if (auto *label = item.entity.tryGetComponent<LabelComponent>())
            {
                applyStyleFor(item.entity);
                const int prevSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
                const int prevNormal = GuiGetStyle(LABEL, TEXT_COLOR_NORMAL);
                const int prevFocused = GuiGetStyle(LABEL, TEXT_COLOR_FOCUSED);
                const int prevPressed = GuiGetStyle(LABEL, TEXT_COLOR_PRESSED);
                const int prevDisabled = GuiGetStyle(LABEL, TEXT_COLOR_DISABLED);
                const int labelColor = packColor(label->color[0], label->color[1], label->color[2], label->color[3]);
                GuiSetStyle(DEFAULT, TEXT_SIZE, label->fontSize);
                GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, labelColor);
                GuiSetStyle(LABEL, TEXT_COLOR_FOCUSED, labelColor);
                GuiSetStyle(LABEL, TEXT_COLOR_PRESSED, labelColor);
                GuiSetStyle(LABEL, TEXT_COLOR_DISABLED, labelColor);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiLabel(rect, label->text.c_str());
                GuiSetStyle(LABEL, TEXT_COLOR_DISABLED, prevDisabled);
                GuiSetStyle(LABEL, TEXT_COLOR_PRESSED, prevPressed);
                GuiSetStyle(LABEL, TEXT_COLOR_FOCUSED, prevFocused);
                GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, prevNormal);
                GuiSetStyle(DEFAULT, TEXT_SIZE, prevSize);
            }

            if (auto *labelButton = item.entity.tryGetComponent<LabelButtonComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                if (GuiLabelButton(rect, labelButton->text.c_str()))
                {
                    auto it = s_buttonCallbacks.find(item.entity.id());
                    if (it != s_buttonCallbacks.end())
                    {
                        for (const auto &callback : it->second)
                        {
                            if (callback)
                            {
                                callback(item.entity);
                            }
                        }
                    }
                }
            }

            if (auto *button = item.entity.tryGetComponent<ButtonComponent>())
            {
                applyStyleFor(item.entity);
                if (button->disabled)
                {
                    GuiDisable();
                }

                const int prevSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
                GuiSetStyle(DEFAULT, TEXT_SIZE, button->fontSize);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                if (GuiButton(rect, button->text.c_str()) && !button->disabled)
                {
                    auto it = s_buttonCallbacks.find(item.entity.id());
                    if (it != s_buttonCallbacks.end())
                    {
                        for (const auto &callback : it->second)
                        {
                            if (callback)
                            {
                                callback(item.entity);
                            }
                        }
                    }
                }
                GuiSetStyle(DEFAULT, TEXT_SIZE, prevSize);

                if (button->disabled)
                {
                    GuiEnable();
                }
            }

            if (auto *toggle = item.entity.tryGetComponent<ToggleComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiToggle(rect, toggle->text.c_str(), &toggle->active);
            }

            if (auto *toggleGroup = item.entity.tryGetComponent<ToggleGroupComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiToggleGroup(rect, toggleGroup->items.c_str(), &toggleGroup->active);
            }

            if (auto *toggleSlider = item.entity.tryGetComponent<ToggleSliderComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiToggleSlider(rect, toggleSlider->text.c_str(), &toggleSlider->active);
            }

            if (auto *checkBox = item.entity.tryGetComponent<CheckBoxComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiCheckBox(rect, checkBox->text.c_str(), &checkBox->checked);
            }

            if (auto *comboBox = item.entity.tryGetComponent<ComboBoxComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiComboBox(rect, comboBox->items.c_str(), &comboBox->active);
            }

            if (auto *dropdown = item.entity.tryGetComponent<DropdownBoxComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                dropdown->editMode = GuiDropdownBox(rect, dropdown->items.c_str(), &dropdown->active, dropdown->editMode) != 0;
            }

            if (auto *valueBox = item.entity.tryGetComponent<ValueBoxComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                valueBox->editMode = GuiValueBox(rect, valueBox->text.c_str(), &valueBox->value,
                                                 valueBox->minValue, valueBox->maxValue, valueBox->editMode) != 0;
            }

            if (auto *spinner = item.entity.tryGetComponent<SpinnerComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                spinner->editMode = GuiSpinner(rect, spinner->text.c_str(), &spinner->value,
                                               spinner->minValue, spinner->maxValue, spinner->editMode) != 0;
            }

            if (auto *slider = item.entity.tryGetComponent<SliderComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiSlider(rect, slider->textLeft.c_str(), slider->textRight.c_str(),
                          &slider->value, slider->minValue, slider->maxValue);
            }

            if (auto *sliderBar = item.entity.tryGetComponent<SliderBarComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiSliderBar(rect, sliderBar->textLeft.c_str(), sliderBar->textRight.c_str(),
                             &sliderBar->value, sliderBar->minValue, sliderBar->maxValue);
            }

            if (auto *progress = item.entity.tryGetComponent<ProgressBarComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiProgressBar(rect, progress->textLeft.c_str(), progress->textRight.c_str(),
                               &progress->value, progress->minValue, progress->maxValue);
            }

            if (auto *listView = item.entity.tryGetComponent<ListViewComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                GuiListView(rect, listView->items.c_str(), &listView->scrollIndex, &listView->active);
            }

            if (auto *colorPicker = item.entity.tryGetComponent<ColorPickerComponent>())
            {
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                Color color = toColor(colorPicker->color);
                GuiColorPicker(rect, "", &color);
                colorPicker->color[0] = color.r;
                colorPicker->color[1] = color.g;
                colorPicker->color[2] = color.b;
                colorPicker->color[3] = color.a;
            }

            if (auto *messageBox = item.entity.tryGetComponent<MessageBoxComponent>())
            {
                if (!messageBox->open)
                {
                    continue;
                }
                applyStyleFor(item.entity);
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                messageBox->result = GuiMessageBox(rect, messageBox->title.c_str(), messageBox->message.c_str(),
                                                  messageBox->buttons.c_str());
                if (messageBox->result >= 0)
                {
                    messageBox->open = false;
                }
            }

            if (auto *textInput = item.entity.tryGetComponent<TextInputBoxComponent>())
            {
                if (!textInput->open)
                {
                    continue;
                }
                applyStyleFor(item.entity);
                const int maxLength = std::max(1, textInput->maxLength);
                std::vector<char> buffer(static_cast<size_t>(maxLength) + 1u, '\0');
                if (!textInput->text.empty())
                {
                    std::size_t count = std::min(static_cast<size_t>(maxLength), textInput->text.size());
                    std::memcpy(buffer.data(), textInput->text.data(), count);
                    buffer[count] = '\0';
                }
                Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
                textInput->result = GuiTextInputBox(rect, textInput->title.c_str(), textInput->message.c_str(),
                                                    textInput->buttons.c_str(), buffer.data(), maxLength,
                                                    &textInput->secretView);
                textInput->text = buffer.data();
                if (textInput->result >= 0)
                {
                    textInput->open = false;
                }
            }

            endClip(clipped);
        }

        for (auto &item : drawItems)
        {
            auto *textEdit = item.entity.tryGetComponent<TextEditComponent>();
            if (!textEdit)
            {
                continue;
            }

            applyStyleFor(item.entity);
            const int maxLength = std::max(1, textEdit->maxLength);
            std::vector<char> buffer(static_cast<size_t>(maxLength) + 1u, '\0');
            if (!textEdit->text.empty())
            {
                std::size_t count = std::min(static_cast<size_t>(maxLength), textEdit->text.size());
                std::memcpy(buffer.data(), textEdit->text.data(), count);
                buffer[count] = '\0';
            }

            Rectangle rect = {item.rect.x, item.rect.y, item.rect.w, item.rect.h};
            const bool editMode = s_focusedTextEdit == item.entity.id() && !textEdit->readOnly;
            if (textEdit->readOnly)
            {
                GuiDisable();
            }
            GuiTextBox(rect, buffer.data(), maxLength, editMode);
            if (textEdit->readOnly)
            {
                GuiEnable();
            }

            textEdit->text = buffer.data();
            if (textEdit->text.empty() && !editMode && !textEdit->placeholder.empty())
            {
                drawTextInRect(textEdit->placeholder, item.rect, textEdit->fontSize, toColor(textEdit->color));
            }
        }
    }

    void ConnectButtonPressed(Entity button, UiButtonCallback callback)
    {
        if (!button.isValid())
        {
            return;
        }

        s_buttonCallbacks[button.id()].push_back(std::move(callback));
    }

    namespace
    {
        class UiRenderSystem : public System
        {
        public:
            void onUpdate(Scene &scene, float dt) override
            {
                (void)dt;
                BeginDrawing();
                ClearBackground({18, 24, 36, 255});
                UpdateUi(scene, GetScreenWidth(), GetScreenHeight());
                DrawUi(scene, GetScreenWidth(), GetScreenHeight());
                EndDrawing();
            }
        };
    }

    void RegisterUiSystems(Scene &scene)
    {
        scene.createSystem<UiRenderSystem>();
    }

    void SetUiThemeStyle(const std::string &stylePath)
    {
        s_globalStylePath = stylePath;
        if (s_globalStylePath.empty())
        {
            return;
        }

        applyStylePath(s_globalStylePath, false);
    }

    void SetUiThemeMelkam()
    {
        if (!IsWindowReady())
        {
            s_pendingMelkamTheme = true;
            return;
        }

        GuiLoadStyleDefault();
        applyMelkamFont();

        const int textNormal = packColor(232, 236, 245, 255);
        const int textMuted = packColor(170, 178, 192, 255);
        const int baseNormal = packColor(36, 42, 54, 255);
        const int baseFocused = packColor(46, 54, 68, 255);
        const int basePressed = packColor(56, 122, 214, 255);
        const int baseDisabled = packColor(28, 33, 42, 255);
        const int borderNormal = packColor(66, 76, 92, 255);
        const int borderFocused = packColor(92, 104, 124, 255);
        const int borderPressed = packColor(56, 122, 214, 255);
        const int borderDisabled = packColor(52, 60, 74, 255);

        GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, textNormal);
        GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, textNormal);
        GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, textNormal);
        GuiSetStyle(DEFAULT, TEXT_COLOR_DISABLED, textMuted);

        GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, baseNormal);
        GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, baseFocused);
        GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, basePressed);
        GuiSetStyle(DEFAULT, BASE_COLOR_DISABLED, baseDisabled);

        GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, borderNormal);
        GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, borderFocused);
        GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, borderPressed);
        GuiSetStyle(DEFAULT, BORDER_COLOR_DISABLED, borderDisabled);

        GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);
        GuiSetStyle(DEFAULT, TEXT_PADDING, 8);
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        GuiSetStyle(DEFAULT, TEXT_SIZE, 18);

        GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
        GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

        GuiSetStyle(TEXTBOX, BASE_COLOR_PRESSED, baseFocused);
        GuiSetStyle(VALUEBOX, BASE_COLOR_PRESSED, baseFocused);

        GuiSetStyle(SLIDER, BASE_COLOR_PRESSED, basePressed);
        GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, basePressed);

        GuiSetStyle(CHECKBOX, BASE_COLOR_PRESSED, basePressed);
        GuiSetStyle(TOGGLE, BASE_COLOR_PRESSED, basePressed);

        GuiSetStyle(LISTVIEW, BORDER_COLOR_NORMAL, borderNormal);
        GuiSetStyle(SCROLLBAR, BASE_COLOR_NORMAL, baseNormal);
        GuiSetStyle(SCROLLBAR, BASE_COLOR_FOCUSED, baseFocused);
        GuiSetStyle(SCROLLBAR, BASE_COLOR_PRESSED, basePressed);
        GuiSetStyle(SCROLLBAR, BORDER_COLOR_NORMAL, borderNormal);

        s_globalStylePath.clear();
        s_currentStylePath = "melkam";
    }
}