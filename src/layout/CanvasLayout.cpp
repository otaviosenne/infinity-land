#include "CanvasLayout.hpp"
#include "../canvas/CanvasViewport.hpp"
#include "../Compositor.hpp"
#include "../helpers/Monitor.hpp"
#include "../desktop/Window.hpp"
#include "../managers/input/InputManager.hpp"
#include "../render/Renderer.hpp"
#include "../managers/KeybindManager.hpp"

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
    const auto canvasDelta = delta / g_pCanvasViewport->scale();
    const auto minSize     = Vector2D{MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT};
    auto       newSize     = (pWindow->m_size + canvasDelta).clamp(minSize);
    pWindow->m_size = newSize;
    pWindow->m_realSize->setValueAndWarp(newSize);
    pWindow->sendWindowSize();
    g_pCanvasViewport->damageAllMonitors();
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

eRectCorner CCanvasLayout::determineDragCorner(const Vector2D& mousePos, const CBox& windowBox) const {
    const bool left = mousePos.x < windowBox.x + windowBox.w / 2.0;
    const bool top  = mousePos.y < windowBox.y + windowBox.h / 2.0;

    if (left && top)
        return CORNER_TOPLEFT;
    if (!left && top)
        return CORNER_TOPRIGHT;
    if (left && !top)
        return CORNER_BOTTOMLEFT;
    return CORNER_BOTTOMRIGHT;
}

void CCanvasLayout::onBeginDragWindow() {
    const auto DRAGGINGWINDOW = g_pInputManager->m_currentlyDraggedWindow.lock();

    if (!validMapped(DRAGGINGWINDOW)) {
        Debug::log(ERR, "CCanvasLayout: drag attempted on invalid window");
        g_pKeybindManager->changeMouseBindMode(MBIND_INVALID);
        return;
    }

    m_dragStartMousePos  = g_pInputManager->getMouseCoordsInternal();
    m_canvasDragStartPos  = DRAGGINGWINDOW->m_realPosition->goal();
    m_canvasDragStartSize = DRAGGINGWINDOW->m_realSize->goal();

    const auto screenBox = g_pCanvasViewport->canvasToScreen(CBox{m_canvasDragStartPos, m_canvasDragStartSize});
    m_dragCorner         = determineDragCorner(m_dragStartMousePos, screenBox);

    const bool isResize = g_pInputManager->m_dragMode == MBIND_RESIZE ||
                          g_pInputManager->m_dragMode == MBIND_RESIZE_FORCE_RATIO ||
                          g_pInputManager->m_dragMode == MBIND_RESIZE_BLOCK_RATIO;

    if (isResize) {
        static const std::array<std::string, 4> CURSOR_NAMES = {"nw-resize", "ne-resize", "sw-resize", "se-resize"};
        static const std::array<eRectCorner, 4> CORNERS      = {CORNER_TOPLEFT, CORNER_TOPRIGHT, CORNER_BOTTOMLEFT, CORNER_BOTTOMRIGHT};
        for (size_t i = 0; i < CORNERS.size(); ++i) {
            if (m_dragCorner == CORNERS[i]) {
                g_pInputManager->setCursorImageUntilUnset(CURSOR_NAMES[i]);
                break;
            }
        }
    } else {
        g_pInputManager->setCursorImageUntilUnset("grabbing");
    }

    g_pHyprRenderer->damageWindow(DRAGGINGWINDOW);
    g_pCompositor->focusWindow(DRAGGINGWINDOW);
    g_pCompositor->changeWindowZOrder(DRAGGINGWINDOW, true);
}

void CCanvasLayout::onEndDragWindow() {
    g_pInputManager->unsetCursorImage();
    g_pInputManager->m_currentlyDraggedWindow.reset();
    g_pInputManager->m_wasDraggingWindow = true;
}

void CCanvasLayout::onMouseMove(const Vector2D& mousePos) {
    if (g_pInputManager->m_currentlyDraggedWindow.expired())
        return;

    const auto DRAGGINGWINDOW = g_pInputManager->m_currentlyDraggedWindow.lock();

    if (!validMapped(DRAGGINGWINDOW)) {
        g_pKeybindManager->changeMouseBindMode(MBIND_INVALID);
        return;
    }

    const auto screenDelta = mousePos - m_dragStartMousePos;
    const auto canvasDelta = screenDelta / g_pCanvasViewport->scale();
    const auto minSize     = Vector2D{MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT};

    if (g_pInputManager->m_dragMode == MBIND_MOVE) {
        const auto newPos = m_canvasDragStartPos + canvasDelta;
        DRAGGINGWINDOW->m_realPosition->setValueAndWarp(newPos);
        DRAGGINGWINDOW->m_position = newPos;
    } else if (g_pInputManager->m_dragMode == MBIND_RESIZE ||
               g_pInputManager->m_dragMode == MBIND_RESIZE_FORCE_RATIO ||
               g_pInputManager->m_dragMode == MBIND_RESIZE_BLOCK_RATIO) {

        auto newSize = m_canvasDragStartSize;
        auto newPos  = m_canvasDragStartPos;

        if (m_dragCorner == CORNER_BOTTOMRIGHT)
            newSize = newSize + canvasDelta;
        else if (m_dragCorner == CORNER_TOPLEFT)
            newSize = newSize - canvasDelta;
        else if (m_dragCorner == CORNER_TOPRIGHT)
            newSize = newSize + Vector2D(canvasDelta.x, -canvasDelta.y);
        else if (m_dragCorner == CORNER_BOTTOMLEFT)
            newSize = newSize + Vector2D(-canvasDelta.x, canvasDelta.y);

        newSize = newSize.clamp(minSize);

        if (m_dragCorner == CORNER_TOPLEFT)
            newPos = newPos - newSize + m_canvasDragStartSize;
        else if (m_dragCorner == CORNER_TOPRIGHT)
            newPos = newPos + Vector2D(0.0, (m_canvasDragStartSize - newSize).y);
        else if (m_dragCorner == CORNER_BOTTOMLEFT)
            newPos = newPos + Vector2D((m_canvasDragStartSize - newSize).x, 0.0);

        DRAGGINGWINDOW->m_realSize->setValueAndWarp(newSize);
        DRAGGINGWINDOW->m_realPosition->setValueAndWarp(newPos);
        DRAGGINGWINDOW->m_size     = newSize;
        DRAGGINGWINDOW->m_position = newPos;
        DRAGGINGWINDOW->sendWindowSize();
    }

    g_pCanvasViewport->damageAllMonitors();
}

void CCanvasLayout::moveActiveWindow(const Vector2D& delta, PHLWINDOW pWindow) {
    auto window = pWindow ? pWindow : g_pCompositor->m_lastWindow.lock();

    if (!validMapped(window))
        return;

    const auto canvasDelta = delta / g_pCanvasViewport->scale();
    const auto newPos      = window->m_position + canvasDelta;
    window->m_position = newPos;
    window->m_realPosition->setValueAndWarp(newPos);
    g_pCanvasViewport->damageAllMonitors();
}
