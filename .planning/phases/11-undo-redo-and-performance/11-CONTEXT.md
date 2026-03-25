# Phase 11: Undo/Redo and Performance - Context

**Gathered:** 2026-03-25
**Status:** Ready for planning

<domain>
## Phase Boundary

Command pattern undo/redo across all canvas actions (window moves, annotations, etc.). Performance validation: 60fps with 15+ windows on AMD RX 590.

</domain>

<decisions>
## Implementation Decisions

### Undo/Redo
- Command pattern: every action creates a Command object with execute/undo methods
- Command types: WindowMove, WindowResize, AnnotationCreate, AnnotationDelete, AnnotationClear
- Stack-based: undo stack + redo stack
- Ctrl+Z = undo, Ctrl+Shift+Z = redo
- Single unified stack (all actions mixed, not per-context)
- Stack size limit: 100 commands

### Performance
- Canvas rendering must maintain 60fps with 15+ open windows
- Visibility culling: skip rendering windows outside viewport
- Damage tracking: only re-render changed regions where possible
- Profile on AMD RX 590 with actual workloads

### Claude's Discretion
- All implementation details
- Performance optimization strategies

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- All canvas modules (viewport, layout, annotations, persistence)
- Hyprland's existing damage tracking system

### Integration Points
- src/canvas/ — add CanvasUndoRedo.hpp/cpp
- src/layout/CanvasLayout.cpp — wrap moves/resizes in commands
- src/canvas/CanvasDrawMode.cpp — wrap annotation actions in commands
- src/managers/KeybindManager.cpp — register Ctrl+Z, Ctrl+Shift+Z

</code_context>

<specifics>
## Specific Ideas

No specific requirements beyond what's listed.

</specifics>

<deferred>
## Deferred Ideas

None — this is the final phase.

</deferred>
