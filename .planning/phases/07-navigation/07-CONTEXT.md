# Phase 7: Navigation - Context

**Gathered:** 2026-03-25
**Status:** Ready for planning

<domain>
## Phase Boundary

Minimap overlay showing all windows, quick jump search by window name, and zoom-to-fit maximize.

</domain>

<decisions>
## Implementation Decisions

### Minimap
- Small overlay in bottom-right corner showing all window positions as colored rectangles
- Current viewport shown as a translucent rectangle
- Clicking on minimap jumps viewport to that area
- Renders as a custom OpenGL overlay (layer on top of everything)
- Fixed screen-space position (not affected by canvas transform)

### Quick Jump
- Keybind (SUPER+J or similar) opens a text input overlay
- Type window name → fuzzy match → jump viewport to center on that window
- Similar to wofi but built into the compositor for speed

### Zoom-to-Fit
- Keybind or action that zooms canvas until target window fills the screen
- Adjusts both scale and offset so window is centered and maximized in viewport
- Stays in canvas mode — not a real fullscreen

### Claude's Discretion
- Minimap size and position (suggested: 200x120 bottom-right with 16px margin)
- Quick jump keybind (suggested: SUPER+J)
- Zoom-to-fit keybind (suggested: SUPER+M or double-click title)
- Minimap colors and opacity
- Quick jump UI style

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- CCanvasViewport has setOffset/setScale for viewport manipulation
- CCanvasLayout tracks all window positions
- Hyprland has existing text rendering via Pango for notifications

### Integration Points
- src/canvas/ — add CanvasMinimap.hpp/cpp, CanvasQuickJump.hpp/cpp
- src/render/Renderer.cpp — render minimap overlay after windows
- src/managers/KeybindManager.cpp — register new keybinds

</code_context>

<specifics>
## Specific Ideas

No specific requirements beyond what's listed.

</specifics>

<deferred>
## Deferred Ideas

None.

</deferred>
