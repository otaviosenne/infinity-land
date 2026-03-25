# Infinity Land

## What This Is

A custom Wayland compositor forked from Hyprland where the desktop is an infinite zoomable canvas. Every window is a node on this canvas — users pan, zoom, drag, resize, and interact with real windows in a spatial layout. It replaces the traditional workspace paradigm with a freeform, visual approach to window management. Built for a single power user on a dual-monitor Fedora/AMD setup.

## Core Value

Every open window lives on one infinite canvas with real interaction — no thumbnails, no static previews. You see and use your actual apps spatially.

## Requirements

### Validated

(None yet — ship to validate)

### Active

- [ ] Infinite canvas layout with pan (middle mouse / space+drag) and zoom (scroll wheel)
- [ ] All Wayland windows render as real interactive surfaces on the canvas
- [ ] Drag windows freely on the canvas (SUPER+mouse, like Hyprland)
- [ ] Resize windows on the canvas (SUPER+right-click, like Hyprland)
- [ ] Windows cannot overlap when dropped — snap to nearest free position
- [ ] Window positions persist across compositor restarts
- [ ] Default window size 1200x800, remembers last size per app
- [ ] New windows appear at center of current viewport
- [ ] Smooth linear zoom animations (fast, no easing)
- [ ] Canvas spans both monitors as one continuous surface (DP-1 + HDMI-A-1)
- [ ] Dot grid background with glow-on-hover animation (radial gradient following cursor)
- [ ] Dark and light theme with toggle button
- [ ] Workspace views that filter which windows are visible (not separate canvases)
- [ ] Minimap in corner showing all windows overview
- [ ] Quick jump — keyboard shortcut to search and jump to a window by name
- [ ] Optional snap-to-grid tiling mode via keybind
- [ ] Maximize = zoom-to-fit (canvas zooms until window fills screen, still in canvas)
- [ ] Color tags on windows for visual organization
- [ ] Drawing mode with toolbar (bottom-right button to activate)
- [ ] Freehand brush with color picker and thickness
- [ ] Arrows/connectors between windows or arbitrary points
- [ ] Sticky notes — floating text notes on the canvas
- [ ] Text tool — click canvas to place text directly
- [ ] Eraser — precision mode (exact area) and object mode (delete entire element)
- [ ] Annotations persist to disk, button to clear (all / by type)
- [ ] Full undo/redo stack (Ctrl+Z / Ctrl+Shift+Z) across all actions
- [ ] Waybar, wofi, SwayNC, and layer-shell apps work normally
- [ ] All Hyprland keybindings and window rules still function

### Out of Scope

- XEmbed / embedding X11 windows inside each other — Wayland isolates surfaces
- Real-time collaborative canvas (multi-user) — single user compositor
- Mobile/touch input — desktop only
- Video recording of canvas — focus on live interaction
- Plugin system — direct fork modifications, not plugins

## Context

Fork of Hyprland v0.51.1 on Fedora 43. The codebase is C++23 using wlroots/aquamarine for Wayland protocol handling. The user's hardware is Intel i5-9400F + AMD RX 590 (Ellesmere) + 64GB RAM with dual 1080p monitors. Development uses a nested compositor workflow (`AQUAMARINE_BACKEND=wayland`) for safe iteration without crashing the real desktop.

Previous prototype (infinite-work) was an Electron app that proved the UX concept with thumbnails — this compositor version delivers real window interaction instead of static captures.

## Constraints

- **Language**: C++23 — Hyprland's codebase, non-negotiable
- **Rendering**: OpenGL/EGL via aquamarine backend — must work on AMD RX 590 (AMDGPU driver)
- **Protocol**: wlr-layer-shell support required for waybar/wofi/swaync
- **Performance**: Must handle 15+ windows on canvas at 60fps with smooth pan/zoom
- **Persistence**: Window positions, annotations, and workspace views saved to `~/.config/infinity-land/`
- **Compatibility**: All existing Hyprland config options and keybindings must work

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Fork Hyprland instead of building from scratch | Inherits protocol support, input handling, GPU rendering, app compat | — Pending |
| v0.51.1 as fork base | Matches user's installed version, known stable on this hardware | — Pending |
| Canvas as custom layout, not plugin | Plugins are fragile across versions, layout is deeply integrated | — Pending |
| One canvas spanning all monitors | Simpler mental model, matches "infinite" concept | — Pending |
| Zoom-to-fit instead of real fullscreen | Stay in canvas paradigm, never leave the spatial context | — Pending |
| Linear zoom (no easing) | User preference — fast, responsive, no animation delay | — Pending |
| Annotations as canvas-level overlay | Drawings/notes exist on the canvas plane, not attached to windows | — Pending |

---
*Last updated: 2026-03-25 after initialization*
