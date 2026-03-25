#pragma once
#include "../defines.hpp"

class CCanvasViewport {
  public:
    Vector2D canvasToScreen(const Vector2D& canvas) const;
    Vector2D screenToCanvas(const Vector2D& screen) const;
    CBox     canvasToScreen(const CBox& box) const;

    void     pan(const Vector2D& delta);
    void     zoom(double factor, const Vector2D& screenAnchor);

    double   scale() const;
    Vector2D offset() const;

    void     setViewCenter(const Vector2D& canvasPos);
    void     zoomToFit(const CBox& canvasBox);
    void     damageAllMonitors();

  private:
    Vector2D m_offset = {0, 0};
    double   m_scale  = 1.0;

    static constexpr double MIN_SCALE = 0.1;
    static constexpr double MAX_SCALE = 3.0;
};

inline UP<CCanvasViewport> g_pCanvasViewport;
