# Phase 4: Layer-Shell Compatibility - Context

**Gathered:** 2026-03-25
**Status:** Ready for planning

<domain>
## Phase Boundary

Ensure layer-shell surfaces (waybar, wofi, SwayNC) render at screen edges unaffected by canvas pan/zoom. Ensure all existing Hyprland keybindings and window rules still function.

</domain>

<decisions>
## Implementation Decisions

### Layer-Shell Exclusion
- Layer-shell surfaces must NOT be transformed by canvas viewport
- They render in screen-space at their configured positions (top, bottom, etc.)
- The render pipeline transform added in Phase 2 must skip layer-shell surfaces
- Input routing for layer-shell surfaces must use screen coordinates (not canvas coords)

### Keybindings
- All existing Hyprland keybindings (SUPER+Q terminal, SUPER+W kill, etc.) must work
- The canvas layout should not interfere with keybind dispatchers
- Window rules from hyprland.conf should still apply

### Claude's Discretion
- Exact detection mechanism for layer-shell surfaces (already handled by Hyprland's layer system)
- Whether layer-shell surfaces render above or below canvas content (above — they're overlays)

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- Hyprland already has LayerSurface handling separate from window rendering
- `renderLayer()` in Renderer.cpp renders layer surfaces independently
- Layer surfaces have their own render path, not going through `renderWindow()`
- Phase 2 transforms only affect `renderWindow()` — layer surfaces may already work

### Key Question
- Phase 2's input transform in `vectorToWindowUnified()` might intercept clicks meant for layer surfaces
- Need to verify layer surface input routing bypasses canvas transform

### Integration Points
- src/render/Renderer.cpp — verify renderLayer is not affected by canvas transform
- src/Compositor.cpp — verify layer surface input routing

</code_context>

<specifics>
## Specific Ideas

This phase may require minimal code changes if Phase 2's transforms are already scoped correctly to windows only. Main risk is input routing.

</specifics>

<deferred>
## Deferred Ideas

None.

</deferred>
