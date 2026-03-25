# Project Research Summary

**Project:** Infinity Land
**Domain:** Wayland compositor with infinite canvas layout (Hyprland fork)
**Researched:** 2026-03-25
**Confidence:** HIGH

## Executive Summary

Infinity Land is a fork of Hyprland v0.51.1 that replaces the traditional workspace/tiling paradigm with an infinite 2D canvas where live, interactive Wayland windows can be freely positioned, panned, and zoomed. This would be the first Wayland compositor with a true infinite canvas -- prior art exists only on X11 (InfiniteGlass, basedwm) and as application-level overlays (WinfiniteCanvas). The concept is proven in design tools (Figma, Miro) and users already have muscle memory for canvas interactions.

The recommended approach is to implement the canvas as a custom `IHyprLayout` implementation, which is Hyprland's native extension point for window positioning. The zoom/pan transform injects at a single point in the OpenGL projection matrix, with layer-shell surfaces (waybar, wofi) explicitly excluded from the transform. Annotations use Cairo rendered to GL textures. Persistence is simple JSON. No heavy external dependencies are needed beyond what Hyprland and the system already provide.

The dominant risk is the rendering pipeline integration: injecting a global coordinate transform breaks damage tracking, layer-shell rendering, cursor positioning, and popup placement simultaneously. This must be solved correctly in the earliest phase or nothing else works. The secondary risk is upstream divergence -- this fork will never merge back, so integration points with Hyprland internals must be minimized and documented. Both risks are manageable with the architectural patterns identified in research.

## Key Findings

### Recommended Stack

The project inherits Hyprland's full C++23/meson/aquamarine/OpenGL stack. Only three new dependencies are needed, all lightweight and available as Fedora system packages.

**Core technologies (new):**
- **Cairo + Pango**: annotation/drawing overlay -- proven GL texture upload pattern, handles strokes/text/paths natively
- **nlohmann/json**: persistence -- header-only, C++23 native, human-readable output for debugging
- **Command pattern (no lib)**: undo/redo -- trivial to implement in C++, covers all canvas mutations

**Key decision: no spatial indexing library.** At 15-50 windows, brute force O(n) and O(n^2) are fast enough. A simple grid hash suffices if needed later.

### Expected Features

**Must have (table stakes):**
- Pan (middle-click/space+drag) and zoom (scroll wheel) at 60fps
- Live interactive windows on canvas (not thumbnails)
- Window drag, resize, free placement
- Layer-shell compatibility (waybar, wofi, swaync work normally)
- Window position persistence across restarts
- Quick jump / search by window title
- Minimap for orientation
- Zoom-to-fit (single window focus)

**Should have (differentiators):**
- Drawing/annotation layer (freehand, arrows, sticky notes, text)
- Connectors between windows (visual relationships)
- Dot grid background with glow
- Color tags on windows
- Workspace views as visibility filters
- Undo/redo across all actions
- Anti-overlap snapping on drop

**Defer (v2+):**
- Snap-to-grid tiling mode
- Dark/light theme toggle
- Multi-monitor as one continuous surface (start with primary monitor only, add second monitor support after core works)

### Architecture Approach

Eight components with clear boundaries, all layered on top of Hyprland's existing infrastructure. The key insight is that ONE global viewport owns all coordinate transforms, and every component goes through it. Windows store canvas-space coordinates; screen-space is computed only at render time. Layer-shell surfaces bypass the canvas transform entirely.

**Major components:**
1. **CCanvasViewport** -- pan/zoom state, all coordinate transforms (canvas <-> screen)
2. **CInfiniteCanvasLayout** -- IHyprLayout implementation, flat map of window positions in canvas space
3. **CCanvasInputRouter** -- dispatches input to canvas actions vs window interaction vs annotation tools
4. **CAnnotationLayer + CAnnotationRenderer** -- data model and GL rendering for drawings/arrows/text
5. **CCanvasPersistence** -- debounced JSON serialization of all canvas state

### Critical Pitfalls

1. **Upstream divergence** -- accept permanent fork from day one; isolate changes to new files; touch fewer than 5 existing Hyprland source files
2. **Rendering pipeline break from global transform** -- inject transform at single point (projection matrix in CHyprOpenGLImpl::begin()); exclude layer-shell; force full redraw on pan/zoom instead of transforming damage regions
3. **Input coordinate space confusion** -- single CCanvasViewport class owns ALL transforms; never do ad-hoc coordinate math; unit test the transform at multiple zoom levels
4. **Nested compositor masking real bugs** -- test on real hardware at least once per phase milestone; keep recovery TTY available

## Implications for Roadmap

### Phase 1: Fork Setup and Canvas Viewport
**Rationale:** Everything depends on the coordinate transform system. Also establishes fork boundaries.
**Delivers:** Building Hyprland from source, CCanvasViewport with unit-tested coordinate transforms, documented integration points
**Addresses:** Foundation for all features
**Avoids:** Upstream divergence (Pitfall 1) by defining boundaries early

### Phase 2: Canvas Layout and Rendering
**Rationale:** Core value proposition. Pan/zoom with live windows is what makes this project unique. Nothing else matters without this.
**Delivers:** CInfiniteCanvasLayout (IHyprLayout), CCanvasInputRouter, working pan/zoom/drag/resize, layer-shell compatibility verified
**Addresses:** Pan, zoom, window drag/resize, layer-shell compatibility
**Avoids:** Rendering pipeline break (Pitfall 2), input confusion (Pitfall 3)

### Phase 3: Canvas Polish and Persistence
**Rationale:** After core rendering works, the canvas needs to be usable for daily driving -- persistence, navigation, background grid.
**Delivers:** CCanvasPersistence, quick jump/search, CCanvasBackground (dot grid), zoom-to-fit, CCanvasMinimap
**Addresses:** Persistence, quick jump, minimap, dot grid, zoom-to-fit
**Avoids:** Persistence race conditions (Pitfall 7), GPU memory issues (Pitfall 5) via visibility culling

### Phase 4: Annotation Layer
**Rationale:** High complexity feature that depends on stable canvas coordinate system. Cairo + Pango integration.
**Delivers:** CAnnotationLayer, CAnnotationRenderer, freehand drawing, arrows, sticky notes, text tool
**Uses:** Cairo, Pango (new dependencies)
**Avoids:** Annotation coordinate confusion (Pitfall 10)

### Phase 5: Advanced Features
**Rationale:** Polish and differentiation features that build on everything above.
**Delivers:** Connectors between windows, color tags, workspace views/filters, undo/redo, anti-overlap snapping
**Addresses:** All remaining differentiators

### Phase Ordering Rationale

- Phases 1-2 are strictly sequential: viewport math must exist before layout can use it, layout must exist before rendering can show it
- Phase 3 groups navigation/persistence features that share a dependency on stable canvas coordinates but are independent of each other
- Phase 4 is isolated because Cairo integration is self-contained and touches different parts of the render pipeline than window rendering
- Phase 5 bundles features that are individually low-medium complexity and independently valuable

### Research Flags

Phases likely needing deeper research during planning:
- **Phase 2:** Core technical risk. Hyprland's render pipeline internals (CHyprRenderer, CHyprOpenGLImpl, CDamageRing) need careful study. The projection matrix injection point and layer-shell exclusion are not well-documented outside the source code.
- **Phase 4:** Cairo-to-GL texture upload pattern needs prototyping. Performance characteristics of re-rendering annotation textures on edit need validation.

Phases with standard patterns (skip research-phase):
- **Phase 1:** Fork setup and coordinate math are straightforward.
- **Phase 3:** JSON persistence, minimap rendering, search -- all well-documented patterns.
- **Phase 5:** Each sub-feature is independent and uses patterns established in earlier phases.

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Inherited stack is locked; new deps are minimal and proven |
| Features | HIGH | Strong prior art from canvas apps and existing WMs; clear feature hierarchy |
| Architecture | HIGH | IHyprLayout interface is well-documented; component boundaries are clean |
| Pitfalls | HIGH | Based on direct Hyprland source inspection and real compositor development experience |

**Overall confidence:** HIGH

### Gaps to Address

- **Projection matrix injection point:** Exact location in CHyprOpenGLImpl where canvas transform should be applied needs source-level investigation during Phase 2 planning
- **Damage tracking strategy:** Full redraw on every pan/zoom frame may be too expensive on RX 590 with many windows; may need incremental approach. Profile early in Phase 2.
- **Multi-monitor canvas:** Research assumed single continuous surface but the exact aquamarine multi-output rendering path needs validation. Defer complexity to late Phase 3 or Phase 5.
- **Window matching for persistence:** How to reliably identify windows across restarts (app_id + title + geometry heuristics) needs design during Phase 3.

## Sources

### Primary (HIGH confidence)
- Hyprland v0.51.1 source code (IHyprLayout, CHyprRenderer, CHyprOpenGLImpl) -- direct inspection
- [Hyprland Layout System - DeepWiki](https://deepwiki.com/hyprwm/Hyprland/5.3-layout-system)
- [Hyprland Rendering Pipeline - DeepWiki](https://deepwiki.com/hyprwm/Hyprland/6.1-rendering-pipeline)
- [Hyprland OpenGL Backend - DeepWiki](https://deepwiki.com/hyprwm/Hyprland/6.2-opengl-backend)

### Secondary (MEDIUM confidence)
- [InfiniteGlass](https://github.com/redhog/InfiniteGlass) -- X11 prior art for infinite canvas WM
- [Niri](https://github.com/niri-wm/niri) -- scrollable Wayland compositor, partial prior art
- [Cairo OpenGL integration](https://www.cairographics.org/OpenGL/) -- texture upload pattern
- [Oakland University thesis](https://our.oakland.edu/handle/10323/4785) -- academic survey of ZUI window managers

### Tertiary (LOW confidence)
- [basedwm](https://github.com/anko/basedwm) -- minimal X11 infinite panning WM, limited documentation
- [WinfiniteCanvas](https://github.com/Nick-de-Bruin/WinfiniteCanvas) -- Windows-only, different paradigm

---
*Research completed: 2026-03-25*
*Ready for roadmap: yes*
