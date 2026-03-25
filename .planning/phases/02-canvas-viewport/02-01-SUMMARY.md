---
phase: 02-canvas-viewport
plan: 01
subsystem: canvas
tags: [hyprland, viewport, layout, coordinate-transform, infinite-canvas]

requires:
  - phase: 01-fork-build
    provides: "Compilable Hyprland fork with build system"
provides:
  - "CCanvasViewport singleton with pan/zoom/transform math"
  - "CCanvasLayout IHyprLayout implementation"
  - "Canvas layout registered in LayoutManager as 'canvas'"
affects: [02-canvas-viewport, 03-window-management, 04-input-system]

tech-stack:
  added: []
  patterns: ["singleton global via UP<T> inline pattern", "IHyprLayout virtual override pattern"]

key-files:
  created:
    - src/canvas/CanvasViewport.hpp
    - src/canvas/CanvasViewport.cpp
    - src/layout/CanvasLayout.hpp
    - src/layout/CanvasLayout.cpp
  modified:
    - src/managers/LayoutManager.hpp
    - src/managers/LayoutManager.cpp

key-decisions:
  - "Used defines.hpp include in viewport header for UP<T> availability"
  - "Default window size 1200x800 placed at canvas viewport center"

patterns-established:
  - "Canvas source files in src/canvas/ directory"
  - "Layout classes follow DwindleLayout pattern for IHyprLayout overrides"

requirements-completed: [CANV-01, CANV-02, CANV-05]

duration: 8min
completed: 2026-03-25
---

# Phase 02 Plan 01: Canvas Viewport and Layout Summary

**CCanvasViewport with pan/zoom coordinate transforms and CCanvasLayout registered as 'canvas' in LayoutManager**

## Performance

- **Duration:** 8 min
- **Started:** 2026-03-25T20:43:25Z
- **Completed:** 2026-03-25T20:51:25Z
- **Tasks:** 2
- **Files modified:** 6

## Accomplishments
- CCanvasViewport singleton with canvasToScreen/screenToCanvas transforms, pan, and cursor-anchored zoom
- CCanvasLayout implementing all IHyprLayout pure virtuals with center-placed window creation
- Canvas layout registered in LayoutManager enum and constructor, project compiles cleanly

## Task Commits

1. **Task 1: Create CCanvasViewport singleton** - `6696638e` (feat)
2. **Task 2: Create CCanvasLayout and register in LayoutManager** - `61c12d43` (feat)

## Files Created/Modified
- `src/canvas/CanvasViewport.hpp` - Viewport class with transform API and global singleton
- `src/canvas/CanvasViewport.cpp` - Transform math, pan, zoom with cursor anchor, monitor damage
- `src/layout/CanvasLayout.hpp` - IHyprLayout subclass declaration with window tracking
- `src/layout/CanvasLayout.cpp` - Layout implementation placing windows at viewport center
- `src/managers/LayoutManager.hpp` - Added LAYOUT_CANVAS enum, include, and member
- `src/managers/LayoutManager.cpp` - Registered canvas layout in constructor

## Decisions Made
- Used `defines.hpp` instead of `Math.hpp` in viewport header to get `UP<T>` type alias
- Default window size 1200x800 centered on current viewport position in canvas space

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Fixed missing includes and wrong member name**
- **Found during:** Task 2 (build verification)
- **Issue:** `UP` type not available without `defines.hpp`; `CMonitor` incomplete without `Monitor.hpp`; `damage` should be `m_damage`
- **Fix:** Changed viewport header include to `defines.hpp`, added `Monitor.hpp` includes, fixed `m_damage` member name
- **Files modified:** src/canvas/CanvasViewport.hpp, src/canvas/CanvasViewport.cpp, src/layout/CanvasLayout.cpp
- **Verification:** Full project build succeeds
- **Committed in:** 6696638e (amended Task 1), 61c12d43 (Task 2)

---

**Total deviations:** 1 auto-fixed (1 blocking)
**Impact on plan:** Include/member name corrections necessary for compilation. No scope creep.

## Issues Encountered
None beyond the auto-fixed include issues.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- Viewport and layout classes ready for Plan 02 to wire into rendering and input
- `g_pCanvasViewport` needs instantiation during compositor init (Plan 02 Task 3)

---
*Phase: 02-canvas-viewport*
*Completed: 2026-03-25*
