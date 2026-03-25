#pragma once
#include "IHyprLayout.hpp"
#include <vector>

class CCanvasLayout : public IHyprLayout {
  public:
    void                     onEnable() override;
    void                     onDisable() override;
    void                     onWindowCreatedTiling(PHLWINDOW, eDirection direction = DIRECTION_DEFAULT) override;
    void                     onWindowRemovedTiling(PHLWINDOW) override;
    bool                     isWindowTiled(PHLWINDOW) override;
    void                     recalculateMonitor(const MONITORID&) override;
    void                     recalculateWindow(PHLWINDOW) override;
    void                     resizeActiveWindow(const Vector2D&, eRectCorner corner = CORNER_NONE, PHLWINDOW pWindow = nullptr) override;
    void                     fullscreenRequestForWindow(PHLWINDOW, eFullscreenMode, eFullscreenMode) override;
    std::any                 layoutMessage(SLayoutMessageHeader, std::string) override;
    SWindowRenderLayoutHints requestRenderHints(PHLWINDOW) override;
    void                     switchWindows(PHLWINDOW, PHLWINDOW) override;
    void                     moveWindowTo(PHLWINDOW, const std::string&, bool silent = false) override;
    void                     alterSplitRatio(PHLWINDOW, float, bool exact = false) override;
    std::string              getLayoutName() override;
    void                     replaceWindowDataWith(PHLWINDOW from, PHLWINDOW to) override;
    Vector2D                 predictSizeForNewWindowTiled() override;
    void                     onBeginDragWindow() override;
    void                     onEndDragWindow() override;
    void                     onMouseMove(const Vector2D&) override;
    void                     moveActiveWindow(const Vector2D&, PHLWINDOW pWindow = nullptr) override;

    const std::vector<PHLWINDOWREF>& windows() const { return m_windows; }

  private:
    std::vector<PHLWINDOWREF> m_windows;

    Vector2D    m_canvasDragStartPos;
    Vector2D    m_canvasDragStartSize;
    Vector2D    m_dragStartMousePos;
    eRectCorner m_dragCorner    = CORNER_TOPLEFT;
    bool        m_dragIsResize = false;

    eRectCorner determineDragCorner(const Vector2D& mousePos, const CBox& windowBox) const;

    static constexpr int     DEFAULT_WIDTH      = 1200;
    static constexpr int     DEFAULT_HEIGHT     = 800;
    static constexpr double  MIN_WINDOW_WIDTH   = 200;
    static constexpr double  MIN_WINDOW_HEIGHT  = 150;
};
