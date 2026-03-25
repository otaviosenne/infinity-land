#pragma once
#include "../defines.hpp"
#include "../helpers/Monitor.hpp"
#include "../helpers/Color.hpp"
#include "../render/Texture.hpp"

struct SAnnotationStroke {
    std::vector<Vector2D> points;
    CHyprColor            color     = CHyprColor(1.0, 0.3, 0.3, 1.0);
    float                 thickness = 4.0f;
    uint64_t              id        = 0;
};

class CCanvasAnnotation {
  public:
    void                                  addStroke(SAnnotationStroke stroke);
    void                                  removeStroke(uint64_t id);
    SAnnotationStroke*                    findNearestStroke(const Vector2D& canvasPos, double threshold = 20.0);
    void                                  render(PHLMONITOR pMonitor, const CRegion& damage);
    void                                  markDirty();
    const std::vector<SAnnotationStroke>& strokes() const;

  private:
    std::vector<SAnnotationStroke> m_strokes;
    SP<CTexture>                   m_texture;
    bool                           m_dirty      = true;
    uint64_t                       m_nextId     = 1;
    int                            m_lastWidth  = 0;
    int                            m_lastHeight = 0;

    double distanceToSegment(const Vector2D& point, const Vector2D& segA, const Vector2D& segB) const;
};

inline UP<CCanvasAnnotation> g_pCanvasAnnotation;
