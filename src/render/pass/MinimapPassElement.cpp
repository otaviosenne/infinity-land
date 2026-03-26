#include "MinimapPassElement.hpp"
#include "../OpenGL.hpp"

CMinimapPassElement::CMinimapPassElement(const SMinimapData& data) : m_data(data) {}

static void renderDirectRect(const CBox& box, const CHyprColor& color, int round) {
    if (box.w <= 0 || box.h <= 0)
        return;

    Mat3x3 matrix = g_pHyprOpenGL->m_renderData.monitorProjection.projectBox(
        box, wlTransformToHyprutils(WL_OUTPUT_TRANSFORM_NORMAL), 0);
    Mat3x3 glMatrix = g_pHyprOpenGL->m_renderData.projection.copy().multiply(matrix);

    auto& shader = g_pHyprOpenGL->m_shaders->m_shQUAD;
    g_pHyprOpenGL->useProgram(shader.program);
    shader.setUniformMatrix3fv(SHADER_PROJ, 1, GL_TRUE, glMatrix.getMatrix());
    shader.setUniformFloat4(SHADER_COLOR,
        color.r * color.a, color.g * color.a, color.b * color.a, color.a);

    CBox tb = box;
    tb.transform(
        wlTransformToHyprutils(invertTransform(g_pHyprOpenGL->m_renderData.pMonitor->m_transform)),
        g_pHyprOpenGL->m_renderData.pMonitor->m_transformedSize.x,
        g_pHyprOpenGL->m_renderData.pMonitor->m_transformedSize.y);

    shader.setUniformFloat2(SHADER_TOP_LEFT, tb.x, tb.y);
    shader.setUniformFloat2(SHADER_FULL_SIZE, tb.width, tb.height);
    shader.setUniformFloat(SHADER_RADIUS, round);
    shader.setUniformFloat(SHADER_ROUNDING_POWER, 2.0f);

    glBindVertexArray(shader.uniformLocations[SHADER_SHADER_VAO]);
    g_pHyprOpenGL->scissor(nullptr);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void CMinimapPassElement::draw(const CRegion& damage) {
    for (const auto& rect : m_data.rects) {
        renderDirectRect(rect.box, rect.color, rect.round);
    }
}

bool CMinimapPassElement::needsLiveBlur() { return false; }
bool CMinimapPassElement::needsPrecomputeBlur() { return false; }
const char* CMinimapPassElement::passName() { return "Minimap"; }
bool CMinimapPassElement::disableSimplification() { return true; }
