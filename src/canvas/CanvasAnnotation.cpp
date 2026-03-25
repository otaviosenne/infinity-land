#include "CanvasAnnotation.hpp"
#include "CanvasPersistence.hpp"
#include "CanvasViewport.hpp"
#include "../render/OpenGL.hpp"

#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <GLES3/gl32.h>
#include <algorithm>
#include <cmath>

void CCanvasAnnotation::addStroke(SAnnotationStroke stroke) {
    stroke.id = m_nextId++;
    m_strokes.push_back(std::move(stroke));
    m_dirty = true;
    if (g_pCanvasPersistence)
        g_pCanvasPersistence->scheduleSave();
}

void CCanvasAnnotation::removeStroke(uint64_t id) {
    std::erase_if(m_strokes, [id](const SAnnotationStroke& s) { return s.id == id; });
    m_dirty = true;
    if (g_pCanvasPersistence)
        g_pCanvasPersistence->scheduleSave();
}

void CCanvasAnnotation::addArrow(SAnnotationArrow arrow) {
    arrow.id = m_nextId++;
    m_arrows.push_back(std::move(arrow));
    m_dirty = true;
    if (g_pCanvasPersistence)
        g_pCanvasPersistence->scheduleSave();
}

void CCanvasAnnotation::removeArrow(uint64_t id) {
    std::erase_if(m_arrows, [id](const SAnnotationArrow& a) { return a.id == id; });
    m_dirty = true;
    if (g_pCanvasPersistence)
        g_pCanvasPersistence->scheduleSave();
}

void CCanvasAnnotation::addStickyNote(SAnnotationStickyNote note) {
    note.id = m_nextId++;
    m_stickyNotes.push_back(std::move(note));
    m_dirty = true;
    if (g_pCanvasPersistence)
        g_pCanvasPersistence->scheduleSave();
}

void CCanvasAnnotation::removeStickyNote(uint64_t id) {
    std::erase_if(m_stickyNotes, [id](const SAnnotationStickyNote& n) { return n.id == id; });
    m_dirty = true;
    if (g_pCanvasPersistence)
        g_pCanvasPersistence->scheduleSave();
}

void CCanvasAnnotation::addText(SAnnotationText text) {
    text.id = m_nextId++;
    m_texts.push_back(std::move(text));
    m_dirty = true;
    if (g_pCanvasPersistence)
        g_pCanvasPersistence->scheduleSave();
}

void CCanvasAnnotation::removeText(uint64_t id) {
    std::erase_if(m_texts, [id](const SAnnotationText& t) { return t.id == id; });
    m_dirty = true;
    if (g_pCanvasPersistence)
        g_pCanvasPersistence->scheduleSave();
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

SAnnotationArrow* CCanvasAnnotation::findNearestArrow(const Vector2D& canvasPos, double threshold) {
    SAnnotationArrow* nearest     = nullptr;
    double            minDistance = threshold;

    for (auto& arrow : m_arrows) {
        const double dist = distanceToSegment(canvasPos, arrow.start, arrow.end);
        if (dist < minDistance) {
            minDistance = dist;
            nearest    = &arrow;
        }
    }

    return nearest;
}

SAnnotationStickyNote* CCanvasAnnotation::findStickyNoteAt(const Vector2D& canvasPos) {
    for (auto& note : m_stickyNotes) {
        if (canvasPos.x >= note.position.x && canvasPos.x <= note.position.x + note.size.x &&
            canvasPos.y >= note.position.y && canvasPos.y <= note.position.y + note.size.y)
            return &note;
    }
    return nullptr;
}

SAnnotationText* CCanvasAnnotation::findTextAt(const Vector2D& canvasPos, double threshold) {
    SAnnotationText* nearest     = nullptr;
    double           minDistance = threshold;

    for (auto& t : m_texts) {
        const Vector2D diff = canvasPos - t.position;
        const double   dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (dist < minDistance) {
            minDistance = dist;
            nearest    = &t;
        }
    }

    return nearest;
}

const std::vector<SAnnotationStroke>& CCanvasAnnotation::strokes() const {
    return m_strokes;
}

const std::vector<SAnnotationArrow>& CCanvasAnnotation::arrows() const {
    return m_arrows;
}

const std::vector<SAnnotationStickyNote>& CCanvasAnnotation::stickyNotes() const {
    return m_stickyNotes;
}

const std::vector<SAnnotationText>& CCanvasAnnotation::texts() const {
    return m_texts;
}

void CCanvasAnnotation::clearAll() {
    m_strokes.clear();
    m_arrows.clear();
    m_stickyNotes.clear();
    m_texts.clear();
    m_dirty = true;
    if (g_pCanvasPersistence)
        g_pCanvasPersistence->scheduleSave();
}

void CCanvasAnnotation::clearByType(const std::string& type) {
    if (type == "strokes")
        m_strokes.clear();
    else if (type == "arrows")
        m_arrows.clear();
    else if (type == "notes")
        m_stickyNotes.clear();
    else if (type == "text")
        m_texts.clear();
    m_dirty = true;
    if (g_pCanvasPersistence)
        g_pCanvasPersistence->scheduleSave();
}

static bool isInVisibleBounds(const Vector2D& point, const CBox& visibleCanvas, double margin) {
    return point.x >= visibleCanvas.x - margin &&
           point.x <= visibleCanvas.x + visibleCanvas.w + margin &&
           point.y >= visibleCanvas.y - margin &&
           point.y <= visibleCanvas.y + visibleCanvas.h + margin;
}

static bool isBoxInVisibleBounds(const CBox& box, const CBox& visibleCanvas) {
    return !(box.x + box.w < visibleCanvas.x ||
             box.x > visibleCanvas.x + visibleCanvas.w ||
             box.y + box.h < visibleCanvas.y ||
             box.y > visibleCanvas.y + visibleCanvas.h);
}

static void renderArrowhead(cairo_t* cr, const Vector2D& from, const Vector2D& to, double scale) {
    const Vector2D dir   = to - from;
    const double   len   = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 1.0)
        return;

    const Vector2D norm  = Vector2D{dir.x / len, dir.y / len};
    const double   headLen = 12.0 * scale;
    const double   angle   = M_PI / 6.0;

    const Vector2D tip = to;
    const Vector2D wing1 = Vector2D{
        tip.x - headLen * (norm.x * std::cos(angle) - norm.y * std::sin(angle)),
        tip.y - headLen * (norm.y * std::cos(angle) + norm.x * std::sin(angle))};
    const Vector2D wing2 = Vector2D{
        tip.x - headLen * (norm.x * std::cos(angle) + norm.y * std::sin(angle)),
        tip.y - headLen * (norm.y * std::cos(angle) - norm.x * std::sin(angle))};

    cairo_move_to(cr, tip.x, tip.y);
    cairo_line_to(cr, wing1.x, wing1.y);
    cairo_line_to(cr, wing2.x, wing2.y);
    cairo_close_path(cr);
    cairo_fill(cr);
}

static void renderPangoText(cairo_t* cr, const std::string& text, double x, double y, double fontSize, const CHyprColor& color) {
    if (text.empty())
        return;

    cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);

    auto* layout = pango_cairo_create_layout(cr);
    auto* desc   = pango_font_description_from_string("Sans");
    pango_font_description_set_absolute_size(desc, fontSize * PANGO_SCALE);
    pango_layout_set_font_description(layout, desc);
    pango_layout_set_text(layout, text.c_str(), -1);

    cairo_move_to(cr, x, y);
    pango_cairo_show_layout(cr, layout);

    pango_font_description_free(desc);
    g_object_unref(layout);
}

void CCanvasAnnotation::render(PHLMONITOR pMonitor, const CRegion& damage) {
    if (m_strokes.empty() && m_arrows.empty() && m_stickyNotes.empty() && m_texts.empty())
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
    const double scale = g_pCanvasViewport->scale();

    const auto topLeft     = g_pCanvasViewport->screenToCanvas(monPos);
    const auto bottomRight = g_pCanvasViewport->screenToCanvas(Vector2D{monPos.x + monitorW, monPos.y + monitorH});
    const CBox visibleCanvas = {topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y};

    for (const auto& stroke : m_strokes) {
        if (stroke.points.size() < 2)
            continue;

        double minX = stroke.points[0].x, maxX = minX;
        double minY = stroke.points[0].y, maxY = minY;
        for (const auto& p : stroke.points) {
            minX = std::min(minX, p.x);
            maxX = std::max(maxX, p.x);
            minY = std::min(minY, p.y);
            maxY = std::max(maxY, p.y);
        }
        const double thicknessMargin = stroke.thickness;
        const CBox strokeBounds = {minX - thicknessMargin, minY - thicknessMargin, maxX - minX + thicknessMargin * 2, maxY - minY + thicknessMargin * 2};
        if (!isBoxInVisibleBounds(strokeBounds, visibleCanvas))
            continue;

        cairo_set_source_rgba(cr, stroke.color.r, stroke.color.g, stroke.color.b, stroke.color.a);
        cairo_set_line_width(cr, stroke.thickness * scale);
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

    for (const auto& arrow : m_arrows) {
        const double arrowMargin = arrow.thickness + 12.0;
        if (!isInVisibleBounds(arrow.start, visibleCanvas, arrowMargin) &&
            !isInVisibleBounds(arrow.end, visibleCanvas, arrowMargin))
            continue;

        const auto screenStart = g_pCanvasViewport->canvasToScreen(arrow.start);
        const auto screenEnd   = g_pCanvasViewport->canvasToScreen(arrow.end);
        const double sx1 = screenStart.x - monPos.x;
        const double sy1 = screenStart.y - monPos.y;
        const double sx2 = screenEnd.x - monPos.x;
        const double sy2 = screenEnd.y - monPos.y;

        cairo_set_source_rgba(cr, arrow.color.r, arrow.color.g, arrow.color.b, arrow.color.a);
        cairo_set_line_width(cr, arrow.thickness * scale);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        cairo_move_to(cr, sx1, sy1);
        cairo_line_to(cr, sx2, sy2);
        cairo_stroke(cr);

        renderArrowhead(cr, Vector2D{sx1, sy1}, Vector2D{sx2, sy2}, scale);
    }

    for (const auto& note : m_stickyNotes) {
        const CBox noteBox = {note.position.x, note.position.y, note.size.x, note.size.y};
        if (!isBoxInVisibleBounds(noteBox, visibleCanvas))
            continue;

        const auto screenPos = g_pCanvasViewport->canvasToScreen(note.position);
        const double sx   = screenPos.x - monPos.x;
        const double sy   = screenPos.y - monPos.y;
        const double sw   = note.size.x * scale;
        const double sh   = note.size.y * scale;
        const double radius = 8.0;

        cairo_new_sub_path(cr);
        cairo_arc(cr, sx + sw - radius, sy + radius, radius, -M_PI / 2.0, 0);
        cairo_arc(cr, sx + sw - radius, sy + sh - radius, radius, 0, M_PI / 2.0);
        cairo_arc(cr, sx + radius, sy + sh - radius, radius, M_PI / 2.0, M_PI);
        cairo_arc(cr, sx + radius, sy + radius, radius, M_PI, 3.0 * M_PI / 2.0);
        cairo_close_path(cr);

        cairo_set_source_rgba(cr, note.color.r, note.color.g, note.color.b, note.color.a);
        cairo_fill_preserve(cr);

        cairo_set_source_rgba(cr, 0.3, 0.3, 0.2, 0.6);
        cairo_set_line_width(cr, 1.0);
        cairo_stroke(cr);

        const double padding = 8.0;
        renderPangoText(cr, note.text, sx + padding, sy + padding, 12.0 * scale, CHyprColor(0.1, 0.1, 0.1, 1.0));
    }

    for (const auto& t : m_texts) {
        const double textMargin = t.fontSize * 20.0;
        if (!isInVisibleBounds(t.position, visibleCanvas, textMargin))
            continue;

        const auto screenPos = g_pCanvasViewport->canvasToScreen(t.position);
        const double sx = screenPos.x - monPos.x;
        const double sy = screenPos.y - monPos.y;

        renderPangoText(cr, t.text, sx, sy, t.fontSize * scale, t.color);
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
