#include "DotGridPassElement.hpp"
#include "../OpenGL.hpp"
#include "../../canvas/CanvasViewport.hpp"
#include "../../canvas/CanvasTheme.hpp"
#include "../../managers/PointerManager.hpp"

static constexpr float BASE_GRID_SPACING = 40.0f;
static constexpr float LOW_ZOOM_THRESHOLD = 0.4f;
static constexpr float DOT_SIZE = 2.0f;
static constexpr float GLOW_RADIUS = 120.0f;
static constexpr float GLOW_DOT_SIZE = 3.0f;
static constexpr float MIN_VISIBLE_SPACING = 8.0f;

static float gridSpacingForScale(double scale) {
    return scale <= LOW_ZOOM_THRESHOLD ? BASE_GRID_SPACING * 2.0f : BASE_GRID_SPACING;
}

CDotGridPassElement::CDotGridPassElement(const SDotGridData& data) : m_data(data) {}

void CDotGridPassElement::draw(const CRegion& damage) {
    const auto pMonitor = m_data.monitor.lock();
    if (!pMonitor || !g_pCanvasViewport || !g_pHyprOpenGL)
        return;

    const double canvasScale = g_pCanvasViewport->scale();
    const auto offset = g_pCanvasViewport->offset();
    const auto monSize = pMonitor->m_transformedSize;
    const float gridSpacing = gridSpacingForScale(canvasScale);
    const float scaledSpacing = gridSpacing * canvasScale;

    if (scaledSpacing < MIN_VISIBLE_SPACING)
        return;

    const auto themeColors = g_pCanvasTheme ? g_pCanvasTheme->colors() : SCanvasThemeColors{
        CHyprColor(0.102f, 0.102f, 0.180f, 1.0f),
        CHyprColor(0.216f, 0.255f, 0.318f, 1.0f),
        CHyprColor(0.612f, 0.639f, 0.686f, 1.0f),
    };

    const auto cursorPos = g_pPointerManager->position() - pMonitor->m_position;

    const float modX = fmod(fmod(offset.x, (double)scaledSpacing) + scaledSpacing, (double)scaledSpacing);
    const float modY = fmod(fmod(offset.y, (double)scaledSpacing) + scaledSpacing, (double)scaledSpacing);

    auto& shader = g_pHyprOpenGL->m_shaders->m_shQUAD;
    g_pHyprOpenGL->useProgram(shader.program);

    glBindVertexArray(shader.uniformLocations[SHADER_SHADER_VAO]);

    for (float y = modY; y < monSize.y; y += scaledSpacing) {
        for (float x = modX; x < monSize.x; x += scaledSpacing) {
            const float dx = x - cursorPos.x;
            const float dy = y - cursorPos.y;
            const float distSq = dx * dx + dy * dy;
            const float glowRadSq = GLOW_RADIUS * GLOW_RADIUS;

            float dotSz = DOT_SIZE;
            CHyprColor dotColor = themeColors.dotColor;

            if (distSq < glowRadSq) {
                const float glowFactor = 1.0f - sqrt(distSq) / GLOW_RADIUS;
                dotSz += (GLOW_DOT_SIZE - DOT_SIZE) * glowFactor;
                dotColor.r += (themeColors.glowDotColor.r - dotColor.r) * glowFactor;
                dotColor.g += (themeColors.glowDotColor.g - dotColor.g) * glowFactor;
                dotColor.b += (themeColors.glowDotColor.b - dotColor.b) * glowFactor;
            }

            const float half = dotSz / 2.0f;
            CBox dotBox = {x - half, y - half, dotSz, dotSz};

            Mat3x3 matrix = g_pHyprOpenGL->m_renderData.monitorProjection.projectBox(
                dotBox, wlTransformToHyprutils(WL_OUTPUT_TRANSFORM_NORMAL), 0);
            Mat3x3 glMatrix = g_pHyprOpenGL->m_renderData.projection.copy().multiply(matrix);

            shader.setUniformMatrix3fv(SHADER_PROJ, 1, GL_TRUE, glMatrix.getMatrix());
            shader.setUniformFloat4(SHADER_COLOR,
                dotColor.r * dotColor.a, dotColor.g * dotColor.a,
                dotColor.b * dotColor.a, dotColor.a);

            CBox transformedBox = dotBox;
            transformedBox.transform(
                wlTransformToHyprutils(invertTransform(pMonitor->m_transform)),
                pMonitor->m_transformedSize.x, pMonitor->m_transformedSize.y);

            shader.setUniformFloat2(SHADER_TOP_LEFT, transformedBox.x, transformedBox.y);
            shader.setUniformFloat2(SHADER_FULL_SIZE, transformedBox.width, transformedBox.height);
            shader.setUniformFloat(SHADER_RADIUS, half);
            shader.setUniformFloat(SHADER_ROUNDING_POWER, 2.0f);

            damage.forEachRect([](const auto& RECT) {
                g_pHyprOpenGL->scissor(&RECT);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            });
        }
    }

    glBindVertexArray(0);
    g_pHyprOpenGL->scissor(nullptr);
}

bool CDotGridPassElement::needsLiveBlur() { return false; }
bool CDotGridPassElement::needsPrecomputeBlur() { return false; }
const char* CDotGridPassElement::passName() { return "DotGrid"; }
bool CDotGridPassElement::disableSimplification() { return true; }
