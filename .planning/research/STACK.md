# Technology Stack

**Project:** Infinity Land
**Researched:** 2026-03-25

## Existing Stack (Inherited from Hyprland v0.51.1)

These are non-negotiable -- they come with the fork.

| Technology | Version | Purpose | Confidence |
|------------|---------|---------|------------|
| C++23 | GCC 14+ / Clang 18+ | Language standard | HIGH |
| aquamarine | >= 0.9.3 | Wayland backend (DRM/KMS, EGL, input) | HIGH |
| hyprutils | >= 0.8.2 | Shared utilities (math, string, signal) | HIGH |
| hyprlang | >= 0.3.2 | Configuration parser | HIGH |
| hyprcursor | >= 0.1.7 | Cursor theming | HIGH |
| hyprgraphics | >= 0.1.3 | Image loading/manipulation | HIGH |
| wayland-protocols | system | Wayland protocol definitions | HIGH |
| OpenGL ES 3.x / EGL | system | GPU rendering pipeline | HIGH |
| re2 | system | Regex for window rules | HIGH |
| meson + cmake | system | Build system (dual, meson primary) | HIGH |

## New Dependencies to Add

### Canvas Layout Engine

No external library needed. Implement `CHyprCanvasLayout : public IHyprLayout` using the existing layout interface. The layout system already provides `onWindowCreated`, `recalcWindow`, `fullscreenRequestForWindow`, and all the hooks needed.

**Canvas state (pan offset + zoom level) stored as:**
- `Vector2D m_vCanvasOffset` -- pan position in canvas coordinates
- `float m_fZoomLevel` -- zoom factor (1.0 = 100%)

Transform applied in the render pass by modifying the projection matrix before window rendering. Hyprland already does per-monitor projection in `CHyprOpenGLImpl` -- extend this.

| Component | Approach | Confidence |
|-----------|----------|------------|
| Pan/zoom transform | Modify OpenGL projection matrix in render pass | HIGH |
| Window placement | Canvas-space coordinates in layout, transform to screen-space | HIGH |
| Collision avoidance | Simple AABB grid spatial hash, check on drop | HIGH |

### Annotation / Drawing Overlay

| Technology | Version | Purpose | Why This |
|------------|---------|---------|----------|
| Cairo | 1.18.x (system) | 2D vector drawing for annotations | Already a Fedora system lib, proven Wayland/EGL integration, handles strokes/paths/text natively. Render to image surface, upload as GL texture. |

**Why Cairo and not raw OpenGL shaders:**
- Freehand brush strokes with variable thickness need bezier curve tessellation -- Cairo does this in 1 call (`cairo_stroke`)
- Text rendering needs font shaping -- Cairo + Pango handle this
- SVG-like arrow/connector paths are trivial in Cairo, painful in raw GL
- Performance is fine: annotations are rendered to a texture once, only re-rendered on edit. The texture is composited as a GL quad in the render pass.

**Why not Skia:**
- Massive dependency (40MB+ build), Google-oriented build system, overkill for annotation overlays
- Cairo is already available as a system package on Fedora

**Why not NanoVG:**
- Abandoned (last commit 2023), no text shaping, no Pango integration
- Smaller community, harder to debug

| Technology | Version | Purpose | Why This |
|------------|---------|---------|----------|
| Pango | 1.54.x (system) | Text layout/rendering for sticky notes and text tool | Cairo's native text companion, handles font fallback, already on system |

**Confidence:** HIGH -- Cairo texture upload to GL is a well-established pattern used by GTK, wlroots clients, and many Wayland compositors.

### Persistence

| Technology | Version | Purpose | Why This |
|------------|---------|---------|----------|
| nlohmann/json | 3.12.0 | Serialize window positions, annotations, workspace views | Header-only, C++17/23 native, industry standard for C++ JSON. Already familiar pattern in Hyprland codebase (uses hyprlang for config but JSON is better for structured data). |

**What gets persisted to `~/.config/infinity-land/`:**
- `canvas-state.json` -- pan offset, zoom level, per-window canvas positions
- `annotations.json` -- drawing strokes, arrows, sticky notes, text elements
- `workspaces.json` -- workspace view definitions and window-to-workspace mapping

**Why JSON and not SQLite:**
- Data is small (hundreds of entries max), read once on startup, written on change
- Human-readable for debugging
- No runtime dependency

**Confidence:** HIGH

### Undo/Redo System

No external library. Implement command pattern in C++:

```cpp
class ICanvasCommand {
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
};
```

Stack of `std::unique_ptr<ICanvasCommand>` with redo stack cleared on new action. Covers window moves, annotation creation/deletion, and all canvas mutations.

**Confidence:** HIGH -- standard pattern, no library needed.

### Minimap

No external library. Render a downscaled version of the canvas using the existing render pipeline with a smaller viewport/FBO. Draw window rectangles as colored quads, highlight current viewport area.

**Confidence:** HIGH

## Supporting Libraries (Already in Hyprland)

| Library | Purpose in Infinity Land |
|---------|------------------------|
| tracy | Performance profiling for pan/zoom frame times |
| udis86 | Inherited, not directly needed |

## Alternatives Considered

| Category | Recommended | Alternative | Why Not |
|----------|-------------|-------------|---------|
| 2D Drawing | Cairo + Pango | Raw GL shaders | Too much work for beziers, text, variable-width strokes |
| 2D Drawing | Cairo + Pango | Skia | Massive dep, complex build, overkill |
| 2D Drawing | Cairo + Pango | NanoVG | Abandoned, no text shaping |
| Persistence | nlohmann/json | SQLite | Overkill for small structured data |
| Persistence | nlohmann/json | hyprlang | Config format, not good for structured data arrays |
| Persistence | nlohmann/json | toml++ | Less natural for arrays of objects (annotations) |
| Spatial index | Simple grid hash | R-tree (boost) | 15-50 windows max, spatial index is overkill |
| Undo system | Command pattern | External lib | Trivial to implement, no lib needed |

## Installation

```bash
# Cairo and Pango (likely already installed as GTK deps)
sudo dnf install cairo-devel pango-devel

# nlohmann-json (header-only, or use as meson subproject)
sudo dnf install json-devel

# Everything else is already present from Hyprland build deps
```

### Meson Integration

Add to `meson.build`:
```meson
cairo_dep = dependency('cairo', version: '>=1.16')
pango_dep = dependency('pangocairo', version: '>=1.50')
json_dep = dependency('nlohmann_json', version: '>=3.11', required: false)
# Fallback: subproject or header-only include
```

## What NOT to Use

| Technology | Why Not |
|------------|---------|
| Qt/QML | Enormous dependency, conflicts with wlroots rendering model |
| Electron/webview overlay | Wrong paradigm, performance disaster for compositor |
| wlr-scene | Hyprland v0.51.1 uses its own render pipeline, not wlr-scene |
| Vulkan | Hyprland uses OpenGL/EGL, switching renderer is a rewrite |
| ECS framework (entt) | 15-50 windows + annotations, standard OOP is fine |
| Protocol extensions for drawing | Annotations are compositor-internal, not client-visible |
| GTK for overlay UI | Layer shell clients work, but drawing toolbar should be compositor-native for zero-latency |

## Build Configuration

The project inherits Hyprland's dual build system (meson primary, cmake secondary). Stick with **meson** for development -- it's what Hyprland CI uses and what handles the subprojects.

```bash
# Development build (debug, nested compositor)
meson setup build -Dbuildtype=debug
ninja -C build

# Run nested
AQUAMARINE_BACKEND=wayland ./build/Hyprland
```

## Sources

- Hyprland v0.51.1 meson.build (direct inspection)
- Hyprland IHyprLayout interface (direct inspection)
- [Cairo OpenGL integration](https://www.cairographics.org/OpenGL/) -- texture upload pattern
- [nlohmann/json v3.12.0](https://github.com/nlohmann/json/releases/tag/v3.12.0)
- Cairo/Pango available on Fedora 43 as system packages (verified via dnf)
