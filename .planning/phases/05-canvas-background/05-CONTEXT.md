# Phase 5: Canvas Background - Context

**Gathered:** 2026-03-25
**Status:** Ready for planning

<domain>
## Phase Boundary

Render a dot grid background behind all windows on the canvas. Dots glow near the cursor with a radial gradient. Grid spacing adapts at low zoom. Dark and light theme with toggle.

</domain>

<decisions>
## Implementation Decisions

### Dot Grid
- Render dots as small circles in a regular grid pattern
- Grid rendered in canvas space (moves/scales with pan/zoom)
- Base grid spacing: 20px in canvas coordinates
- Dot radius: ~1.5px, glow dot radius: ~2px
- At low zoom (<=40%), double grid spacing to avoid visual clutter

### Glow Animation
- Radial gradient centered on cursor position
- Glow radius: ~130px in screen space
- Dots within glow radius render brighter/larger
- Smooth follow of mouse movement (rAF-equivalent in compositor = per-frame update)
- When no mouse hover on canvas, dots at base (dim) brightness

### Theme
- Dark theme: dim gray dots (#374151), glow dots slightly brighter (#9CA3AF)
- Light theme: light gray dots (#D1D5DB), glow dots darker gray (#9CA3AF)
- Background: dark (#1a1a2e-ish), light (#f5f5f5-ish)
- Toggle via a keybind or hyprctl command

### Rendering Approach
- Use OpenGL to render dot grid as a custom render pass BEFORE windows
- Each frame: iterate visible grid positions, draw small circles or point sprites
- For performance: use instanced rendering or a fragment shader approach
- The glow mask can be a simple distance check from cursor in the shader

### Claude's Discretion
- Exact OpenGL technique (point sprites vs instanced quads vs fragment shader)
- Theme toggle mechanism (keybind vs hyprctl dispatch)
- Exact color values and transitions

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- CHyprOpenGLImpl has full OpenGL context for custom rendering
- Renderer.cpp renderWorkspace() is where background should be injected
- CCanvasViewport provides offset/scale for grid coordinate transforms
- Hyprland already uses custom shaders for blur, shadow effects

### Integration Points
- src/render/Renderer.cpp — add grid render call in renderWorkspace before windows
- src/render/OpenGL.hpp/cpp — add grid rendering method
- src/canvas/ — add CanvasBackground.hpp/cpp for grid logic

</code_context>

<specifics>
## Specific Ideas

Same visual style as the Electron prototype: Stitch-like dot glow that follows cursor.

</specifics>

<deferred>
## Deferred Ideas

None.

</deferred>
