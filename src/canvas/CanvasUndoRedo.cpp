#include "CanvasUndoRedo.hpp"

void CCanvasUndoRedo::push(UP<ICanvasCommand> cmd) {
    m_redoStack.clear();

    if (m_undoStack.size() >= MAX_STACK_SIZE)
        m_undoStack.erase(m_undoStack.begin());

    m_undoStack.push_back(std::move(cmd));
}

void CCanvasUndoRedo::undo() {
    if (m_undoStack.empty())
        return;

    auto cmd = std::move(m_undoStack.back());
    m_undoStack.pop_back();
    cmd->undo();
    m_redoStack.push_back(std::move(cmd));
}

void CCanvasUndoRedo::redo() {
    if (m_redoStack.empty())
        return;

    auto cmd = std::move(m_redoStack.back());
    m_redoStack.pop_back();
    cmd->execute();
    m_undoStack.push_back(std::move(cmd));
}

bool CCanvasUndoRedo::canUndo() const {
    return !m_undoStack.empty();
}

bool CCanvasUndoRedo::canRedo() const {
    return !m_redoStack.empty();
}
