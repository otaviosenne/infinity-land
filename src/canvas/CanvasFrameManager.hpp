#pragma once
#include "CanvasFrame.hpp"
#include "../helpers/Monitor.hpp"
#include <vector>
#include <cstdint>

class CCanvasFrameManager {
  public:
    void           createFrame();
    void           deleteFrame(uint64_t id);
    void           assignFocusedWindow();
    bool           assignWindowIfInsideFrame(PHLWINDOW pWindow);
    CCanvasFrame*  frameAtCanvasPos(const Vector2D& canvasPos) const;
    void           switchTab(int tabIndex);
    void           nextTab();
    void           prevTab();
    bool           isWindowInAnyFrame(PHLWINDOW pWindow) const;
    void           renderAll(PHLMONITOR pMonitor, const CRegion& damage);
    CCanvasFrame*  activeFrame() const;

    void           beginFrameDrag(const Vector2D& mouseScreenPos);
    void           updateFrameDrag(const Vector2D& mouseScreenPos);
    void           endFrameDrag();
    bool           isFrameDragging() const { return m_isDraggingFrame; }

  private:
    std::vector<UP<CCanvasFrame>> m_frames;
    uint64_t                      m_nextId   = 1;
    uint64_t                      m_activeId = 0;

    void parkAllFrameWindows(const CCanvasFrame& frame);

    bool     m_isDraggingFrame     = false;
    Vector2D m_frameDragStartMouse;
    Vector2D m_frameDragStartPos;
    uint64_t m_draggingFrameId     = 0;
};

inline UP<CCanvasFrameManager> g_pCanvasFrameManager;
