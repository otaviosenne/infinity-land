# Phase 3: Canvas Layout - Context

**Gathered:** 2026-03-25
**Status:** Ready for planning

<domain>
## Phase Boundary

Make windows fully interactive on the canvas: SUPER+left-click drag to move, SUPER+right-click drag to resize. Windows open at viewport center with 1200x800 default. All windows render as live interactive Wayland surfaces (not thumbnails). This extends the CCanvasLayout created in Phase 2.

</domain>

<decisions>
## Implementation Decisions

### Window Interaction
- SUPER+left-click drag moves windows in canvas space (Hyprland-style)
- SUPER+right-click drag resizes windows (Hyprland-style)
- Drag/resize operate in canvas coordinates (divide mouse delta by scale)
- Windows are always interactive — clicking inside a window sends input to it
- SUPER modifier distinguishes "move/resize window" from "interact with window content"

### Window Positioning
- Default size 1200x800 for new windows
- New windows spawn at center of current viewport
- CCanvasLayout already handles this from Phase 2 — verify and extend if needed

### Implementation Approach
- CCanvasLayout already implements IHyprLayout from Phase 2
- Need to implement `moveActiveWindow()`, `resizeActiveWindow()` properly
- Drag: update `m_position` directly in canvas coordinates
- Resize: update `m_size` with minimum bounds
- Input routing already inverse-transforms via Phase 2's vectorToWindowUnified changes

### Claude's Discretion
- Minimum window size (suggested: 200x150)
- Resize handle behavior (edge vs corner)
- Animation during drag (smooth vs immediate)

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- CCanvasViewport (Phase 2): coordinate transforms, scale/offset state
- CCanvasLayout (Phase 2): IHyprLayout implementation with window positioning
- Hyprland's existing SUPER+mouse drag/resize infrastructure in InputManager
- DwindleLayout's moveActiveWindow/resizeActiveWindow as reference

### Established Patterns
- Layouts handle move/resize via IHyprLayout virtual methods
- InputManager routes SUPER+mouse events to active layout
- Window position updates go through m_position/m_size then animate to m_realPosition/m_realSize

### Integration Points
- src/layout/CanvasLayout.cpp — extend moveActiveWindow, resizeActiveWindow
- src/managers/input/InputManager.cpp — verify SUPER+mouse routing works with canvas layout

</code_context>

<specifics>
## Specific Ideas

No specific requirements beyond Hyprland-style SUPER+mouse interaction.

</specifics>

<deferred>
## Deferred Ideas

- Anti-overlap snap on drop (Phase 8)
- Snap-to-grid tiling (Phase 8)
- Color tags (Phase 8)

</deferred>
