#include "CanvasAnnotation.hpp"
#include "CanvasViewport.hpp"
#include "../render/OpenGL.hpp"

#include <cairo/cairo.h>
#include <GLES3/gl32.h>
#include <algorithm>
#include <cmath>

void CCanvasAnnotation::addStroke(SAnnotationStroke stroke) {
    stroke.id = m_nextId++;
    m_strokes.push_back(std::move(stroke));
    m_dirty = true;
}

void CCanvasAnnotation::removeStroke(uint64_t id) {
    std::erase_if(m_strokes, [id](const SAnnotationStroke& s) { return s.id == id; });
    m_dirty = true;
}

SAnnotationStroke* CCanvasAnnotation::findNearestStroke(const Vector2D& canvasPos, double threshold) {
    SAnnotationStroke* nearest     = nullptr;
    double             minDistance = threshold;

    for (auto& stroke : m_strokes) {
        for (size_t i = 0; i + 1 < stroke.points.size(); ++i) {
            const double dist = distanceToSegment(canvasPos, stroke.points[i], stroke.points[i + 1]);
            if (dist < minDistance) {
                minDistance = dist;
                nearest    = &stroke;
            }
        }
    }

    return nearest;
}

void CCanvasAnnotation::render(PHLMONITOR pMonitor, const CRegion& damage) {
    if (m_strokes.empty())
        return;

    if (!g_pCanvasViewport)
        return;

    const int monitorW = pMonitor->m_pixelSize.x;
    const int monitorH = pMonitor->m_pixelSize.y;

    const bool sizeChanged = (monitorW != m_lastWidth || monitorH != m_lastHeight);

    if (!m_dirty && m_texture && !sizeChanged) {
        CBox monBox = {pMonitor->m_position.x, pMonitor->m_position.y, (double)monitorW, (double)monitorH};
        g_pHyprOpenGL->renderTexture(m_texture, monBox, {.a = 1.0f});
        return;
    }

    auto* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, monitorW, monitorH);
    auto* cr      = cairo_create(surface);

    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    const auto monPos = pMonitor->m_position;

    for (const auto& stroke : m_strokes) {
        if (stroke.points.size() < 2)
            continue;

        cairo_set_source_rgba(cr, stroke.color.r, stroke.color.g, stroke.color.b, stroke.color.a);
        cairo_set_line_width(cr, stroke.thickness * g_pCanvasViewport->scale());
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

        bool first = true;
        for (const auto& point : stroke.points) {
            const auto screenPos = g_pCanvasViewport->canvasToScreen(point);
            const double sx      = screenPos.x - monPos.x;
            const double sy      = screenPos.y - monPos.y;

            if (first) {
                cairo_move_to(cr, sx, sy);
                first = false;
            } else {
                cairo_line_to(cr, sx, sy);
            }
        }

        cairo_stroke(cr);
    }

    cairo_surface_flush(surface);

    if (!m_texture)
        m_texture = makeShared<CTexture>();

    m_texture->allocate();
    m_texture->m_size = {monitorW, monitorH};
    m_texture->bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, monitorW, monitorH, 0, GL_RGBA, GL_UNSIGNED_BYTE, cairo_image_surface_get_data(surface));

    m_texture->unbind();

    CBox monBox = {pMonitor->m_position.x, pMonitor->m_position.y, (double)monitorW, (double)monitorH};
    g_pHyprOpenGL->renderTexture(m_texture, monBox, {.a = 1.0f});

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    m_lastWidth  = monitorW;
    m_lastHeight = monitorH;
    m_dirty      = false;
}

void CCanvasAnnotation::markDirty() {
    m_dirty = true;
}

const std::vector<SAnnotationStroke>& CCanvasAnnotation::strokes() const {
    return m_strokes;
}

double CCanvasAnnotation::distanceToSegment(const Vector2D& point, const Vector2D& segA, const Vector2D& segB) const {
    const Vector2D ab = segB - segA;
    const Vector2D ap = point - segA;

    const double lengthSq = ab.x * ab.x + ab.y * ab.y;

    if (lengthSq == 0.0)
        return std::sqrt(ap.x * ap.x + ap.y * ap.y);

    const double t       = std::clamp((ap.x * ab.x + ap.y * ab.y) / lengthSq, 0.0, 1.0);
    const Vector2D proj  = segA + Vector2D{ab.x * t, ab.y * t};
    const Vector2D diff  = point - proj;

    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}
