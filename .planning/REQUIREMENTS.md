# Requirements: Infinity Land

**Defined:** 2026-03-25
**Core Value:** Every open window lives on one infinite canvas with real interaction

## v1 Requirements

### Canvas

- [x] **CANV-01**: User can pan the canvas with middle mouse button or space+drag
- [x] **CANV-02**: User can zoom the canvas with scroll wheel (linear, fast, no easing)
- [x] **CANV-03**: Canvas displays a dot grid background at all zoom levels
- [x] **CANV-04**: Dot grid glows near the cursor with radial gradient animation
- [x] **CANV-05**: Canvas spans both monitors (DP-1 + HDMI-A-1) as one continuous surface
- [x] **CANV-06**: Window positions and sizes persist across compositor restarts
- [x] **CANV-07**: Grid dots adapt spacing at low zoom levels to avoid visual clutter

### Window Management

- [ ] **WNDW-01**: User can drag windows on canvas with SUPER+left-click
- [ ] **WNDW-02**: User can resize windows on canvas with SUPER+right-click
- [x] **WNDW-03**: Windows cannot overlap when dropped — snap to nearest free position
- [ ] **WNDW-04**: New windows appear at center of current viewport
- [ ] **WNDW-05**: Default window size is 1200x800
- [x] **WNDW-06**: Window remembers its last size per app class across restarts
- [x] **WNDW-07**: User can maximize a window via zoom-to-fit (canvas zooms until window fills screen)
- [x] **WNDW-08**: User can snap selected windows into grid layout via keybind
- [x] **WNDW-09**: User can assign color tags to windows for visual organization
- [ ] **WNDW-10**: All windows render as real interactive Wayland surfaces (not thumbnails)

### Navigation

- [x] **NAVG-01**: Minimap in corner shows overview of all windows on canvas
- [x] **NAVG-02**: User can click minimap to jump to that area of the canvas
- [x] **NAVG-03**: User can search windows by name and jump to them via keybind

### Annotations

- [x] **ANOT-01**: User can activate drawing mode via button (bottom-right corner)
- [x] **ANOT-02**: Freehand brush with color picker and thickness adjustment
- [ ] **ANOT-03**: Arrow/connector tool between windows or arbitrary points
- [ ] **ANOT-04**: Sticky note tool — floating text notes on the canvas
- [ ] **ANOT-05**: Text tool — click canvas to place text directly
- [x] **ANOT-06**: Eraser with precision mode (exact area) and object mode (delete whole element)
- [ ] **ANOT-07**: All annotations persist to disk across restarts
- [ ] **ANOT-08**: Clear button with options: clear all, clear by type
- [ ] **ANOT-09**: Drawing toolbar shows brush, arrow, sticky, text, eraser tools

### System

- [ ] **SYST-01**: Full undo/redo stack across all actions (Ctrl+Z / Ctrl+Shift+Z)
- [x] **SYST-02**: Dark and light theme with toggle button
- [x] **SYST-03**: Workspace views filter which windows are visible on the canvas
- [ ] **SYST-04**: Waybar, wofi, SwayNC, and layer-shell apps work normally (excluded from canvas transform)
- [ ] **SYST-05**: All existing Hyprland keybindings and window rules still function
- [ ] **SYST-06**: Smooth zoom animations at 60fps with 15+ windows on AMD RX 590

## v2 Requirements

### Advanced Navigation

- **NAVG-04**: Breadcrumb trail showing recently visited canvas areas
- **NAVG-05**: Bookmark positions on canvas for quick return

### Advanced Annotations

- **ANOT-10**: Group annotations into layers (show/hide layers)
- **ANOT-11**: Lock annotations to prevent accidental editing

### Collaboration

- **COLB-01**: Export canvas layout as image/PDF

## Out of Scope

| Feature | Reason |
|---------|--------|
| XEmbed window embedding | Wayland isolates surfaces — not technically possible |
| Multi-user collaboration | Single-user compositor, complexity not justified |
| Touch/mobile input | Desktop-only compositor |
| Video recording of canvas | Focus on live interaction, screen recording tools exist |
| Plugin system | Direct fork modifications, avoiding ABI fragility |
| Fractal/infinite zoom | Impractical for GPU-composited live windows, bounded zoom range |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| CANV-01 | Phase 2 | Complete |
| CANV-02 | Phase 2 | Complete |
| CANV-03 | Phase 5 | Complete |
| CANV-04 | Phase 5 | Complete |
| CANV-05 | Phase 2 | Complete |
| CANV-06 | Phase 6 | Complete |
| CANV-07 | Phase 5 | Complete |
| WNDW-01 | Phase 3 | Pending |
| WNDW-02 | Phase 3 | Pending |
| WNDW-03 | Phase 8 | Complete |
| WNDW-04 | Phase 3 | Pending |
| WNDW-05 | Phase 3 | Pending |
| WNDW-06 | Phase 6 | Complete |
| WNDW-07 | Phase 7 | Complete |
| WNDW-08 | Phase 8 | Complete |
| WNDW-09 | Phase 8 | Complete |
| WNDW-10 | Phase 3 | Pending |
| NAVG-01 | Phase 7 | Complete |
| NAVG-02 | Phase 7 | Complete |
| NAVG-03 | Phase 7 | Complete |
| ANOT-01 | Phase 9 | Complete |
| ANOT-02 | Phase 9 | Complete |
| ANOT-03 | Phase 10 | Pending |
| ANOT-04 | Phase 10 | Pending |
| ANOT-05 | Phase 10 | Pending |
| ANOT-06 | Phase 9 | Complete |
| ANOT-07 | Phase 10 | Pending |
| ANOT-08 | Phase 10 | Pending |
| ANOT-09 | Phase 9 | Pending |
| SYST-01 | Phase 11 | Pending |
| SYST-02 | Phase 5 | Complete |
| SYST-03 | Phase 8 | Complete |
| SYST-04 | Phase 4 | Pending |
| SYST-05 | Phase 4 | Pending |
| SYST-06 | Phase 11 | Pending |

**Coverage:**
- v1 requirements: 35 total
- Mapped to phases: 35
- Unmapped: 0

---
*Requirements defined: 2026-03-25*
*Last updated: 2026-03-25 after roadmap creation*
