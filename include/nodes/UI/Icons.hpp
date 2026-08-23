#pragma once

#include "helper/color/Color.hpp"
#include "helper/IconsFontAwesome6.hpp"
#include "helper/vectors/Vector2.hpp"
#include "renderers/Font.hpp"
#include "renderers/Renderer2D.hpp"
#include <string>

enum class IconType {
  ChevronDown,
  ChevronUp,
  ChevronLeft,
  ChevronRight,
  Close,
  Check,
  Plus,
  Minus,
  Info,
  Warning,
  Error,
  Play,
  Pause,
  Stop,
  Gear,
  Menu,
  ArrowBack,
  ArrowForward,
  Search,
  Folder,
  File
};

// FontAwesome 6 Icon Engine for MelkamEngine Canvas UI
class Icons {
public:
  static const char *getGlyph(IconType type) {
    switch (type) {
    case IconType::ChevronDown:  return ICON_FA_CHEVRON_DOWN;
    case IconType::ChevronUp:    return ICON_FA_CHEVRON_UP;
    case IconType::ChevronLeft:  return ICON_FA_CHEVRON_LEFT;
    case IconType::ChevronRight: return ICON_FA_CHEVRON_RIGHT;
    case IconType::Close:        return ICON_FA_XMARK;
    case IconType::Check:        return ICON_FA_CHECK;
    case IconType::Plus:         return ICON_FA_PLUS;
    case IconType::Minus:        return ICON_FA_MINUS;
    case IconType::Info:         return ICON_FA_CIRCLE_INFO;
    case IconType::Warning:      return ICON_FA_TRIANGLE_EXCLAMATION;
    case IconType::Error:        return ICON_FA_CIRCLE_XMARK;
    case IconType::Play:         return ICON_FA_PLAY;
    case IconType::Pause:        return ICON_FA_PAUSE;
    case IconType::Stop:         return ICON_FA_STOP;
    case IconType::Gear:         return ICON_FA_GEAR;
    case IconType::Menu:         return ICON_FA_BARS;
    case IconType::ArrowBack:    return ICON_FA_ARROW_LEFT;
    case IconType::ArrowForward: return ICON_FA_ARROW_RIGHT;
    case IconType::Search:       return ICON_FA_MAGNIFYING_GLASS;
    case IconType::Folder:       return ICON_FA_FOLDER;
    case IconType::File:         return ICON_FA_FILE;
    default:                     return "";
    }
  }

  // Draws a FontAwesome icon centered at 'center' with specified font size
  static void draw(IconType type, const Vector2 &center, float size, const Color &color, float strokeWidth = 0.0f) {
    (void)strokeWidth;
    const char *glyph = getGlyph(type);
    if (!glyph || !*glyph) return;

    Vector2 sz = Font::getDefaultFont()->getStringSize(glyph, size);
    Vector2 drawPos = center - sz * 0.5f;
    Renderer2D::drawText(glyph, drawPos, color, size);
  }

  static void draw(const std::string &glyph, const Vector2 &center, float size, const Color &color, float strokeWidth = 0.0f) {
    (void)strokeWidth;
    if (glyph.empty()) return;
    Vector2 sz = Font::getDefaultFont()->getStringSize(glyph, size);
    Vector2 drawPos = center - sz * 0.5f;
    Renderer2D::drawText(glyph, drawPos, color, size);
  }
};
