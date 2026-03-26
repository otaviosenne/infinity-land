#include "CanvasMinimap.hpp"
#include "CanvasViewport.hpp"
#include "../Compositor.hpp"
#include "../render/pass/MinimapPassElement.hpp"
#include "../render/Renderer.hpp"

CBox CCanvasMinimap::canvasBounds() const {
    double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
    bool   found = false;

    for (const auto& w : g_pCompositor->m_windows) {
        if (w->isHidden() || !w->m_isMapped)
            continue;

        const auto pos  = w->m_realPosition->goal();
        const auto size = w->m_realSize->goal();

        minX  = std::min(minX, pos.x);
        minY  = std::min(minY, pos.y);
        maxX  = std::max(maxX, pos.x + size.x);
        maxY  = std::max(maxY, pos.y + size.y);
        found = true;
    }

    if (!found)
        return {0, 0, 0, 0};

    const auto pMonitor = g_pCompositor->m_lastMonitor.lock();
    if (pMonitor && g_pCanvasViewport) {
        const auto vpTL = g_pCanvasViewport->screenToCanvas(pMonitor->m_position);
        const auto vpBR = g_pCanvasViewport->screenToCanvas(pMonitor->m_position + pMonitor->m_size);
        minX = std::min(minX, vpTL.x);
        minY = std::min(minY, vpTL.y);
        maxX = std::max(maxX, vpBR.x);
        maxY = std::max(maxY, vpBR.y);
    }

    return {minX - BOUNDS_PADDING, minY - BOUNDS_PADDING,
            (maxX - minX) + BOUNDS_PADDING * 2,
            (maxY - minY) + BOUNDS_PADDING * 2};
}

CBox CCanvasMinimap::minimapScreenBox(PHLMONITOR pMonitor) const {
    const auto monSize = pMonitor->m_transformedSize;
    return {monSize.x - MINIMAP_WIDTH - MINIMAP_MARGIN,
            monSize.y - MINIMAP_HEIGHT - MINIMAP_MARGIN,
            MINIMAP_WIDTH, MINIMAP_HEIGHT};
}

Vector2D CCanvasMinimap::canvasToMinimap(const Vector2D& canvasPos, const CBox& canvasBBox, const CBox& minimapBox) const {
    if (canvasBBox.w <= 0 || canvasBBox.h <= 0)
        return minimapBox.pos();

    const double scaleX    = minimapBox.w / canvasBBox.w;
    const double scaleY    = minimapBox.h / canvasBBox.h;
    const double fitScale  = std::min(scaleX, scaleY);

    const double mappedW = canvasBBox.w * fitScale;
    const double mappedH = canvasBBox.h * fitScale;
    const double offsetX = (minimapBox.w - mappedW) / 2.0;
    const double offsetY = (minimapBox.h - mappedH) / 2.0;

    return {minimapBox.x + offsetX + (canvasPos.x - canvasBBox.x) * fitScale,
            minimapBox.y + offsetY + (canvasPos.y - canvasBBox.y) * fitScale};
}

void CCanvasMinimap::render(PHLMONITOR pMonitor, const CRegion& damage) {
    return;
    const auto canvasBBox  = canvasBounds();
    const auto mmBox = minimapScreenBox(pMonitor);

    CMinimapPassElement::SMinimapData mmData;

    mmData.rects.push_back({mmBox, CHyprColor(0.1, 0.1, 0.1, 0.7), 6});

    if (canvasBBox.w <= 0 || canvasBBox.h <= 0) {
        const double zoomPct = g_pCanvasViewport ? g_pCanvasViewport->scale() : 1.0;
        const double barY = mmBox.y + mmBox.h + 6;
        const double barW = mmBox.w;
        const double barH = 4;
        const double barX = mmBox.x;
        mmData.rects.push_back({CBox{barX, barY, barW, barH}, CHyprColor(0.2, 0.2, 0.2, 0.6), 2});
        const double fillRatio = std::clamp((zoomPct - 0.1) / 2.9, 0.0, 1.0);
        mmData.rects.push_back({CBox{barX, barY, barW * fillRatio, barH}, CHyprColor(0.4, 0.6, 0.9, 0.9), 2});
        g_pHyprRenderer->m_renderPass.add(makeUnique<CMinimapPassElement>(mmData));
        return;
    }

    for (const auto& w : g_pCompositor->m_windows) {
        if (w->isHidden() || !w->m_isMapped)
            continue;

        const auto wPos        = w->m_realPosition->goal();
        const auto wSize       = w->m_realSize->goal();
        const auto topLeft     = canvasToMinimap(wPos, canvasBBox, mmBox);
        const auto bottomRight = canvasToMinimap(wPos + wSize, canvasBBox, mmBox);

        mmData.rects.push_back({
            CBox{topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y},
            CHyprColor(0.4, 0.6, 0.9, 0.8), 2});
    }

    const auto vpTopLeft     = g_pCanvasViewport->screenToCanvas(pMonitor->m_position);
    const auto vpBottomRight = g_pCanvasViewport->screenToCanvas(pMonitor->m_position + pMonitor->m_size);
    auto mmVpTL = canvasToMinimap(vpTopLeft, canvasBBox, mmBox);
    auto mmVpBR = canvasToMinimap(vpBottomRight, canvasBBox, mmBox);

    mmVpTL.x = std::max(mmVpTL.x, mmBox.x);
    mmVpTL.y = std::max(mmVpTL.y, mmBox.y);
    mmVpBR.x = std::min(mmVpBR.x, mmBox.x + mmBox.w);
    mmVpBR.y = std::min(mmVpBR.y, mmBox.y + mmBox.h);

    const double vpW = mmVpBR.x - mmVpTL.x;
    const double vpH = mmVpBR.y - mmVpTL.y;
    if (vpW > 0 && vpH > 0) {
        mmData.rects.push_back({
            CBox{mmVpTL.x, mmVpTL.y, vpW, vpH},
            CHyprColor(1.0, 1.0, 1.0, 0.2), 1});
    }

    const double zoomPct = g_pCanvasViewport->scale();
    const double barY = mmBox.y + mmBox.h + 6;
    const double barW = mmBox.w;
    const double barH = 4;
    const double barX = mmBox.x;

    mmData.rects.push_back({CBox{barX, barY, barW, barH}, CHyprColor(0.2, 0.2, 0.2, 0.6), 2});

    const double fillRatio = std::clamp((zoomPct - 0.1) / 2.9, 0.0, 1.0);
    const double fillW = barW * fillRatio;
    mmData.rects.push_back({CBox{barX, barY, fillW, barH}, CHyprColor(0.4, 0.6, 0.9, 0.9), 2});

    g_pHyprRenderer->m_renderPass.add(makeUnique<CMinimapPassElement>(mmData));
}

bool CCanvasMinimap::handleClick(const Vector2D& screenPos) {
    const auto pMonitor = g_pCompositor->m_lastMonitor.lock();
    if (!pMonitor)
        return false;

    const auto mmBox = minimapScreenBox(pMonitor);
    if (screenPos.x < mmBox.x || screenPos.x > mmBox.x + mmBox.w ||
        screenPos.y < mmBox.y || screenPos.y > mmBox.y + mmBox.h)
        return false;

    const auto canvasBBox = canvasBounds();
    if (canvasBBox.w <= 0 || canvasBBox.h <= 0)
        return false;

    const double scaleX   = mmBox.w / canvasBBox.w;
    const double scaleY   = mmBox.h / canvasBBox.h;
    const double fitScale = std::min(scaleX, scaleY);

    const double mappedW = canvasBBox.w * fitScale;
    const double mappedH = canvasBBox.h * fitScale;
    const double offsetX = (mmBox.w - mappedW) / 2.0;
    const double offsetY = (mmBox.h - mappedH) / 2.0;

    const double canvasX = canvasBBox.x + (screenPos.x - mmBox.x - offsetX) / fitScale;
    const double canvasY = canvasBBox.y + (screenPos.y - mmBox.y - offsetY) / fitScale;

    g_pCanvasViewport->setViewCenter({canvasX, canvasY});
    return true;
}
