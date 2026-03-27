#include "CanvasFrame.hpp"
#include "CanvasViewport.hpp"
#include "../Compositor.hpp"
#include "../render/Renderer.hpp"
#include "../render/pass/RectPassElement.hpp"
#include <cmath>
#include <algorithm>

CCanvasFrame::CCanvasFrame(uint64_t id, const Vector2D& pos, const Vector2D& size)
    : m_id(id), m_position(pos), m_size(size) {
    m_tabs.push_back({.index = 1});
}

std::vector<CBox> CCanvasFrame::computeTileLayout(int count) const {
    const double contentY = 0;
    const double contentW = m_size.x;
    const double contentH = m_size.y;

    if (count <= 0) return {};
    if (count == 1) return {{0, contentY, contentW, contentH}};
    if (count == 2) return {
        {0,              contentY, contentW / 2, contentH},
        {contentW / 2,   contentY, contentW / 2, contentH}
    };
    if (count == 3) return {
        {0,            contentY,                contentW / 2, contentH},
        {contentW / 2, contentY,                contentW / 2, contentH / 2},
        {contentW / 2, contentY + contentH / 2, contentW / 2, contentH / 2}
    };

    const int    cols = (int)std::ceil(std::sqrt((double)count));
    const int    rows = (int)std::ceil((double)count / cols);
    const double tw   = contentW / cols;
    const double th   = contentH / rows;

    std::vector<CBox> tiles;
    for (int i = 0; i < count; i++)
        tiles.push_back({(i % cols) * tw, contentY + (i / cols) * th, tw, th});

    return tiles;
}

void CCanvasFrame::applyTileToWindow(PHLWINDOW pWindow, const CBox& tileBox) {
    const Vector2D newPos  = {m_position.x + tileBox.x, m_position.y + tileBox.y};
    const Vector2D newSize = {tileBox.w, tileBox.h};

    pWindow->m_position = newPos;
    pWindow->m_size     = newSize;
    pWindow->m_realPosition->setValueAndWarp(newPos);
    pWindow->m_realSize->setValueAndWarp(newSize);
    pWindow->sendWindowSize();
    pWindow->updateWindowDecos();
}

void CCanvasFrame::parkWindow(PHLWINDOW pWindow, int tabIdx) {
    const Vector2D parkPos = {m_position.x, m_position.y + m_size.y + PARK_OFFSET + tabIdx * 120.0};
    pWindow->m_position = parkPos;
    pWindow->m_realPosition->setValueAndWarp(parkPos);
}

void CCanvasFrame::retile() {
    if (m_tabs.empty()) return;
    const auto& tab = m_tabs[m_activeTabIndex];

    std::vector<PHLWINDOW> mapped;
    for (const auto& ref : tab.windows) {
        const auto w = ref.lock();
        if (w && w->m_isMapped)
            mapped.push_back(w);
    }

    const auto tiles = computeTileLayout((int)mapped.size());
    for (size_t i = 0; i < mapped.size() && i < tiles.size(); i++)
        applyTileToWindow(mapped[i], tiles[i]);
}

void CCanvasFrame::assignWindow(PHLWINDOW pWindow) {
    for (auto& tab : m_tabs) {
        tab.windows.erase(
            std::remove_if(tab.windows.begin(), tab.windows.end(),
                [&](const PHLWINDOWREF& ref) { return ref.lock() == pWindow; }),
            tab.windows.end());
    }
    m_tabs[m_activeTabIndex].windows.push_back(pWindow);
    retile();
}

void CCanvasFrame::unassignWindow(PHLWINDOW pWindow) {
    for (auto& tab : m_tabs) {
        tab.windows.erase(
            std::remove_if(tab.windows.begin(), tab.windows.end(),
                [&](const PHLWINDOWREF& ref) { return ref.lock() == pWindow; }),
            tab.windows.end());
    }
    retile();
}

bool CCanvasFrame::hasWindow(PHLWINDOW pWindow) const {
    for (const auto& tab : m_tabs)
        for (const auto& ref : tab.windows)
            if (ref.lock() == pWindow) return true;
    return false;
}

void CCanvasFrame::setActiveTab(int tabIndex) {
    if (tabIndex < 0 || tabIndex >= (int)m_tabs.size() || tabIndex == m_activeTabIndex)
        return;

    for (const auto& ref : m_tabs[m_activeTabIndex].windows) {
        const auto w = ref.lock();
        if (w && w->m_isMapped)
            parkWindow(w, m_activeTabIndex);
    }

    m_activeTabIndex = tabIndex;
    retile();
}

void CCanvasFrame::addTab() {
    const int nextIndex = (int)m_tabs.size() + 1;
    m_tabs.push_back({.index = nextIndex});
    setActiveTab((int)m_tabs.size() - 1);
}

void CCanvasFrame::removeActiveTab() {
    if ((int)m_tabs.size() <= 1) return;

    for (const auto& ref : m_tabs[m_activeTabIndex].windows) {
        const auto w = ref.lock();
        if (w && w->m_isMapped)
            parkWindow(w, m_activeTabIndex);
    }

    m_tabs.erase(m_tabs.begin() + m_activeTabIndex);
    m_activeTabIndex = std::max(0, m_activeTabIndex - 1);
    retile();
}

void CCanvasFrame::render(PHLMONITOR pMonitor) {
    if (!g_pCanvasViewport) return;

    const double scale    = g_pCanvasViewport->scale();
    const auto   screenTL = g_pCanvasViewport->canvasToScreen(m_position);
    const auto   screenBR = g_pCanvasViewport->canvasToScreen(m_position + m_size);
    const double sw       = screenBR.x - screenTL.x;
    const double sh       = screenBR.y - screenTL.y;

    if (sw <= 2 || sh <= 2) return;

    const double bw = std::max(1.0, BORDER_W * scale);

    auto addRect = [&](const CBox& box, const CHyprColor& color) {
        CRectPassElement::SRectData data;
        data.box   = box;
        data.color = color;
        data.round = 0;
        g_pHyprRenderer->m_renderPass.add(makeUnique<CRectPassElement>(data));
    };

    addRect({screenTL.x,      screenTL.y,      sw, bw}, CHyprColor(1.f, 1.f, 1.f, 0.85f));
    addRect({screenTL.x,      screenBR.y - bw, sw, bw}, CHyprColor(1.f, 1.f, 1.f, 0.85f));
    addRect({screenTL.x,      screenTL.y,      bw, sh}, CHyprColor(1.f, 1.f, 1.f, 0.85f));
    addRect({screenBR.x - bw, screenTL.y,      bw, sh}, CHyprColor(1.f, 1.f, 1.f, 0.85f));
}
