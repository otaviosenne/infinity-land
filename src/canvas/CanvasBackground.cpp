#include "CanvasBackground.hpp"
#include "CanvasTheme.hpp"
#include "CanvasViewport.hpp"
#include "../render/OpenGL.hpp"
#include "../managers/PointerManager.hpp"
#include "../render/Renderer.hpp"
#include "../Compositor.hpp"

static constexpr float BASE_GRID_SPACING = 40.0f;
static constexpr float LOW_ZOOM_THRESHOLD = 0.4f;
static constexpr float DOT_SIZE = 2.0f;
static constexpr float GLOW_RADIUS = 120.0f;
static constexpr float GLOW_DOT_SIZE = 3.0f;
static constexpr float MIN_VISIBLE_SPACING = 8.0f;
static constexpr int MAX_DOTS_PER_AXIS = 120;

void CCanvasBackground::render(PHLMONITOR pMonitor, const CRegion& damage) {
    if (!g_pCanvasViewport || !g_pHyprOpenGL)
        return;

    const double canvasScale = g_pCanvasViewport->scale();
    const auto offset = g_pCanvasViewport->offset();
    const auto monSize = pMonitor->m_transformedSize;
    const float gridSpacing = gridSpacingForScale(canvasScale);
    const float scaledSpacing = gridSpacing * canvasScale;

    const auto themeColors = g_pCanvasTheme ? g_pCanvasTheme->colors() : SCanvasThemeColors{
        CHyprColor(0.102f, 0.102f, 0.180f, 1.0f),
        CHyprColor(0.216f, 0.255f, 0.318f, 1.0f),
        CHyprColor(0.612f, 0.639f, 0.686f, 1.0f),
    };

    g_pHyprOpenGL->renderRect(CBox{0, 0, monSize.x, monSize.y}, themeColors.bgColor, {});

    if (scaledSpacing < MIN_VISIBLE_SPACING)
        return;

    const auto cursorPos = g_pPointerManager->position() - pMonitor->m_position;

    const float modX = fmod(fmod(offset.x, (double)scaledSpacing) + scaledSpacing, (double)scaledSpacing);
    const float modY = fmod(fmod(offset.y, (double)scaledSpacing) + scaledSpacing, (double)scaledSpacing);

    const int dotsX = std::min(MAX_DOTS_PER_AXIS, static_cast<int>(monSize.x / scaledSpacing) + 2);
    const int dotsY = std::min(MAX_DOTS_PER_AXIS, static_cast<int>(monSize.y / scaledSpacing) + 2);

    for (int iy = 0; iy < dotsY; ++iy) {
        const float y = modY + iy * scaledSpacing;
        if (y >= monSize.y) break;

        for (int ix = 0; ix < dotsX; ++ix) {
            const float x = modX + ix * scaledSpacing;
            if (x >= monSize.x) break;

            const float dx = x - cursorPos.x;
            const float dy = y - cursorPos.y;
            const float distSq = dx * dx + dy * dy;
            const float glowRadSq = GLOW_RADIUS * GLOW_RADIUS;

            float dotSize = DOT_SIZE;
            CHyprColor dotColor = themeColors.dotColor;

            if (distSq < glowRadSq) {
                const float glowFactor = 1.0f - sqrt(distSq) / GLOW_RADIUS;
                dotSize += (GLOW_DOT_SIZE - DOT_SIZE) * glowFactor;
                dotColor.r += (themeColors.glowDotColor.r - dotColor.r) * glowFactor;
                dotColor.g += (themeColors.glowDotColor.g - dotColor.g) * glowFactor;
                dotColor.b += (themeColors.glowDotColor.b - dotColor.b) * glowFactor;
            }

            const float half = dotSize / 2.0f;
            g_pHyprOpenGL->renderRect(CBox{x - half, y - half, dotSize, dotSize}, dotColor, {});
        }
    }
}

float CCanvasBackground::gridSpacingForScale(double scale) const {
    if (scale <= LOW_ZOOM_THRESHOLD)
        return BASE_GRID_SPACING * 2.0f;
    return BASE_GRID_SPACING;
}
