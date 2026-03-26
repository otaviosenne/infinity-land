#include "DotGridPassElement.hpp"
#include "../OpenGL.hpp"
#include "../../canvas/CanvasViewport.hpp"
#include "../../canvas/CanvasTheme.hpp"
#include "../../managers/PointerManager.hpp"

static constexpr float BASE_GRID_SPACING  = 40.0f;
static constexpr float LOW_ZOOM_THRESHOLD = 0.4f;
static constexpr float DOT_SIZE           = 2.0f;
static constexpr float GLOW_RADIUS        = 120.0f;
static constexpr float GLOW_DOT_SIZE      = 3.0f;
static constexpr float MIN_VISIBLE_SPACING = 8.0f;
static constexpr int   MAX_DOTS_PER_AXIS  = 120;

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
    const float  gridSpacing    = gridSpacingForScale(canvasScale);
    const float  scaledSpacing  = gridSpacing * canvasScale;

    if (scaledSpacing < MIN_VISIBLE_SPACING)
        return;

    const auto themeColors = g_pCanvasTheme ? g_pCanvasTheme->colors() : SCanvasThemeColors{
        CHyprColor(0.11f, 0.11f, 0.12f, 1.0f),
        CHyprColor(0.25f, 0.25f, 0.27f, 1.0f),
        CHyprColor(0.55f, 0.55f, 0.58f, 1.0f),
    };

    const auto cursorPos = g_pPointerManager->position() - pMonitor->m_position;

    const float modX = fmod(fmod(offset.x, (double)scaledSpacing) + scaledSpacing, (double)scaledSpacing);
    const float modY = fmod(fmod(offset.y, (double)scaledSpacing) + scaledSpacing, (double)scaledSpacing);

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

            float      dotSz    = DOT_SIZE;
            CHyprColor dotColor = themeColors.dotColor;

            if (distSq < GLOW_RADIUS * GLOW_RADIUS) {
                const float glowFactor = 1.0f - sqrtf(distSq) / GLOW_RADIUS;
                dotSz += (GLOW_DOT_SIZE - DOT_SIZE) * glowFactor;
                dotColor.r += (themeColors.glowDotColor.r - dotColor.r) * glowFactor;
                dotColor.g += (themeColors.glowDotColor.g - dotColor.g) * glowFactor;
                dotColor.b += (themeColors.glowDotColor.b - dotColor.b) * glowFactor;
            }

            const float half = dotSz / 2.0f;
            g_pHyprOpenGL->renderRect(CBox{x - half, y - half, dotSz, dotSz}, dotColor, {});
        }
    }
}

bool CDotGridPassElement::needsLiveBlur() { return false; }
bool CDotGridPassElement::needsPrecomputeBlur() { return false; }
const char* CDotGridPassElement::passName() { return "DotGrid"; }
bool CDotGridPassElement::disableSimplification() { return true; }
std::optional<CBox> CDotGridPassElement::boundingBox() { return std::nullopt; }
