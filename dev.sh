#!/bin/bash
cd "$(dirname "$0")"

pkill -f "Hyprland.*infinity-land-dev" 2>/dev/null
pkill -f "kitty.*wayland-[2-9]" 2>/dev/null
sleep 1

cmake --build build -j$(nproc) 2>&1 | tail -5
if [ $? -eq 0 ]; then
    AQUAMARINE_BACKEND=wayland ./build/Hyprland -c ~/.config/hypr/infinity-land-dev.conf
else
    echo "Build failed"
    exit 1
fi
