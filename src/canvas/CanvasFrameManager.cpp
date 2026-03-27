#include "CanvasFrameManager.hpp"
#include "CanvasViewport.hpp"
#include "../Compositor.hpp"
#include <algorithm>

void CCanvasFrameManager::createFrame() {
    if (!g_pCanvasViewport) return;

    const auto pMonitor = g_pCompositor->m_lastMonitor.lock();
    if (!pMonitor) return;

    const Vector2D frameSize = pMonitor->m_size;
    const auto     vpCenter  = g_pCanvasViewport->screenToCanvas(
        pMonitor->m_position + pMonitor->m_size / 2.0);
    const Vector2D framePos = vpCenter - frameSize / 2.0;

    const uint64_t id = m_nextId++;
    m_frames.push_back(makeUnique<CCanvasFrame>(id, framePos, frameSize));
    m_activeId = id;
}

void CCanvasFrameManager::deleteFrame(uint64_t id) {
    m_frames.erase(
        std::remove_if(m_frames.begin(), m_frames.end(),
            [id](const UP<CCanvasFrame>& f) { return f->id() == id; }),
        m_frames.end());

    if (m_activeId == id)
        m_activeId = m_frames.empty() ? 0 : m_frames.back()->id();
}

void CCanvasFrameManager::assignFocusedWindow() {
    const auto pWindow = g_pCompositor->m_lastWindow.lock();
    if (!pWindow || m_frames.empty()) return;

    auto* frame = activeFrame();
    if (frame)
        frame->assignWindow(pWindow);
}

void CCanvasFrameManager::switchTab(int tabIndex) {
    auto* frame = activeFrame();
    if (frame)
        frame->setActiveTab(tabIndex);
}

void CCanvasFrameManager::nextTab() {
    auto* frame = activeFrame();
    if (!frame) return;
    frame->setActiveTab((frame->activeTabIndex() + 1) % (int)frame->tabs().size());
}

void CCanvasFrameManager::prevTab() {
    auto* frame = activeFrame();
    if (!frame) return;
    const int prev = (frame->activeTabIndex() - 1 + (int)frame->tabs().size()) % (int)frame->tabs().size();
    frame->setActiveTab(prev);
}

bool CCanvasFrameManager::isWindowInAnyFrame(PHLWINDOW pWindow) const {
    for (const auto& f : m_frames)
        if (f->hasWindow(pWindow)) return true;
    return false;
}

void CCanvasFrameManager::renderAll(PHLMONITOR pMonitor, const CRegion& damage) {
    for (const auto& frame : m_frames)
        frame->render(pMonitor);
}

CCanvasFrame* CCanvasFrameManager::activeFrame() const {
    if (m_frames.empty()) return nullptr;
    for (const auto& f : m_frames)
        if (f->id() == m_activeId) return f.get();
    return m_frames.back().get();
}
