# Phase 9: Annotation Foundation - Context

**Gathered:** 2026-03-25
**Status:** Ready for planning

<domain>
## Phase Boundary

Drawing mode activated by button/keybind. Freehand brush with color/thickness. Eraser with precision and object modes. Toolbar showing available tools.

</domain>

<decisions>
## Implementation Decisions

### Architecture
- Annotations exist in canvas space (move/scale with pan/zoom)
- Annotation data model separate from rendering (enables undo/redo in Phase 11)
- Use Cairo to render annotations to an image surface, upload as GL texture
- Annotation layer renders BETWEEN background grid and windows (or above windows — Claude's discretion)

### Drawing Mode
- Toggle via keybind (SUPER+D) or hyprctl dispatch `canvas:drawmode`
- When active: mouse clicks create brush strokes instead of interacting with windows
- Visual indicator that draw mode is on (cursor change or overlay)

### Brush
- Freehand paths stored as vector of points
- Color picker: preset palette (8-10 colors)
- Thickness: 3 levels (thin=2px, medium=4px, thick=8px)
- Smooth line rendering with Cairo line_to

### Eraser
- Precision mode: erases exact area under cursor (like a brush but transparent)
- Object mode: clicking near a stroke deletes the entire stroke
- Toggle between modes via toolbar or keybind

### Toolbar
- Rendered as overlay in bottom-right corner when draw mode active
- Shows: brush, eraser, color, thickness
- Horizontal bar with icon buttons

### Claude's Discretion
- Exact toolbar position and size
- Cairo surface resolution relative to canvas
- How to efficiently re-render only changed annotation regions
- Annotation layer z-order (above or below windows)

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- Cairo and Pango are system libs already available (used by GTK stack)
- CHyprOpenGLImpl can upload textures from raw pixel data
- CCanvasViewport provides coordinate transforms for canvas-space rendering

### Integration Points
- src/canvas/ — add CanvasAnnotation.hpp/cpp (data model), CanvasDrawTool.hpp/cpp (tools)
- src/render/Renderer.cpp — render annotation texture overlay
- CMakeLists.txt — add Cairo dependency

</code_context>

<specifics>
## Specific Ideas

No specific requirements beyond what's listed.

</specifics>

<deferred>
## Deferred Ideas

- Arrows/connectors (Phase 10)
- Sticky notes and text tool (Phase 10)
- Annotation persistence (Phase 10)

</deferred>
