#include "CanvasTheme.hpp"
#include "CanvasViewport.hpp"

const SCanvasThemeColors CCanvasTheme::DARK_COLORS = {
    CHyprColor(0.102f, 0.102f, 0.180f, 1.0f),
    CHyprColor(0.216f, 0.255f, 0.318f, 1.0f),
    CHyprColor(0.612f, 0.639f, 0.686f, 1.0f),
};

const SCanvasThemeColors CCanvasTheme::LIGHT_COLORS = {
    CHyprColor(0.961f, 0.961f, 0.961f, 1.0f),
    CHyprColor(0.820f, 0.836f, 0.855f, 1.0f),
    CHyprColor(0.612f, 0.639f, 0.686f, 1.0f),
};

CCanvasTheme::CCanvasTheme() = default;

void CCanvasTheme::toggle() {
    m_theme = (m_theme == CANVAS_THEME_DARK) ? CANVAS_THEME_LIGHT : CANVAS_THEME_DARK;
    if (g_pCanvasViewport)
        g_pCanvasViewport->damageAllMonitors();
}

eCanvasTheme CCanvasTheme::current() const {
    return m_theme;
}

SCanvasThemeColors CCanvasTheme::colors() const {
    return (m_theme == CANVAS_THEME_DARK) ? DARK_COLORS : LIGHT_COLORS;
}
