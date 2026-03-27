# Infinity Land

A Hyprland fork that replaces traditional workspaces with an **infinite 2D canvas** — pan, zoom, and freely arrange windows across an endless space.

> Built on top of [Hyprland](https://github.com/hyprwm/Hyprland) · C++26 · Wayland-native

---

## What is this?

Infinity Land extends Hyprland with a new `canvas` layout that transforms the workspace into an infinite, zoomable plane. Instead of flipping between numbered workspaces, you pan around a single canvas and zoom out to see everything at once.

---

## Features

### Canvas Layout
- **Infinite 2D canvas** — windows float freely at any position
- **Pan** — `SUPER` + drag or middle-click drag to move around
- **Zoom** — scroll wheel to zoom in/out (10% → 100%)
- **Viewport snap** — `fitmonitor` resizes a window to fill the visible area

### MonitorFrame
A bordered container rendered on the canvas that behaves like a classic Hyprland workspace:
- **Monitor-sized frame** with a white border drawn directly on the canvas
- **Numbered workspace tabs** — switch between independent sets of windows inside the frame
- **Auto-tiling** — 1 window fills the frame, 2 split side-by-side, 3 master+stack, 4+ grid
- Create with `SUPER+D`, assign windows with `SUPER+SHIFT+D`

### Minimap
- Bottom-right corner overview of all canvas windows
- Logarithmic zoom bar with center tick at scale = 1
- Click to navigate directly to any canvas area

### Dot Grid
- Adaptive background grid that scales with zoom
- Rendered as smooth circles via GLSL

### Window Snapping
- Automatic overlap resolution on drag end
- Minimum-translation algorithm, up to 50 iterations
- 16px gap enforced between windows

### Annotations
- Draw strokes, arrows, sticky notes, and text on the canvas
- Full undo/redo support

### Tags & Views
- Color-tag windows for visual grouping
- Multiple canvas view modes

---

## Building

### Dependencies

Same as upstream Hyprland:

```
cmake  ninja  pkg-config  wayland-protocols  libdrm  libinput
aquamarine  hyprlang  hyprutils  hyprcursor  hyprgraphics
pango  cairo  pixman  libxkbcommon  gbm
```

### Compile

```bash
git clone https://github.com/otaviosenne/infinity-land
cd infinity-land
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Run nested (for testing)

```bash
AQUAMARINE_BACKEND=wayland ./build/Hyprland -c /path/to/your.conf
```

---

## Configuration

Set the layout to `canvas` in your config:

```ini
general {
    layout = canvas
}
```

### Keybinds

```ini
# Canvas
bindm = SUPER, mouse:272, movewindow
bindm = SUPER, mouse:273, resizewindow
bind  = SUPER, mouse:275, layoutmsg, fitmonitor

# MonitorFrame
bind = SUPER,       D, layoutmsg, createframe
bind = SUPER SHIFT, D, layoutmsg, assigntoframe
bind = SUPER, bracketright, layoutmsg, nexttab
bind = SUPER, bracketleft,  layoutmsg, prevtab
bind = SUPER, 1, layoutmsg, frametab 1
bind = SUPER, 2, layoutmsg, frametab 2
bind = SUPER, 3, layoutmsg, frametab 3
```

---

## layoutmsg reference

| Command | Description |
|---|---|
| `fitmonitor` | Resize focused window to fill the current viewport |
| `createframe` | Spawn a MonitorFrame at the viewport center |
| `assigntoframe` | Assign focused window to the active frame tab |
| `nexttab` | Next tab in the active frame |
| `prevtab` | Previous tab in the active frame |
| `frametab <n>` | Jump to tab `n` (1-based) in the active frame |

---

## Credits

Infinity Land is a fork of [Hyprland](https://github.com/hyprwm/Hyprland) by [vaxerski](https://github.com/vaxerski) and contributors. All original Hyprland features are preserved.
