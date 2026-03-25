# Phase 2: Canvas Viewport - Context

**Gathered:** 2026-03-25
**Status:** Ready for planning

<domain>
## Phase Boundary

Implement the core infinite canvas viewport: a coordinate transform system that allows pan (middle mouse / space+drag) and zoom (scroll wheel) across the entire canvas. The canvas must span both monitors (DP-1 + HDMI-A-1) as one continuous surface. All coordinate transforms must be correct at all zoom levels (clicks hit the right window).

</domain>

<decisions>
## Implementation Decisions

### Viewport Transform Architecture
- Create a single global `CCanvasViewport` class owning all canvas state: offset (pan), scale (zoom), and coordinate transform math
- The viewport is NOT per-monitor — one viewport spans all monitors
- Viewport stores: `Vector2D m_offset` (pan offset in canvas space), `double m_scale` (zoom level, 1.0 = normal)
- Zoom is linear (no easing), fast response to scroll wheel
- Pan via middle mouse or space+left-click drag

### Coordinate Transform
- Screen-to-canvas: `canvasPos = (screenPos - m_offset) / m_scale`
- Canvas-to-screen: `screenPos = canvasPos * m_scale + m_offset`
- All input coordinates (mouse clicks, hover) must be inverse-transformed before window hit-testing
- All rendering coordinates must be forward-transformed before drawing

### Integration Points (from codebase exploration)
- **Layout:** Create a new `CCanvasLayout : public IHyprLayout` that positions windows in canvas space
- **Rendering:** In `CHyprRenderer::renderWindow()`, apply canvas transform to window position/size before rendering. Window's `m_realPosition` stays in canvas coordinates; screen position = canvasToScreen(m_realPosition)
- **Input:** Override coordinate lookup in `CCompositor::vectorToWindowUnified()` to inverse-transform mouse position before hit-testing
- **Monitors:** Both monitors share the same canvas — offset their viewport by their physical position in the global space

### Key Codebase References
- `IHyprLayout` interface: `src/layout/IHyprLayout.hpp` — implement `recalculateMonitor`, `onWindowCreatedTiling`, `resizeActiveWindow`, `onMouseMove`
- Window positioning: `m_position`, `m_size`, `m_realPosition`, `m_realSize` in `src/desktop/Window.hpp`
- Render pipeline: `CHyprRenderer::renderWindow()` at `src/render/Renderer.cpp:465` — where textureBox is built from m_realPosition
- Input mapping: `CCompositor::vectorToWindowUnified()` and `vectorToSurfaceLocal()` in `src/Compositor.cpp`
- OpenGL projection: `CHyprOpenGLImpl` in `src/render/OpenGL.hpp` — `SCurrentRenderData.projection`

### Claude's Discretion
- Exact file structure for new canvas code (suggested: `src/layout/CanvasLayout.cpp/hpp`, `src/canvas/CanvasViewport.cpp/hpp`)
- Zoom min/max bounds (suggested: 0.1 to 3.0)
- Pan speed sensitivity
- How to register the canvas layout as the active layout (config keyword or hardcoded for now)

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `IHyprLayout` interface provides clean plugin point for custom layouts
- `Vector2D` class for all 2D math
- `Mat3x3` for projection matrices
- `CBox` for rectangle math (contains, intersect, scale)
- Existing `DwindleLayout` as reference implementation (~600 LOC)

### Established Patterns
- Layouts implement `IHyprLayout` and are registered via `g_pLayoutManager`
- Window position set via `m_position`/`m_size`, animated to `m_realPosition`/`m_realSize`
- Input coordinates go through `CCompositor::vectorToWindowUnified()` for hit-testing
- Rendering uses `CHyprRenderer::renderWindow()` which builds screen-space boxes from `m_realPosition`

### Integration Points
- `src/layout/` — add new CanvasLayout files
- `src/managers/LayoutManager.cpp` — register the canvas layout
- `src/render/Renderer.cpp` — inject canvas transform in renderWindow
- `src/Compositor.cpp` — inject inverse transform in input coordinate lookup

</code_context>

<specifics>
## Specific Ideas

- Pan with middle mouse (button 2) or space+left-click drag — same as the Electron prototype
- Zoom with scroll wheel — linear, no easing, immediate response
- One canvas spanning both monitors as single surface
- Zoom range: 0.1x to 3.0x suggested

</specifics>

<deferred>
## Deferred Ideas

- Dot grid background (Phase 5)
- Glow animation (Phase 5)
- Window drag/resize on canvas (Phase 3)
- Layer-shell exclusion from transform (Phase 4)

</deferred>
