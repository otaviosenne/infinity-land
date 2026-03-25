#pragma once
#include "../defines.hpp"
#include "../helpers/Color.hpp"

enum eCanvasTheme : uint8_t {
    CANVAS_THEME_DARK = 0,
    CANVAS_THEME_LIGHT,
};

struct SCanvasThemeColors {
    CHyprColor bgColor;
    CHyprColor dotColor;
    CHyprColor glowDotColor;
};

class CCanvasTheme {
  public:
    CCanvasTheme();

    void               toggle();
    eCanvasTheme       current() const;
    SCanvasThemeColors colors() const;

  private:
    eCanvasTheme m_theme = CANVAS_THEME_DARK;

    static const SCanvasThemeColors DARK_COLORS;
    static const SCanvasThemeColors LIGHT_COLORS;
};

inline UP<CCanvasTheme> g_pCanvasTheme;
