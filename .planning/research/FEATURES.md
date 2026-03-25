# Feature Landscape

**Domain:** Infinite canvas Wayland compositor (spatial window manager)
**Researched:** 2026-03-25

## Prior Art

| Project | Type | Key Insight |
|---------|------|-------------|
| InfiniteGlass | X11 WM, infinite zoom+pan | Proved the concept works; SVG-based infinite resolution windows |
| basedwm | X11 WM, infinite pan | Minimalist — pan only, no zoom, no decorations |
| WinfiniteCanvas | Windows overlay | Drags all windows on infinite canvas; inspired by Figma/Obsidian |
| Niri / PaperWM | Scrolling tiling WM | 1D infinite strip (horizontal scroll only), not 2D canvas |
| Apple visionOS | Spatial computing | 3D window placement, up to 20 windows, infinite canvas metaphor |
| Miro / Figma / Muse | Canvas apps | Established UX patterns users already know: pan, zoom, minimap, annotations |

## Table Stakes

Features users expect from any infinite canvas experience. Missing any of these makes the product feel broken — users from Figma/Miro/Obsidian Canvas already have muscle memory for these interactions.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Pan (middle-click drag or space+drag) | Universal canvas interaction since Figma popularized it | Low | Already in PROJECT.md requirements |
| Zoom (scroll wheel, pinch) | Fundamental canvas navigation; without it, infinite space is unusable | Medium | Must transform window rendering, not just viewport magnification |
| Smooth pan/zoom at 60fps | Anything below feels broken; canvas apps set this bar | High | GPU compositing critical — this is the core technical challenge |
| Drag windows freely | Basic spatial interaction; every WM has this | Low | Hyprland already supports SUPER+drag |
| Resize windows | Expected from any WM | Low | Hyprland already supports SUPER+right-drag |
| New window placement at viewport center | Users expect new content to appear where they're looking | Low | Simple coordinate transform |
| Window position persistence | Losing spatial layout on restart destroys the value proposition | Medium | JSON/TOML state file in ~/.config/ |
| Quick jump / search | Infinite space demands fast navigation; getting lost kills usability | Medium | Fuzzy search by window title, teleport viewport |
| Minimap | Orientation in large spaces; every canvas app has one | Medium | Render scaled-down window rectangles in corner overlay |
| Layer-shell compatibility | Waybar, wofi, swaync must work or desktop is unusable | Medium | These render outside canvas transform — fixed to screen |
| Existing Hyprland keybinds work | Fork promise: everything you had still works | Low | Preserve existing dispatch system |
| Zoom-to-fit (maximize) | Users need a way to focus on one window without leaving canvas | Medium | Animate viewport to frame single window |

## Differentiators

Features that set Infinity Land apart from both traditional WMs and existing infinite canvas WMs. Not expected, but create the "wow" factor.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Real interactive windows on canvas | InfiniteGlass used X11 redirects; basedwm had no zoom; this would be the first Wayland compositor with true infinite canvas | Very High | Core differentiator — windows are live, not thumbnails |
| Drawing/annotation layer | No existing WM has native canvas annotations; bridges WM and whiteboard | High | Canvas-level overlay with freehand, arrows, sticky notes, text |
| Connectors between windows | Visual relationships between apps (e.g., terminal connected to editor) | Medium | Arrow/line objects snapped to window edges |
| Color tags on windows | Visual categorization without workspaces; unique to spatial WMs | Low | Border tint or corner badge |
| Workspace views as filters | Not separate spaces but visibility filters on one canvas | Medium | Toggle which windows show; preserves spatial context |
| Dot grid background with glow | Provides spatial grounding; prevents "endless white void" disorientation | Low | Shader-based grid that scales with zoom |
| Snap-to-grid tiling mode | Best of both worlds: freeform default, structured on demand | Medium | Keybind toggles grid snapping |
| Anti-overlap snapping | Windows push to nearest free position on drop; keeps canvas tidy | Medium | Collision detection + displacement algorithm |
| Undo/redo across all actions | No WM has this; canvas apps do; bridges the gap | High | Action stack for window moves, annotations, all mutations |
| Dark/light theme toggle | Polish feature; canvas apps all offer this | Low | Theme variables for grid, background, UI chrome |
| Multi-monitor as one surface | Both monitors show one continuous canvas; no per-monitor strips like Niri | Medium | Viewport spans combined resolution |

## Anti-Features

Features to explicitly NOT build. These are traps that waste time or harm the core experience.

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|-------------------|
| Traditional workspaces | Contradicts the infinite canvas paradigm; Niri already does scrolling workspaces better | Use filter-based workspace views on one canvas |
| Window thumbnails/previews | The entire point is real interactive windows; thumbnails are what infinite-work (Electron prototype) already tried and outgrew | Always render live window content |
| Infinite zoom (fractal nesting) | InfiniteGlass attempted this — windows inside windows. Massive complexity, unclear UX value | Bound zoom to practical range (e.g., 10%-500%) |
| Collaborative/multi-user | Single-user compositor; networking adds enormous complexity for zero value here | Keep it local, single user |
| Plugin/extension system | Fork modifications are more stable than plugin APIs across Hyprland versions | Direct C++ modifications to the fork |
| Touch/mobile input | Desktop-only compositor; touch adds input complexity for hardware that doesn't exist | Mouse + keyboard only |
| Window grouping/stacking | Tabs and stacks fight against spatial layout; if you need groups, use color tags or proximity | Spatial proximity IS the grouping mechanism |
| Auto-layout algorithms | Force-directed or auto-arrange fights user's spatial memory; placement should be manual | Provide snap-to-grid as opt-in, never auto-rearrange |
| Presentation/slideshow mode | Scope creep; use actual presentation tools | Zoom-to-fit individual windows serves this need |

## Feature Dependencies

```
Pan/Zoom rendering -----> All canvas features (everything depends on this)
    |
    +--> Window position persistence (needs canvas coordinate system)
    |       |
    |       +--> Workspace views/filters (needs persisted positions)
    |
    +--> Minimap (needs canvas coordinate system + window positions)
    |
    +--> Quick jump (needs window position index)
    |
    +--> Zoom-to-fit (needs zoom + window bounds)
    |
    +--> Anti-overlap snapping (needs window bounds in canvas space)
    |
    +--> Drawing/annotation layer (needs canvas coordinate system)
    |       |
    |       +--> Connectors between windows (needs annotation layer + window tracking)
    |       |
    |       +--> Sticky notes / text tool (needs annotation layer)
    |
    +--> Dot grid background (needs zoom-aware rendering)

Layer-shell compatibility -----> Independent (renders outside canvas transform)

Color tags -----> Independent (window decoration, no canvas dependency)

Undo/redo -----> Depends on action tracking across all mutation types
```

## MVP Recommendation

Prioritize in this order:

1. **Pan/zoom with live window rendering** — the entire value proposition; nothing else matters without this
2. **Window dragging and resize on canvas** — basic spatial interaction
3. **Layer-shell compatibility** — waybar/wofi must work or the desktop is unusable
4. **Window position persistence** — losing layout on restart is a dealbreaker
5. **Quick jump** — infinite space without fast navigation is hostile
6. **Minimap** — orientation in the canvas
7. **Dot grid background** — spatial grounding, prevents disorientation
8. **Zoom-to-fit** — the "maximize" equivalent for focus mode

Defer to later phases:
- **Drawing/annotation layer** — high complexity, not needed for core WM usage
- **Undo/redo** — requires action tracking infrastructure across all features
- **Workspace views** — useful but the canvas itself must work first
- **Anti-overlap snapping** — nice polish, not critical for initial use

## Sources

- [InfiniteGlass](https://github.com/redhog/InfiniteGlass) — X11 WM with infinite zoom
- [basedwm](https://github.com/anko/basedwm) — Minimalist infinite panning X11 WM
- [WinfiniteCanvas](https://github.com/Nick-de-Bruin/WinfiniteCanvas) — Windows infinite canvas overlay
- [Niri](https://github.com/niri-wm/niri) — Scrollable-tiling Wayland compositor
- [Niri blog post](https://davidyat.es/2026/01/28/niri/) — Niri's infinite canvas concept
- [Muse infinite canvas memo](https://museapp.com/memos/2020-12-infinite-canvas/) — Design philosophy
- [infinitecanvas.tools](https://github.com/museapphq/infinitecanvas.tools) — Category definition
- [On Compositional Window Management](https://bandukwala.me/on-compositional-window-management/) — WM design theory
- [Oakland University thesis](https://our.oakland.edu/handle/10323/4785) — Academic survey of ZUI window managers
