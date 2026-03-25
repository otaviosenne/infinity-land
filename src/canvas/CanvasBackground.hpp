#pragma once
#include "../defines.hpp"
#include "../helpers/Monitor.hpp"

class CCanvasBackground {
  public:
    void render(PHLMONITOR pMonitor, const CRegion& damage);

  private:
    float gridSpacingForScale(double scale) const;
};

inline UP<CCanvasBackground> g_pCanvasBackground;
