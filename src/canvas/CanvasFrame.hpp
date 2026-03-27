#pragma once
#include "../defines.hpp"
#include "../helpers/math/Math.hpp"
#include "../desktop/Window.hpp"
#include "../helpers/Monitor.hpp"
#include <vector>
#include <cstdint>

struct SCanvasFrameTab {
    int                       index = 1;
    std::vector<PHLWINDOWREF> windows;
};

class CCanvasFrame {
  public:
    CCanvasFrame(uint64_t id, const Vector2D& pos, const Vector2D& size);

    void        assignWindow(PHLWINDOW pWindow);
    void        unassignWindow(PHLWINDOW pWindow);
    bool        hasWindow(PHLWINDOW pWindow) const;
    void        setActiveTab(int tabIndex);
    void        addTab();
    void        removeActiveTab();
    void        retile();
    void        render(PHLMONITOR pMonitor);

    Vector2D    position() const { return m_position; }
    Vector2D    size() const { return m_size; }
    int         activeTabIndex() const { return m_activeTabIndex; }
    const std::vector<SCanvasFrameTab>& tabs() const { return m_tabs; }
    uint64_t    id() const { return m_id; }

    static constexpr double TITLE_BAR_H  = 32.0;
    static constexpr double TABS_BAR_H   = 28.0;
    static constexpr double TOTAL_HEADER = TITLE_BAR_H + TABS_BAR_H;
    static constexpr double BORDER_W     = 2.0;
    static constexpr double PARK_OFFSET  = 8000.0;

  private:
    uint64_t                     m_id;
    Vector2D                     m_position;
    Vector2D                     m_size;
    std::vector<SCanvasFrameTab> m_tabs;
    int                          m_activeTabIndex = 0;

    std::vector<CBox>            computeTileLayout(int count) const;
    void                         applyTileToWindow(PHLWINDOW pWindow, const CBox& tileBox);
    void                         parkWindow(PHLWINDOW pWindow, int tabIdx);
};
