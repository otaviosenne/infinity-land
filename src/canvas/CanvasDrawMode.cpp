#include "CanvasDrawMode.hpp"
#include "CanvasViewport.hpp"
#include "CanvasAnnotation.hpp"

void CCanvasDrawMode::toggle() {
    m_active = !m_active;
    if (!m_active)
        m_drawing = false;
}

bool CCanvasDrawMode::isActive() const {
    return m_active;
}

eDrawTool CCanvasDrawMode::activeTool() const {
    return m_tool;
}

void CCanvasDrawMode::setTool(eDrawTool tool) {
    m_tool = tool;
}

void CCanvasDrawMode::setColor(const CHyprColor& color) {
    m_color = color;
}

void CCanvasDrawMode::setThickness(float thickness) {
    m_thickness = thickness;
}

CHyprColor CCanvasDrawMode::activeColor() const {
    return m_color;
}

float CCanvasDrawMode::activeThickness() const {
    return m_thickness;
}

bool CCanvasDrawMode::handleMouseButton(const Vector2D& screenPos, bool pressed) {
    if (!m_active || !g_pCanvasViewport || !g_pCanvasAnnotation)
        return false;

    const auto canvasPos = g_pCanvasViewport->screenToCanvas(screenPos);

    if (m_tool == DRAW_BRUSH) {
        if (pressed) {
            m_currentStroke          = SAnnotationStroke{};
            m_currentStroke.color    = m_color;
            m_currentStroke.thickness = m_thickness;
            m_currentStroke.points.push_back(canvasPos);
            m_drawing = true;
        } else if (m_drawing) {
            if (m_currentStroke.points.size() >= 2) {
                g_pCanvasAnnotation->addStroke(m_currentStroke);
                g_pCanvasAnnotation->markDirty();
                g_pCanvasViewport->damageAllMonitors();
            }
            m_drawing = false;
        }
    } else if (m_tool == DRAW_ERASER && pressed) {
        auto* stroke = g_pCanvasAnnotation->findNearestStroke(canvasPos);
        if (stroke) {
            g_pCanvasAnnotation->removeStroke(stroke->id);
            g_pCanvasAnnotation->markDirty();
            g_pCanvasViewport->damageAllMonitors();
        }
    }

    return true;
}

bool CCanvasDrawMode::handleMouseMove(const Vector2D& screenPos) {
    if (!m_active)
        return false;

    if (m_drawing && g_pCanvasViewport && g_pCanvasAnnotation) {
        const auto canvasPos = g_pCanvasViewport->screenToCanvas(screenPos);
        m_currentStroke.points.push_back(canvasPos);
        g_pCanvasAnnotation->markDirty();
        g_pCanvasViewport->damageAllMonitors();
    }

    return true;
}
