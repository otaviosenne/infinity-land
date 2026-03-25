#pragma once
#include "../defines.hpp"
#include "../helpers/Color.hpp"
#include "../helpers/math/Math.hpp"
#include "../render/Texture.hpp"
#include "../helpers/Monitor.hpp"
#include "CanvasDrawMode.hpp"

constexpr int    TOOLBAR_MARGIN  = 16;
constexpr int    BUTTON_SIZE     = 32;
constexpr int    BUTTON_GAP      = 6;
constexpr int    TOOLBAR_PADDING = 8;
constexpr int    SEPARATOR_GAP   = 16;

enum eButtonType : uint8_t {
    TBTN_BRUSH,
    TBTN_ERASER,
    TBTN_COLOR,
    TBTN_THICKNESS,
};

struct SToolbarButton {
    CBox        box;
    eButtonType type;
    CHyprColor  color;
    float       thickness = 0.0f;
    eDrawTool   tool      = DRAW_BRUSH;
};

class CCanvasToolbar {
  public:
    void render(PHLMONITOR pMonitor, const CRegion& damage);
    bool handleClick(const Vector2D& screenPos);
    bool containsPoint(const Vector2D& screenPos) const;

  private:
    SP<CTexture>              m_texture;
    bool                      m_dirty = true;
    std::vector<SToolbarButton> m_buttons;
    CBox                      m_bounds;

    void buildLayout(double monitorX, double monitorY, int monitorW, int monitorH);
};

inline UP<CCanvasToolbar> g_pCanvasToolbar;
