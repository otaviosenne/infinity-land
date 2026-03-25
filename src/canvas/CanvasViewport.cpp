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

void CCanvasViewport::setViewCenter(const Vector2D& canvasPos) {
    const auto pMonitor = g_pCompositor->m_lastMonitor.lock();
    if (!pMonitor)
        return;

    const auto monitorCenter = pMonitor->m_position + pMonitor->m_size / 2.0;
    m_offset = monitorCenter - canvasPos * m_scale;
    damageAllMonitors();
}

void CCanvasViewport::zoomToFit(const CBox& canvasBox) {
    const auto pMonitor = g_pCompositor->m_lastMonitor.lock();
    if (!pMonitor || canvasBox.w <= 0 || canvasBox.h <= 0)
        return;

    const double FIT_PADDING = 0.9;
    const double newScale    = std::clamp(
        std::min(pMonitor->m_size.x / canvasBox.w, pMonitor->m_size.y / canvasBox.h) * FIT_PADDING,
        MIN_SCALE, MAX_SCALE);

    m_scale = newScale;

    const auto monitorCenter = pMonitor->m_position + pMonitor->m_size / 2.0;
    const auto boxCenter     = Vector2D{canvasBox.x + canvasBox.w / 2.0, canvasBox.y + canvasBox.h / 2.0};
    m_offset = monitorCenter - boxCenter * m_scale;
    damageAllMonitors();
}

void CCanvasViewport::damageAllMonitors() {
    for (auto const& m : g_pCompositor->m_monitors) {
        m->m_damage.damageEntire();
    }
}
