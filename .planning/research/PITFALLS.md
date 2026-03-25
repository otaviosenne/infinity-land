# Domain Pitfalls

**Domain:** Custom Wayland compositor (Hyprland fork with infinite canvas layout)
**Researched:** 2026-03-25

## Critical Pitfalls

Mistakes that cause rewrites or project abandonment.

### Pitfall 1: Upstream Divergence Death Spiral

**What goes wrong:** You fork Hyprland v0.51.1, make deep modifications to the rendering pipeline and layout system, then discover you can never merge upstream fixes. Every Hyprland release restructures internals (vaxry refactors aggressively), making your fork permanently frozen on a stale base.
**Why it happens:** Hyprland has no stable internal API. The layout interface (`IHyprLayout`), rendering pipeline (`CHyprRenderer`, `CHyprOpenGLImpl`), and aquamarine integration all change between versions. Deep modifications to these systems create merge conflicts on every upstream update.
**Consequences:** Security vulnerabilities go unpatched. New protocol support (e.g., xdg-dialog, ext-transient-seat) never arrives. Bug fixes require manual backporting. The fork becomes unmaintainable within 6-12 months.
**Prevention:** Accept this fork as a permanent divergence from day one. Do NOT plan to merge upstream. Instead: (1) isolate canvas logic into separate files/classes that touch Hyprland internals at minimal, well-defined integration points, (2) document every integration point so upstream changes can be manually evaluated, (3) keep a list of upstream commits that matter (security, driver compat) for cherry-picking.
**Detection:** If you find yourself modifying more than 5 existing Hyprland source files, your integration surface is too large.
**Phase:** Phase 1 (fork setup) -- establish integration boundaries before writing any canvas code.

### Pitfall 2: Breaking the Rendering Pipeline with Global Transforms

**What goes wrong:** You inject a zoom/pan transform matrix into Hyprland's rendering pipeline, and it breaks damage tracking, layer shell rendering, cursor positioning, and popup placement simultaneously.
**Why it happens:** Hyprland's rendering pipeline uses `CDamageRing` for incremental damage tracking with scissor-tested regions. A global canvas transform means damage regions must be transformed too. The pipeline renders in strict order (background layers -> tiled windows -> floating windows -> overlay layers), and layer-shell surfaces (waybar, wofi, swaync) expect screen-space coordinates, not canvas-space. The cursor position reported to clients via `CPointerManager` must also be inverse-transformed.
**Consequences:** Visual artifacts (partial redraws in wrong locations), layer-shell apps render at wrong positions or wrong scales, mouse clicks land in wrong places, popups appear offset from their parent windows.
**Prevention:** The transform must be injected at a single point: modify the projection matrix in `CHyprOpenGLImpl::begin()` and apply inverse transforms to input coordinates in `CPointerManager`. Layer-shell surfaces must be excluded from the canvas transform (they render in screen-space, always). When zoom/pan changes, call `damageMonitor()` to force full redraw rather than trying to transform damage regions incrementally.
**Detection:** Test with waybar visible -- if waybar scales/moves with the canvas, the layer-shell exclusion is broken. Test popups (right-click menus) -- if they appear offset, input transform is broken.
**Phase:** Phase 2 (canvas rendering) -- this is the core technical risk of the entire project.

### Pitfall 3: Input Coordinate Space Confusion

**What goes wrong:** Mouse events, touch events, and keyboard focus all operate in screen-space. Your canvas operates in canvas-space (with zoom and pan offsets). Failing to correctly and consistently transform between these spaces causes windows to be unclickable, drag to feel wrong, and resize handles to miss.
**Why it happens:** Wayland clients receive surface-local coordinates. The compositor must transform screen-space input -> canvas-space -> window surface-local. With zoom, a 1px mouse movement corresponds to more than 1px of canvas movement. Hyprland's existing input handling in `CInputManager` assumes a 1:1 mapping between screen and layout coordinates.
**Consequences:** Windows become impossible to interact with at non-1x zoom levels. Drag operations jump or stutter. Click targets are offset from visual positions.
**Prevention:** Create a single `CanvasTransform` class that owns all coordinate transformations: `screenToCanvas()`, `canvasToScreen()`, `screenToSurface()`. Every input path must go through this class. Never do ad-hoc coordinate math. Write unit tests for the transform math (zoom in, zoom out, pan offset, multi-monitor offset combinations).
**Detection:** Zoom to 50% and try to click a button inside a window. If the click lands in the wrong spot, the transform is broken.
**Phase:** Phase 2 (canvas rendering) -- must be solved alongside the rendering transform.

### Pitfall 4: Nested Compositor Workflow Masking Real Bugs

**What goes wrong:** You develop exclusively in nested mode (`AQUAMARINE_BACKEND=wayland`) and ship code that works nested but crashes on real hardware. DRM/KMS behavior, VSync timing, GPU memory limits, and multi-monitor scan-out differ fundamentally between nested and native modes.
**Why it happens:** In nested mode, the parent compositor handles all hardware interaction. Aquamarine's Wayland backend is simpler than the DRM backend. Buffer allocation, display timing, and GPU memory pressure behave differently.
**Consequences:** Code works in development, crashes on the real desktop. Particularly dangerous for: multi-monitor buffer management, zoom animations (frame timing), and GPU memory when rendering many scaled surfaces.
**Prevention:** Test on real hardware at least once per phase milestone. Keep a second TTY available for recovery (`Ctrl+Alt+F2`). Set up a systemd service or script that auto-kills the compositor after 30 seconds of unresponsiveness for safe testing.
**Detection:** If you have never tested outside nested mode for more than 2 weeks, you are accumulating hidden bugs.
**Phase:** Every phase -- establish a hardware testing ritual from Phase 1.

## Moderate Pitfalls

### Pitfall 5: GPU Memory Exhaustion from Offscreen Surfaces

**What goes wrong:** With 15+ windows on an infinite canvas, most are offscreen at any given zoom level. If the compositor keeps all window textures in GPU memory at full resolution, VRAM fills up (RX 590 has 8GB, but much is used by running apps).
**Prevention:** Implement level-of-detail: windows far from the viewport or very small at current zoom get lower-resolution textures or placeholder thumbnails. Only fully render windows that are visible and large enough to interact with. Hyprland already skips rendering for windows on inactive workspaces -- extend this logic for canvas visibility culling.
**Detection:** Monitor VRAM usage (`radeontop` or `cat /sys/class/drm/card*/device/mem_info_vram_used`) with 15+ windows open. If it climbs above 4GB, you need culling.
**Phase:** Phase 3 (performance optimization) -- not needed for MVP but critical before daily use.

### Pitfall 6: Layer Shell Z-Order Conflicts with Canvas

**What goes wrong:** Waybar (bottom layer), wofi (top layer), and swaync (overlay layer) use wlr-layer-shell protocol which defines strict z-ordering layers. Your canvas windows must render between the bottom and top layers, but canvas zoom could visually conflict with layer expectations.
**Prevention:** Layer-shell surfaces are never part of the canvas. They render at fixed screen positions with no transform. The canvas renders only in the "normal window" layer between background and top. This is how Hyprland already works -- do not change it.
**Detection:** If waybar disappears behind canvas windows or wofi renders underneath them, the z-order is broken.
**Phase:** Phase 2 -- verify this works correctly when implementing canvas rendering.

### Pitfall 7: Window Position Persistence Race Conditions

**What goes wrong:** Saving window positions to `~/.config/infinity-land/` on every move creates disk I/O storms. Saving only on exit loses data on crashes. Loading positions on startup races with window creation.
**Prevention:** Debounce saves (write at most once per 5 seconds). Use a write-ahead approach: keep state in memory, flush periodically, and flush on graceful shutdown. For crash recovery, the periodic flush provides a recent-enough state. On startup, load positions before accepting client connections so the map is ready when `onWindowCreatedTiling()` fires.
**Detection:** Kill the compositor with `SIGKILL` and restart -- if window positions are lost, persistence is too lazy.
**Phase:** Phase 3 (persistence) -- after canvas and window placement work.

### Pitfall 8: Collision Detection Performance at Scale

**What goes wrong:** "Windows cannot overlap when dropped" requires checking every window position on every drop. Naive O(n^2) checking works for 10 windows but stalls for 50+.
**Prevention:** Use a spatial index (grid-based or quadtree) for window positions. On drop, query only nearby cells. The canvas is 2D, so a simple grid hash (`position / cell_size -> bucket`) is sufficient and trivial to implement.
**Detection:** Open 30+ windows and drag one -- if there is visible stutter on drop, collision detection is too slow.
**Phase:** Phase 2 (window placement) -- implement spatial indexing from the start, do not retrofit.

## Minor Pitfalls

### Pitfall 9: Smooth Zoom Animation Frame Budget

**What goes wrong:** Zoom animations that recalculate and redraw the entire canvas every frame drop below 60fps, especially with many visible windows.
**Prevention:** During zoom animation, reduce rendering quality (skip window content updates, use cached textures, only update the projection matrix). Full-quality render only on the final frame.
**Phase:** Phase 3 (performance).

### Pitfall 10: Drawing/Annotation Layer Coordinate Confusion

**What goes wrong:** Annotations exist in canvas-space but are rendered after the canvas transform. If the transform is applied twice (once for windows, once for annotations), drawings appear at wrong positions or wrong scales relative to windows.
**Prevention:** Annotations and windows share the same canvas coordinate space and the same transform. Render annotations as part of the same render pass, not as a separate overlay.
**Phase:** Phase 4 (annotations) -- but the coordinate system must be designed in Phase 2.

### Pitfall 11: Minimap Rendering Overhead

**What goes wrong:** The minimap must show all windows at a tiny scale, requiring either a separate render pass of everything or maintaining a separate low-res texture atlas.
**Prevention:** Render the minimap using solid-color rectangles representing window positions, not actual window content. Window titles as tiny labels. This is fast and sufficient for navigation.
**Phase:** Phase 4 (minimap).

## Phase-Specific Warnings

| Phase Topic | Likely Pitfall | Mitigation |
|-------------|---------------|------------|
| Fork setup | Upstream divergence (#1) | Define integration boundaries, accept permanent fork |
| Canvas rendering | Rendering pipeline break (#2), input confusion (#3) | Single transform injection point, `CanvasTransform` class |
| Window placement | Collision performance (#8) | Spatial index from day one |
| Persistence | Race conditions (#7) | Debounced periodic flush |
| Annotations | Coordinate confusion (#10) | Share canvas coordinate space with windows |
| Performance | GPU memory (#5), zoom budget (#9) | Visibility culling, cached textures during animation |
| All phases | Nested-only testing (#4) | Regular hardware testing ritual |

## Sources

- [Hyprland Layout System (IHyprLayout) - DeepWiki](https://deepwiki.com/hyprwm/Hyprland/5.3-layout-system)
- [Hyprland Rendering Pipeline - DeepWiki](https://deepwiki.com/hyprwm/Hyprland/6.1-rendering-pipeline)
- [Hyprland OpenGL Backend - DeepWiki](https://deepwiki.com/hyprwm/Hyprland/6.2-opengl-backend)
- [Writing a Wayland Compositor - Drew DeVault](https://drewdevault.com/2018/02/17/Writing-a-Wayland-compositor-1.html)
- [Friendly Fork Management - GitHub Blog](https://github.blog/developer-skills/github/friend-zone-strategies-friendly-fork-management/)
- [Thoughts on writing a Wayland compositor with wlroots](https://inclem.net/2021/04/17/wayland/writing_a_wayland_compositor_with_wlroots/)
