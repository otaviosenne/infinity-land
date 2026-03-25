#include "CanvasMinimap.hpp"
#include "CanvasViewport.hpp"
#include "../Compositor.hpp"
#include "../render/pass/RectPassElement.hpp"
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

    return {minX - BOUNDS_PADDING, minY - BOUNDS_PADDING,
            (maxX - minX) + BOUNDS_PADDING * 2,
            (maxY - minY) + BOUNDS_PADDING * 2};
}

CBox CCanvasMinimap::minimapScreenBox(PHLMONITOR pMonitor) const {
    return {pMonitor->m_position.x + pMonitor->m_size.x - MINIMAP_WIDTH - MINIMAP_MARGIN,
            pMonitor->m_position.y + pMonitor->m_size.y - MINIMAP_HEIGHT - MINIMAP_MARGIN,
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
    const auto canvasBBox  = canvasBounds();
    if (canvasBBox.w <= 0 || canvasBBox.h <= 0)
        return;

    const auto mmBox = minimapScreenBox(pMonitor);

    CRectPassElement::SRectData bgData;
    bgData.box   = mmBox;
    bgData.color = CHyprColor(0.1, 0.1, 0.1, 0.7);
    bgData.round = 6;
    g_pHyprRenderer->m_renderPass.add(makeUnique<CRectPassElement>(bgData));

    for (const auto& w : g_pCompositor->m_windows) {
        if (w->isHidden() || !w->m_isMapped)
            continue;

        const auto wPos        = w->m_realPosition->goal();
        const auto wSize       = w->m_realSize->goal();
        const auto topLeft     = canvasToMinimap(wPos, canvasBBox, mmBox);
        const auto bottomRight = canvasToMinimap(wPos + wSize, canvasBBox, mmBox);

        CRectPassElement::SRectData winData;
        winData.box   = CBox{topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y};
        winData.color = CHyprColor(0.4, 0.6, 0.9, 0.8);
        winData.round = 2;
        g_pHyprRenderer->m_renderPass.add(makeUnique<CRectPassElement>(winData));
    }

    const auto vpTopLeft     = g_pCanvasViewport->screenToCanvas(pMonitor->m_position);
    const auto vpBottomRight = g_pCanvasViewport->screenToCanvas(pMonitor->m_position + pMonitor->m_size);
    const auto mmTopLeft     = canvasToMinimap(vpTopLeft, canvasBBox, mmBox);
    const auto mmBottomRight = canvasToMinimap(vpBottomRight, canvasBBox, mmBox);

    CRectPassElement::SRectData vpData;
    vpData.box   = CBox{mmTopLeft.x, mmTopLeft.y, mmBottomRight.x - mmTopLeft.x, mmBottomRight.y - mmTopLeft.y};
    vpData.color = CHyprColor(1.0, 1.0, 1.0, 0.3);
    vpData.round = 1;
    g_pHyprRenderer->m_renderPass.add(makeUnique<CRectPassElement>(vpData));
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
