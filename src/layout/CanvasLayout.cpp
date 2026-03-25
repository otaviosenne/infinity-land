#include "CanvasLayout.hpp"
#include "../canvas/CanvasViewport.hpp"
#include "../Compositor.hpp"
#include "../helpers/Monitor.hpp"

void CCanvasLayout::onEnable() {
    //
}

void CCanvasLayout::onDisable() {
    m_windows.clear();
}

void CCanvasLayout::onWindowCreatedTiling(PHLWINDOW pWindow, eDirection) {
    const auto PMONITOR = g_pCompositor->getMonitorFromID(pWindow->monitorID());

    const auto screenCenter = PMONITOR->m_position + PMONITOR->m_size / 2.0;
    const auto canvasCenter = g_pCanvasViewport->screenToCanvas(screenCenter);
    const auto halfSize     = Vector2D{DEFAULT_WIDTH / 2.0, DEFAULT_HEIGHT / 2.0};

    pWindow->m_position      = canvasCenter - halfSize;
    *pWindow->m_realPosition = canvasCenter - halfSize;
    pWindow->m_size          = Vector2D{DEFAULT_WIDTH, DEFAULT_HEIGHT};
    *pWindow->m_realSize     = Vector2D{DEFAULT_WIDTH, DEFAULT_HEIGHT};

    m_windows.push_back(pWindow);
}

void CCanvasLayout::onWindowRemovedTiling(PHLWINDOW pWindow) {
    std::erase_if(m_windows, [&](const auto& ref) { return ref.lock() == pWindow; });
}

bool CCanvasLayout::isWindowTiled(PHLWINDOW pWindow) {
    return std::ranges::any_of(m_windows, [&](const auto& ref) { return ref.lock() == pWindow; });
}

void CCanvasLayout::recalculateMonitor(const MONITORID&) {
    //
}

void CCanvasLayout::recalculateWindow(PHLWINDOW) {
    //
}

void CCanvasLayout::resizeActiveWindow(const Vector2D& delta, eRectCorner, PHLWINDOW pWindow) {
    if (!pWindow)
        return;
    pWindow->m_size = pWindow->m_size + delta;
    *pWindow->m_realSize = pWindow->m_size;
}

void CCanvasLayout::fullscreenRequestForWindow(PHLWINDOW, eFullscreenMode, eFullscreenMode) {
    //
}

std::any CCanvasLayout::layoutMessage(SLayoutMessageHeader, std::string) {
    return {};
}

SWindowRenderLayoutHints CCanvasLayout::requestRenderHints(PHLWINDOW) {
    return {};
}

void CCanvasLayout::switchWindows(PHLWINDOW, PHLWINDOW) {
    //
}

void CCanvasLayout::moveWindowTo(PHLWINDOW, const std::string&, bool) {
    //
}

void CCanvasLayout::alterSplitRatio(PHLWINDOW, float, bool) {
    //
}

std::string CCanvasLayout::getLayoutName() {
    return "canvas";
}

void CCanvasLayout::replaceWindowDataWith(PHLWINDOW from, PHLWINDOW to) {
    for (auto& ref : m_windows) {
        if (ref.lock() == from) {
            ref = to;
            return;
        }
    }
}

Vector2D CCanvasLayout::predictSizeForNewWindowTiled() {
    return Vector2D{DEFAULT_WIDTH, DEFAULT_HEIGHT};
}
