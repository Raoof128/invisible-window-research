#!/bin/bash
# build.sh — Compile the Invisible Window Linux PoC
# Author: Mohammad Raouf Abedini
#
# Requires: libx11-dev libxcomposite-dev libxfixes-dev
# Install: sudo apt install libx11-dev libxcomposite-dev libxfixes-dev

set -e

echo "╔══════════════════════════════════════════╗"
echo "║  Building Invisible Window — Linux PoC   ║"
echo "╚══════════════════════════════════════════╝"
echo ""

# Check dependencies
for lib in x11 xcomposite xfixes; do
    if pkg-config --exists "$lib" 2>/dev/null; then
        echo "[OK] $lib found"
    else
        echo "[WARN] $lib not found — install with: sudo apt install lib${lib}-dev"
    fi
done

echo ""

# Compile
gcc invisible_window_x11.c -o invisible_window_x11 \
    $(pkg-config --cflags --libs x11 xcomposite xfixes) \
    -Wall -O2

echo ""
echo "[BUILD] Success! Binary: invisible_window_x11"
echo ""
echo "Run with:"
echo "  ./invisible_window_x11"
echo ""
echo "NOTE: Linux lacks true display affinity APIs."
echo "This PoC tests partial approaches (override-redirect, XComposite)"
echo "but full capture invisibility is NOT achievable on Linux."
