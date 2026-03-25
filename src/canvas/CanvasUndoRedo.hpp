#pragma once
#include "../defines.hpp"
#include "CanvasCommand.hpp"

#include <vector>

class CCanvasUndoRedo {
  public:
    void push(UP<ICanvasCommand> cmd);
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;

  private:
    static constexpr size_t          MAX_STACK_SIZE = 100;
    std::vector<UP<ICanvasCommand>> m_undoStack;
    std::vector<UP<ICanvasCommand>> m_redoStack;
};

inline UP<CCanvasUndoRedo> g_pCanvasUndoRedo;
