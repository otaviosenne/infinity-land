#pragma once
#include "../defines.hpp"
#include "../helpers/Color.hpp"
#include "CanvasAnnotation.hpp"

#include <string>

enum eDrawTool : uint8_t {
    DRAW_BRUSH  = 0,
    DRAW_ERASER = 1,
    DRAW_ARROW  = 2,
    DRAW_STICKY = 3,
    DRAW_TEXT   = 4,
};

enum eEraserMode : uint8_t {
    ERASER_OBJECT = 0,
};

class CCanvasDrawMode {
  public:
    void       toggle();
    bool       isActive() const;
    eDrawTool  activeTool() const;
    void       setTool(eDrawTool tool);
    void       setColor(const CHyprColor& color);
    void       setThickness(float thickness);
    CHyprColor activeColor() const;
    float      activeThickness() const;
    bool       handleMouseButton(const Vector2D& screenPos, bool pressed);
    bool       handleMouseMove(const Vector2D& screenPos);
    bool       isAwaitingText() const;
    void       submitText(const std::string& text);
    void       cancelText();

  private:
    bool              m_active       = false;
    eDrawTool         m_tool         = DRAW_BRUSH;
    CHyprColor        m_color        = CHyprColor(1.0, 0.3, 0.3, 1.0);
    float             m_thickness    = 4.0f;
    bool              m_drawing      = false;
    bool              m_awaitingText = false;
    SAnnotationStroke m_currentStroke;
    SAnnotationArrow  m_currentArrow;
    Vector2D          m_pendingPosition;
};

inline UP<CCanvasDrawMode> g_pCanvasDrawMode;
