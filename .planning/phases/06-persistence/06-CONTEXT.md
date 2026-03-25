# Phase 6: Persistence - Context

**Gathered:** 2026-03-25
**Status:** Ready for planning

<domain>
## Phase Boundary

Save and restore canvas state across compositor restarts. Window positions, sizes, and per-app size defaults persist in JSON at `~/.config/infinity-land/`.

</domain>

<decisions>
## Implementation Decisions

### Claude's Discretion
All implementation choices are at Claude's discretion — infrastructure phase.

Key guidelines:
- Save to `~/.config/infinity-land/canvas-state.json`
- Use nlohmann/json (already researched as recommended dependency)
- Save on window close/move/resize events (debounced)
- Load on compositor start, match windows by app class
- Per-app size defaults: remember last size for each app class
- Default 1200x800 for unknown apps

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- CCanvasLayout tracks all window positions
- Window class has appName/class info for per-app matching
- Hyprland already uses JSON for some config (via glaze library bundled in build)

### Integration Points
- src/canvas/ — add CanvasPersistence.hpp/cpp
- src/layout/CanvasLayout.cpp — call save on position changes
- Compositor startup — load state after layout init

</code_context>

<specifics>
## Specific Ideas

No specific requirements — standard persistence pattern.

</specifics>

<deferred>
## Deferred Ideas

None.

</deferred>
