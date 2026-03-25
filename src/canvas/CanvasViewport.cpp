#include "CanvasViewport.hpp"
#include "../Compositor.hpp"
#include "../helpers/Monitor.hpp"

Vector2D CCanvasViewport::canvasToScreen(const Vector2D& canvas) const {
    return canvas * m_scale + m_offset;
}

Vector2D CCanvasViewport::screenToCanvas(const Vector2D& screen) const {
    return (screen - m_offset) / m_scale;
}

CBox CCanvasViewport::canvasToScreen(const CBox& box) const {
    return CBox{box.x * m_scale + m_offset.x, box.y * m_scale + m_offset.y,
                box.w * m_scale, box.h * m_scale};
}

void CCanvasViewport::pan(const Vector2D& delta) {
    m_offset = m_offset + delta;
    damageAllMonitors();
}

void CCanvasViewport::zoom(double factor, const Vector2D& screenAnchor) {
    const auto canvasAnchor = screenToCanvas(screenAnchor);
    m_scale = std::clamp(m_scale * factor, MIN_SCALE, MAX_SCALE);
    m_offset = screenAnchor - canvasAnchor * m_scale;
    damageAllMonitors();
}

double CCanvasViewport::scale() const {
    return m_scale;
}

Vector2D CCanvasViewport::offset() const {
    return m_offset;
}

void CCanvasViewport::damageAllMonitors() {
    for (auto const& m : g_pCompositor->m_monitors) {
        m->m_damage.damageEntire();
    }
}
