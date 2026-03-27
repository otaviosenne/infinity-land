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
    void           switchTab(int tabIndex);
    void           nextTab();
    void           prevTab();
    bool           isWindowInAnyFrame(PHLWINDOW pWindow) const;
    void           renderAll(PHLMONITOR pMonitor, const CRegion& damage);
    CCanvasFrame*  activeFrame() const;

  private:
    std::vector<UP<CCanvasFrame>> m_frames;
    uint64_t                      m_nextId   = 1;
    uint64_t                      m_activeId = 0;
};

inline UP<CCanvasFrameManager> g_pCanvasFrameManager;
