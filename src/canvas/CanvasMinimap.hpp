#pragma once
#include "../defines.hpp"
#include "../helpers/Monitor.hpp"

class CCanvasMinimap {
  public:
    void render(PHLMONITOR pMonitor, const CRegion& damage);
    bool handleClick(const Vector2D& screenPos);

  private:
    CBox     minimapScreenBox(PHLMONITOR pMonitor) const;
    CBox     canvasBounds() const;
    Vector2D canvasToMinimap(const Vector2D& canvasPos, const CBox& canvasBBox, const CBox& minimapBox) const;
    double   zoomBarFillRatio(double scale) const;

    static constexpr int MINIMAP_WIDTH   = 200;
    static constexpr int MINIMAP_HEIGHT  = 120;
    static constexpr int MINIMAP_MARGIN  = 16;
    static constexpr int BOUNDS_PADDING  = 200;
};

inline UP<CCanvasMinimap> g_pCanvasMinimap;
