#include "CanvasLayout.hpp"
#include "CanvasSnap.hpp"
#include "../canvas/CanvasCommand.hpp"
#include "../canvas/CanvasFrameManager.hpp"
#include "../canvas/CanvasPersistence.hpp"
#include "../canvas/CanvasTags.hpp"
#include "../canvas/CanvasUndoRedo.hpp"
#include "../canvas/CanvasViews.hpp"
#include "../canvas/CanvasViewport.hpp"
#include "../Compositor.hpp"
#include "../helpers/Monitor.hpp"
#include "../desktop/Window.hpp"
#include "../managers/input/InputManager.hpp"
#include "../managers/PointerManager.hpp"
#include "../render/Renderer.hpp"
#include "../managers/KeybindManager.hpp"

void CCanvasLayout::onEnable() {
    g_pCanvasTags        = makeUnique<CCanvasTags>();
    g_pCanvasViews       = makeUnique<CCanvasViews>();
    g_pCanvasPersistence = makeUnique<CCanvasPersistence>();
    g_pCanvasPersistence->load();
    g_pCanvasUndoRedo = makeUnique<CCanvasUndoRedo>();
}

void CCanvasLayout::onDisable() {
    if (g_pCanvasPersistence)
        g_pCanvasPersistence->scheduleSave();
    m_windows.clear();
}

void CCanvasLayout::onWindowCreatedTiling(PHLWINDOW pWindow, eDirection) {
    const auto appClass = pWindow->m_initialClass;
    const auto saved    = g_pCanvasPersistence->findByClass(appClass);

    const auto cursorScreen = g_pPointerManager->position();
    const auto canvasCursor = g_pCanvasViewport->screenToCanvas(cursorScreen);

    const bool cursorInFrame = g_pCanvasFrameManager &&
        g_pCanvasFrameManager->frameAtCanvasPos(canvasCursor) != nullptr;

    Vector2D pos, size;

    if (!cursorInFrame) {
        const auto appDefault = g_pCanvasPersistence->getAppDefault(appClass);
        const auto PMONITOR   = g_pCompositor->getMonitorFromID(pWindow->monitorID());
        const auto monSize    = PMONITOR->m_size;

        const auto savedSize = saved.has_value() ? saved->size : appDefault.size;
        size.x = std::min(savedSize.x, monSize.x * 0.85);
        size.y = std::min(savedSize.y, monSize.y * 0.85);

        pos = canvasCursor - size / 2.0;

        const auto viewTopLeft  = g_pCanvasViewport->screenToCanvas(Vector2D{0, 0});
        const auto viewBotRight = g_pCanvasViewport->screenToCanvas(PMONITOR->m_size);
        pos.x = std::clamp(pos.x, viewTopLeft.x, std::max(viewTopLeft.x, viewBotRight.x - size.x));
        pos.y = std::clamp(pos.y, viewTopLeft.y, std::max(viewTopLeft.y, viewBotRight.y - size.y));

        std::vector<CBox> otherBoxes;
        for (const auto& ref : m_windows) {
            const auto w = ref.lock();
            if (!w)
                continue;
            otherBoxes.emplace_back(CBox{w->m_position.x, w->m_position.y, w->m_size.x, w->m_size.y});
        }
        pos = resolveOverlap(pos, size, otherBoxes);
        pos.x = std::clamp(pos.x, viewTopLeft.x, std::max(viewTopLeft.x, viewBotRight.x - size.x));
        pos.y = std::clamp(pos.y, viewTopLeft.y, std::max(viewTopLeft.y, viewBotRight.y - size.y));
    } else {
        pos  = canvasCursor;
        size = {DEFAULT_WIDTH, DEFAULT_HEIGHT};
    }

    pWindow->m_position      = pos;
    *pWindow->m_realPosition = pos;
    pWindow->m_size          = size;
    *pWindow->m_realSize     = size;

    m_windows.push_back(pWindow);

    if (g_pCanvasFrameManager)
        g_pCanvasFrameManager->assignWindowIfInsideFrame(pWindow);

    g_pCanvasPersistence->trackWindow(appClass, pos, size);
    g_pCanvasPersistence->scheduleSave();
    g_pCanvasViewport->damageAllMonitors();
}

void CCanvasLayout::onWindowRemovedTiling(PHLWINDOW pWindow) {
    g_pCanvasPersistence->trackWindow(pWindow->m_initialClass, pWindow->m_position, pWindow->m_size);
    g_pCanvasPersistence->scheduleSave();
    std::erase_if(m_windows, [&](const auto& ref) { return ref.lock() == pWindow; });
    g_pCanvasViewport->damageAllMonitors();
}

bool CCanvasLayout::isWindowTiled(PHLWINDOW pWindow) {
    return std::ranges::any_of(m_windows, [&](const auto& ref) { return ref.lock() == pWindow; });
}

void CCanvasLayout::recalculateMonitor(const MONITORID&) {
    g_pCanvasViewport->damageAllMonitors();
}

void CCanvasLayout::recalculateWindow(PHLWINDOW) {
    g_pCanvasViewport->damageAllMonitors();
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

    g_pCanvasPersistence->trackWindow(pWindow->m_initialClass, pWindow->m_position, pWindow->m_size);
    g_pCanvasPersistence->scheduleSave();
}

void CCanvasLayout::fullscreenRequestForWindow(PHLWINDOW, eFullscreenMode, eFullscreenMode) {
    //
}

std::any CCanvasLayout::layoutMessage(SLayoutMessageHeader header, std::string msg) {
    if (msg == "fitmonitor" && header.pWindow)
        fitWindowToMonitor(header.pWindow);
    else if (msg == "fitwindow80" && header.pWindow)
        fitWindowTo80Percent(header.pWindow);
    else if (msg == "createframe" && g_pCanvasFrameManager)
        g_pCanvasFrameManager->createFrame();
    else if (msg == "assigntoframe" && g_pCanvasFrameManager)
        g_pCanvasFrameManager->assignFocusedWindow();
    else if (msg == "nexttab" && g_pCanvasFrameManager)
        g_pCanvasFrameManager->nextTab();
    else if (msg == "prevtab" && g_pCanvasFrameManager)
        g_pCanvasFrameManager->prevTab();
    else if (msg.rfind("frametab ", 0) == 0 && g_pCanvasFrameManager) {
        const int idx = std::stoi(msg.substr(9)) - 1;
        g_pCanvasFrameManager->switchTab(idx);
    } else if (msg == "deleteframe" && g_pCanvasFrameManager) {
        const auto* frame = g_pCanvasFrameManager->activeFrame();
        if (frame)
            g_pCanvasFrameManager->deleteFrame(frame->id());
    }
    return {};
}

void CCanvasLayout::fitWindowToMonitor(PHLWINDOW pWindow) {
    if (!validMapped(pWindow))
        return;

    const auto PMONITOR = g_pCompositor->getMonitorFromID(pWindow->monitorID());
    if (!PMONITOR)
        return;

    const auto newSize   = PMONITOR->m_size;
    const auto candidate = g_pCanvasViewport->screenToCanvas(Vector2D{0, 0});

    std::vector<CBox> otherBoxes;
    for (const auto& ref : m_windows) {
        const auto w = ref.lock();
        if (!w || w == pWindow)
            continue;
        otherBoxes.emplace_back(CBox{w->m_position.x, w->m_position.y, w->m_size.x, w->m_size.y});
    }

    const auto newPos = resolveOverlap(candidate, newSize, otherBoxes);

    pWindow->m_position = newPos;
    pWindow->m_size     = newSize;
    pWindow->m_realPosition->setValueAndWarp(newPos);
    pWindow->m_realSize->setValueAndWarp(newSize);
    pWindow->sendWindowSize();
    pWindow->updateWindowDecos();
    g_pHyprRenderer->damageWindow(pWindow, true);

    g_pCanvasPersistence->trackWindow(pWindow->m_initialClass, newPos, newSize);
    g_pCanvasPersistence->scheduleSave();

    g_pCanvasViewport->snapViewToBox(CBox{newPos.x, newPos.y, newSize.x, newSize.y});
    g_pHyprRenderer->damageWindow(pWindow, true);
}

void CCanvasLayout::fitWindowTo80Percent(PHLWINDOW pWindow) {
    if (!validMapped(pWindow))
        return;

    const auto PMONITOR = g_pCompositor->getMonitorFromID(pWindow->monitorID());
    if (!PMONITOR)
        return;

    const auto newSize = PMONITOR->m_size * 0.8;
    const auto center  = g_pCanvasViewport->screenToCanvas(PMONITOR->m_position + PMONITOR->m_size / 2.0);
    const auto newPos  = center - newSize / 2.0;

    pWindow->m_position = newPos;
    pWindow->m_size     = newSize;
    pWindow->m_realPosition->setValueAndWarp(newPos);
    pWindow->m_realSize->setValueAndWarp(newSize);
    pWindow->sendWindowSize();
    pWindow->updateWindowDecos();
    g_pHyprRenderer->damageWindow(pWindow, true);
    g_pCanvasViewport->damageAllMonitors();
    g_pCanvasPersistence->trackWindow(pWindow->m_initialClass, newPos, newSize);
    g_pCanvasPersistence->scheduleSave();
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

    m_dragIsResize = isResize;

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
    const auto DRAGGINGWINDOW = g_pInputManager->m_currentlyDraggedWindow.lock();
    if (DRAGGINGWINDOW) {
        g_pCanvasPersistence->trackWindow(DRAGGINGWINDOW->m_initialClass, DRAGGINGWINDOW->m_position, DRAGGINGWINDOW->m_size);
        g_pCanvasPersistence->scheduleSave();

        const bool assignedToFrame = g_pCanvasFrameManager &&
            g_pCanvasFrameManager->assignWindowIfInsideFrame(DRAGGINGWINDOW);

        if (!assignedToFrame) {
            std::vector<CBox> otherBoxes;
            for (const auto& ref : m_windows) {
                const auto w = ref.lock();
                if (!w || w == DRAGGINGWINDOW)
                    continue;
                otherBoxes.emplace_back(CBox{w->m_position.x, w->m_position.y, w->m_size.x, w->m_size.y});
            }

            const auto resolved = resolveOverlap(DRAGGINGWINDOW->m_position, DRAGGINGWINDOW->m_size, otherBoxes);
            if (resolved.x != DRAGGINGWINDOW->m_position.x || resolved.y != DRAGGINGWINDOW->m_position.y) {
                DRAGGINGWINDOW->m_position = resolved;
                g_pHyprRenderer->damageWindow(DRAGGINGWINDOW);
                *DRAGGINGWINDOW->m_realPosition = resolved;
                g_pHyprRenderer->damageWindow(DRAGGINGWINDOW);
                g_pCanvasPersistence->trackWindow(DRAGGINGWINDOW->m_initialClass, resolved, DRAGGINGWINDOW->m_size);
                g_pCanvasPersistence->scheduleSave();
            }
        }

        if (g_pCanvasUndoRedo) {
            const auto finalPos  = DRAGGINGWINDOW->m_position;
            const auto finalSize = DRAGGINGWINDOW->m_size;

            if (m_dragIsResize) {
                if (finalPos != m_canvasDragStartPos || finalSize != m_canvasDragStartSize)
                    g_pCanvasUndoRedo->push(makeUnique<WindowResizeCommand>(DRAGGINGWINDOW, m_canvasDragStartPos, m_canvasDragStartSize, finalPos, finalSize));
            } else {
                if (finalPos != m_canvasDragStartPos)
                    g_pCanvasUndoRedo->push(makeUnique<WindowMoveCommand>(DRAGGINGWINDOW, m_canvasDragStartPos, finalPos));
            }
        }
    }
    g_pInputManager->unsetCursorImage();
    g_pInputManager->m_currentlyDraggedWindow.reset();
    g_pInputManager->m_wasDraggingWindow = true;
}

void CCanvasLayout::onMouseMove(const Vector2D& mousePos) {
    if (g_pCanvasFrameManager && g_pCanvasFrameManager->isFrameDragging()) {
        g_pCanvasFrameManager->updateFrameDrag(mousePos);
        return;
    }

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
        DRAGGINGWINDOW->updateWindowDecos();
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
