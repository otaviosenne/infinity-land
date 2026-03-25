#include "CanvasBackground.hpp"
#include "CanvasViewport.hpp"
#include "../render/OpenGL.hpp"

static constexpr float BASE_GRID_SPACING = 20.0f;
static constexpr float LOW_ZOOM_THRESHOLD = 0.4f;
static constexpr float DOT_RADIUS = 1.5f;

void CCanvasBackground::render(PHLMONITOR pMonitor, const CRegion& damage) {
    if (!g_pCanvasViewport || !g_pHyprOpenGL)
        return;

    const auto scale = static_cast<float>(g_pCanvasViewport->scale());
    const auto offset = g_pCanvasViewport->offset();
    const auto monSize = pMonitor->m_transformedSize;
    const float spacing = gridSpacingForScale(scale);

    auto& shader = g_pHyprOpenGL->m_shaders->m_shDOTGRID;

    CBox fullBox = {0, 0, monSize.x, monSize.y};
    Mat3x3 matrix = g_pHyprOpenGL->m_renderData.monitorProjection.projectBox(
        fullBox, wlTransformToHyprutils(WL_OUTPUT_TRANSFORM_NORMAL), 0);
    Mat3x3 glMatrix = g_pHyprOpenGL->m_renderData.projection.copy().multiply(matrix);

    g_pHyprOpenGL->useProgram(shader.program);
    shader.setUniformMatrix3fv(SHADER_PROJ, 1, GL_TRUE, glMatrix.getMatrix());

    glUniform2f(glGetUniformLocation(shader.program, "u_resolution"),
        monSize.x, monSize.y);
    glUniform2f(glGetUniformLocation(shader.program, "u_canvasOffset"),
        static_cast<float>(offset.x), static_cast<float>(offset.y));
    glUniform1f(glGetUniformLocation(shader.program, "u_canvasScale"), scale);
    glUniform1f(glGetUniformLocation(shader.program, "u_gridSpacing"), spacing);
    glUniform1f(glGetUniformLocation(shader.program, "u_dotRadius"), DOT_RADIUS);

    glUniform4f(glGetUniformLocation(shader.program, "u_dotColor"),
        0.216f, 0.255f, 0.318f, 1.0f);
    glUniform4f(glGetUniformLocation(shader.program, "u_bgColor"),
        0.102f, 0.102f, 0.180f, 1.0f);

    glBindVertexArray(shader.uniformLocations[SHADER_SHADER_VAO]);

    damage.forEachRect([](const auto& RECT) {
        g_pHyprOpenGL->scissor(&RECT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    });

    glBindVertexArray(0);
    g_pHyprOpenGL->scissor(nullptr);
}

float CCanvasBackground::gridSpacingForScale(double scale) const {
    if (scale <= LOW_ZOOM_THRESHOLD)
        return BASE_GRID_SPACING * 2.0f;
    return BASE_GRID_SPACING;
}
