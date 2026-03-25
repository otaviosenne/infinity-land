#include "CanvasCommand.hpp"
#include "CanvasAnnotation.hpp"
#include "CanvasPersistence.hpp"
#include "CanvasViewport.hpp"

static void damageAndDirty() {
    if (g_pCanvasAnnotation)
        g_pCanvasAnnotation->markDirty();
    if (g_pCanvasViewport)
        g_pCanvasViewport->damageAllMonitors();
}

WindowMoveCommand::WindowMoveCommand(PHLWINDOWREF window, const Vector2D& oldPos, const Vector2D& newPos)
    : m_window(window), m_oldPos(oldPos), m_newPos(newPos) {}

void WindowMoveCommand::execute() {
    applyPosition(m_newPos);
}

void WindowMoveCommand::undo() {
    applyPosition(m_oldPos);
}

void WindowMoveCommand::applyPosition(const Vector2D& pos) {
    const auto w = m_window.lock();
    if (!w)
        return;

    w->m_position = pos;
    w->m_realPosition->setValueAndWarp(pos);

    if (g_pCanvasViewport)
        g_pCanvasViewport->damageAllMonitors();

    if (g_pCanvasPersistence) {
        g_pCanvasPersistence->trackWindow(w->m_initialClass, w->m_position, w->m_size);
        g_pCanvasPersistence->scheduleSave();
    }
}

WindowResizeCommand::WindowResizeCommand(PHLWINDOWREF window, const Vector2D& oldPos, const Vector2D& oldSize, const Vector2D& newPos,
                                         const Vector2D& newSize)
    : m_window(window), m_oldPos(oldPos), m_oldSize(oldSize), m_newPos(newPos), m_newSize(newSize) {}

void WindowResizeCommand::execute() {
    applyState(m_newPos, m_newSize);
}

void WindowResizeCommand::undo() {
    applyState(m_oldPos, m_oldSize);
}

void WindowResizeCommand::applyState(const Vector2D& pos, const Vector2D& size) {
    const auto w = m_window.lock();
    if (!w)
        return;

    w->m_position = pos;
    w->m_realPosition->setValueAndWarp(pos);
    w->m_size = size;
    w->m_realSize->setValueAndWarp(size);
    w->sendWindowSize();

    if (g_pCanvasViewport)
        g_pCanvasViewport->damageAllMonitors();

    if (g_pCanvasPersistence) {
        g_pCanvasPersistence->trackWindow(w->m_initialClass, w->m_position, w->m_size);
        g_pCanvasPersistence->scheduleSave();
    }
}

AnnotationAddCommand::AnnotationAddCommand(const std::string& type, uint64_t id, SAnnotationData data) : m_type(type), m_id(id), m_data(std::move(data)) {}

void AnnotationAddCommand::execute() {
    addAnnotation();
}

void AnnotationAddCommand::undo() {
    removeAnnotation();
}

void AnnotationAddCommand::addAnnotation() {
    if (!g_pCanvasAnnotation)
        return;

    if (m_type == "stroke")
        g_pCanvasAnnotation->addStroke(std::get<SAnnotationStroke>(m_data));
    else if (m_type == "arrow")
        g_pCanvasAnnotation->addArrow(std::get<SAnnotationArrow>(m_data));
    else if (m_type == "sticky")
        g_pCanvasAnnotation->addStickyNote(std::get<SAnnotationStickyNote>(m_data));
    else if (m_type == "text")
        g_pCanvasAnnotation->addText(std::get<SAnnotationText>(m_data));

    damageAndDirty();
}

void AnnotationAddCommand::removeAnnotation() {
    if (!g_pCanvasAnnotation)
        return;

    if (m_type == "stroke")
        g_pCanvasAnnotation->removeStroke(m_id);
    else if (m_type == "arrow")
        g_pCanvasAnnotation->removeArrow(m_id);
    else if (m_type == "sticky")
        g_pCanvasAnnotation->removeStickyNote(m_id);
    else if (m_type == "text")
        g_pCanvasAnnotation->removeText(m_id);

    damageAndDirty();
}

AnnotationRemoveCommand::AnnotationRemoveCommand(const std::string& type, uint64_t id, SAnnotationData data)
    : m_type(type), m_id(id), m_data(std::move(data)) {}

void AnnotationRemoveCommand::execute() {
    removeAnnotation();
}

void AnnotationRemoveCommand::undo() {
    addAnnotation();
}

void AnnotationRemoveCommand::addAnnotation() {
    if (!g_pCanvasAnnotation)
        return;

    if (m_type == "stroke")
        g_pCanvasAnnotation->addStroke(std::get<SAnnotationStroke>(m_data));
    else if (m_type == "arrow")
        g_pCanvasAnnotation->addArrow(std::get<SAnnotationArrow>(m_data));
    else if (m_type == "sticky")
        g_pCanvasAnnotation->addStickyNote(std::get<SAnnotationStickyNote>(m_data));
    else if (m_type == "text")
        g_pCanvasAnnotation->addText(std::get<SAnnotationText>(m_data));

    damageAndDirty();
}

void AnnotationRemoveCommand::removeAnnotation() {
    if (!g_pCanvasAnnotation)
        return;

    if (m_type == "stroke")
        g_pCanvasAnnotation->removeStroke(m_id);
    else if (m_type == "arrow")
        g_pCanvasAnnotation->removeArrow(m_id);
    else if (m_type == "sticky")
        g_pCanvasAnnotation->removeStickyNote(m_id);
    else if (m_type == "text")
        g_pCanvasAnnotation->removeText(m_id);

    damageAndDirty();
}

AnnotationClearCommand::AnnotationClearCommand(std::vector<SAnnotationStroke> strokes, std::vector<SAnnotationArrow> arrows,
                                               std::vector<SAnnotationStickyNote> notes, std::vector<SAnnotationText> texts)
    : m_strokes(std::move(strokes)), m_arrows(std::move(arrows)), m_notes(std::move(notes)), m_texts(std::move(texts)) {}

void AnnotationClearCommand::execute() {
    if (!g_pCanvasAnnotation)
        return;

    g_pCanvasAnnotation->clearAll();
    damageAndDirty();
}

void AnnotationClearCommand::undo() {
    if (!g_pCanvasAnnotation)
        return;

    for (const auto& s : m_strokes)
        g_pCanvasAnnotation->addStroke(s);
    for (const auto& a : m_arrows)
        g_pCanvasAnnotation->addArrow(a);
    for (const auto& n : m_notes)
        g_pCanvasAnnotation->addStickyNote(n);
    for (const auto& t : m_texts)
        g_pCanvasAnnotation->addText(t);

    damageAndDirty();
}
