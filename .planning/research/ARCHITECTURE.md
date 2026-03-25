# Architecture Patterns

**Domain:** Wayland compositor with infinite canvas layout (Hyprland fork)
**Researched:** 2026-03-25

## Recommended Architecture

Infinity Land is a fork of Hyprland where the traditional workspace/tiling paradigm is replaced by a single infinite 2D canvas. The architecture layers on top of Hyprland's existing compositor infrastructure, replacing the layout engine and adding an annotation overlay.

```
+------------------------------------------------------------------+
|                        Input Router                               |
|  (keyboard, mouse, touch — dispatches to canvas or window)        |
+------------------------------------------------------------------+
         |                    |                    |
         v                    v                    v
+------------------+  +------------------+  +------------------+
|  Canvas Layout   |  | Annotation Layer |  | Layer Shell Mgr  |
|  (IHyprLayout)   |  | (overlay render) |  | (waybar, wofi)   |
+------------------+  +------------------+  +------------------+
         |                    |                    |
         v                    v                    v
+------------------------------------------------------------------+
|                     Canvas Viewport                               |
|  (pan offset, zoom level, coordinate transforms)                  |
+------------------------------------------------------------------+
         |                    |
         v                    v
+------------------+  +------------------+
|  Window Renderer |  | Annotation       |
|  (scaled/offset  |  | Renderer         |
|   wl_surfaces)   |  | (OpenGL overlay) |
+------------------+  +------------------+
         |                    |
         v                    v
+------------------------------------------------------------------+
|              Hyprland Rendering Pipeline                          |
|  (aquamarine backend, EGL/OpenGL, damage tracking)                |
+------------------------------------------------------------------+
         |
         v
+------------------------------------------------------------------+
|              Monitor Output (DP-1 + HDMI-A-1)                     |
|  (one canvas spans both monitors as continuous surface)            |
+------------------------------------------------------------------+
```

### Component Boundaries

| Component | Responsibility | Communicates With |
|-----------|---------------|-------------------|
| **CInfiniteCanvasLayout** | IHyprLayout implementation. Positions windows in canvas space. No tiling tree — just (x,y) positions per window. | Canvas Viewport, Persistence, CCompositor |
| **CCanvasViewport** | Manages pan offset (Vector2D) and zoom level (float). Converts canvas coords to screen coords and back. | Layout, Renderers, Input Router |
| **CCanvasInputRouter** | Intercepts mouse/keyboard. Routes to canvas actions (pan, zoom) vs window interaction vs annotation tool. | Viewport, Layout, Annotation Layer |
| **CAnnotationLayer** | Stores drawing primitives (strokes, arrows, sticky notes, text). Canvas-space coordinates. | Viewport, Annotation Renderer, Persistence |
| **CAnnotationRenderer** | OpenGL rendering of annotation primitives, transformed through viewport. | Annotation Layer, Hyprland render pipeline |
| **CCanvasMinimap** | Renders scaled overview of all windows + annotations in corner. Click to jump. | Viewport, Layout |
| **CCanvasPersistence** | Saves/loads window positions, annotation data, viewport state to `~/.config/infinity-land/`. | Layout, Annotation Layer |
| **CCanvasBackground** | Renders dot grid, applies glow effect at cursor position. Scales with zoom. | Viewport, Hyprland render pipeline |

### Data Flow

**Window Creation:**
```
New wl_surface → CCompositor → CInfiniteCanvasLayout::onWindowCreatedTiling()
  → assign canvas position (center of viewport or snap-to-free-space)
  → store in window node map
  → CCanvasViewport transforms to screen coords
  → CCompositor renders at transformed position
```

**Pan/Zoom:**
```
Mouse scroll / middle-drag → CCanvasInputRouter
  → updates CCanvasViewport (offset/zoom)
  → CInfiniteCanvasLayout::recalculateMonitor() [all windows reposition on screen]
  → damage entire output → re-render all visible windows + annotations + background
```

**Annotation Drawing:**
```
Drawing mode active → CCanvasInputRouter captures mouse
  → mouse events → CAnnotationLayer (creates/extends stroke primitives)
  → CAnnotationRenderer draws overlay after window render pass
  → CCanvasPersistence auto-saves on stroke complete
```

**Hit Testing (window interaction in canvas):**
```
Click event → CCanvasInputRouter
  → CCanvasViewport::screenToCanvas(mousePos)
  → check: annotation tool active? → route to CAnnotationLayer
  → check: over a window? → route to CCompositor (normal Wayland input)
  → check: empty canvas? → start pan
```

## Patterns to Follow

### Pattern 1: IHyprLayout as the Integration Point

Implement `CInfiniteCanvasLayout` as a concrete IHyprLayout. This is the correct integration point because Hyprland's compositor already delegates ALL window positioning to the active layout. No need to modify the core compositor loop.

Key methods to implement:
- `onWindowCreatedTiling` — place at viewport center, resolve overlaps
- `onWindowRemovedTiling` — remove from canvas node map
- `recalculateMonitor` — apply viewport transform to all window positions
- `resizeActiveWindow` — resize in canvas space, re-transform
- `getLayoutName` — return "infinitecanvas"

The layout stores a flat map of `windowID -> CanvasNode{canvasX, canvasY, width, height}` instead of a tree structure.

### Pattern 2: Viewport as Coordinate Transform Layer

All positions exist in two coordinate spaces:
- **Canvas space**: absolute position on the infinite plane (doubles, not ints)
- **Screen space**: pixel position on monitor after pan+zoom transform

```cpp
struct CCanvasViewport {
    Vector2D offset;  // pan offset in canvas space
    double zoom;      // 1.0 = 100%, 0.5 = zoomed out, 2.0 = zoomed in

    Vector2D canvasToScreen(Vector2D canvasPos, MONITORID monitor);
    Vector2D screenToCanvas(Vector2D screenPos, MONITORID monitor);
    CBox canvasRectToScreen(CBox canvasRect, MONITORID monitor);
};
```

The viewport is the SINGLE source of truth for the transform. Every component that needs screen coords goes through it.

### Pattern 3: Render Hook for Overlays

Hyprland's render pipeline has hooks. Annotations and the dot grid background render as overlays:
- Background dot grid: render BEFORE windows (in the background pass)
- Annotations: render AFTER windows (overlay pass)
- Minimap: render LAST (always on top, screen-space not canvas-space)

Use `HyprlandAPI::registerCallbackDynamic` on render events or directly modify the render loop in the fork.

### Pattern 4: Overlap Prevention via Spatial Index

When a window is dropped, find the nearest non-overlapping position. Use a simple spatial approach (quadtree or just iterate all windows — with 15-30 windows, brute force is fine at 60fps).

## Anti-Patterns to Avoid

### Anti-Pattern 1: Modifying CCompositor Core Loop
**What:** Putting canvas logic directly in the compositor's main render/input loop.
**Why bad:** Makes the fork impossible to maintain against upstream Hyprland updates.
**Instead:** Keep changes isolated to: the layout class, render hooks, and input interceptors. The less you touch core compositor code, the easier rebasing is.

### Anti-Pattern 2: Storing Screen-Space Positions
**What:** Saving window positions as screen pixels instead of canvas coordinates.
**Why bad:** Positions break when zoom changes, monitors reconfigure, or viewport pans.
**Instead:** Always store canvas-space coordinates. Transform to screen-space only at render time.

### Anti-Pattern 3: Annotation Data in the Render Layer
**What:** Coupling annotation state with the OpenGL rendering code.
**Why bad:** Can't serialize, can't undo/redo, can't test without GPU.
**Instead:** Annotations are a data model (CAnnotationLayer) with a separate renderer. The model stores primitives; the renderer draws them.

### Anti-Pattern 4: One Viewport Per Monitor
**What:** Each monitor having its own independent viewport state.
**Why bad:** The canvas should be ONE continuous surface. Two viewports means complex sync logic.
**Instead:** One global viewport. Each monitor shows a rectangular slice of the canvas at a fixed offset (based on monitor physical arrangement). Pan/zoom affects the global viewport; monitors just have different screen-space rectangles.

## Build Order (Dependencies)

Each component depends on the ones above it:

```
Phase 1: CCanvasViewport
  └─ No dependencies. Pure math (coordinate transforms).

Phase 2: CInfiniteCanvasLayout (implements IHyprLayout)
  └─ Depends on: CCanvasViewport
  └─ Windows appear on canvas, can be dragged/resized.

Phase 3: CCanvasInputRouter
  └─ Depends on: CCanvasViewport, CInfiniteCanvasLayout
  └─ Pan, zoom, window interaction all work.

Phase 4: CCanvasBackground
  └─ Depends on: CCanvasViewport
  └─ Dot grid renders behind windows.

Phase 5: CCanvasPersistence
  └─ Depends on: CInfiniteCanvasLayout
  └─ Window positions survive restarts.

Phase 6: CAnnotationLayer + CAnnotationRenderer
  └─ Depends on: CCanvasViewport, render pipeline hooks
  └─ Drawing, arrows, sticky notes, text.

Phase 7: CCanvasMinimap
  └─ Depends on: CCanvasViewport, CInfiniteCanvasLayout, CAnnotationLayer
  └─ Overview of everything.
```

## Scalability Considerations

| Concern | At 5 windows | At 20 windows | At 50+ windows |
|---------|-------------|---------------|----------------|
| Layout recalc | Trivial | Trivial (flat map, no tree) | Still fast, O(n) iteration |
| Render cost | All visible, no culling needed | Viewport culling useful | Must cull off-screen windows |
| Overlap detection | Brute force O(n^2) fine | Still fine (~400 comparisons) | Consider spatial index |
| Annotation count | No concern | No concern | Batch draw calls, consider LOD |
| Persistence I/O | Instant | Instant | JSON still fine, async write |

**Key insight:** At the target scale (15-30 windows on AMD RX 590), brute force approaches work. Don't over-engineer spatial indexing until profiling shows a need.

## Multi-Monitor Strategy

Both monitors (DP-1 1920x1080 + HDMI-A-1 1920x1080) show the SAME canvas. The global viewport defines the canvas region visible on the primary monitor. The secondary monitor shows the adjacent region based on physical layout offset.

```
Canvas space:
  ... [DP-1 visible region] [HDMI-A-1 visible region] ...
       ← 1920px →            ← 1920px →
```

When panning, both monitors shift together. When zooming, both monitors zoom from the cursor position. This is handled by `CCanvasViewport::canvasToScreen()` taking a MONITORID parameter to apply the correct monitor offset.

## Sources

- [Hyprland Layout System (IHyprLayout) - DeepWiki](https://deepwiki.com/hyprwm/Hyprland/5.3-layout-system)
- [Hyprland Plugin Architecture - DeepWiki](https://deepwiki.com/hyprwm/hyprland-plugins/2-plugin-architecture)
- [Dwindle Layout Implementation - DeepWiki](https://deepwiki.com/hyprwm/Hyprland/5.4-dwindle-layout)
- [Niri Scrollable Compositor](https://github.com/YaLTeR/niri) — prior art for non-traditional layout
- [Hyprland GitHub](https://github.com/hyprwm/Hyprland)
