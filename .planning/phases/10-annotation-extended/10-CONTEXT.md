# Phase 10: Annotation Extended - Context

**Gathered:** 2026-03-25
**Status:** Ready for planning

<domain>
## Phase Boundary

Add arrows/connectors, sticky notes, text tool to annotation system. Persist all annotations to disk. Clear button with options (all / by type).

</domain>

<decisions>
## Implementation Decisions

### Arrow/Connector Tool
- Click start point, drag to end point — draws arrow with arrowhead
- Can connect from window edge to window edge or arbitrary points
- Stored as start/end points in canvas coordinates

### Sticky Notes
- Click canvas to place a resizable text box
- Editable text content (basic text input)
- Yellow background with shadow (classic sticky note look)
- Stored as position + size + text content

### Text Tool
- Click canvas to place text directly (no background box)
- Text renders at click position in canvas space
- Uses Pango for text rendering

### Persistence
- Save all annotations to `~/.config/infinity-land/annotations.json`
- Load on compositor start
- Save on every annotation change (debounced)
- JSON format: array of annotation objects with type, points, color, thickness, text

### Clear
- hyprctl dispatch `canvas:clearannotations all` — clears everything
- `canvas:clearannotations strokes` — clears only freehand strokes
- `canvas:clearannotations arrows` — clears only arrows
- `canvas:clearannotations notes` — clears only sticky notes
- `canvas:clearannotations text` — clears only text

### Claude's Discretion
- All implementation details

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- CanvasAnnotation from Phase 9 — data model with AnnotationType enum
- CanvasDrawMode from Phase 9 — tool switching, input routing
- CanvasToolbar from Phase 9 — button rendering
- Cairo rendering pipeline already established
- nlohmann/json for persistence (Phase 6)

### Integration Points
- src/canvas/CanvasAnnotation.hpp/cpp — extend with new types
- src/canvas/CanvasDrawMode.hpp/cpp — add tool handlers
- src/canvas/CanvasToolbar.hpp/cpp — toolbar already has button slots

</code_context>

<specifics>
## Specific Ideas

No specific requirements beyond what's listed.

</specifics>

<deferred>
## Deferred Ideas

None.

</deferred>
