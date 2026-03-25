# Phase 8: Window Organization - Context

**Gathered:** 2026-03-25
**Status:** Ready for planning

<domain>
## Phase Boundary

Anti-overlap snap on drop, grid snap layout via keybind, color tags on windows, workspace views that filter visible windows.

</domain>

<decisions>
## Implementation Decisions

### Anti-Overlap
- When a window is dropped and overlaps another, snap to nearest free position
- Use minimum translation vector approach (push in direction of least displacement)
- Same algorithm as the Electron prototype's resolveOverlap

### Grid Snap
- Keybind (SUPER+G) arranges all visible windows into a grid layout
- Grid calculated based on window count and viewport size
- Windows animate to grid positions

### Color Tags
- Each window can have a color tag (visual label)
- Rendered as a small colored dot or border accent on the window
- Set via hyprctl dispatch command: `canvas:tag <color>`
- Tags persist in canvas-state.json

### Workspace Views
- Named views that filter which windows are visible
- "All" view shows everything (default)
- Create/switch views via hyprctl dispatch
- Each view remembers which window classes/addresses are included
- Windows not in current view are hidden (not rendered, not receiving input)

### Claude's Discretion
- Grid snap animation style
- Tag color palette
- View management UI details

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- CCanvasLayout has all window tracking
- CCanvasPersistence saves state to JSON
- resolveOverlap algorithm from Electron prototype (minimum translation vector)

### Integration Points
- src/layout/CanvasLayout.cpp — add overlap resolution in endDrag
- src/canvas/ — add CanvasTags.hpp/cpp, CanvasViews.hpp/cpp

</code_context>

<specifics>
## Specific Ideas

Anti-overlap uses minimum translation vector: for each overlap, push in the direction (left/right/up/down) requiring least displacement.

</specifics>

<deferred>
## Deferred Ideas

None.

</deferred>
