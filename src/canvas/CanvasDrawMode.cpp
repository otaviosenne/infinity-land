#include "CanvasDrawMode.hpp"
#include "CanvasCommand.hpp"
#include "CanvasUndoRedo.hpp"
#include "CanvasViewport.hpp"
#include "CanvasAnnotation.hpp"

void CCanvasDrawMode::toggle() {
    m_active = !m_active;
    if (!m_active) {
        m_drawing      = false;
        m_awaitingText = false;
    }
}

bool CCanvasDrawMode::isActive() const {
    return m_active;
}

eDrawTool CCanvasDrawMode::activeTool() const {
    return m_tool;
}

void CCanvasDrawMode::setTool(eDrawTool tool) {
    m_tool         = tool;
    m_awaitingText = false;
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

bool CCanvasDrawMode::isAwaitingText() const {
    return m_awaitingText;
}

void CCanvasDrawMode::submitText(const std::string& text) {
    if (!m_awaitingText || !g_pCanvasAnnotation || !g_pCanvasViewport)
        return;

    if (m_tool == DRAW_STICKY) {
        SAnnotationStickyNote note;
        note.position = m_pendingPosition;
        note.text     = text;
        note.color    = CHyprColor(1.0, 0.95, 0.6, 1.0);
        g_pCanvasAnnotation->addStickyNote(std::move(note));
        const auto noteId = g_pCanvasAnnotation->stickyNotes().back().id;
        auto       savedN = g_pCanvasAnnotation->stickyNotes().back();
        if (g_pCanvasUndoRedo)
            g_pCanvasUndoRedo->push(makeUnique<AnnotationAddCommand>("sticky", noteId, std::move(savedN)));
    } else if (m_tool == DRAW_TEXT) {
        SAnnotationText t;
        t.position = m_pendingPosition;
        t.text     = text;
        t.color    = m_color;
        t.fontSize = 16.0f;
        g_pCanvasAnnotation->addText(std::move(t));
        const auto textId = g_pCanvasAnnotation->texts().back().id;
        auto       savedT = g_pCanvasAnnotation->texts().back();
        if (g_pCanvasUndoRedo)
            g_pCanvasUndoRedo->push(makeUnique<AnnotationAddCommand>("text", textId, std::move(savedT)));
    }

    g_pCanvasAnnotation->markDirty();
    g_pCanvasViewport->damageAllMonitors();
    m_awaitingText = false;
}

void CCanvasDrawMode::cancelText() {
    m_awaitingText = false;
}

static double findMinEraserDistance(const Vector2D& canvasPos) {
    if (!g_pCanvasAnnotation)
        return 1e9;

    double minDist = 1e9;

    auto* stroke = g_pCanvasAnnotation->findNearestStroke(canvasPos);
    if (stroke)
        minDist = 0.0;

    auto* arrow = g_pCanvasAnnotation->findNearestArrow(canvasPos);
    if (arrow)
        minDist = 0.0;

    auto* note = g_pCanvasAnnotation->findStickyNoteAt(canvasPos);
    if (note)
        minDist = 0.0;

    auto* text = g_pCanvasAnnotation->findTextAt(canvasPos);
    if (text)
        minDist = 0.0;

    return minDist;
}

bool CCanvasDrawMode::handleMouseButton(const Vector2D& screenPos, bool pressed) {
    if (!m_active || !g_pCanvasViewport || !g_pCanvasAnnotation)
        return false;

    const auto canvasPos = g_pCanvasViewport->screenToCanvas(screenPos);

    if (m_tool == DRAW_BRUSH) {
        if (pressed) {
            m_currentStroke           = SAnnotationStroke{};
            m_currentStroke.color     = m_color;
            m_currentStroke.thickness = m_thickness;
            m_currentStroke.points.push_back(canvasPos);
            m_drawing = true;
        } else if (m_drawing) {
            if (m_currentStroke.points.size() >= 2) {
                g_pCanvasAnnotation->addStroke(m_currentStroke);
                const auto addedId = g_pCanvasAnnotation->strokes().back().id;
                auto       stored  = g_pCanvasAnnotation->strokes().back();
                if (g_pCanvasUndoRedo)
                    g_pCanvasUndoRedo->push(makeUnique<AnnotationAddCommand>("stroke", addedId, std::move(stored)));
                g_pCanvasAnnotation->markDirty();
                g_pCanvasViewport->damageAllMonitors();
            }
            m_drawing = false;
        }
    } else if (m_tool == DRAW_ERASER && pressed) {
        auto* stroke = g_pCanvasAnnotation->findNearestStroke(canvasPos);
        if (stroke) {
            auto saved = *stroke;
            if (g_pCanvasUndoRedo)
                g_pCanvasUndoRedo->push(makeUnique<AnnotationRemoveCommand>("stroke", saved.id, std::move(saved)));
            g_pCanvasAnnotation->removeStroke(stroke->id);
            g_pCanvasAnnotation->markDirty();
            g_pCanvasViewport->damageAllMonitors();
            return true;
        }

        auto* arrow = g_pCanvasAnnotation->findNearestArrow(canvasPos);
        if (arrow) {
            auto saved = *arrow;
            if (g_pCanvasUndoRedo)
                g_pCanvasUndoRedo->push(makeUnique<AnnotationRemoveCommand>("arrow", saved.id, std::move(saved)));
            g_pCanvasAnnotation->removeArrow(arrow->id);
            g_pCanvasAnnotation->markDirty();
            g_pCanvasViewport->damageAllMonitors();
            return true;
        }

        auto* note = g_pCanvasAnnotation->findStickyNoteAt(canvasPos);
        if (note) {
            auto saved = *note;
            if (g_pCanvasUndoRedo)
                g_pCanvasUndoRedo->push(makeUnique<AnnotationRemoveCommand>("sticky", saved.id, std::move(saved)));
            g_pCanvasAnnotation->removeStickyNote(note->id);
            g_pCanvasAnnotation->markDirty();
            g_pCanvasViewport->damageAllMonitors();
            return true;
        }

        auto* text = g_pCanvasAnnotation->findTextAt(canvasPos);
        if (text) {
            auto saved = *text;
            if (g_pCanvasUndoRedo)
                g_pCanvasUndoRedo->push(makeUnique<AnnotationRemoveCommand>("text", saved.id, std::move(saved)));
            g_pCanvasAnnotation->removeText(text->id);
            g_pCanvasAnnotation->markDirty();
            g_pCanvasViewport->damageAllMonitors();
            return true;
        }
    } else if (m_tool == DRAW_ARROW) {
        if (pressed) {
            m_currentArrow       = SAnnotationArrow{};
            m_currentArrow.start = canvasPos;
            m_drawing            = true;
        } else if (m_drawing) {
            m_currentArrow.end       = canvasPos;
            m_currentArrow.color     = m_color;
            m_currentArrow.thickness = m_thickness;
            g_pCanvasAnnotation->addArrow(m_currentArrow);
            const auto arrowId = g_pCanvasAnnotation->arrows().back().id;
            auto       savedAr = g_pCanvasAnnotation->arrows().back();
            if (g_pCanvasUndoRedo)
                g_pCanvasUndoRedo->push(makeUnique<AnnotationAddCommand>("arrow", arrowId, std::move(savedAr)));
            g_pCanvasAnnotation->markDirty();
            g_pCanvasViewport->damageAllMonitors();
            m_drawing = false;
        }
    } else if ((m_tool == DRAW_STICKY || m_tool == DRAW_TEXT) && pressed) {
        m_pendingPosition = canvasPos;
        m_awaitingText    = true;
    }

    return true;
}

bool CCanvasDrawMode::handleMouseMove(const Vector2D& screenPos) {
    if (!m_active)
        return false;

    if (m_drawing && g_pCanvasViewport && g_pCanvasAnnotation) {
        const auto canvasPos = g_pCanvasViewport->screenToCanvas(screenPos);

        if (m_tool == DRAW_BRUSH) {
            m_currentStroke.points.push_back(canvasPos);
            g_pCanvasAnnotation->markDirty();
            g_pCanvasViewport->damageAllMonitors();
        }
    }

    return true;
}
