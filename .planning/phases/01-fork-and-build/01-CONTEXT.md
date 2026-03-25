# Phase 1: Fork and Build - Context

**Gathered:** 2026-03-25
**Status:** Ready for planning

<domain>
## Phase Boundary

Establish a working build from the Hyprland v0.51.1 fork with nested compositor development workflow. Developer can compile, run, and iterate safely without affecting the real desktop.

</domain>

<decisions>
## Implementation Decisions

### Claude's Discretion
All implementation choices are at Claude's discretion — pure infrastructure phase.

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- Full Hyprland v0.51.1 source on branch `infinity-land`
- CMake build system already configured and tested
- `dev.sh` script for nested launch with `AQUAMARINE_BACKEND=wayland`
- Dev config at `~/.config/hypr/infinity-land-dev.conf`

### Established Patterns
- CMake + Ninja build (already working: `cmake -B build -G Ninja`)
- Build produces binary at `build/Hyprland`
- Nested launch verified working with `AQUAMARINE_BACKEND=wayland`

### Integration Points
- `upstream` remote points to official Hyprland repo
- `infinity-land` branch is the development branch

</code_context>

<specifics>
## Specific Ideas

No specific requirements — infrastructure phase.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>
