# Roadmap: Infinity Land

## Overview

Transform Hyprland v0.51.1 into an infinite canvas compositor where every window is a live, interactive surface on a pannable/zoomable 2D plane. The roadmap progresses from fork setup through core rendering, window management, navigation, annotations, and polish -- each phase delivering a verifiable capability.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

Decimal phases appear between their surrounding integers in numeric order.

- [ ] **Phase 1: Fork and Build** - Hyprland builds from source with nested compositor dev workflow
- [ ] **Phase 2: Canvas Viewport** - Pan and zoom transform system with coordinate space math
- [ ] **Phase 3: Canvas Layout** - Live windows render on canvas with drag and resize
- [ ] **Phase 4: Layer-Shell Compatibility** - Waybar, wofi, SwayNC, and keybindings work normally
- [x] **Phase 5: Canvas Background** - Dot grid, cursor glow, and theme toggle
- [x] **Phase 6: Persistence** - Window positions, sizes, and per-app defaults survive restarts (completed 2026-03-25)
- [ ] **Phase 7: Navigation** - Minimap, quick jump, and zoom-to-fit
- [ ] **Phase 8: Window Organization** - Anti-overlap snap, grid snap, color tags, workspace views
- [ ] **Phase 9: Annotation Foundation** - Drawing mode with toolbar, freehand brush, and eraser
- [ ] **Phase 10: Annotation Extended** - Arrows, sticky notes, text tool, persistence, and clear
- [ ] **Phase 11: Undo/Redo and Performance** - Command pattern across all actions, 60fps target

## Phase Details

### Phase 1: Fork and Build
**Goal**: Developer can build and run Infinity Land from source in nested compositor mode
**Depends on**: Nothing (first phase)
**Requirements**: (foundation -- no user-facing requirements, enables all others)
**Success Criteria** (what must be TRUE):
  1. `cmake -B build -G Ninja && cmake --build build` produces a working compositor binary
  2. Compositor launches inside existing Hyprland session via `AQUAMARINE_BACKEND=wayland`
  3. A terminal window opens and is interactive inside the nested compositor
**Plans**: 1 plan

Plans:
- [ ] 01-01-PLAN.md — Verify build system and nested compositor workflow

### Phase 2: Canvas Viewport
**Goal**: User can pan and zoom an infinite canvas that spans both monitors
**Depends on**: Phase 1
**Requirements**: CANV-01, CANV-02, CANV-05
**Success Criteria** (what must be TRUE):
  1. User can pan the canvas with middle mouse button or space+drag and the view moves smoothly
  2. User can zoom in/out with scroll wheel with linear (no easing) animation
  3. Canvas extends seamlessly across DP-1 and HDMI-A-1 as one continuous surface
  4. Coordinate transforms are correct at all zoom levels (clicks hit the right spot)
**Plans**: 2 plans

Plans:
- [ ] 02-01-PLAN.md — CCanvasViewport singleton and CCanvasLayout with LayoutManager registration
- [ ] 02-02-PLAN.md — Wire transforms into rendering, input, and pan/zoom handlers

### Phase 3: Canvas Layout
**Goal**: All windows render as live interactive Wayland surfaces on the canvas
**Depends on**: Phase 2
**Requirements**: WNDW-01, WNDW-02, WNDW-04, WNDW-05, WNDW-10
**Success Criteria** (what must be TRUE):
  1. Opening an app shows a live, interactive window on the canvas (not a thumbnail)
  2. User can drag windows freely with SUPER+left-click
  3. User can resize windows with SUPER+right-click
  4. New windows appear at center of current viewport with 1200x800 default size
**Plans**: 1 plan

Plans:
- [ ] 03-01-PLAN.md — Override drag/resize in CCanvasLayout with canvas-space coordinate conversion

### Phase 4: Layer-Shell Compatibility
**Goal**: Waybar, wofi, SwayNC, and all existing Hyprland config still work
**Depends on**: Phase 3
**Requirements**: SYST-04, SYST-05
**Success Criteria** (what must be TRUE):
  1. Waybar renders at screen edge and is not affected by canvas pan/zoom
  2. Wofi launcher opens and accepts input normally
  3. SwayNC notification center appears and dismisses correctly
  4. All existing Hyprland keybindings (SUPER+Q, SUPER+W, etc.) still function
**Plans**: 1 plan

Plans:
- [ ] 04-01-PLAN.md — Audit layer-shell isolation and verify compatibility

### Phase 5: Canvas Background
**Goal**: Canvas has a visible dot grid with cursor glow and supports dark/light themes
**Depends on**: Phase 2
**Requirements**: CANV-03, CANV-04, CANV-07, SYST-02
**Success Criteria** (what must be TRUE):
  1. Dot grid is visible behind windows at all zoom levels
  2. Dot spacing adapts at low zoom to avoid visual clutter
  3. Dots near the cursor glow with a radial gradient that follows mouse movement
  4. User can toggle between dark and light theme via a button
**Plans**: 2 plans

Plans:
- [x] 05-01-PLAN.md — Dot grid fragment shader and background rendering pipeline
- [x] 05-02-PLAN.md — Cursor glow effect and dark/light theme toggle

### Phase 6: Persistence
**Goal**: Canvas state survives compositor restarts
**Depends on**: Phase 3
**Requirements**: CANV-06, WNDW-06
**Success Criteria** (what must be TRUE):
  1. After restart, previously open windows reappear at their saved canvas positions
  2. Window sizes are remembered per app class (e.g., kitty always opens at last-used size)
  3. State is saved to `~/.config/infinity-land/` as human-readable JSON
**Plans**: 1 plan

Plans:
- [ ] 06-01-PLAN.md — Canvas state persistence with nlohmann/json save/load

### Phase 7: Navigation
**Goal**: User can orient and jump around the canvas efficiently
**Depends on**: Phase 3
**Requirements**: NAVG-01, NAVG-02, NAVG-03, WNDW-07
**Success Criteria** (what must be TRUE):
  1. Minimap in corner shows positions of all windows on the canvas
  2. Clicking a spot on the minimap jumps the viewport to that canvas area
  3. User can press a keybind, type a window name, and jump to that window
  4. Maximize action zooms the canvas until the target window fills the screen
**Plans**: 2 plans

Plans:
- [ ] 07-01-PLAN.md — Minimap overlay and zoom-to-fit viewport manipulation
- [ ] 07-02-PLAN.md — Quick jump text search overlay

### Phase 8: Window Organization
**Goal**: User can organize windows with snapping, grid layout, color tags, and filtered views
**Depends on**: Phase 3
**Requirements**: WNDW-03, WNDW-08, WNDW-09, SYST-03
**Success Criteria** (what must be TRUE):
  1. Dropping a window near another snaps it to the nearest non-overlapping position
  2. User can press a keybind to arrange selected windows into a grid layout
  3. User can assign color tags to windows and see them visually on the canvas
  4. User can switch workspace views that filter which windows are visible
**Plans**: TBD

Plans:
- [ ] 08-01: TBD
- [ ] 08-02: TBD

### Phase 9: Annotation Foundation
**Goal**: User can draw freehand on the canvas and erase drawings
**Depends on**: Phase 2
**Requirements**: ANOT-01, ANOT-02, ANOT-06, ANOT-09
**Success Criteria** (what must be TRUE):
  1. Clicking the bottom-right button activates drawing mode with a visible toolbar
  2. User can draw freehand strokes with configurable color and thickness
  3. Toolbar shows brush, arrow, sticky, text, and eraser tool buttons
  4. Eraser works in precision mode (area) and object mode (whole element)
**Plans**: TBD

Plans:
- [ ] 09-01: TBD
- [ ] 09-02: TBD

### Phase 10: Annotation Extended
**Goal**: User can place arrows, sticky notes, text, and all annotations persist
**Depends on**: Phase 9
**Requirements**: ANOT-03, ANOT-04, ANOT-05, ANOT-07, ANOT-08
**Success Criteria** (what must be TRUE):
  1. User can draw arrows/connectors between windows or arbitrary canvas points
  2. User can create sticky notes (floating text boxes) on the canvas
  3. User can click the canvas to place text directly
  4. All annotations persist to disk and survive restarts
  5. Clear button offers options to clear all annotations or clear by type
**Plans**: TBD

Plans:
- [ ] 10-01: TBD
- [ ] 10-02: TBD

### Phase 11: Undo/Redo and Performance
**Goal**: All canvas actions are undoable and the compositor runs at 60fps with 15+ windows
**Depends on**: Phase 10
**Requirements**: SYST-01, SYST-06
**Success Criteria** (what must be TRUE):
  1. Ctrl+Z undoes the last action (window move, annotation, etc.)
  2. Ctrl+Shift+Z redoes the last undone action
  3. Undo/redo works across window actions and annotation actions
  4. Pan/zoom with 15+ open windows maintains 60fps on AMD RX 590
**Plans**: TBD

Plans:
- [ ] 11-01: TBD
- [ ] 11-02: TBD

## Progress

**Execution Order:**
Phases execute in numeric order: 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 9 -> 10 -> 11
(Phases 5, 6, 7, 8, 9 depend on Phase 2 or 3 and can partially parallelize)

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Fork and Build | 0/? | Not started | - |
| 2. Canvas Viewport | 1/2 | In Progress|  |
| 3. Canvas Layout | 0/1 | Not started | - |
| 4. Layer-Shell Compatibility | 0/1 | Not started | - |
| 5. Canvas Background | 0/2 | Not started | - |
| 6. Persistence | 1/1 | Complete   | 2026-03-25 |
| 7. Navigation | 0/2 | Not started | - |
| 8. Window Organization | 0/? | Not started | - |
| 9. Annotation Foundation | 0/? | Not started | - |
| 10. Annotation Extended | 0/? | Not started | - |
| 11. Undo/Redo and Performance | 0/? | Not started | - |
