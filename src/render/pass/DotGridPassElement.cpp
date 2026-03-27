#include "DotGridPassElement.hpp"
#include "../OpenGL.hpp"
#include "../../canvas/CanvasViewport.hpp"
#include "../../canvas/CanvasTheme.hpp"
#include "../../managers/PointerManager.hpp"
#include "../../helpers/Monitor.hpp"

static constexpr float BASE_GRID_SPACING   = 40.0f;
static constexpr float LOW_ZOOM_THRESHOLD  = 0.4f;
static constexpr float DOT_SPACING_RATIO   = 0.05f;
static constexpr float DOT_GLOW_RATIO      = 0.075f;
static constexpr float DOT_MIN_PX          = 1.5f;
static constexpr float GLOW_RADIUS         = 120.0f;
static constexpr float MIN_VISIBLE_SPACING = 8.0f;
static constexpr int   MAX_DOTS_PER_AXIS   = 120;

static float gridSpacingForScale(double scale) {
    return scale <= LOW_ZOOM_THRESHOLD ? BASE_GRID_SPACING * 2.0f : BASE_GRID_SPACING;
}

CDotGridPassElement::CDotGridPassElement(const SDotGridData& data) : m_data(data) {}

void CDotGridPassElement::draw(const CRegion& damage) {
    const auto pMonitor = m_data.monitor.lock();
    if (!pMonitor || !g_pCanvasViewport || !g_pHyprOpenGL)
        return;

    const double canvasScale    = g_pCanvasViewport->scale();
    const auto   offset         = g_pCanvasViewport->offset();
    const auto   monSize        = pMonitor->m_transformedSize;
    const float  monScale       = pMonitor->m_scale;
    const float  gridSpacing    = gridSpacingForScale(canvasScale);
    const float  scaledSpacing  = gridSpacing * canvasScale * monScale;

    if (scaledSpacing < MIN_VISIBLE_SPACING)
        return;

    const auto themeColors = g_pCanvasTheme ? g_pCanvasTheme->colors() : SCanvasThemeColors{
        CHyprColor(0.11f, 0.11f, 0.12f, 1.0f),
        CHyprColor(0.25f, 0.25f, 0.27f, 1.0f),
        CHyprColor(0.55f, 0.55f, 0.58f, 1.0f),
    };

    const auto cursorPos = (g_pPointerManager->position() - pMonitor->m_position) * monScale;

    const float offsetXPx = offset.x * monScale;
    const float offsetYPx = offset.y * monScale;
    const float modX = fmod(fmod(offsetXPx, (double)scaledSpacing) + scaledSpacing, (double)scaledSpacing);
    const float modY = fmod(fmod(offsetYPx, (double)scaledSpacing) + scaledSpacing, (double)scaledSpacing);

    const int dotsX = std::min(MAX_DOTS_PER_AXIS, static_cast<int>(monSize.x / scaledSpacing) + 2);
    const int dotsY = std::min(MAX_DOTS_PER_AXIS, static_cast<int>(monSize.y / scaledSpacing) + 2);

    for (int iy = 0; iy < dotsY; ++iy) {
        const float y = modY + iy * scaledSpacing;
        if (y >= monSize.y)
            break;

        for (int ix = 0; ix < dotsX; ++ix) {
            const float x = modX + ix * scaledSpacing;
            if (x >= monSize.x)
                break;

            const float dx     = x - cursorPos.x;
            const float dy     = y - cursorPos.y;
            const float distSq = dx * dx + dy * dy;

            float      dotSz    = std::max(DOT_MIN_PX, scaledSpacing * DOT_SPACING_RATIO);
            CHyprColor dotColor = themeColors.dotColor;

            const float glowRadiusPx = GLOW_RADIUS * monScale;
            if (distSq < glowRadiusPx * glowRadiusPx) {
                const float glowFactor = 1.0f - sqrtf(distSq) / glowRadiusPx;
                dotSz += (scaledSpacing * DOT_GLOW_RATIO - dotSz) * glowFactor;
                dotColor.r += (themeColors.glowDotColor.r - dotColor.r) * glowFactor;
                dotColor.g += (themeColors.glowDotColor.g - dotColor.g) * glowFactor;
                dotColor.b += (themeColors.glowDotColor.b - dotColor.b) * glowFactor;
            }

            const float half = dotSz / 2.0f;
            g_pHyprOpenGL->renderRect(CBox{x - half, y - half, dotSz, dotSz}, dotColor, {.round = (int)half});
        }
    }

    damageGlowArea(pMonitor, cursorPos);
}

bool CDotGridPassElement::needsLiveBlur() { return false; }
bool CDotGridPassElement::needsPrecomputeBlur() { return false; }
const char* CDotGridPassElement::passName() { return "DotGrid"; }
bool CDotGridPassElement::disableSimplification() { return true; }
std::optional<CBox> CDotGridPassElement::boundingBox() { return std::nullopt; }

void CDotGridPassElement::damageGlowArea(PHLMONITOR pMonitor, const Vector2D& cursorPos) const {
    const float glowRadiusPx = GLOW_RADIUS * pMonitor->m_scale;
    pMonitor->addDamage(CBox{cursorPos.x - glowRadiusPx, cursorPos.y - glowRadiusPx, glowRadiusPx * 2.0f, glowRadiusPx * 2.0f});
}
