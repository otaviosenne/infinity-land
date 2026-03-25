# Phase 2: Canvas Viewport - Research

**Researched:** 2026-03-25
**Domain:** Hyprland coordinate transform / custom layout
**Confidence:** HIGH

## Summary

This phase creates the infinite canvas viewport by implementing a `CCanvasLayout` (IHyprLayout) and a `CCanvasViewport` singleton for coordinate transforms. The codebase has clean integration points: LayoutManager accepts new layouts via `addLayout()`, renderWindow builds textureBox from `m_realPosition`, and `vectorToWindowUnified` / `vectorToSurfaceLocal` use `m_realPosition` for hit-testing.

The critical insight is that `m_realPosition` is the animated position used by BOTH rendering and input. The canvas transform must intercept at two points: (1) rendering path in `renderWindow()` line 481-485 where REALPOS and textureBox are computed, and (2) input path in `vectorToWindowUnified()` and `vectorToSurfaceLocal()` where `m_realPosition` / `getWindowBoxUnified()` are used for containment checks.

**Primary recommendation:** Keep windows in canvas coordinates (m_position/m_realPosition stay in canvas space). Apply forward transform (canvas-to-screen) only at render time and inverse transform (screen-to-canvas) only at input time. Do not modify window positions themselves.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- Single global CCanvasViewport class owning offset (pan), scale (zoom), and transform math
- One viewport spans all monitors (NOT per-monitor)
- Screen-to-canvas: `canvasPos = (screenPos - m_offset) / m_scale`
- Canvas-to-screen: `screenPos = canvasPos * m_scale + m_offset`
- Pan via middle mouse or space+left-click drag
- Zoom with scroll wheel, linear (no easing)
- CCanvasLayout : public IHyprLayout for window positioning
- Rendering: apply canvas transform in renderWindow(), m_realPosition stays in canvas coords
- Input: inverse-transform mouse position in vectorToWindowUnified()
- Both monitors share same canvas

### Claude's Discretion
- File structure (suggested: src/layout/CanvasLayout.cpp/hpp, src/canvas/CanvasViewport.cpp/hpp)
- Zoom min/max bounds (suggested: 0.1 to 3.0)
- Pan speed sensitivity
- How to register canvas layout (config keyword vs hardcoded)

### Deferred Ideas (OUT OF SCOPE)
- Dot grid background (Phase 5)
- Glow animation (Phase 5)
- Window drag/resize on canvas (Phase 3)
- Layer-shell exclusion from transform (Phase 4)
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|-----------------|
| CANV-01 | Pan with middle mouse or space+drag | Input interception in vectorToWindowUnified, mouse event handling in layout onBeginDragWindow/onMouseMove |
| CANV-02 | Zoom with scroll wheel (linear, fast) | CCanvasViewport scale field, re-render on scale change, textureBox scaling in renderWindow |
| CANV-05 | Canvas spans both monitors as one surface | Monitor m_position offsets already in global space; viewport transform applied per-monitor in renderMonitor; each monitor renders its portion of the canvas |
</phase_requirements>

## Architecture Patterns

### Recommended File Structure
```
src/
├── canvas/
│   ├── CanvasViewport.hpp    # CCanvasViewport class (singleton)
│   └── CanvasViewport.cpp    # Transform math, pan/zoom state
├── layout/
│   ├── CanvasLayout.hpp      # CCanvasLayout : IHyprLayout
│   └── CanvasLayout.cpp      # Window positioning in canvas space
```

### Pattern 1: Layout Registration

LayoutManager uses a simple vector of (name, pointer) pairs. Registration:

```cpp
// In LayoutManager.hpp — add member:
CCanvasLayout m_canvasLayout;

// In LayoutManager.cpp constructor — add:
m_layouts.emplace_back(std::make_pair<>("canvas", &m_canvasLayout));

// In LayoutManager.hpp enum:
LAYOUT_CANVAS // after LAYOUT_MASTER
```

Then switch via: `g_pLayoutManager->switchToLayout("canvas")` — can be triggered by config keyword `general:layout = canvas` or hardcoded initially.

**Confidence: HIGH** — directly read from LayoutManager.cpp lines 3-6 and 28-37.

### Pattern 2: Render Transform Injection

In `CHyprRenderer::renderWindow()` at line 481:
```cpp
// CURRENT CODE:
const auto REALPOS = pWindow->m_realPosition->value() + (pWindow->m_pinned ? Vector2D{} : PWORKSPACE->m_renderOffset->value());
CBox textureBox = {REALPOS.x, REALPOS.y, std::max(pWindow->m_realSize->value().x, 5.0), std::max(pWindow->m_realSize->value().y, 5.0)};

// CANVAS TRANSFORM — insert after textureBox creation:
// textureBox.x = textureBox.x * scale + offset.x
// textureBox.y = textureBox.y * scale + offset.y
// textureBox.w *= scale
// textureBox.h *= scale
```

The textureBox is then used for renderdata.pos and renderdata.w/h (lines 487-490). Transform here affects all downstream rendering.

**Confidence: HIGH** — read directly from Renderer.cpp:465-490.

### Pattern 3: Input Transform Injection

`vectorToWindowUnified()` (Compositor.cpp:878) calls `getWindowBoxUnified()` which returns a CBox from `m_realPosition`. The `containsPoint()` check uses the raw mouse position from `g_pPointerManager->position()`.

Two injection points:
1. **vectorToWindowUnified** — inverse-transform the `pos` parameter before all window matching
2. **vectorToSurfaceLocal** (Compositor.cpp:1057) — subtracts `m_realPosition->goal()` from vec. Must inverse-transform vec first, OR account for canvas coords in the subtraction.

The cleanest approach: transform the input position at the TOP of `vectorToWindowUnified` before any window iteration:
```cpp
const auto canvasPos = g_pCanvasViewport->screenToCanvas(pos);
// Then use canvasPos instead of pos for all hit-testing
```

For `vectorToSurfaceLocal`: since it computes `vec - m_realPosition->goal()`, and m_realPosition is in canvas space, we need `canvasVec - m_realPosition->goal()` to get surface-local coords. So transform vec here too.

**Confidence: HIGH** — read directly from Compositor.cpp:878-957 and 1057-1084.

### Pattern 4: Multi-Monitor Canvas

`renderMonitor()` (Renderer.cpp:1191) renders each monitor independently. Each monitor has `m_position` (global pixel offset) and `m_size`. The rendering loop at line 461 calls `renderWindow()` per visible window.

For canvas spanning both monitors: the viewport transform already handles this naturally. Monitor DP-1 at (0,0) and HDMI-A-1 at (1920,0) both render windows using the same canvas transform. A window at canvas position (1000, 500) will appear on whichever monitor's screen space it maps to after transform.

Key: windows outside a monitor's screen rect are naturally clipped by the GPU/damage system. No extra culling needed.

**Confidence: HIGH** — monitors already have independent render passes with global positions.

### Pattern 5: Window Positioning in CCanvasLayout

DwindleLayout sets `m_position` (logical target) and `*m_realPosition = pos` (animated value). The canvas layout should do the same — place windows at canvas coordinates:

```cpp
void CCanvasLayout::onWindowCreatedTiling(PHLWINDOW pWindow, eDirection) {
    // Place at center of current viewport
    const auto center = g_pCanvasViewport->screenToCanvas(screenCenter);
    pWindow->m_position = center;
    *pWindow->m_realPosition = center;
    pWindow->m_size = Vector2D{1200, 800}; // default
    *pWindow->m_realSize = pWindow->m_size;
}
```

### Anti-Patterns to Avoid
- **Modifying m_realPosition to screen coords:** Never change the window's stored position. Transform only at render/input boundaries.
- **Per-monitor viewport:** The CONTEXT.md explicitly locks this as one global viewport.
- **Transforming in the OpenGL projection matrix:** Tempting but breaks damage tracking, cursor rendering, and layer-shell surfaces.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| 2D vector math | Custom math functions | Existing `Vector2D` class | Already has all operators |
| Rectangle math | Custom box transforms | `CBox` class (scale, translate, expand, containsPoint) | Battle-tested in codebase |
| Animation | Custom tweening | `PHLANIMVAR<Vector2D>` system | Used by m_realPosition already |

## Common Pitfalls

### Pitfall 1: Forgetting vectorToSurfaceLocal
**What goes wrong:** Clicks land on wrong part of window surface (offset by canvas transform)
**Why:** vectorToSurfaceLocal converts global coords to surface-local. If you only fix vectorToWindowUnified, you find the right window but click the wrong pixel.
**How to avoid:** Transform the input vector in BOTH functions.

### Pitfall 2: Workspace renderOffset interaction
**What goes wrong:** Workspace animations (slide transitions) fight with canvas transform
**Why:** Line 481 adds `PWORKSPACE->m_renderOffset->value()` to REALPOS
**How to avoid:** For canvas layout, workspace renderOffset should be zero. Canvas windows don't switch workspaces in the traditional sense.

### Pitfall 3: Damage tracking
**What goes wrong:** Screen not repainting when viewport moves (pan/zoom)
**Why:** Hyprland uses damage tracking to only repaint changed regions
**How to avoid:** When viewport changes, mark entire monitor as damaged: `pMonitor->damage.damageEntire()`

### Pitfall 4: Pinned windows bypass
**What goes wrong:** Pinned windows (line 481 ternary) skip workspace offset but should also skip canvas transform
**Why:** Pinned windows are meant to stay fixed on screen
**How to avoid:** Check `pWindow->m_pinned` before applying canvas transform. Pinned windows render at screen coords directly.

### Pitfall 5: Popup/subsurface positioning
**What goes wrong:** Menus and tooltips appear at wrong positions
**Why:** Popups use `coordsGlobal()` relative to parent window position. If parent is in canvas coords, popups need the same transform.
**How to avoid:** Verify popup rendering uses the same transform path (they go through renderWindow with the parent surface tree).

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | Manual testing in nested Hyprland session |
| Config file | N/A — C++ compositor, no unit test framework set up |
| Quick run command | `cd /home/senne/Claude/Projects/infinity-land && cmake --build build -j$(nproc) && build/Hyprland` |
| Full suite command | Same as quick — run nested, test pan/zoom/click manually |

### Phase Requirements to Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| CANV-01 | Pan with middle mouse / space+drag | manual-only | Run nested session, middle-click drag | N/A |
| CANV-02 | Zoom with scroll wheel | manual-only | Run nested session, scroll to zoom | N/A |
| CANV-05 | Canvas spans both monitors | manual-only | Run nested session, verify windows visible across monitor boundary | N/A |

**Manual-only justification:** This is a Wayland compositor — rendering and input behavior can only be verified in a running graphical session. No headless test harness exists.

### Sampling Rate
- **Per task commit:** Build succeeds: `cmake --build build -j$(nproc)`
- **Per wave merge:** Build + run nested session, verify pan/zoom/click
- **Phase gate:** Full manual verification of CANV-01, CANV-02, CANV-05

### Wave 0 Gaps
None — no automated test infrastructure applicable for compositor rendering.

## Code Examples

### CCanvasViewport Core Interface
```cpp
// src/canvas/CanvasViewport.hpp
#pragma once
#include "../helpers/math/Math.hpp"

class CCanvasViewport {
  public:
    Vector2D canvasToScreen(const Vector2D& canvas) const;
    Vector2D screenToCanvas(const Vector2D& screen) const;
    CBox     canvasToScreen(const CBox& box) const;

    void     pan(const Vector2D& delta);
    void     zoom(double factor, const Vector2D& screenAnchor);

    double   scale() const;
    Vector2D offset() const;

  private:
    Vector2D m_offset = {0, 0};
    double   m_scale  = 1.0;

    static constexpr double MIN_SCALE = 0.1;
    static constexpr double MAX_SCALE = 3.0;
};

inline UP<CCanvasViewport> g_pCanvasViewport;
```

### Transform Functions
```cpp
Vector2D CCanvasViewport::canvasToScreen(const Vector2D& canvas) const {
    return canvas * m_scale + m_offset;
}

Vector2D CCanvasViewport::screenToCanvas(const Vector2D& screen) const {
    return (screen - m_offset) / m_scale;
}

CBox CCanvasViewport::canvasToScreen(const CBox& box) const {
    return CBox{box.x * m_scale + m_offset.x, box.y * m_scale + m_offset.y, box.w * m_scale, box.h * m_scale};
}
```

### Zoom Anchored to Cursor
```cpp
void CCanvasViewport::zoom(double factor, const Vector2D& screenAnchor) {
    const auto canvasAnchor = screenToCanvas(screenAnchor);
    m_scale = std::clamp(m_scale * factor, MIN_SCALE, MAX_SCALE);
    m_offset = screenAnchor - canvasAnchor * m_scale;
}
```

## Open Questions

1. **Where to intercept scroll events for zoom**
   - What we know: Input goes through `CInputManager`, scroll events are `onMouseWheel`
   - What's unclear: Exact function to hook — need to check InputManager for scroll handling
   - Recommendation: Research InputManager::onMouseWheel in Phase 2 planning, likely add canvas zoom check before default scroll behavior

2. **Where to intercept middle mouse for pan**
   - What we know: IHyprLayout has `onBeginDragWindow()` and `onMouseMove()` for drag handling
   - What's unclear: Whether middle mouse triggers onBeginDragWindow or needs separate input interception
   - Recommendation: Check `CInputManager::onMouseButton` for mouse button routing

## Sources

### Primary (HIGH confidence)
- Direct source code reading: IHyprLayout.hpp (full interface, 238 lines)
- Direct source code reading: LayoutManager.cpp (registration mechanism, 61 lines)
- Direct source code reading: Renderer.cpp:465-530 (renderWindow textureBox construction)
- Direct source code reading: Compositor.cpp:878-957 (vectorToWindowUnified hit-testing)
- Direct source code reading: Compositor.cpp:1057-1084 (vectorToSurfaceLocal)
- Direct source code reading: DwindleLayout.hpp/cpp (reference layout implementation)

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — all integration points read from source
- Architecture: HIGH — transform math is straightforward, injection points identified
- Pitfalls: HIGH — identified from actual code paths (damage tracking, pinned windows, popups)

**Research date:** 2026-03-25
**Valid until:** Stable — Hyprland fork is pinned at v0.51.1
