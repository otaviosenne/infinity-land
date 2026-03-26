#include "CanvasTheme.hpp"
#include "CanvasViewport.hpp"

const SCanvasThemeColors CCanvasTheme::DARK_COLORS = {
    CHyprColor(0.11f, 0.11f, 0.12f, 1.0f),
    CHyprColor(0.25f, 0.25f, 0.27f, 1.0f),
    CHyprColor(0.55f, 0.55f, 0.58f, 1.0f),
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
