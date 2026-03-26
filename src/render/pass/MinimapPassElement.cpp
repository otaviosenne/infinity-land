#include "MinimapPassElement.hpp"
#include "../OpenGL.hpp"

CMinimapPassElement::CMinimapPassElement(const SMinimapData& data) : m_data(data) {}

void CMinimapPassElement::draw(const CRegion& damage) {
    for (const auto& rect : m_data.rects) {
        if (rect.box.w <= 0 || rect.box.h <= 0)
            continue;

        CBox newBox = rect.box;

        Mat3x3 matrix = g_pHyprOpenGL->m_renderData.monitorProjection.projectBox(
            newBox, wlTransformToHyprutils(WL_OUTPUT_TRANSFORM_NORMAL), 0);
        Mat3x3 glMatrix = g_pHyprOpenGL->m_renderData.projection.copy().multiply(matrix);

        auto& shader = g_pHyprOpenGL->m_shaders->m_shQUAD;
        g_pHyprOpenGL->useProgram(shader.program);
        shader.setUniformMatrix3fv(SHADER_PROJ, 1, GL_TRUE, glMatrix.getMatrix());
        shader.setUniformFloat4(SHADER_COLOR,
            rect.color.r * rect.color.a, rect.color.g * rect.color.a,
            rect.color.b * rect.color.a, rect.color.a);

        CBox transformedBox = newBox;
        transformedBox.transform(
            wlTransformToHyprutils(invertTransform(g_pHyprOpenGL->m_renderData.pMonitor->m_transform)),
            g_pHyprOpenGL->m_renderData.pMonitor->m_transformedSize.x,
            g_pHyprOpenGL->m_renderData.pMonitor->m_transformedSize.y);

        shader.setUniformFloat2(SHADER_TOP_LEFT, transformedBox.x, transformedBox.y);
        shader.setUniformFloat2(SHADER_FULL_SIZE, transformedBox.width, transformedBox.height);
        shader.setUniformFloat(SHADER_RADIUS, rect.round);
        shader.setUniformFloat(SHADER_ROUNDING_POWER, 2.0f);

        glBindVertexArray(shader.uniformLocations[SHADER_SHADER_VAO]);
        g_pHyprOpenGL->scissor(nullptr);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    g_pHyprOpenGL->scissor(nullptr);
}

bool CMinimapPassElement::needsLiveBlur() { return false; }
bool CMinimapPassElement::needsPrecomputeBlur() { return false; }
const char* CMinimapPassElement::passName() { return "Minimap"; }
bool CMinimapPassElement::disableSimplification() { return true; }
